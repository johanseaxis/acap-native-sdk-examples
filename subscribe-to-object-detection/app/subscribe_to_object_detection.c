/**
 * Copyright (C) 2020, Axis Communications AB, Lund, Sweden
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * brief This example illustrates how to subscribe to video object detection
 *
 * Video object detection or VOD for short is an API that sends events of VOD
 * detection events.
 *
 * Error handling has been minimized for the sake of brevity.
 */

#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>

#include <protobuf-c/protobuf-c.h>
#include "video_object_detection_subscriber.h"
#include "video-object-detection.pb-c.h"

#define INFO(...) syslog(LOG_INFO, __VA_ARGS__)
#define ERR(...) syslog(LOG_ERR, __VA_ARGS__)
#define REPORT_FMT "[#%u] id = %d. class[%i] = %s. score = %u >= %d. " \
    "bbox[l,r,t,b]=[%f, %f, %f, %f]. \n"

/**
 * Object detection score thresholds
 *
 * See README file for more information on threshold values and what different
 * values means for detection quality. These thresholds come from the output
 * of function print_calibration_data().
 */
#define CALIB_HUMAN 23
#define CALIB_VEHICLE 16
#define CALIB_BIKE 22
#define CALIB_DEFAULT 10


/***** Struct and enum declarations *******************************************/

typedef enum {
    FORMAT_NORMALIZED = 0,
    FORMAT_SCALED = 1,
    FORMAT_UNKNOWN = -1
} data_output_format_t;

typedef struct {
    int32_t id;
    const char *class_name;
    int32_t threshold;
} class_info_t;

typedef struct {
    data_output_format_t output_format;
    int output_width;
    int output_height;
    video_object_detection_subscriber_class_t **det_classes;
    int num_classes;
    class_info_t **class_info;
    int *lookup_table;
} test_app_data_t;

typedef struct {
    float left;
    float top;
    float right;
    float bottom;
    int32_t id;
    int32_t det_class;
    uint32_t score;
} video_object_detection_subscriber_hit_t;

typedef struct {
    uint64_t  timestamp;                      // Image timestamp
    uint32_t  nbr_of_detections;              // Number of found detections
    video_object_detection_subscriber_hit_t
    detection_list[];                         // Array of coordinates and sizes
} video_object_detection_subscriber_detections_t;


/***** Global variables *******************************************************/

static video_object_detection_subscriber_t *subscriber = NULL;
static data_output_format_t output_format = FORMAT_NORMALIZED;
static bool receive_empty_hits = false;
static int channel = 0;
extern char *optarg;
static int output_format_width = -1;
static int output_format_height = -1;
static volatile bool should_run = true;


/***** Help functions *********************************************************/

/**
 * brief Signal handler for the application
 *
 * param signum Caught signal
 */
static void
signal_handler(int signum)
{
    switch (signum) {
        case SIGINT:
            INFO("Caught SIGINT signal\n");
            should_run = false;
            break;

        default:
            // No action
            break;
    }
}

/**
 * brief Setup function for signals
 */
