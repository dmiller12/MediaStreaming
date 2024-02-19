
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
    pkg_check_modules(GST_RTSP_SERVER REQUIRED IMPORTED_TARGET gstreamer-rtsp-server-1.0)
    add_executable(server src/server.c)
    target_link_libraries(server
        PkgConfig::GSTREAMER
        PkgConfig::GST_RTSP_SERVER
    )
endif()

if(WITH_PLAYER)
    pkg_check_modules(GTK REQUIRED IMPORTED_TARGET gtk+-3.0)
    add_executable(player src/player.c)
    target_link_libraries(player
        PkgConfig::GTK
        PkgConfig::GSTREAMER
    )
endif()
