cmake_minimum_required(VERSION 3.6)
project(websockets)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
        ./base64/base64.cpp
        ./base64/base64.h
        ./sockets.cpp
        ./websockets/websockets.cpp
        ./websockets/websockets.h)

add_executable(sockets ${SOURCE_FILES})