static void
setup_signals(void)
{
    struct sigaction sa;
    sa.sa_flags = 0;
    if (sigemptyset(&sa.sa_mask) < 0) {
        ERR("Failed: sigemptyset %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    sa.sa_handler = signal_handler;
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        ERR("Failed: sigaction %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

/**
 * brief Function that print calibration data to syslog
 */
static bool
print_calibration_data(void)
{
    uint8_t *buffer = NULL;
    size_t size = 0;
    int ret = 0;
    VOD__DetectorInformation *detector_info;
    VOD__Calibration **calibrations;

    ret = video_object_detection_subscriber_get_detector_information(&buffer,
                                                                     &size);
    if (ret != 0) {
      return false;
    }

    detector_info = vod__detector_information__unpack(NULL, size, buffer);
    if (detector_info == NULL) {
        ERR("Error in unpacking detector information");
        return false;
    }

    unsigned i;
    for (i = 0; i < detector_info->n_classes; i++) {
        INFO("class[%i] = %s, nbr_calibrations: %i",
             detector_info->classes[i]->id,
             detector_info->classes[i]->name,
             detector_info->classes[i]->n_calibrations);

        calibrations = detector_info->classes[i]->calibrations;
        unsigned j;
        for (j = 0; j < detector_info->classes[i]->n_calibrations; j++) {
            INFO("  threshold: %-5.2f -->  precision: %-5.2f, recall: %-5.2f",
                 calibrations[j]->threshold,
                 calibrations[j]->precision,
                 calibrations[j]->recall);
        }
    }

    vod__detector_information__free_unpacked(detector_info, NULL);
    free(buffer);

    return true;
}


/***** Command line functions *************************************************/

/**
 * brief Print help text to commandline call
 *
 * param name Name of application
 */
static void
print_help(const char *name)
{
    static const char msg[] =
        "Usage:\n  %s [options]...\n\n"
        "Options are:\n"
        "  -e               Receive data even if no hits are found\n"
        "  -o               Output format: normalized (default) | <width>x<height>\n"
        "  -c channel       Select input channel to subcribe to detections for\n"
        "  -h               Print this help\n\n";

    fprintf(stderr, msg, name);
}

/**
 * brief Parse output resolution
 *
 * param res Resolution string "<width>x<height>"
 * param width Width variable to set
 * param height Height variable to set
 */
static bool
parse_output_resolution(const char *res, int *width, int *height)
{
    int _width, _height;
    int result = sscanf(res, "%5dx%5d", &_width, &_height);
    if (result == 2) {
        *width = _width;
        *height = _height;
        return true;
    }
    return false;
}

/**
 * brief Parse output format from commandline option
 *
 * param format Format string - "normalized" or "<width>x<height>"
 */
static data_output_format_t
parse_output_format(char *format)
{
    if (format == NULL) {
        return FORMAT_UNKNOWN;
    } else if (strcmp(format, "normalized") == 0) {
        return FORMAT_NORMALIZED;
    } else {
        if (parse_output_resolution(format,
                                    &output_format_width,
                                    &output_format_height) == false) {
            return FORMAT_UNKNOWN;
        }
        return FORMAT_SCALED;
    }
}

/**
 * brief Parse arguments if called from commandline
 *
 * param argc Number of commandline arguments
 * param argv Array of commandline arguments
 */
static void
parse_args(int argc, char **argv)
{
    int ch;

    /* Get options */
    while ((ch = getopt(argc, argv, "eho:c:")) != -1) {
        switch (ch) {
            case 'e':
                receive_empty_hits = true;
                break;

            case 'o':
                output_format = parse_output_format(optarg);
                break;

            case 'c':
                channel = atoi(optarg);
                break;

            case 'h':
            case '?':
                print_help(argv[0]);
                exit(EXIT_SUCCESS);
        }
    }
}


/***** Object detection functions *********************************************/

/**
 * brief Function to get object class name
 *
 * param det_classes VOD class variable
 * param num_classes Number of class objects
 * param class_id Class object id of detection
 */
static const char *
get_class_name_from_id(video_object_detection_subscriber_class_t **det_classes,
    int num_classes, int class_id)
{
    int i;
    for (i = 0; i < num_classes; i++) {
        if (video_object_detection_subscriber_det_class_id(det_classes[i]) ==
            class_id) {
            return video_object_detection_subscriber_det_class_name(
                det_classes[i]);
        }
    }
    return "None";
}

/**
 * brief Scale a coordinate
 *
 * Scale from normalized ONVIF coordinate space to pixel coordinate space
 *
 * param value ONVIF coordinate value
 * param base Height or width in pixels
 */
static float
scale_pos(float value, float base)
{
    return (value + 1.0) * base / 2.0;
}

/**
 * brief Print detection info to syslog
 *
 * Detections with score higher than class object threshold will be printed to
 * syslog.
 *
 * param detections VOD detections variable
 * param app_data User data
 */
static void
report_normalized_detections(
    const video_object_detection_subscriber_detections_t *detections,
    test_app_data_t *app_data)
{
    unsigned int i;
    int accepted_hits = 0;
    for (i = 0; i < detections->nbr_of_detections; i++) {
        video_object_detection_subscriber_hit_t detection =
            detections->detection_list[i];
        if(detection.score >=
           app_data->class_info[app_data->lookup_table[detection.det_class]]->
           threshold) {
            accepted_hits += 1;
            INFO(REPORT_FMT,
                 i,
                 detection.id,
                 detection.det_class,
                 get_class_name_from_id(app_data->det_classes,
                                        app_data->num_classes,
                                        detection.det_class),
                 detection.score,
                 app_data->class_info[app_data->
                     lookup_table[detection.det_class]]->threshold,
                 detection.left,
                 detection.right,
                 detection.top,
                 detection.bottom);
        }
    }

    if(accepted_hits > 0) {
        INFO("[timestamp %" PRIu64 "]: %i/%u OK detections!\n",
             detections->timestamp,
             accepted_hits,
             detections->nbr_of_detections);
    }

}

/**
 * brief Print detection info to syslog with bounding box in pixels
 *
 * Detections with score higher than class object threshold will be printed to
 * syslog.
 *
 * param detections VOD detections variable
 * param app_data User data
 */
static void
report_scaled_detections(
    const video_object_detection_subscriber_detections_t *detections,
    test_app_data_t *app_data)
{
    if (app_data->output_width == -1 || app_data->output_height == -1) {
        ERR("Illegal format\n");
        return;
    }

    unsigned int i;
    int accepted_hits = 0;
    for (i = 0; i < detections->nbr_of_detections; i++) {
        video_object_detection_subscriber_hit_t detection =
            detections->detection_list[i];
        if(detection.score >=
           app_data->class_info[app_data->lookup_table[detection.det_class]]->
           threshold) {
            accepted_hits += 1;
            /**
             * Convert the ONVIF normalized cartesian coordinates to pixel
             * coordinates. N.b. the minus sign is needed for top and bottom
             * coordinates since the positive y axis in ONVIF space is pointing
             * upwards.
             */
            float left_px = scale_pos(detection.left, app_data->output_width);
            float top_px = scale_pos(-detection.top, app_data->output_height);
            float right_px = scale_pos(detection.right, app_data->output_width);
            float bottom_px = scale_pos(-detection.bottom, app_data->output_height);
            INFO(REPORT_FMT,
                 i,
                 detection.id,
                 detection.det_class,
                 get_class_name_from_id(app_data->det_classes,
                                        app_data->num_classes,
                                        detection.det_class),
                 detection.score,
                 app_data->class_info[app_data->
                     lookup_table[detection.det_class]]->threshold,
                 left_px,
                 top_px,
                 right_px,
                 bottom_px);
        }
    }

    if(accepted_hits > 0) {
        INFO("[timestamp %" PRIu64 "]: %i/%u OK detections!\n",
             detections->timestamp,
             accepted_hits,
             detections->nbr_of_detections);
    }

}

/**
 * brief Helper function to unpack serialized data
 *
 * Use libprotobuf-c and generated code to unpack serialized detection data.
 *
 * param object_detections VOD detections variable
 * param buffer Data buffer
 * param buffer_size Buffer size
 */
static bool
get_detections(
    video_object_detection_subscriber_detections_t **object_detections,
    const uint8_t *buffer, const size_t buffer_size)
{
    VOD__Scene *recv_scene;

    // Deserialize the serialized input
    recv_scene = vod__scene__unpack(NULL, buffer_size, buffer);
    if (recv_scene == NULL) {
        ERR("Error in unpacking detections");
        return false;
    }
    unsigned int nbr_of_detections = recv_scene->n_detections;
    VOD__Detection **detection_list = recv_scene->detections;

    // Allocate memory for all detected objects
    video_object_detection_subscriber_detections_t *detections = malloc(
        sizeof(video_object_detection_subscriber_detections_t) +
        nbr_of_detections * sizeof(video_object_detection_subscriber_hit_t));
    if (detections == NULL) {
        ERR("Out of memory, was not able to get detections");
        vod__scene__free_unpacked(recv_scene, NULL);
        return false;
    }

    detections->timestamp = recv_scene->timestamp;
    detections->nbr_of_detections = nbr_of_detections;

    unsigned i;
    for (i = 0; i < nbr_of_detections; i++) {
        video_object_detection_subscriber_hit_t hit;
        VOD__Detection *recv_det = detection_list[i];

        // Get the ONVIF normalized coordinates for detection box
        hit.left = recv_det->left;
        hit.top = recv_det->top;
        hit.right = recv_det->right;
        hit.bottom = recv_det->bottom;

        /**
         * Get class label and score for detection. Convert the class label to
         * signed int, this way it may be used directly to match class nice
         * names with functions video_object_detection_subscriber_det_class_name
         * and video_object_detection_subscriber_det_class_id
         */
        hit.det_class = (int32_t)recv_det->det_class;
        hit.score = recv_det->score;

        /**
         * Get an id for detection.
         * N.b. this id is not guaranteed to be consistent across frames, for
         * some FW versions it's not even expected.
         */
        hit.id = recv_det->id;

        detections->detection_list[i] = hit;
    }

    vod__scene__free_unpacked(recv_scene, NULL);

    *object_detections = detections;

    return true;
}

/**
 * brief Callback function for detections
 *
 * Function is called when object detection data has been produced. A quick copy
 * of detections should be made and then return. Further processing of detection
 * data should be avoided here to not block new detections. A better way to make
 * more processing might be to use a second main thread.
 *
 * param detection_data Data buffer
 * param data_size Buffer size
 * param user_data User data
 */
static void
get_detections_callback(const uint8_t *detection_data,
    size_t data_size, void *user_data)
{
    test_app_data_t *app_data = (test_app_data_t *)user_data;

    video_object_detection_subscriber_detections_t *detections = NULL;
    bool ret = get_detections(&detections, detection_data, data_size);
    if (!ret) {
        ERR("Failed deserializing data\n");
        return;
    }

    switch (app_data->output_format) {
        case FORMAT_NORMALIZED: {
            report_normalized_detections(detections, app_data);
            break;
        }
        case FORMAT_SCALED: {
            report_scaled_detections(detections, app_data);
            break;
        }
        case FORMAT_UNKNOWN: {
            ERR("Wrong format\n");
            break;
        }
    }
    free(detections);

    return;
}

/**
 * brief Teardown function
 *
 * If the video object detection service goes down this function will cleanup
 * and exit.
 *
 * param user_data User data
 */
static void
teardown_callback(void *user_data)
{
    test_app_data_t *app_data = (test_app_data_t *)user_data;
    (void)app_data;
    INFO("Teardown\n");
    should_run = false;
    return;
}


/***** Main *******************************************************************/

/**
 * brief Main function
 */
int
main(int argc, char **argv)
{
    test_app_data_t app_data;

    // Setup syslog and signal handler
    openlog(NULL, LOG_PID, LOG_USER);
    setup_signals();

    // Set output format
    if (argc) {
        parse_args(argc, argv);
    }
    app_data.output_format = output_format;
    app_data.output_width  = output_format_width;
    app_data.output_height = output_format_height;

    // Print calibration data
    INFO("Available object classes and calibration data:\n");
    if (!print_calibration_data()) {
        ERR("Failed to print calibration data\n");
    }

    // Get available classes
    app_data.det_classes = NULL;
    app_data.num_classes = video_object_detection_subscriber_det_classes_get(
        &app_data.det_classes);
    if (app_data.num_classes < 0) {
        ERR("Failed to get available classes\n");
    }

    // Allocate and assign class info
    class_info_t *class_info[app_data.num_classes];
    int i;
    int max_id = 0;
    INFO("Threshold value set for each object class:\n");
    for (i = 0; i < app_data.num_classes; i++) {
        class_info[i] = malloc(sizeof(class_info_t));
        class_info[i]->id =
            video_object_detection_subscriber_det_class_id(
                app_data.det_classes[i]);
        class_info[i]->class_name =
            video_object_detection_subscriber_det_class_name(
                app_data.det_classes[i]);

        // Set threshold for each object class
        if (strcmp(class_info[i]->class_name, "Human") == 0) {
            class_info[i]->threshold = CALIB_HUMAN;
        } else if (strcmp(class_info[i]->class_name, "Vehicle") == 0) {
            class_info[i]->threshold = CALIB_VEHICLE;
        } else if (strcmp(class_info[i]->class_name, "Bike") == 0) {
            class_info[i]->threshold = CALIB_BIKE;
        } else {
            class_info[i]->threshold = CALIB_DEFAULT;
        }
        INFO("class[%i] = %s, threshold=%d\n", class_info[i]->id,
             class_info[i]->class_name, class_info[i]->threshold);

        // Check if new max id to use in creation of lookup table
        if (max_id < class_info[i]->id) {
            max_id = class_info[i]->id;
        }
    }
    app_data.class_info = class_info;

    // Create lookup table
    int lookup_table[max_id + 1];
    for (i = 0; i < app_data.num_classes; i++) {
        lookup_table[class_info[i]->id] = i;
    }
    app_data.lookup_table = lookup_table;

    // Create VOD subscriber instance
    if (video_object_detection_subscriber_create(&subscriber, &app_data,
                                                 channel) != 0) {
        ERR("Creation failed\n");
        goto EXIT;
    }

    // Register callback function
    if (video_object_detection_subscriber_set_get_detection_callback(subscriber,
            get_detections_callback) != 0) {
        ERR("Failed to set get detections callback\n");
    }

    // Register callback teardown function
    if (video_object_detection_subscriber_set_teardown_callback(subscriber,
            teardown_callback) != 0) {
        ERR("Failed to set teardown callback\n");
    }

    // Start subscription of detections from VOD service
    if (video_object_detection_subscriber_subscribe(subscriber) != 0) {
        ERR("Subscribe failed\n");
        goto EXIT;
    }

    // Run while service is running and no exit signal received
    while (should_run &&
           video_object_detection_subscriber_running(subscriber)) {
        sleep(5);
    }

    // Cancel subscription of detections from VOD service
    if (video_object_detection_subscriber_unsubscribe(subscriber) != 0) {
        ERR("Unsubscribe failed\n");
    }

EXIT:
    // Free VOD subscriber instance
    if (video_object_detection_subscriber_delete(&subscriber) != 0) {
        ERR("Destroy failed\n");
    }

    // Free allocated object detection classes
    video_object_detection_subscriber_det_classes_free(app_data.det_classes,
        app_data.num_classes);

    // Free allocated class info
    for (i = 0; i < app_data.num_classes; i++) {
        free(app_data.class_info[i]);
    }

    return 0;
}
