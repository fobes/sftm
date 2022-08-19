cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)

set(PROJECT_NAME sftm)
project(${PROJECT_NAME})

set(HEADERS
	include/Common/configs.h
	include/Common/Utils.hpp
	include/Common/CSpinLock.hpp
	include/Common/CSyncPrimitive.hpp
	include/Common/CChainController.hpp
	include/Common/CConcurrentPtrQueue.hpp
	include/Sync/CTask.hpp	
	include/Sync/CWorker.hpp
	include/Sync/CTaskManager.hpp	
	include/Async/CAsyncTask.hpp	
	include/Async/CAsyncWorker.hpp		
	include/Async/CAsyncTaskManager.hpp	
)

set(SOURCES
	source/main.cpp
)

if (MSVC)
	foreach(source IN LISTS HEADERS)
		get_filename_component(source_path "${source}" PATH)
		string(REPLACE "/" "\\" source_path_msvc "${source_path}")
		source_group("${source_path_msvc}" FILES "${source}")
	endforeach()
endif()

if (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)