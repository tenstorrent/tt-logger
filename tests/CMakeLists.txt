include(${PROJECT_SOURCE_DIR}/cmake/CPM.cmake)

add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)

add_executable(${PROJECT_NAME}-test ${PROJECT_NAME}-test.cpp)
add_executable(${PROJECT_NAME}-remote-test ${PROJECT_NAME}-remote-test.cpp)

target_link_libraries(
    ${PROJECT_NAME}-test
    PRIVATE ${PROJECT_NAME}::${PROJECT_NAME}
)
target_link_libraries(
    ${PROJECT_NAME}-remote-test
    PRIVATE ${PROJECT_NAME}::${PROJECT_NAME}
)
