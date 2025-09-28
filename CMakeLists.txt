cmake_minimum_required(VERSION 3.10)
project(project-name)

set(CMAKE_CXX_STANDARD 11)

enable_testing()

add_subdirectory(test)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(basic_test test/basic_test.cpp)
target_link_libraries(basic_test gtest gtest_main)

add_test(NAME basic_test COMMAND basic_test)