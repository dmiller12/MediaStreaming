#include "playerappwin.h"
#include "playerapp.h"
#include "playervideo.h"
#include <gst/gst.h>
#include <gtk/gtk.h>

struct _PlayerAppWindow {
    GtkApplicationWindow parent;

    GtkWidget *video1;
    GtkWidget *video2;
    GtkWidget *video3;
    GtkWidget *main_pane;
    GtkWidget *fullVideo;
};

G_DEFINE_TYPE(PlayerAppWindow, player_app_window, GTK_TYPE_APPLICATION_WINDOW)

typedef struct _Streams {
    CustomData *front;
    CustomData *back;
    CustomData *inhand;
} Streams;

typedef struct _NetworkParams {
    gchar *host;
    gint port;
    char *path;
} NetworkParams;

static gboolean on_key_release(GtkWidget *widget, GdkEventKey *event, PlayerAppWindow *app_window) {
    guint keyval = event->keyval;
    if ((keyval == GDK_KEY_0) || (keyval == GDK_KEY_Escape)) {
        video_widget_set_fullscreen(PLAYER_VIDEO_WIDGET(app_window->video1), FALSE);
        video_widget_set_fullscreen(PLAYER_VIDEO_WIDGET(app_window->video2), FALSE);
        video_widget_set_fullscreen(PLAYER_VIDEO_WIDGET(app_window->video3), FALSE);

        return TRUE;
    } else if (keyval == GDK_KEY_1) {
        video_widget_set_fullscreen(PLAYER_VIDEO_WIDGET(app_window->video2), FALSE);
        video_widget_set_fullscreen(PLAYER_VIDEO_WIDGET(app_window->video3), FALSE);
        video_widget_set_fullscreen(PLAYER_VIDEO_WIDGET(app_window->video1), TRUE);
        return TRUE;
    } else if (keyval == GDK_KEY_2) {
        video_widget_set_fullscreen(PLAYER_VIDEO_WIDGET(app_window->video1), FALSE);
        video_widget_set_fullscreen(PLAYER_VIDEO_WIDGET(app_window->video3), FALSE);
        video_widget_set_fullscreen(PLAYER_VIDEO_WIDGET(app_window->video2), TRUE);
        return TRUE;
    } else if (keyval == GDK_KEY_3) {
        video_widget_set_fullscreen(PLAYER_VIDEO_WIDGET(app_window->video1), FALSE);
        video_widget_set_fullscreen(PLAYER_VIDEO_WIDGET(app_window->video2), FALSE);
        video_widget_set_fullscreen(PLAYER_VIDEO_WIDGET(app_window->video3), TRUE);
        return TRUE;
    }
    return FALSE;
}

static gboolean fullscreen_changed(GtkWidget *widget, GParamSpec *pspec, PlayerAppWindow *win) {
    VideoWidget *video_widget = PLAYER_VIDEO_WIDGET(widget);
    GtkWidget *left_pane = gtk_paned_get_child1(GTK_PANED(win->main_pane));
    GtkWidget *right_pane = gtk_paned_get_child2(GTK_PANED(win->main_pane));
    gboolean is_fullscreen = video_widget_get_fullscreen(video_widget);
    if (is_fullscreen) {
        if (widget == left_pane) {
            gtk_widget_set_visible(right_pane, FALSE);
        } else {
            gtk_widget_set_visible(right_pane, TRUE);
        }

        gtk_widget_set_visible(widget, TRUE);

        if (widget != win->video1) {
            gtk_widget_set_visible(win->video1, FALSE);
        }

        if (widget != win->video2) {
            gtk_widget_set_visible(win->video2, FALSE);
        }

        if (widget != win->video3) {
            gtk_widget_set_visible(win->video3, FALSE);
        }
    } else {
        gtk_widget_set_visible(right_pane, TRUE);
        gtk_widget_set_visible(win->video1, TRUE);
        gtk_widget_set_visible(win->video2, TRUE);
        gtk_widget_set_visible(win->video3, TRUE);
    }

    return TRUE;
}

static gboolean main_changed(GtkWidget *widget, GParamSpec *pspec, PlayerAppWindow *win) {
    VideoWidget *video_widget = PLAYER_VIDEO_WIDGET(widget);
    gboolean is_main = video_widget_get_main(video_widget);
    if (is_main) {

        GtkWidget *left_pane = gtk_paned_get_child1(GTK_PANED(win->main_pane));
        GtkWidget *right_pane = gtk_paned_get_child2(GTK_PANED(win->main_pane));
        g_object_ref(widget);
        g_object_ref(left_pane);
        gtk_container_remove(GTK_CONTAINER(right_pane), GTK_WIDGET(widget));
        gtk_container_remove(GTK_CONTAINER(win->main_pane), GTK_WIDGET(left_pane));
        gtk_paned_pack1(GTK_PANED(win->main_pane), GTK_WIDGET(widget), TRUE, TRUE);
        gtk_box_pack_end(GTK_BOX(right_pane), GTK_WIDGET(left_pane), TRUE, TRUE, 0);
        video_widget_set_main(PLAYER_VIDEO_WIDGET(left_pane), FALSE);

        g_object_unref(widget);
        g_object_unref(left_pane);
    }

    return TRUE;
}

