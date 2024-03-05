
set(GLIB_FLAGS --target=resources.c)
set(GLIB_FLAGS ${GLIB_FLAGS} --generate-source)
set(GLIB_FLAGS ${GLIB_FLAGS} --sourcedir=${CMAKE_CURRENT_SOURCE_DIR})

add_custom_command(
        OUTPUT resources.c
        COMMAND glib-compile-resources ${CMAKE_CURRENT_SOURCE_DIR}/src/player.gresources.xml ${GLIB_FLAGS}
        DEPENDS src/player.gresources.xml
        src/ui/window.ui
        src/ui/video.ui
        src/ui/video-menu.ui
        src/style/style.css
)


pkg_check_modules(GTK REQUIRED IMPORTED_TARGET gtk+-3.0)
add_executable(player 
    src/player.c 
    ${CMAKE_CURRENT_BINARY_DIR}/resources.c 
    src/playerapp.c 
    src/playerappwin.c
    src/playervideo.c
)
target_link_libraries(player
    PkgConfig::GTK
    PkgConfig::GSTREAMER
)

add_custom_target(generate_resources DEPENDS resources.c)
add_dependencies(player generate_resources)
