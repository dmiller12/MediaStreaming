
cmake_minimum_required(VERSION 3.0)

project(MediaStreaming)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED IMPORTED_TARGET gtk+-3.0)
pkg_search_module(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-1.0)
pkg_check_modules(GST_RTSP_SERVER REQUIRED IMPORTED_TARGET gstreamer-rtsp-server-1.0)

add_executable(server src/server.c)
target_link_libraries(server
    PkgConfig::GTK
    PkgConfig::GSTREAMER
    PkgConfig::GST_RTSP_SERVER
)

add_executable(player src/player.c)
target_link_libraries(player
    PkgConfig::GTK
    PkgConfig::GSTREAMER
)
