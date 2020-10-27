 *Copyright (C) 2020, Axis Communications AB, Lund, Sweden. All Rights Reserved.*

# A video scene subscriber based ACAP3 application on an edge device
This README file explains how to build an ACAP3 that uses the video scene subscriber API. It is achieved by using the containerized Axis API and toolchain images.

Together with this README file you should be able to find a directory called app, that directory contains the "video_scene_subscriber_client" application source code which can easily be compiled and run with the help of the tools and step by step below.

This example illustrates how to connect to the video scene provider service and subscribe to motion based object tracking metadata called video scene data. Some statistics computed from the the scene metadata is logged in the Application log.

## Getting started
These instructions will guide you on how to execute the code. Below is the structure and scripts used in the example:

```bash
video-scene-subscriber
├── app
│   ├── LICENSE
│   ├── Makefile
│   ├── video_scene_subscriber_client.cpp
├── Dockerfile
└── README.md
```
* **app/LICENSE** - Text file which lists all open source licensed source code distributed with the application.
* **app/Makefile** - Makefile containing the build and link instructions for building the ACAP3 application.
* **app/video_scene_subscriber_client.cpp** - Application to subscribe to metadata from the video scene provider service.
* **Dockerfile** - Docker file with the specified Axis toolchain and API container to build the example specified.
* **README.md** - Step by step instructions on how to run the example.

### Limitations
* An Axis network camera with video scene provider support is required. Basically any Axis camera besides the Companion line are supported. 

### How to run the code
Below is the step by step instructions on how to execute the program. So basically starting with the generation of the .eap file to running it on a device:

#### Build the application

> [!IMPORTANT]
> *Depending on the network you are connected to.
The file that needs those settings is: *~/.docker/config.json.*
For reference please see: https://docs.docker.com/network/proxy/ and a
[script for Axis device here](../FAQs.md#HowcanIset-upnetworkproxysettingsontheAxisdevice?).*

Standing in your working directory run the following commands.
First build the docker container image and compile the ACAP:
```
docker build --tag <APP_IMAGE> .
```

<APP_IMAGE> is the name to tag the image with, e.g., `vss_example`.

Then copy the result from the container image to a local directory build:
```
docker cp $(docker create  <APP_IMAGE>):/opt/app/ build
```

The working directory now contains a build folder with the following files:

```bash
video-scene-subscriber
├── build
│   ├── generated
│   ├── lib
│   ├── package.conf
│   ├── package.conf.orig
│   ├── param.conf
│   ├── video_scene_subscriber_client*
│   ├── video_scene_subscriber_client_1_0_0_armv7hf.eap
│   ├── video_scene_subscriber_client_1_0_0_LICENSE.txt
|   ├── video_scene_subscriber_client.cpp
```
* **generated** - Folder containing the [protobuf](https://developers.google.com/protocol-buffers/docs/cpptutorial) generated C++ code.
* **lib** - Folder containing the libraries to be bundled in this ACAP .eap file. These are copied from the API container image.
* **package.conf** - Defines the application and its configuration.
* **package.conf.orig** - Defines the application and its configuration, original file.
* **param.conf** - File containing application parameters.
* **video_scene_subscriber_client** - Application executable binary file.
* **video_scene_subscriber_client_1_0_0_LICENSE.txt** - Copy of LICENSE file.
* **video_scene_subscriber_client_1_0_0_armv7hf.eap** - Application package .eap file.
* **video_scene_subscriber_client.cpp** - Application to subscribe to metadata from the video scene provider service.

#### Install your application
Installing your application on an Axis video device is as simple as:

Browse to the following page (replace <axis_device_ip> with the IP number of your Axis video device)

```bash
http://<axis_device_ip>/#settings/apps
```

*Goto your device web page above > Click on the tab **App** in the device GUI > Add **(+)** sign and browse to
the newly built **video_scene_subscriber_client_1_0_0_armv7hf.eap** > Click **Install** > Run the application by enabling the **Start** switch*

#### The expected output
The statistics computed from the video scene subscriber metadata is printed to the application log which is found directly at:
```
http://<axis_device_ip>/axis-cgi/admin/systemlog.cgi?appname=video_scene_subscriber_client
```

or by clicking on the "**App log**" link in the device GUI.
The statistics in this example are counting the number of objects and filtering on size and velocity and counting the number of objects which are "large" and "speeding".

## License
**[Apache License 2.0](./LICENSE)**
