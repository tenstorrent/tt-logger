cmake_minimum_required(VERSION 3.14)
project(tt-logger VERSION 1.0.2 LANGUAGES CXX)

# Set default C++17 when building tests, otherwise verify requirement
if(TT_LOGGER_BUILD_TESTING AND NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

# Verify C++17 requirement since we use CTAD
if(NOT DEFINED CMAKE_CXX_STANDARD)
    message(
        FATAL_ERROR
        "tt-logger requires C++17, but CMAKE_CXX_STANDARD is not set"
    )
elseif(CMAKE_CXX_STANDARD LESS 17)
    message(
        FATAL_ERROR
        "tt-logger requires at least C++17, but C++${CMAKE_CXX_STANDARD} was specified"
    )
endif()

include(GNUInstallDirs)

include(cmake/CPM.cmake)
if(NOT TARGET fmt::fmt-header-only)
    cpmaddpackage(NAME fmt GITHUB_REPOSITORY fmtlib/fmt GIT_TAG 11.1.4)
endif()

if(NOT TARGET spdlog::spdlog_header_only)
    cpmaddpackage(
        NAME spdlog
        GITHUB_REPOSITORY gabime/spdlog
        VERSION 1.15.2
        OPTIONS
            "SPDLOG_FMT_EXTERNAL_HO ON"
            "SPDLOG_INSTALL ON"
    )
endif()

# Header-only library
add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.23)
    target_sources(
        ${PROJECT_NAME}
        INTERFACE
            FILE_SET api
            TYPE HEADERS
            BASE_DIRS ${CMAKE_INSTALL_INCLUDEDIR}
            FILES
                ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/tt-logger.hpp
                ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/tt-logger-initializer.hpp
    )
endif()

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

if(CMAKE_VERSION VERSION_LESS 3.23)
    target_include_directories(
        ${PROJECT_NAME}
        INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
endif()

target_link_libraries(
    ${PROJECT_NAME}
    INTERFACE spdlog::spdlog_header_only fmt::fmt-header-only
)

option(TT_LOGGER_INSTALL "Configure for installation" OFF)

if(TT_LOGGER_INSTALL)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.23)
        install(
            TARGETS ${PROJECT_NAME}
            EXPORT ${PROJECT_NAME}-targets
            FILE_SET api
                DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
                COMPONENT ${PROJECT_NAME}-dev
        )
    else()
        install(
            TARGETS ${PROJECT_NAME}
            EXPORT ${PROJECT_NAME}-targets
            COMPONENT
            ${PROJECT_NAME}-dev
        )
        install(
            DIRECTORY include/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            COMPONENT ${PROJECT_NAME}-dev
        )
    endif()

    # Install export file
    install(
        EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}-targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
        COMPONENT ${PROJECT_NAME}-dev
    )

    include(CMakePackageConfigHelpers)

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}-config-version.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}-config.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}-config.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}-config-version.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
        COMPONENT ${PROJECT_NAME}-dev
    )
endif()

option(TT_LOGGER_BUILD_TESTING "Build tests" OFF)
if(TT_LOGGER_BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
