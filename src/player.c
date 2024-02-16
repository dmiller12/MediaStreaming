#include <gst/gstelementfactory.h>
#include <string.h>

#include <gst/gst.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

typedef struct _CustomData {
  GstElement *pipeline;
  GtkWidget *sink_widget;
} CustomData;

static void stop_cb(GtkButton *button, CustomData *data) {
}

static void delete_event_cb (GtkWidget *widget, GdkEvent *event, CustomData *data) {
  stop_cb(NULL, data);
  gtk_main_quit();
}

/* This creates all the GTK+ widgets that compose our application, and registers the callbacks */
static void create_ui (CustomData *data) {
  GtkWidget *main_window;  /* The uppermost window, containing all other windows */
  GtkWidget *main_box;     /* VBox to hold main_hbox and the controls */
  GtkWidget *main_hbox;    /* HBox to hold the video sink and the stream info text widget */

  main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (G_OBJECT (main_window), "delete-event", G_CALLBACK (delete_event_cb), data);

  main_hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start (GTK_BOX (main_hbox), data->sink_widget, TRUE, TRUE, 0);

  main_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start (GTK_BOX (main_box), main_hbox, TRUE, TRUE, 0);
  gtk_container_add (GTK_CONTAINER (main_window), main_box);
  gtk_window_set_default_size (GTK_WINDOW (main_window), 640, 480);

  gtk_widget_show_all (main_window);
}

/* This function is called when an error message is posted on the bus */
static void error_cb (GstBus *bus, GstMessage *msg, CustomData *data) {
  GError *err;
  gchar *debug_info;

  /* Print error details on the screen */
  gst_message_parse_error (msg, &err, &debug_info);
  g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
  g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
  g_clear_error (&err);
  g_free (debug_info);

  /* Set the pipeline to READY (which stops playback) */
  gst_element_set_state (data->pipeline, GST_STATE_READY);
}

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

int main(int argc, char *argv[]) {
  GstBus *bus;
  GstStateChangeReturn ret;
  GtkWidget *sink_widget;
  CustomData data;

  gtk_init(&argc, &argv);

  /* Initialize GStreamer */
  gst_init (&argc, &argv);

  memset(&data, 0, sizeof(data));

  /* Build the pipeline */
  GstElement *source = gst_element_factory_make("rtspsrc", "rtspsrc");
  GstElement *decode_pay = gst_element_factory_make("rtph264depay", "rtph264depay");
  GstElement *parse = gst_element_factory_make("h264parse", "h264parse");
  GstElement *decoder = gst_element_factory_make("avdec_h264", "avdec_h264");
  GstElement *converter = gst_element_factory_make("videoconvert", "videoconver");
  GstElement *videosink = gst_element_factory_make("glsinkbin", "glsinkbin");
  GstElement *gtkglsink = gst_element_factory_make("gtkglsink", "gtkglsink");
  // pipeline = gst_parse_launch ("rtspsrc location=rtsp://192.168.1.11:8554/test latency=0 do-retransmission=false is-live=true  debug=true ! rtph264depay ! h264parse ! avdec_h264 ! autovideosink sync=false", NULL);

  if ( (videosink != NULL) && (gtkglsink != NULL)) {
    g_object_set(videosink, "sink", gtkglsink, NULL);
    g_object_get(videosink, "widget", &data.sink_widget, NULL);
  } else {
    videosink = gst_element_factory_make("gtksink", "gtksink");
    g_object_get(videosink, "widget", &data.sink_widget, NULL);    
  }
  
  if (!source || !decode_pay || !parse || !decoder) {
    g_printerr("Not all elements could be created.\n");
    return -1;
  }
  data.pipeline = gst_pipeline_new("pipeline");
  gst_bin_add_many(GST_BIN(data.pipeline), source, decode_pay, parse, decoder, converter, videosink, NULL);
  if (!gst_element_link_many(decode_pay, parse, decoder, converter, videosink, NULL)) {
    g_printerr("Elements could not be linked.\n");
    return -1;
  }

  g_object_set(source, "location", "rtsp://192.168.1.11:8554/test", "latency", 0, "do-retransmission", FALSE, "is-live", TRUE, NULL);
  g_signal_connect(source, "pad-added", G_CALLBACK(pad_added_handler), decode_pay);


  create_ui(&data);
  
  bus = gst_element_get_bus (data.pipeline);
  gst_bus_add_signal_watch (bus);
  g_signal_connect (G_OBJECT (bus), "message::error", (GCallback)error_cb, &data);
  gst_object_unref (bus);

  /* Start playing */
  ret = gst_element_set_state (data.pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (data.pipeline);
    return -1;
  } else if (ret == GST_STATE_CHANGE_NO_PREROLL) {
    g_print("live stream\n");
  }

  gtk_main();

  /* Free resources */
  gst_element_set_state (data.pipeline, GST_STATE_NULL);
  gst_object_unref (data.pipeline);
  gst_object_unref(videosink);
  return 0;
}
