cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(exe_name client)

# start configuration
project(${project_name})

include_directories(${Boost_INCLUDE_DIR}) 
include_directories(${Asio_INCLUDE_DIR}) 

add_executable (${exe_name} main.cpp data_model.h client.h processor.h)

target_link_libraries (${exe_name} ws2_32 wsock32)
