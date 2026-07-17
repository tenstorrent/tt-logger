// SPDX-FileCopyrightText: © 2025 Tenstorrent AI ULC
//
// SPDX-License-Identifier: Apache-2.0
#pragma once

namespace tt::internal {

// These are the environment variable names that we use to control the behaviour of logging.
constexpr const char* tt_logger_file_env            = "TT_LOGGER_FILE";
constexpr const char* tt_metal_logger_file_env      = "TT_METAL_LOGGER_FILE";
constexpr const char* tt_log_level_env              = "TT_LOGGER_LEVEL";
constexpr const char* tt_metal_logger_level_env     = "TT_METAL_LOGGER_LEVEL";
constexpr const char* tt_remote_logger_env          = "TT_REMOTE_LOGGER";
constexpr const char* tt_remote_logger_socket_env   = "TT_REMOTE_LOGGER_SOCKET_FILE";
constexpr const char* tt_logger_types_env           = "TT_LOGGER_TYPES";
constexpr const char* tt_metal_logger_types_env     = "TT_METAL_LOGGER_TYPES";

// Various buffer sizes.
constexpr const size_t send_buffer_size     = 62000L;
constexpr const size_t large_message_size   = 60000L;

} // namespace tt::internal
