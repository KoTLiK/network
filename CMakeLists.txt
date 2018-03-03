cmake_minimum_required(VERSION 3.9)
project(network)

set(CMAKE_CXX_STANDARD 14)

add_executable(server src/server.cpp src/Network.h)
add_executable(client src/client.cpp src/Network.h)
