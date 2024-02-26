#include <gst/gstelementfactory.h>
#include <string.h>

#include <gdk/gdk.h>
#include <gst/gst.h>
#include <gtk/gtk.h>

#include "playerapp.h"

typedef struct _CustomData {
    GstElement *pipeline;
    GdkPaintable *sink_widget;
} CustomData;

typedef struct _Streams {
    CustomData *front;
    CustomData *back;
    CustomData *inhand;
} Streams;

static void stop_cb(GtkButton *button, CustomData *data) {}

static void delete_event_cb(GtkWidget *widget, GdkEvent *event, CustomData *data) {
    stop_cb(NULL, data);
    gtk_window_destroy(GTK_WINDOW(widget));
}

static gboolean on_new_ssrc_callback(GstElement *sess, guint ssrc, gpointer udata) {}

/* This creates all the GTK+ widgets that compose our application, and registers
 * the callbacks */
static void create_ui(GtkApplication *app, Streams *data) {
    GtkWidget *main_box;  /* VBox to hold main_hbox and the controls */
    GtkWidget *main_hbox; /* HBox to hold the video sink and the stream info
                             text widget */

    CustomData *front_data = data->front;
    CustomData *back_data = data->back;
    CustomData *inhand_data = data->inhand;

    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "/Users/dylanmiller/Projects/MediaStreaming/gstreamer_test/src/ui/window.ui",
                              NULL);

    GObject *main_window = gtk_builder_get_object(builder, "window");
    gtk_window_set_application(GTK_WINDOW(main_window), app);

    g_signal_connect(main_window, "close_request", G_CALLBACK(delete_event_cb), front_data);

    // main_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    //
    // GtkWidget *frontcam_widget = gtk_picture_new_for_paintable(front_data->sink_widget);
    //
    // GtkWidget *overlay = gtk_overlay_new();
    //
    // GtkWidget *label = gtk_label_new("Front Camera");
    // gtk_widget_set_valign(label, GTK_ALIGN_START);
    //
    // gtk_overlay_set_child(GTK_OVERLAY(overlay), frontcam_widget);
    // gtk_overlay_add_overlay(GTK_OVERLAY(overlay), label);
    //
    // gtk_box_append(GTK_BOX(main_hbox), overlay);
    //
    // GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    // gtk_widget_set_valign(main_vbox, GTK_ALIGN_CENTER);
    // GtkWidget *backcam_widget = gtk_picture_new_for_paintable(back_data->sink_widget);
    // GtkWidget *inhandcam_widget = gtk_picture_new_for_paintable(inhand_data->sink_widget);
    //
    // gtk_box_append(GTK_BOX(main_vbox), backcam_widget);
    // gtk_box_append(GTK_BOX(main_vbox), inhandcam_widget);
    //
    // // main_box = gtk_box_new(jGTK_ORIENTATION_HORIZONTAL, 5);
    // main_box = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    //
    // gtk_paned_set_start_child(GTK_PANED(main_box), main_hbox);
    // gtk_paned_set_end_child(GTK_PANED(main_box), main_vbox);
    // // gtk_box_append(GTK_BOX(main_box), main_hbox);
    // // gtk_box_append(GTK_BOX(main_box), main_vbox);
    //
    // gtk_window_set_child(GTK_WINDOW(main_window), main_box);
    //
    // gst_element_set_state(front_data->pipeline, GST_STATE_PLAYING);
    // gst_element_set_state(back_data->pipeline, GST_STATE_PLAYING);
    // gst_element_set_state(inhand_data->pipeline, GST_STATE_PLAYING);

    gtk_window_present(GTK_WINDOW(main_window));
    g_object_unref(builder);
}

