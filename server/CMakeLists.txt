cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

function(VerifyVarDefined)
  foreach(lib ${ARGV})
    if(DEFINED ${lib})
    else(DEFINED ${lib})
      message(FATAL_ERROR "Variable ${lib} is not defined")
    endif(DEFINED ${lib})
  endforeach()
endfunction(VerifyVarDefined)

set(exe_name server)

# start configuration
project(${project_name})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# boost dependency
find_package(Boost 1.57.0 REQUIRED)
# asio dependency
VerifyVarDefined(Asio_INCLUDE_DIR)

include_directories(${Boost_INCLUDE_DIR})
include_directories(${Asio_INCLUDE_DIR})


add_subdirectory(./../ThreadPool ThreadPool)
include_directories(${THREADPOOL_LIB_INCLUDE_DIR})

add_executable (${exe_name} main.cpp worker_session.h server.h matrix.h protocol/dotprod_frame.h protocol/frame.h protocol/chunk_frame.h protocol/chunk_response.h)

target_link_libraries (${exe_name} ws2_32 wsock32)
