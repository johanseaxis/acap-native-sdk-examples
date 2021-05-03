/**
 * Copyright (C) 2021, Axis Communications AB, Lund, Sweden
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
 * brief This example illustrates how to subscribe to motion based object
 * tracking metadata, video scene data, and compute some running statistics
 * from the data. The running statistics are printed to the syslog.
 *
 * Data is deserialized and queued in a callback function and further
 * processed in the main loop.
 *
 * Error handling has been minimized for the sake of brevity.
 */

#include <arpa/inet.h>
#include <atomic>
#include <cmath>
#include <csignal>
#include <cstddef>
#include <mutex>
#include <queue>
#include <syslog.h>
#include <thread>
#include <time.h>

#include <video_scene_subscriber.h>

#include "scene.pb.h"

// Flag to control whether app should run or exit
static std::atomic<bool> should_run{true};

// Structure to hold application user data
struct app_data_t
{
    // Queue with scene data
    std::queue<Scene> scene_queue;
    // Mutex to guard the scene queue
    std::mutex scene_queue_mutex;

    // Keep track of active objects
    std::set<int> objects_all_set;
    std::set<int> objects_large_set;
    std::set<int> objects_speeding_set;

    // Thresholds for computing statistics
    const float speed_limit = 0.2f;
    const float area_limit = 0.05f;

    // Running statistics
    int objects_total = 0;
    int objects_total_large = 0;
    int objects_total_speeding = 0;
};

/**
 * brief Callback function for scene data
 *
 * Function is called when new video scene data has been produced.
 * Scene data is unpacked using protobuf and put in a queue for further processing.
 * Further processing of scene data is avoided here to not block new data and done
 * in the main loop.
 *
 * param scene_data Data buffer
 * param user_data User data
 */
void on_message_arrived(char *scene_data, void *user_data)
{
    // Unpack the header which is expected to be the size
    // of the protobuf payload and the header size according to
    // the library documentation for data format SCENE_PROTO_1
    uint32_t message_size = *(uint32_t *)scene_data;
    message_size = ntohl(message_size) - sizeof(uint32_t);
    // Unpack protobuf content, data starts after the 4 byte header
    const char *payload = scene_data + sizeof(uint32_t);
    std::string payload_string(payload, message_size);
    Scene scene;
    if (!scene.ParseFromString(payload_string))
    {
        throw std::runtime_error("Failed to unpack protobuf scene data");
    }
    // Free the scene data after it is deserialized
    free(scene_data);

    // Cast the user data to access queue
    app_data_t *app_data = (app_data_t *)user_data;

    // Put the scene data with objects or events in queue for further processing
    if (scene.objects_size() > 0 || scene.events_size() > 0)
    {
        std::lock_guard<std::mutex> lock(app_data->scene_queue_mutex);
        app_data->scene_queue.push(scene);
    }
}

/**
 * brief Process a scene data object and compute some statistics based on it.
 *
 * - Count objects with an absolute speed higher than the speed limit.
 * - Count objects with an area larger than the area limit.
 *
 * param scene scene data object
 * param app_data User data
 */
static void process_scene(const Scene scene, app_data_t *app_data)
{
    time_t scene_time = scene.real_time() / 1E9; // convert nanoseconds to seconds
    syslog(LOG_INFO, "%s: %d objects, %d events\n", ctime(&scene_time),
           scene.objects_size(), scene.events_size());

    // Iterate over all objects in scene data
    for (int i = 0; i < scene.objects_size(); i++)
    {
        const Object &object = scene.objects(i);

        // Check if object is new or already in active set
        if (app_data->objects_all_set.find(object.id()) == app_data->objects_all_set.end())
        {
            app_data->objects_all_set.insert(object.id());
            app_data->objects_total++;
        }

        // Check for "speeding" object if velocity is enabled and present in scene data
        if (object.has_velocity())
        {
            float vx = object.velocity().vx();
            float vy = object.velocity().vy();
            float object_speed = std::sqrt(vx * vx + vy * vy);
            // Check speed limit and increase statistic if speeding object
            if (object_speed > app_data->speed_limit)
            {
                if (app_data->objects_speeding_set.find(object.id()) == app_data->objects_speeding_set.end())
                {
                    app_data->objects_speeding_set.insert(object.id());
                    app_data->objects_total_speeding++;
                }
            }
        }

        // Check if "large" object if bounding box is enabled and present in scene data
        if (object.has_bounding_box())
        {
            const BoundingBox &bbox = object.bounding_box();
            float width = bbox.right() - bbox.left();
            float height = bbox.top() - bbox.bottom();
            float area = width * height;
            // Check area limit and increase statistic if large object
            if (area > app_data->speed_limit)
            {
                if (app_data->objects_large_set.find(object.id()) == app_data->objects_large_set.end())
                {
                    app_data->objects_large_set.insert(object.id());
                    app_data->objects_total_large++;
                }
            }
        }
    }

    // Iterate over all events in scene data
    for (int i = 0; i < scene.events_size(); i++)
    {
        // Check event and remove deleted object from active sets
        const Event &event = scene.events(i);
        if (event.action() == EVENT_DELETE)
        {
            app_data->objects_all_set.erase(event.object_id());
            app_data->objects_large_set.erase(event.object_id());
            app_data->objects_speeding_set.erase(event.object_id());
        }
    }
}

/**
 * brief Signal handler for the application
 *
 * param sig Caught signal
 */
static void terminationHandler(int sig)
{
    (void)sig;
    should_run = false;
}

/**
 * brief Main function
 */
int main(void)
{
    // Setup logging and signal handling
    openlog(NULL, LOG_PID, LOG_USER);
    syslog(LOG_INFO, "Started app!\n");
    signal(SIGTERM, terminationHandler);
    signal(SIGQUIT, terminationHandler);
    signal(SIGINT, terminationHandler);

    // Create the app user data structure
    app_data_t app_data;

    // Create a subscriber instance with default data format for video channel 0
    int channel = 0;
    video_scene_subscriber_t *subscriber = video_scene_subscriber_create(channel, SCENE_PROTO_1, &app_data);

    // Set the required callback functions and some options
    video_scene_subscriber_set_on_message_arrived_callback(subscriber, on_message_arrived);
    video_scene_subscriber_set_velocity_enabled(subscriber);
    video_scene_subscriber_set_bounding_box_enabled(subscriber);
    video_scene_subscriber_set_reconnect(subscriber, true);

    // Subscribe to the video scene data
    if (video_scene_subscriber_subscribe(subscriber) < 0)
    {
        throw std::runtime_error("Failed to subscribe");
    }

    // Main loop processing the scene data queue
    while (should_run)
    {
        // Poll the queue 10 times per second to avoid busy wait
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Process any scene data in queue
        while (app_data.scene_queue.size() > 0)
        {
            std::unique_lock<std::mutex> lock(app_data.scene_queue_mutex);
            Scene scene = app_data.scene_queue.front();
            app_data.scene_queue.pop();
            lock.unlock(); // avoids blocking the queue while processing
            process_scene(scene, &app_data);
        }
        syslog(LOG_INFO, "Video scene summary: Seen %d objects total, %d/%d large, %d/%d speeding.",
               app_data.objects_total, app_data.objects_total_large, app_data.objects_total,
               app_data.objects_total_speeding, app_data.objects_total);
    }

    // Unsubscribe and delete
    video_scene_subscriber_unsubscribe(subscriber);
    video_scene_subscriber_delete(subscriber);

    return 0;
}
