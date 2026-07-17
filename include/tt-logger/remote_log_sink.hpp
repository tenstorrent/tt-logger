// SPDX-FileCopyrightText: © 2025 Tenstorrent AI ULC
//
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <iterator>
#include <simple_concurrent_queue.hpp>
#include <tt-logger/socket_handle.hpp>
#include <tt-logger/consts.hpp>

#include <spdlog/sinks/base_sink.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <iomanip>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

namespace tt::internal {

/**
 * This sink hooks into spdlog and queues the log messages to be fired over the socket
 * to the receiving side.  We use a UDP domain socket.
 */
template <typename Mutex>
class RemoteLogSink: public spdlog::sinks::base_sink<Mutex> {
public:
    RemoteLogSink(const std::string& server_path): server_path_(server_path) {
        init_();
    }

    RemoteLogSink() {
        init_();
    }

    ~RemoteLogSink() override {
        try {
            process_messages_();
            sender_queue_.shutdown();
            sender_thread_.join();
            ::unlink(server_path_.c_str());
        } catch (...) {
            // Not much we can do.
        }
    }

    void disable() {
        disabled_.store(true);
        sender_queue_.shutdown();
    }

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override {
        if (disabled_.load() == true) {
            return;
        }

        // If there is no connection, we have nothing to do.
        // TODO: What can we do when we can't send on socket.
        if (socket_handle_.socket_handle == -1) {
            sender_queue_.shutdown();
            return;
        }

        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        std::string log_body(formatted.data(), formatted.size());

        LogMessageInfo info{
            .message = log_body
        };
        
        sender_queue_.push(std::move(info));
    }

    void flush_() override {
        process_messages_();
    }

private:
    // Make sure this path matches on the server side.
    const std::string server_path_ = "/tmp/tenstorrent_log_uds_server.sock";

    // TODO: Add more info. to the message as needed.
    struct LogMessageInfo {
        std::string message;
    };

    std::atomic<bool> disabled_{false};
    SimpleConcurrentQueue<LogMessageInfo> sender_queue_;
    SocketHandle socket_handle_;
    std::thread sender_thread_;
    struct sockaddr_un server_addr_{};
    std::array<uint8_t, send_buffer_size> send_buffer_;
    size_t buffer_len_{0};

    /**
     * Initializes and gets things ready for use.
     */
    void init_() {
        try {
            auto result = init_socket_();
            if (result) {
                sender_thread_ = std::thread([this]() {
                    this->process_();
                });
            }
        } catch (...) {
            // NOP.
        }
    }

    /**
     * Initializes the socket and gets it ready for use.
     */
    bool init_socket_() {
        int server_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (server_fd == -1) {
            std::cerr << "Socket creation failed\n";
            return false;
        }

        socket_handle_ = SocketHandle(server_fd);

        server_addr_.sun_family = AF_UNIX;
        std::strncpy(server_addr_.sun_path, server_path_.c_str(), sizeof(server_addr_.sun_path) - 1);

        return true;
    }

    /**
     * Processes messages in the queue.
     */
    void process_() {

        while(!sender_queue_.is_shutdown()) {  
            // We let the number of messages queue up for 100 ms, then we send what's there.
            for(int i = 0; i < 10; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                if (sender_queue_.is_shutdown()) {
                    break;
                }
            }          

            if (sender_queue_.is_shutdown()) {
                break;
            }

            if (sender_queue_.empty()) {
                continue;
            }

            process_messages_();
        }

        // Process any left over messages.
        process_messages_();
    }

    /**
     * Process messages we got and anything else in the queue currently.
     */
    void process_messages_() {
        std::vector<LogMessageInfo> msgs;

        if (!sender_queue_.empty()) {
            sender_queue_.process_all([&msgs](LogMessageInfo curr_msg) {
                msgs.push_back(std::move(curr_msg));
            });
        }

        // We keep building buffers and sending them until we sent all messages.
        while(!msgs.empty()) {
            build_buffer_(msgs);
            auto result = send_buffer_on_socket_();
            if (!result) {
                break;
            }
        }
    }

    /**
     * Build a buffer that we want to send.
     */
    void build_buffer_(std::vector<LogMessageInfo>& msgs) {
        buffer_len_ = 0;
        int idx = 0;

        //
        // Binary framing (all integers big-endian / network byte order):
        //   [uint8_t  num_msgs]
        //   repeated num_msgs times:
        //     [uint16_t msg_len][msg_len raw bytes]
        //
        // num_msgs caps at 0xFF (1 byte); msg_len caps at what fits in buffer at the given time.
        //
        std::size_t num_msgs = std::min<std::size_t>(msgs.size(), 0xFF);
        send_buffer_[idx++] = num_msgs & 0xFF;
        int num_appended = 0;

        for (int i = 0; i < num_msgs; ++i) {
            const std::string& body = msgs[i].message;

            // Large messages go in their own message block.
            if (body.size() > large_message_size) {
                std::size_t len = large_message_size;
                buffer_len_ = 0;
                send_buffer_[buffer_len_++] = 1;

                send_buffer_[buffer_len_++] = static_cast<uint8_t>(len >> 8) & 0xFF;
                send_buffer_[buffer_len_++] = static_cast<uint8_t>(len & 0xFF);                

                std::copy(std::begin(body), std::begin(body) + len, &send_buffer_[buffer_len_]);
                buffer_len_ += len;

                msgs.erase(std::begin(msgs) + i, std::begin(msgs) + i + 1);

                return;
            }

            // You only get what can fit in the message buffer.
            std::size_t len = std::min<std::size_t>(body.length()-1, send_buffer_.size() - buffer_len_ - sizeof(uint16_t));

            // 2-byte length, big-endian.
            send_buffer_[idx++] = static_cast<uint8_t>(len >> 8) & 0xFF;
            send_buffer_[idx++] = static_cast<uint8_t>(len & 0xFF);

            std::copy(std::begin(body), std::begin(body) + len, &send_buffer_[idx]);

            idx += len;
            buffer_len_ = idx;
            ++num_appended;
        }

        // Remove the messages we processed.
        msgs.erase(std::begin(msgs), std::begin(msgs) + num_appended);
    }

    /**
     * Attempts to send the buffer we constructed on the socket.
     */
    bool send_buffer_on_socket_() {
        if (disabled_.load() == true) {
            return false;
        }

        if (socket_handle_.socket_handle == -1) {
            return false;
        }

        while(true) {
            int rc = ::sendto(socket_handle_.socket_handle,
                              send_buffer_.data(), 
                              buffer_len_, 
                              0,
                              (struct sockaddr*)&server_addr_, sizeof(server_addr_));
            if (rc == -1) {
                if (errno == EAGAIN) {
                    continue;
                }
                socket_handle_ = -1;
                return false;
            }

            break;
        }
        
        return true;
    }
};

using RemoteLogSink_t = RemoteLogSink<std::mutex>;

} // namespace tt::internal

