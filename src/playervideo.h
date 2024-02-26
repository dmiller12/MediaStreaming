#pragma once

#include "playerappwin.h"
#include <gtk/gtk.h>

#define VIDEO_TYPE_WIDGET (video_widget_get_type())
G_DECLARE_FINAL_TYPE(VideoWidget, video_widget, PLAYER, VIDEO_WIDGET, GtkBox)

void video_widget_set_video(VideoWidget *widget, CustomData *video_data);
void video_widget_set_label_text(VideoWidget *widget, const gchar *text);
gboolean video_widget_get_fullscreen(VideoWidget *widget);
void video_widget_set_fullscreen(VideoWidget *widget, gboolean fullscreen);
void video_widget_set_main(VideoWidget *widget, gboolean main);
gboolean video_widget_get_main(VideoWidget *widget);
