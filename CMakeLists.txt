cmake_minimum_required(VERSION 3.10)

project(httper)

add_executable(httper src/main.cpp)

install(TARGETS httper)

target_link_libraries(httper pthread)
