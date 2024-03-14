#include <gst/gst.h>
#include <stdio.h>

#include <gst/rtsp-server/rtsp-server.h>

int main(int argc, char *argv[]) {
    GMainLoop *loop;
    GstRTSPServer *server;
    GstRTSPMountPoints *mounts;
    GstRTSPMediaFactory *front_factory, *back_factory, *inhand_factory;

    gst_init(&argc, &argv);

    loop = g_main_loop_new(NULL, FALSE);

    /* create a server instance */
    server = gst_rtsp_server_new();

    /* get the mount points for this server, every server has a default object
     * that be used to map uri mount points to media factories */
    mounts = gst_rtsp_server_get_mount_points(server);

    gboolean isTest = FALSE;
    if (argc > 1 && strcmp(argv[1], "test") == 0) {
        isTest = TRUE;
    }
    gchar *pipeline_str;
    gchar front_pipeline[1024];
    gchar back_pipeline[1024];
    char inhand_pipeline[1024];
    if (isTest) {
        pipeline_str = "( videotestsrc ! video/x-raw, width=960, height=720, framerate=30/1 ! "
                       "videoconvert ! x264enc speed-preset=veryfast tune=zerolatency ! "
                       "rtph264pay name=pay0 pt=96 )";
        g_strlcpy(front_pipeline, pipeline_str, sizeof(front_pipeline));
        g_strlcpy(back_pipeline, pipeline_str, sizeof(back_pipeline));
        g_strlcpy(inhand_pipeline, pipeline_str, sizeof(inhand_pipeline));
    } else {

        pipeline_str =
            "( aravissrc camera-name=%s exposure-auto=on gain-auto=on ! video/x-raw, width=960, height=720, "
            "framerate=30/1, format=RGB ! videoconvert ! vaapih264enc quality-level=7 ! queue ! rtph264pay name=pay0 pt=96 )";
        snprintf(front_pipeline, sizeof(front_pipeline), pipeline_str, "FLIR-1E100119E8A8-0119E8A8");
        g_strlcpy(back_pipeline,
                  "( v4l2src device=/dev/video2 ! video/x-raw, width=800, height=448, framerate=30/1 ! "
                  "videoconvert ! vaapih264enc quality-level=7 ! queue ! rtph264pay name=pay0 pt=96 )",
                  sizeof(back_pipeline));
        snprintf(inhand_pipeline, sizeof(inhand_pipeline), pipeline_str, "FLIR-1E100119E8AE-0119E8AE");
    }

    /* make a media factory for a test stream. The default media factory can use
     * gst-launch syntax to create pipelines.
     * any launch line works as long as it contains elements named pay%d. Each
     * element with pay%d names will be a stream */
    front_factory = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_launch(front_factory, front_pipeline);
    gst_rtsp_media_factory_set_shared(front_factory, TRUE);
    gst_rtsp_media_factory_set_profiles(front_factory, GST_RTSP_PROFILE_AVPF);

    back_factory = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_launch(back_factory, back_pipeline);
    gst_rtsp_media_factory_set_shared(back_factory, TRUE);
    gst_rtsp_media_factory_set_profiles(back_factory, GST_RTSP_PROFILE_AVPF);

    inhand_factory = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_launch(inhand_factory, inhand_pipeline);
    gst_rtsp_media_factory_set_shared(inhand_factory, TRUE);
    gst_rtsp_media_factory_set_profiles(inhand_factory, GST_RTSP_PROFILE_AVPF);

    /* attach the test factory to the /test url */
    gst_rtsp_mount_points_add_factory(mounts, "/front", front_factory);
    gst_rtsp_mount_points_add_factory(mounts, "/back", back_factory);
    gst_rtsp_mount_points_add_factory(mounts, "/inhand", inhand_factory);


    /* attach the server to the default maincontext */
    gst_rtsp_server_attach(server, NULL);

    /* don't need the ref to the mapper anymore */
    g_object_unref(mounts);

    /* start serving */
    g_print("stream ready at rtsp://127.0.0.1:8554/{front, back, inhand}\n");
    g_main_loop_run(loop);

    return 0;
}
