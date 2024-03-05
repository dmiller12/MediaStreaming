
cmake_minimum_required(VERSION 3.0)

project(MediaStreaming)

set(WITH_SERVER ON CACHE BOOL "build with server")
set(WITH_PLAYER ON CACHE BOOL "build with player")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Add compiler flags for debug and release builds
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -g -fsanitize=address -fsanitize=undefined")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")


find_package(PkgConfig REQUIRED)
pkg_search_module(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-1.0)


if(WITH_SERVER)
    add_subdirectory(server)
endif()

if(WITH_PLAYER)
    add_subdirectory(player)
endif()
