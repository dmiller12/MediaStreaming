#include "playervideo.h"
#include "playerappwin.h"
#include <gst/gst.h>
#include <gtk/gtk.h>

struct _VideoWidget {
    GtkBox parent_instance;

    GtkWidget *frame_label;
    GtkWidget *video;
    GtkWidget *menu_button;
    gboolean fullscreen;
    gboolean main;

    CustomData *video_data;
};

G_DEFINE_TYPE(VideoWidget, video_widget, GTK_TYPE_BOX)

enum {
    PROP_FULLSCREEN = 1,
    PROP_MAIN = 2,
};


static void on_hide(VideoWidget *video_widget, gpointer user_data) {
    if (video_widget->video_data != NULL) {
        gst_element_set_state(video_widget->video_data->pipeline, GST_STATE_NULL);
    }
}

static void on_show(VideoWidget *video_widget, gpointer user_data) {
    if (video_widget->video_data != NULL) {
        gst_element_set_state(video_widget->video_data->pipeline, GST_STATE_PLAYING);
    }
}


void video_widget_set_video(VideoWidget *widget, CustomData *video_data) {

    // if (widget->video != NULL) {
    //     // Dispose the existing video widget if needed
    //     gtk_widget_destroy(widget->video);
    //
    //
    widget->video_data = video_data;
    gtk_picture_set_paintable(GTK_PICTURE(widget->video), widget->video_data->sink_widget);
}

void video_widget_set_label_text(VideoWidget *widget, const gchar *text) {

    gtk_label_set_text(GTK_LABEL(widget->frame_label), text);
}

static void video_widget_dispose(GObject *gobject) {
    gtk_widget_dispose_template(GTK_WIDGET(gobject), VIDEO_TYPE_WIDGET);

    // G_OBJECT_CLASS(video_widget_parent_class)->dispose(gobject);
}

// Define a new property for the "fullscreen" property
static void video_widget_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    VideoWidget *widget = PLAYER_VIDEO_WIDGET(object);

    switch (prop_id) {
        case PROP_FULLSCREEN:
            widget->fullscreen = g_value_get_boolean(value);
            break;
        case PROP_MAIN:
            widget->main = g_value_get_boolean(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void video_widget_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    VideoWidget *widget = PLAYER_VIDEO_WIDGET(object);

    switch (prop_id) {
        case PROP_FULLSCREEN:
            g_value_set_boolean(value, widget->fullscreen);
            break;
        case PROP_MAIN:
            g_value_set_boolean(value, widget->main);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

gboolean video_widget_get_fullscreen(VideoWidget *widget) {
    return widget->fullscreen;
}

void video_widget_set_fullscreen(VideoWidget *widget, gboolean fullscreen) {
    if (widget->fullscreen == fullscreen) {
        return;
    }
    widget->fullscreen = fullscreen;

    g_object_notify(G_OBJECT(widget), "fullscreen");
}

gboolean video_widget_get_main(VideoWidget *widget) {
    return widget->main;
}

void video_widget_set_main(VideoWidget *widget, gboolean main) {
    if (widget->main == main) {
        return;
    }
    widget->main = main;

    g_object_notify(G_OBJECT(widget), "main");
}

static void video_widget_class_init(VideoWidgetClass *class) {
    GObjectClass *object_class = G_OBJECT_CLASS(class);

    object_class->set_property = video_widget_set_property;
    object_class->get_property = video_widget_get_property;

    g_object_class_install_property(object_class,
                                    PROP_FULLSCREEN,
                                    g_param_spec_boolean("fullscreen",
                                                         "Fullscreen",
                                                         "Whether the widget is in fullscreen mode",
                                                         FALSE,
                                                         G_PARAM_READWRITE));

    g_object_class_install_property(object_class,
                                    PROP_MAIN,
                                    g_param_spec_boolean("main",
                                                         "Main",
                                                         "Whether the widget is in the Main pane",
                                                         FALSE,
                                                         G_PARAM_READWRITE));
    object_class->dispose = video_widget_dispose;

    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class), "/com/ualberta/robotics/video.ui");
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), VideoWidget, frame_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), VideoWidget, video);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), VideoWidget, menu_button);

}

static void video_widget_init(VideoWidget *widget) {
    gtk_widget_init_template(GTK_WIDGET(widget));

    GtkBuilder *builder = gtk_builder_new_from_resource("/com/ualberta/robotics/video-menu.ui");
    GMenuModel *menu = G_MENU_MODEL(gtk_builder_get_object(builder, "menu"));
    gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(widget->menu_button), menu);
    GtkPopover *popover = gtk_menu_button_get_popover(GTK_MENU_BUTTON(widget->menu_button));
    gtk_widget_set_halign(GTK_WIDGET(popover), GTK_ALIGN_END);
    g_object_unref(builder);

    GSimpleActionGroup *action_group = g_simple_action_group_new();
    GAction *action_full = (GAction*) g_property_action_new ("fullscreen", widget, "fullscreen");
    GAction *action_main = (GAction*) g_property_action_new ("main", widget, "main");
    g_action_map_add_action(G_ACTION_MAP(action_group), action_full);
    g_action_map_add_action(G_ACTION_MAP(action_group), action_main);

    gtk_widget_insert_action_group(GTK_WIDGET(widget), "video", G_ACTION_GROUP(action_group));

    g_object_unref (action_full);
    g_object_unref (action_main);


    g_signal_connect(widget, "hide", G_CALLBACK(on_hide), NULL);
    g_signal_connect(widget, "show", G_CALLBACK(on_show), NULL);
}
