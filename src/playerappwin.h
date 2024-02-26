#pragma once

#include "playerapp.h"
#include <gst/gst.h>
#include <gtk/gtk.h>

#define PLAYER_APP_WINDOW_TYPE (player_app_window_get_type())
G_DECLARE_FINAL_TYPE(PlayerAppWindow, player_app_window, PLAYER, APP_WINDOW, GtkApplicationWindow)

typedef struct _CustomData {
    GstElement *pipeline;
    GdkPaintable *sink_widget;
} CustomData;

PlayerAppWindow *player_app_window_new(PlayerApp *app);
