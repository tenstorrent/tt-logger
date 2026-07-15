// SPDX-FileCopyrightText: © 2025 Tenstorrent AI ULC
//
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <simple_concurrent_queue.hpp>
#include <tt-logger/socket_handle.hpp>

#include <spdlog/sinks/base_sink.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <atomic>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <memory>
#include <mutex>
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

    // TODO: Add more info.
    struct LogMessageInfo {
        std::string message;
    };

    std::atomic<bool> disabled_{false};
    SimpleConcurrentQueue<LogMessageInfo> sender_queue_;
    SocketHandle socket_handle_;
    std::thread sender_thread_;
    struct sockaddr_un server_addr_{};

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
            auto items = sender_queue_.pop_all_wait();
            if (items.empty()) {
                if (sender_queue_.is_shutdown()) {
                    break;
                }
                continue;
            }

            process_messages_(std::move(items));
        }

        // Process any left over messages.
        process_messages_();
    }

    /**
     * Process messages we got and anything else in the queue currently.
     */
    void process_messages_(std::vector<LogMessageInfo> items = std::vector<LogMessageInfo>()) {
        std::vector<LogMessageInfo> msgs;
        
        for(auto& curr_item: items) {
            msgs.push_back(std::move(curr_item));
        }

        // If there's nothing passed to us, check for anything in the queue.
        if (items.empty()) {
            sender_queue_.process_all([&msgs](LogMessageInfo curr_msg) {
                msgs.push_back(std::move(curr_msg));
            });
        }

        for(const auto& curr: msgs) {
            if (!send_message_(curr)) {
                break;
            }
        }
    }

    /**
     * Attempts to send the message on the socket.
     */
    bool send_message_(const LogMessageInfo& msg) {
        if (disabled_.load() == true) {
            return false;
        }

        if (socket_handle_.socket_handle == -1) {
            return false;
        }

        while(true) {
            int rc = ::sendto(socket_handle_.socket_handle,
                              msg.message.c_str(), 
                              msg.message.length(), 
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

