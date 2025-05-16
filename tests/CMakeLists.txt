include(${PROJECT_SOURCE_DIR}/cmake/CPM.cmake)
CPMAddPackage(NAME catch2 GIT_REPOSITORY https://github.com/catchorg/Catch2 VERSION 3.8.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME}-test ${PROJECT_NAME}-test.cpp)

target_link_libraries(
    ${PROJECT_NAME}-test
    PRIVATE
        ${PROJECT_NAME}::${PROJECT_NAME}
        Catch2::Catch2WithMain
)

include(CTest)
enable_testing()
add_test(${PROJECT_NAME}-test ${PROJECT_NAME}-test)
