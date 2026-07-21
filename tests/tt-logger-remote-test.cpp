// SPDX-FileCopyrightText: © 2025 Tenstorrent AI ULC
//
// SPDX-License-Identifier: Apache-2.0
#include <tt-logger/tt-logger-initializer.hpp>
#include <tt-logger/tt-logger.hpp>


#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

int main() {
    tt::LoggerInitializer log_init;
    
    std::cout << "Test: Send a message using default logger" << std::endl;
    spdlog::info("Logger initialized");

    std::cout << "Test: log_info performance (1000 iterations)" << std::endl;
    auto start_info = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        log_info(tt::LogDevice, "Performance test iteration {}", i);
    }

    auto end_info      = std::chrono::high_resolution_clock::now();
    auto duration_info = std::chrono::duration_cast<std::chrono::microseconds>(end_info - start_info);

    std::cout << "log_info average time per call: " << duration_info.count() / 1000.0 << " microseconds" << std::endl;


    std::cout << "Test: log_info large message test." << std::endl;
    std::string large_message(62000L, 'a');

    start_info = std::chrono::high_resolution_clock::now();
    log_info(tt::LogUMD, "{}", large_message);
    end_info      = std::chrono::high_resolution_clock::now();
    duration_info = std::chrono::duration_cast<std::chrono::microseconds>(end_info - start_info);

    std::cout << "log_info for large call: " << duration_info.count() << " microseconds" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(5));
}

