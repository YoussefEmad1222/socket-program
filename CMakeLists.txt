cmake_minimum_required(VERSION 3.26)
project(socket_program)

set(CMAKE_CXX_STANDARD 14)

add_executable(socket_program
        server.cpp
        sendReceiveFiles.cpp)
