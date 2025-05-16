include(${PROJECT_SOURCE_DIR}/cmake/CPM.cmake)
cpmaddpackage("gh:catchorg/Catch2@3.4.0")

add_executable(${PROJECT_NAME}-test ${PROJECT_NAME}-test.cpp)

target_link_libraries(
    ${PROJECT_NAME}-test
    PRIVATE ${PROJECT_NAME}::${PROJECT_NAME} Catch2::Catch2WithMain
)

add_executable(
    ${PROJECT_NAME}-initializer-test
    ${PROJECT_NAME}-initializer-test.cpp
)
target_link_libraries(
    ${PROJECT_NAME}-initializer-test
    PRIVATE ${PROJECT_NAME}::${PROJECT_NAME}
)

include(CTest)
enable_testing()
add_test(${PROJECT_NAME}-test ${PROJECT_NAME}-test)
