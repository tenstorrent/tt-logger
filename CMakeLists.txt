cmake_minimum_required(VERSION 3.23)
project(tt-logger VERSION 1.0.0 LANGUAGES CXX)

include(cmake/CPM.cmake)
if(NOT TARGET fmt::fmt-header-only)
    CPMAddPackage(NAME fmt GITHUB_REPOSITORY fmtlib/fmt GIT_TAG 11.1.4)
endif()

if(NOT TARGET spdlog::spdlog_header_only)
    CPMAddPackage(
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

target_sources(
    ${PROJECT_NAME}
    INTERFACE
        FILE_SET api
        TYPE HEADERS
        BASE_DIRS include
        FILES include/${PROJECT_NAME}/tt-logger.hpp
)

target_include_directories(${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_link_libraries(
    ${PROJECT_NAME}
    INTERFACE
        spdlog::spdlog_header_only
        fmt::fmt-header-only
)

option(TT_LOGGER_INSTALL "Configure for installation" OFF)
if(TT_LOGGER_INSTALL)
    install(
        TARGETS
            ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}-targets
        FILE_SET
        api
            DESTINATION include
            COMPONENT ${PROJECT_NAME}-dev
    )

    # Install export file
    install(
        EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}-targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION lib/cmake/${PROJECT_NAME}
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
        INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
    )

    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}-config.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}-config-version.cmake"
        DESTINATION lib/cmake/${PROJECT_NAME}
        COMPONENT ${PROJECT_NAME}-dev
    )
endif()

option(TT_LOGGER_BUILD_TESTING "Build tests" OFF)
if(TT_LOGGER_BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
