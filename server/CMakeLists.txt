
pkg_check_modules(GST_RTSP_SERVER REQUIRED IMPORTED_TARGET gstreamer-rtsp-server-1.0)
add_executable(server src/server.c)
target_link_libraries(server
    PkgConfig::GSTREAMER
    PkgConfig::GST_RTSP_SERVER
)
