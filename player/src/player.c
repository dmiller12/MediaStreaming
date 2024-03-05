#include <gst/gstelementfactory.h>
#include <string.h>

#include <gdk/gdk.h>
#include <gst/gst.h>
#include <gtk/gtk.h>

#include "playerapp.h"

typedef struct _CustomData {
    GstElement *pipeline;
    GtkWidget *sink_widget;
} CustomData;

static gboolean on_new_ssrc_callback(GstElement *sess, guint ssrc, gpointer udata) {}

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

// TODO: make sure to reutn TRUE to get successive messages, otherwise it stops
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

int main(int argc, char *argv[]) {
    GstBus *bus;

    char *host = "192.168.1.11";
    // char *host = "127.0.0.1";
    char *port = "8554";

    /* Initialize GStreamer */
    gst_init(&argc, &argv);

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
    int status = g_application_run(G_APPLICATION(player_app_new()), argc, argv);

    /* Free resources */
    // gst_element_set_state(front_data.pipeline, GST_STATE_NULL);
    // gst_object_unref(front_data.pipeline);
    // g_object_unref(app);
    // TODO: make sure to clean resources properly
    // gst_object_unref(videosink);
    return 0;
}
