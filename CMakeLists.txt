cmake_minimum_required(VERSION 2.8)

project(RCC)

if(UNIX AND NOT APPLE)
	set(BUILD_TYPE SHARED)
else()
	set(BUILD_TYPE STATIC)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

include(Aurora/cmake/ProjectFilesExternal.cmake)
include_directories(${IObject_INCLUDE})

message(STATUS "IObject include dir: ${IObject_INCLUDE}")

add_library(RuntimeCompiler ${BUILD_TYPE} ${RuntimeCompiler_SRCS})
add_library(RuntimeObjectSystem ${BUILD_TYPE} ${RuntimeObjectSystem_SRCS})

target_link_libraries(RuntimeObjectSystem RuntimeCompiler)

if(UNIX)
	target_link_libraries(RuntimeObjectSystem dl)
endif()

SET(RCC_INCLUDE_DIRS "${IObject_INCLUDE};${CMAKE_CURRENT_SOURCE_DIR}/Aurora/RuntimeObjectSystem;${CMAKE_CURRENT_SOURCE_DIR}/Aurora/RuntimeCompiler" PARENT_SCOPE)
