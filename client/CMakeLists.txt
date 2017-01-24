cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

function(VerifyVarDefined)
  foreach(lib ${ARGV}) 
    if(DEFINED ${lib})
    else(DEFINED ${lib})
      message(FATAL_ERROR "Variable ${lib} is not defined")
    endif(DEFINED ${lib})
  endforeach()
endfunction(VerifyVarDefined)


set(exe_name client)


# start configuration
project(${project_name})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# boost dependency
find_package(Boost 1.57.0 REQUIRED)
# asio dependency
VerifyVarDefined(Asio_INCLUDE_DIR)

include_directories(${Boost_INCLUDE_DIR}) 
include_directories(${Asio_INCLUDE_DIR}) 

link_directories(../ThreadPool)

add_executable (${exe_name} main.cpp data_model.h client.h processor.cpp processor.h)

target_link_libraries (${exe_name} ws2_32 wsock32)
target_link_libraries(${exe_name} ThreadPool)
