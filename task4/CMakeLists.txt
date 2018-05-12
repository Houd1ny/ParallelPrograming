cmake_minimum_required(VERSION 2.8)

project(task4_close_pairs)

SET(CMAKE_CXX_FLAGS -pthread)
set (CMAKE_CXX_STANDARD 11)

add_executable(${PROJECT_NAME} "main.cpp")
