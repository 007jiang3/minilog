cmake_minimum_required(VERSION 3.15)

project(test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(test test.cpp)
add_executable(test2 test2.cpp)