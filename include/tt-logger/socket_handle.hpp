// SPDX-FileCopyrightText: © 2025 Tenstorrent AI ULC
//
// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <utility>

#include <unistd.h>

namespace tt {

/**
 * Simple handle that makes sure we close the socket once we leave a scope.
 */
struct SocketHandle {
    SocketHandle() = default;
    SocketHandle(int handle): socket_handle{handle} {

    }

    ~SocketHandle() {
        if (socket_handle != -1) {
            ::close(socket_handle);
            socket_handle = -1;
        }
    }

    SocketHandle(const SocketHandle&) = delete;
    SocketHandle& operator=(const SocketHandle&) = delete;
    
    SocketHandle(SocketHandle&& other) noexcept {
        std::swap(socket_handle, other.socket_handle);
    }

    SocketHandle& operator=(SocketHandle&& other) noexcept {
        // This makes sure even if you try to use the socket you can't once it is moved.
        if (socket_handle != -1) {
            ::close(socket_handle);
            socket_handle = -1;
        }

        std::swap(socket_handle, other.socket_handle);
        return *this;
    }

    int socket_handle = -1;
};
}