/* This function is called periodically to refresh the GUI */
static gboolean refresh_ui(CustomData *data) {
    GstStructure *stats, *twcc_stats;
    guint brvalue;
    gdouble bw;

    GstElement *rtpsession = gst_bin_get_by_name(GST_BIN(data->pipeline), "rtpsession0");
    if (!rtpsession) {
        g_printerr("Failed to get rtpsession\n");
        return 1;
    }
    g_object_get(rtpsession, "bandwidth", &bw, NULL);
    g_print("Bandwidth: %0.2f\n", bw);

    g_object_get(rtpsession, "twcc-stats", &twcc_stats, NULL);
    if (twcc_stats != NULL) {
        if (gst_structure_get_uint(twcc_stats, "bitrate-recv", &brvalue)) {
            g_print("RTP bitrate-recv %d\n", brvalue);
        }
        gst_structure_free(twcc_stats);
    } else {
        g_print("twcc-stats not found.\n");
    }

    g_object_get(rtpsession, "stats", &stats, NULL);
    if (!stats) {
        g_print("stats not found\n");
        return 1;
    }

    GValueArray *source_stats_array;
    if (gst_structure_get(stats, "source-stats", G_TYPE_VALUE_ARRAY, &source_stats_array, NULL)) {
        guint num_sources = source_stats_array != NULL ? source_stats_array->n_values : 0;

        // Iterate through the array of source-stats
        for (guint i = 0; i < num_sources; ++i) {
            const GValue *source_stat_value = g_value_array_get_nth(source_stats_array, i);
            if (source_stat_value != NULL && G_VALUE_HOLDS_BOXED(source_stat_value)) {
                // Extract and handle each source-stat structure
                GstStructure *source_stat_structure = g_value_get_boxed(source_stat_value);
                if (source_stat_structure != NULL) {
                    // Extract and handle fields from the source-stat structure
                    // Example: extract SSRC
                    guint64 bitrate;
                    guint jitter, round_trip;
                    gint packets_lost;

                    if (gst_structure_get(source_stat_structure, "bitrate", G_TYPE_UINT64, &bitrate, "jitter",
                                          G_TYPE_UINT, &jitter, "rb-round-trip", G_TYPE_UINT, &round_trip,
                                          "packets-lost", G_TYPE_INT, &packets_lost, NULL)) {
                        g_print("Source bitrate: %llu, jitter: %u, "
                                "packets-lost: %u, round trip: %u\n",
                                bitrate, jitter, packets_lost, round_trip);
                    }
                }
            }
        }
    } else {
        // Handling if the array retrieval fails
        g_print("Failed to retrieve source-stats array\n");
    }

    // Free the structure
    gst_structure_free(stats);

    // GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(data->pipeline),
    // GST_DEBUG_GRAPH_SHOW_FULL_PARAMS, "graph");
    return TRUE;
}

