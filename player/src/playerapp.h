#pragma once

#include <gtk/gtk.h>

#define PLAYER_APP_TYPE (player_app_get_type())
G_DECLARE_FINAL_TYPE(PlayerApp, player_app, PLAYER, APP, GtkApplication)

PlayerApp *player_app_new(void);

extern gchar *opt_host;
extern gint opt_port;