// TODO: gst_element_sync_state_with_parent ()
static void pad_added_handler(GstElement *src, GstPad *new_pad, gpointer user_data) {
    GstElement *decode_pay = GST_ELEMENT(user_data);
    GstPad *sink_pad = gst_element_get_static_pad(decode_pay, "sink");

    if (!gst_pad_is_linked(sink_pad)) {
        GstPadLinkReturn ret;
        GstCaps *new_pad_caps = NULL;
        GstStructure *new_pad_struct = NULL;
        const gchar *new_pad_type = NULL;

        g_print("Received new pad '%s' from '%s':\n", GST_PAD_NAME(new_pad), GST_ELEMENT_NAME(src));

        new_pad_caps = gst_pad_get_current_caps(new_pad);
        new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
        new_pad_type = gst_structure_get_name(new_pad_struct);

        if (!g_str_has_prefix(new_pad_type, "application/x-rtp")) {
            g_print("  It has type '%s' which is not video. Ignoring.\n", new_pad_type);
            goto exit;
        }

        ret = gst_pad_link(new_pad, sink_pad);
        if (GST_PAD_LINK_FAILED(ret)) {
            g_print("  Type is '%s' but link failed.\n", new_pad_type);
        } else {
            g_print("  Link succeeded (type '%s').\n", new_pad_type);
        }

    exit:
        if (new_pad_caps != NULL)
            gst_caps_unref(new_pad_caps);
        gst_object_unref(sink_pad);
    }
}

int create_pipeline(NetworkParams *params, CustomData *data) {

    /* Build the pipeline */
    GstElement *source = gst_element_factory_make("rtspsrc", "rtspsrc");
    GstElement *decode_pay = gst_element_factory_make("rtph264depay", "rtph264depay");
    GstElement *parse = gst_element_factory_make("h264parse", "h264parse");
    GstElement *decoder = gst_element_factory_make("avdec_h264", "avdec_h264");
    GstElement *converter = gst_element_factory_make("videoconvert", "videoconver");
    GstElement *videosink = gst_element_factory_make("gtksink", "gtksink");

    g_object_get(videosink, "widget", &(data->sink_widget), NULL);

    if (!source || !decode_pay || !parse || !decoder || !videosink) {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }
    data->pipeline = gst_pipeline_new("pipeline");
    gst_bin_add_many(GST_BIN(data->pipeline), source, decode_pay, parse, decoder, converter, videosink, NULL);
    if (!gst_element_link_many(decode_pay, parse, decoder, converter, videosink, NULL)) {
        g_printerr("Elements could not be linked.\n");
        return -1;
    }

    char uri[1000];

    snprintf(uri, sizeof(uri), "rtsp://%s:%d%s", params->host, params->port, params->path);

    g_object_set(source, "location", uri, "latency", 10, "do-retransmission", FALSE, "is-live", TRUE,
                 "default-rtsp-version", 32, NULL);
    g_signal_connect(source, "pad-added", G_CALLBACK(pad_added_handler), decode_pay);
    return 0;
}

Streams *setup_streams(void) {

    Streams *streams;

    streams = (Streams *)malloc(sizeof(Streams));

    streams->front = (CustomData *)malloc(sizeof(CustomData));
    streams->back = (CustomData *)malloc(sizeof(CustomData));
    streams->inhand = (CustomData *)malloc(sizeof(CustomData));

    NetworkParams frontParams = {.host = opt_host, .port = opt_port, .path = "/front"};
    NetworkParams backParams = {.host = opt_host, .port = opt_port, .path = "/back"};
    NetworkParams inhandParams = {.host = opt_host, .port = opt_port, .path = "/inhand"};

    int r;
    r = create_pipeline(&frontParams, streams->front);
    if (r < 0) {
        return NULL;
    }
    r = create_pipeline(&backParams, streams->back);
    if (r < 0) {
        return NULL;
    }
    r = create_pipeline(&inhandParams, streams->inhand);
    if (r < 0) {
        return NULL;
    }
    return streams;
}

static void player_app_window_init(PlayerAppWindow *win) {

    Streams *streams = setup_streams();

    gtk_widget_init_template(GTK_WIDGET(win));

    video_widget_set_video((VideoWidget *)win->video1, streams->front);
    video_widget_set_label_text((VideoWidget *)win->video1, "1: Front");

    video_widget_set_video((VideoWidget *)win->video2, streams->back);
    video_widget_set_label_text((VideoWidget *)win->video2, "2: Back");

    video_widget_set_video((VideoWidget *)win->video3, streams->inhand);
    video_widget_set_label_text((VideoWidget *)win->video3, "3: In-hand");

    gst_element_set_state(streams->front->pipeline, GST_STATE_PLAYING);
    gst_element_set_state(streams->back->pipeline, GST_STATE_PLAYING);
    gst_element_set_state(streams->inhand->pipeline, GST_STATE_PLAYING);

    gdk_window_set_events(GTK_WINDOW(win), gdk_window_get_events(GTK_WINDOW(win)) | GDK_KEY_RELEASE_MASK);

    g_signal_connect(GTK_WIDGET(win), "key-release-event", G_CALLBACK(on_key_release), win);

    // Load the CSS file
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(provider, "/com/ualberta/robotics/style.css");

    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);

    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_object_unref(provider);
    gtk_widget_show_all(GTK_WIDGET(win));
}

static void player_app_window_class_init(PlayerAppWindowClass *class) {
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class), "/com/ualberta/robotics/window.ui");
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), PlayerAppWindow, video1);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), PlayerAppWindow, video2);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), PlayerAppWindow, video3);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), PlayerAppWindow, main_pane);

    gtk_widget_class_bind_template_callback(GTK_WIDGET_CLASS(class), fullscreen_changed);
    gtk_widget_class_bind_template_callback(GTK_WIDGET_CLASS(class), main_changed);
}

PlayerAppWindow *player_app_window_new(PlayerApp *app) {
    return g_object_new(PLAYER_APP_WINDOW_TYPE, "application", app, NULL);
}
