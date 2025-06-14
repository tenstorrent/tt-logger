include(${PROJECT_SOURCE_DIR}/cmake/CPM.cmake)

add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)

add_executable(${PROJECT_NAME}-test ${PROJECT_NAME}-test.cpp)

target_link_libraries(
    ${PROJECT_NAME}-test
    PRIVATE ${PROJECT_NAME}::${PROJECT_NAME}
)
