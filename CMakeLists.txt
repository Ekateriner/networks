cmake_minimum_required(VERSION 3.17)
project(networks)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

add_executable(networks main.cpp)