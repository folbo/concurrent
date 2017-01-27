cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(project_name Concurrent)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_subdirectory(ThreadPool)

add_subdirectory(server)
add_subdirectory(client)

add_definitions(-std=c++11)