// TODO make sure to reutn TRUE to get successive messages, otherwise it stops
// triggering the callback
static gboolean error_cb(GstBus *bus, GstMessage *msg, CustomData *data) {
    GError *err;
    gchar *debug_info;

    /* Print error details on the screen */
    gst_message_parse_error(msg, &err, &debug_info);
    g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
    g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
    g_clear_error(&err);
    g_free(debug_info);

    /* Set the pipeline to READY (which stops playback) */
    gst_element_set_state(data->pipeline, GST_STATE_READY);
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

typedef struct _NetworkParams {
    char *host;
    char *port;
    char *path;
} NetworkParams;

// int create_pipeline(NetworkParams *params, CustomData *data) {
//
//     /* Build the pipeline */
//     GstElement *source = gst_element_factory_make("rtspsrc", "rtspsrc");
//     GstElement *decode_pay = gst_element_factory_make("rtph264depay", "rtph264depay");
//     GstElement *parse = gst_element_factory_make("h264parse", "h264parse");
//     GstElement *decoder = gst_element_factory_make("avdec_h264", "avdec_h264");
//     GstElement *converter = gst_element_factory_make("videoconvert", "videoconver");
//     GstElement *videosink = gst_element_factory_make("gtk4paintablesink", "gtk4paintablesink");
//
//     g_object_get(videosink, "paintable", &(data->sink_widget), NULL);
//
//     if (!source || !decode_pay || !parse || !decoder || !videosink) {
//         g_printerr("Not all elements could be created.\n");
//         return -1;
//     }
//     data->pipeline = gst_pipeline_new("pipeline");
//     gst_bin_add_many(GST_BIN(data->pipeline), source, decode_pay, parse, decoder, converter, videosink, NULL);
//     if (!gst_element_link_many(decode_pay, parse, decoder, converter, videosink, NULL)) {
//         g_printerr("Elements could not be linked.\n");
//         return -1;
//     }
//
//     char uri[1000];
//
//     snprintf(uri, sizeof(uri), "rtsp://%s:%s%s", params->host, params->port, params->path);
//
//     g_object_set(source, "location", uri, "latency", 10, "do-retransmission", FALSE, "is-live", TRUE,
//                  "default-rtsp-version", 32, NULL);
//     g_signal_connect(source, "pad-added", G_CALLBACK(pad_added_handler), decode_pay);
//     return 0;
// }

int main(int argc, char *argv[]) {
    GstBus *bus;
    GstStateChangeReturn ret;
    GtkWidget *sink_widget;
    CustomData front_data, back_data, inhand_data;

    // char *host = "192.168.1.11";
    char *host = "127.0.0.1";
    char *port = "8554";

    // GtkApplication *app = gtk_application_new("com.ualberta.robotics", G_APPLICATION_DEFAULT_FLAGS);

    /* Initialize GStreamer */
    gst_init(&argc, &argv);

    // memset(&front_data, 0, sizeof(front_data));
    // memset(&back_data, 0, sizeof(back_data));
    //
    // NetworkParams frontParams = {.host = host, .port = port, .path = "/front"};
    // NetworkParams backParams = {.host = host, .port = port, .path = "/back"};
    // NetworkParams inhandParams = {.host = host, .port = port, .path = "/inhand"};
    //
    // int r;
    // r = create_pipeline(&frontParams, &front_data);
    // if (r < 0) {
    //     return -1;
    // }
    // r = create_pipeline(&backParams, &back_data);
    // if (r < 0) {
    //     return -1;
    // }
    // r = create_pipeline(&inhandParams, &inhand_data);
    // if (r < 0) {
    //     return -1;
    // }
    //
    // bus = gst_element_get_bus(front_data.pipeline);
    // gst_bus_add_signal_watch(bus);
    // // TDOD: Check if this is actually called. Don't think so because we dont
    // // use glib mainloop
    // g_signal_connect(G_OBJECT(bus), "message::error", (GCallback)error_cb, &front_data);
    // gst_object_unref(bus);

    /* Start playing */
    // ret = gst_element_set_state(front_data.pipeline, GST_STATE_PLAYING);
    // if (ret == GST_STATE_CHANGE_FAILURE) {
    //     g_printerr("Unable to set the pipeline to the playing state.\n");
    //     gst_object_unref(front_data.pipeline);
    //     return -1;
    // } else if (ret == GST_STATE_CHANGE_NO_PREROLL) {
    //     g_print("live stream\n");
    // }
    //
    // ret = gst_element_set_state(back_data.pipeline, GST_STATE_PLAYING);
    // ret = gst_element_set_state(inhand_data.pipeline, GST_STATE_PLAYING);

    // Streams streams = {.front = &front_data, .back = &back_data, .inhand = &inhand_data};
    //
    // g_timeout_add_seconds(1, (GSourceFunc)refresh_ui, &front_data);
    // g_signal_connect(app, "activate", G_CALLBACK(create_ui), &streams);
    int status = g_application_run(G_APPLICATION(player_app_new()), argc, argv);

    /* Free resources */
    // gst_element_set_state(front_data.pipeline, GST_STATE_NULL);
    // gst_object_unref(front_data.pipeline);
    // g_object_unref(app);
    // TODO: make sure to clean resources properly
    // gst_object_unref(videosink);
    return 0;
}
