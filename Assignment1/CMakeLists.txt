cmake_minimum_required(VERSION 3.6)
project(Assignment1)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp)
add_executable(Assignment1 ${SOURCE_FILES})
set(CMAKE_BUILD_TYPE Release)