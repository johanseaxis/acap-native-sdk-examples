 *Copyright (C) 2020, Axis Communications AB, Lund, Sweden. All Rights Reserved.*

# A video object detection subscriber based ACAP3 application on an edge device
This README file explains how to build an ACAP3 that uses the video object detection subscriber API. It is achieved by using the containerized Axis API and toolchain images.

Together with this README file you should be able to find a directory called app. That directory contains the "subscribe_to_object_detection" application source code which can easily be compiled and run with the help of the tools and step by step below.

This example illustrates how to connect to the video-object-detection service
and subscribe to object detection metadata. The object detection metadata is logged in the Application log.

To enhance this example and make better use of the API, see section [Object detection](#object-detection).

## Getting started
These instructions will guide you on how to execute the code. Below is the structure and scripts used in the example:

```bash
subscribe-to-object-detection
├── app
│   ├── LICENSE
│   ├── Makefile
│   └── subscribe_to_object_detection.c
├── Dockerfile
└── README.md
```
* **app/LICENSE** - Text file which lists all open source licensed source code distributed with the application.
* **app/Makefile** - Makefile containing the build and link instructions for building the ACAP3 application.
* **app/subscribe_to_object_detection.c** - Application to subscribe to metadata from the video-object-detection service.
* **Dockerfile** - Docker file with the specified Axis toolchain and API container to build the example specified.
* **README.md** - Step by step instructions on how to run the example.

### Limitations
* ARTPEC-7 camera with video object detection service support.
* The number of available objects to detect varies between products.

### How to run the code
Below is the step by step instructions on how to execute the program. So basically starting with the generation of the .eap file to running it on a device:

#### Build the application

> [!IMPORTANT]
> *Depending on the network you are connected to.
The file that needs those settings is: *~/.docker/config.json.*
For reference please see: https://docs.docker.com/network/proxy/ and a
[script for Axis device here](../FAQs.md#HowcanIset-upnetworkproxysettingsontheAxisdevice?).*

```bash
docker build --tag <APP_IMAGE> .
```

<APP_IMAGE> is the name to tag the image with, e.g., stod:1.0

Copy the result from the container image to a local directory build:

```bash
docker cp $(docker create <APP_IMAGE>):/opt/app ./build
```

The working directory now contains a build folder with the following files:

```bash
subscribe-to-object-detection
├── build
│   ├── generated
│   ├── include
│   ├── lib
│   ├── LICENSE
│   ├── Makefile
│   ├── package.conf
│   ├── package.conf.orig
│   ├── param.conf
│   ├── subscribe_to_object_detection*
│   ├── subscribe_to_object_detection_1_0_0_armv7hf.eap
│   ├── subscribe_to_object_detection_1_0_0_LICENSE.txt
│   ├── subscribe_to_object_detection.c
│   └── video-object-detection.proto
```
* **generated** - Folder containing the [protobuf-c](https://github.com/protobuf-c/protobuf-c) generated C code.
* **include** - Folder containing include files for libprotobuf-c.
* **lib** - Folder containing compiled library files for libprotobuf-c.
* **package.conf** - Defines the application and its configuration.
* **package.conf.orig** - Defines the application and its configuration, original file.
* **param.conf** - File containing application parameters.
* **subscribe_to_object_detection** - Application executable binary file.
* **subscribe_to_object_detection_1_0_0_LICENSE.txt** - Copy of LICENSE file.
* **subscribe_to_object_detection_1_0_0_armv7hf.eap** - Application package .eap file.
* **video-object-detection.proto** - The [protocol buffer](https://developers.google.com/protocol-buffers/docs/proto3) file describing the data format.

#### Install your application
Installing your application on an Axis video device is as simple as:

Browse to the following page (replace <axis_device_ip> with the IP number of your Axis video device)

```bash
http://<axis_device_ip>/#settings/apps
```

*Goto your device web page above > Click on the tab **App** in the device GUI > Add **(+)** sign and browse to
the newly built **subscribe_to_object_detection_1_0_0_armv7hf.eap** > Click **Install** > Run the application by enabling the **Start** switch*

#### The expected output
The video object detection (VOD) metadata is printed to the application log which is found directly at:
```
http://<axis_device_ip>/axis-cgi/admin/systemlog.cgi?appname=subscribe_to_object_detection
```

or by clicking on the "**App log**" link in the device GUI or by extracting the logs using following commands
in the terminal.
> [!IMPORTANT]
*> Please make sure SSH is enabled on the device to run the following commands.*

```bash
ssh root@<axis_device_ip>
tail -f /var/info.log | grep subscribe_to_object_detection
```
```bash
----- Contents of SYSTEM_LOG for 'subscribe_to_object_detection' -----

[ INFO ] subscribe_to_object_detection[1234]: The following object classes are available:
[ INFO ] subscribe_to_object_detection[1234]: class[0] = Vehicle, nbr_calibrations: 3
[ INFO ] subscribe_to_object_detection[1234]:   threshold: 1.00  -->  precision: 83.00, recall: 60.00
[ INFO ] subscribe_to_object_detection[1234]:   threshold: 2.00  -->  precision: 85.00, recall: 59.00
[ INFO ] subscribe_to_object_detection[1234]:   threshold: 16.00 -->  precision: 93.00, recall: 52.00
[ INFO ] subscribe_to_object_detection[1234]: class[1] = Human, nbr_calibrations: 3
[ INFO ] subscribe_to_object_detection[1234]:   threshold: 2.00  -->  precision: 70.00, recall: 60.00
[ INFO ] subscribe_to_object_detection[1234]:   threshold: 3.00  -->  precision: 71.00, recall: 60.00
[ INFO ] subscribe_to_object_detection[1234]:   threshold: 23.00 -->  precision: 88.00, recall: 47.00
[ INFO ] subscribe_to_object_detection[1234]: class [2] = Bike, nbr_calibrations: 3
[ INFO ] subscribe_to_object_detection[1234]:   threshold: 2.00  -->  precision: 84.00, recall: 57.00
[ INFO ] subscribe_to_object_detection[1234]:   threshold: 2.00  -->  precision: 84.00, recall: 57.00
[ INFO ] subscribe_to_object_detection[1234]:   threshold: 22.00 -->  precision: 95.00, recall: 49.00
[ INFO ] subscribe_to_object_detection[1234]: Threshold value set for each object class:
[ INFO ] subscribe_to_object_detection[1234]: class[0] = Vehicle, threshold=16
[ INFO ] subscribe_to_object_detection[1234]: class[1] = Human, threshold=23
[ INFO ] subscribe_to_object_detection[1234]: class[2] = Bike, threshold=22
[ INFO ] subscribe_to_object_detection[1234]: [#0] id = 0. class[0] = Vehicle. score = 45 >= 16. bbox[l,r,t,b]=[-0.236458, 0.227083, -0.179630, -0.529630].
[ INFO ] subscribe_to_object_detection[1234]: [timestamp 19567993362000]: 1/4 OK detections!
[ INFO ] subscribe_to_object_detection[1234]: [#0] id = 0. class[0] = Vehicle. score = 43 >= 16. bbox[l,r,t,b]=[-0.236458, 0.227083, -0.179630, -0.529630].
[ INFO ] subscribe_to_object_detection[1234]: [#1] id = 1. class[2] = Bike. score = 22 >= 22. bbox[l,r,t,b]=[-0.833333, -0.775000, -0.481481, -0.585185].
[ INFO ] subscribe_to_object_detection[1234]: [timestamp 19568360035000]: 2/4 OK detections!
...
```

When the application starts, for each object available to detect on your device, calibration data is printed(see more under [VOD calibration API](#vod-calibration-api)) followed by the threshold chosen(see more under [Filter on score](#filter-on-score). Then, depending on if any objects are detected in the scene and the set threshold, detection data will be printed.

The video object detection subscriber API looks for objects a number of times per second and in each run a number of objects may have been detected. If any of the detected objects have a score above or equal to the object threshold they are printed to syslog, followed by a resume with a timestamp and how many objects of the total detected that were above or equal to threshold. A closer look at a detection print:

```bash
[ INFO ] subscribe_to_object_detection[1234]: [#0] id = 0. class[0] = Vehicle. score = 43 >= 16. bbox[l,r,t,b]=[-0.236458, 0.227083, -0.179630, -0.529630].
[ INFO ] subscribe_to_object_detection[1234]: [#1] id = 1. class[2] = Bike. score = 22 >= 22. bbox[l,r,t,b]=[-0.833333, -0.775000, -0.481481, -0.585185].
[ INFO ] subscribe_to_object_detection[1234]: [timestamp 19568360035000]: 2/4 OK detections!
...
```

* **[#x]** is an index for the objects above threshold in each detection. In this detection two objects #0 and #1 are seen.
* **id** is the unique number for each object that has been detected. Not to be confused with class id.
* **class[x] = Object** is the class id x and nice name Object of a detected object, e.g. class id 0 maps to nice name Vehicle.
* **score = x >= y** shows the score x of the detection and the object threshold y
* **bbox[l,r,t,b]=[left, right, top, bottom]** shows the bounding box coordinates of the detection


## Object detection
There are different factors that affect the result of video object detection.

### Filter on score
A lot of information is available about object detection, but two basic parameters to know of are *precision* and *recall*, where 100 % precision means no false detections and 100 % recall means finding all objects. Each detection comes with a *score* that is a value of how certain it is that the detection was correct, the higher the score the better the detection.

**N.b. the score is not in percentage. In this example, only detections with a score above or equal to a *threshold* will be printed to the syslog.**

The chosen threshold for each object in the example is based on the highest value given by the calibration API. These values are hardcoded in the example code and may easily be changed to test what different thresholds means.


#### VOD calibration API
A calibration API is available on devices with VOD since firmware 9.80. The example prints all calibrations made for each object class and what a certain threshold for an object means in terms of trade-off between precision and recall.

```bash
[ INFO ] subscribe_to_object_detection[1234]: The following object classes are available:
[ INFO ] subscribe_to_object_detection[1234]: class[0] = Vehicle, nbr_calibrations: 3
[ INFO ] subscribe_to_object_detection[1234]:   threshold: 1.00  -->  precision: 83.00, recall: 60.00
[ INFO ] subscribe_to_object_detection[1234]:   threshold: 2.00  -->  precision: 85.00, recall: 59.00
[ INFO ] subscribe_to_object_detection[1234]:   threshold: 16.00 -->  precision: 93.00, recall: 52.00
[ INFO ] subscribe_to_object_detection[1234]: class[1] = Human, nbr_calibrations: 3
[ INFO ] subscribe_to_object_detection[1234]:   threshold: 2.00  -->  precision: 70.00, recall: 60.00
[ INFO ] subscribe_to_object_detection[1234]:   threshold: 3.00  -->  precision: 71.00, recall: 60.00
[ INFO ] subscribe_to_object_detection[1234]:   threshold: 23.00 -->  precision: 88.00, recall: 47.00
[ INFO ] subscribe_to_object_detection[1234]: class[2] = Bike, nbr_calibrations: 3
[ INFO ] subscribe_to_object_detection[1234]:   threshold: 2.00  -->  precision: 84.00, recall: 57.00
[ INFO ] subscribe_to_object_detection[1234]:   threshold: 2.00  -->  precision: 84.00, recall: 57.00
[ INFO ] subscribe_to_object_detection[1234]:   threshold: 22.00 -->  precision: 95.00, recall: 49.00
...
```

### Setup and position of device
Some factors that may improve the object detection:

* The position of the device
    * Mounted on a wall, seeing objects slightly from above.
    * Keep the center of the image below the horizon line.
    * Avoid a roll angle, i.e. the horizon should not be a slope.
    * A device looking down straight from above will not work very well, e.g. mounted on an indoor roof.
    * Objects must have a minimum size to be detectable, at least 4 % of image height is a good start.
* Scene conditions
    * Weather and lighting may affect performance.
    * Clear sight, only partially visible objects might not be detected.
    * If scene is very zoomed in and objects pass by very fast, object detection might be hard.

## License
**[Apache License 2.0](../LICENSE)**
