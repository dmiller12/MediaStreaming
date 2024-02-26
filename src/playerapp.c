
#include <gtk/gtk.h>

#include "playerapp.h"
#include "playerappwin.h"

struct _PlayerApp {
    GtkApplication parent;
};

G_DEFINE_TYPE(PlayerApp, player_app, GTK_TYPE_APPLICATION);

static void player_app_init(PlayerApp *app) {}

static void preferences_activated(GSimpleAction *action, GVariant *parameter, gpointer app) {}

static void quit_activated(GSimpleAction *action, GVariant *parameter, gpointer app) {
    g_application_quit(G_APPLICATION(app));
}

static GActionEntry app_entries[] = {{"preferences", preferences_activated, NULL, NULL, NULL},
                                     {"quit", quit_activated, NULL, NULL, NULL}};

static void player_app_startup(GApplication *app) {
    const char *quit_accels[2] = {"<Ctrl>Q", NULL};

    G_APPLICATION_CLASS(player_app_parent_class)->startup(app);

    g_action_map_add_action_entries(G_ACTION_MAP(app), app_entries, G_N_ELEMENTS(app_entries), app);
    gtk_application_set_accels_for_action(GTK_APPLICATION(app), "app.quit", quit_accels);
}

static void player_app_activate(GApplication *app) {
    PlayerAppWindow *win;

    win = player_app_window_new(PLAYER_APP(app));
    gtk_window_present(GTK_WINDOW(win));
}

static void player_app_class_init(PlayerAppClass *class) {
    G_APPLICATION_CLASS(class)->startup = player_app_startup;
    G_APPLICATION_CLASS(class)->activate = player_app_activate;
}

PlayerApp *player_app_new(void) {
    return g_object_new(PLAYER_APP_TYPE, "application-id", "com.ualberta.robotics", "flags",
                        G_APPLICATION_DEFAULT_FLAGS, NULL);
}
