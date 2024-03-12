#include "playerapp.h"
#include <gdk/gdk.h>
#include <gst/gst.h>
#include <gst/gstelementfactory.h>
#include <gtk/gtk.h>
#include <string.h>

typedef struct _CustomData {
    GstElement *pipeline;
    GtkWidget *sink_widget;
} CustomData;

int main(int argc, char *argv[]) {

    /* Initialize GStreamer */
    gst_init(&argc, &argv);
    int status = g_application_run(G_APPLICATION(player_app_new()), argc, argv);

    return 0;
}
