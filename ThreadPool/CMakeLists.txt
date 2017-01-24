set(THREADPOOL_LIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(THREADPOOL_LIB_INCLUDE_DIR ${THREADPOOL_LIB_INCLUDE_DIR} PARENT_SCOPE)

message(${THREADPOOL_LIB_INCLUDE_DIR})


if(TARGET ThreadPool)
	message("ThreadPool is already defined")
else()
	include_directories(${THREADPOOL_LIB_INCLUDE_DIR})

	set(ThreadPool_source_files
	  ThreadPool.h
	  ThreadPool.cpp
	)

	add_library(ThreadPool STATIC ${ThreadPool_source_files})
endif()




