cmake_minimum_required(VERSION 3.10)

project(httper)

add_executable(httper src/main.cpp src/network.cpp src/filesystem.cpp)

install(TARGETS httper)
