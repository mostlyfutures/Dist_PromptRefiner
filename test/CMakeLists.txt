cmake_minimum_required(VERSION 3.10)
project(BasicTest)

enable_testing()

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(basic_test basic_test.cpp)
target_link_libraries(basic_test ${GTEST_LIBRARIES} pthread)

add_test(NAME BasicTest COMMAND basic_test)