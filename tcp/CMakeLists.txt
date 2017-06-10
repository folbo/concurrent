cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(project_name Concurrent)


function(VerifyVarDefined)
    foreach(lib ${ARGV})
        if(DEFINED ${lib})
        else(DEFINED ${lib})
            message(FATAL_ERROR "Variable ${lib} is not defined")
        endif(DEFINED ${lib})
    endforeach()
endfunction(VerifyVarDefined)



set(CMAKE_CXX_FLAGS "-std=c++11")

# set VAR for boost dependency
find_package(Boost 1.57.0 REQUIRED)
# find VAR for asio dependency
VerifyVarDefined(Asio_INCLUDE_DIR)

add_subdirectory(server)
add_subdirectory(client)