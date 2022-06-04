cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)

set(PROJECT_NAME sftm)
project(${PROJECT_NAME})

set(HEADERS
	include/configs.h
	include/CChainController.hpp
	include/CConcurrentPtrQueue.hpp
	include/CTask.hpp
	include/CTaskManager.hpp
	include/CWorker.hpp
	include/Utils.hpp
	include/CSyncPrimitive.hpp
)

set(SOURCES
	source/main.cpp
)

if (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

