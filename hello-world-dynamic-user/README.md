 *Copyright (C) 2021, Axis Communications AB, Lund, Sweden. All Rights Reserved.*

# A hello-world ACAP3 application using manifest
This README file explains how to build an ACAP3 application that uses the axoverlay API. It is achieved by using the containerized Axis API and toolchain images.

Together with this README file, you should be able to find a directory called app. That directory contains the "axoverlay" application source code which can easily be compiled and run with the help of the tools and step by step below.

This example illustrates how to draw overlays in a video stream and Cairo is used as rendering API, see [documentation](https://www.cairographics.org/). In this example two plain boxes in different colors and one overlay text are drawn.

It is preferable to use Palette color space for large overlays like plain boxes, to lower the memory usage.
More detailed overlays like text overlays, should instead use ARGB32 color space.

Different stream resolutions are logged in the Application log.

## Getting started
These instructions will guide you on how to execute the code. Below is the structure and scripts used in the example:

```bash
hello_world_dynamic_user
├── app
│   ├── hello_world_dynamic_user.c
│   ├── LICENSE
│   └── Makefile
├── Dockerfile
└── README.md
```

* **app/hello_world_dynamic_user.c** - Hello World application which writes to system-log.
* **app/LICENSE** - Text file which lists all open source licensed source code distributed with the application.
* **app/Makefile** - Makefile containing the build and link instructions for building the ACAP3 application.
* **Dockerfile** - Docker file with the specified Axis toolchain and API container to build the example specified.
* **README.md** - Step by step instructions on how to run the example.

### Limitations
* ARTPEC-7 and ARTPEC-6 based devices.

### How to run the code
Below is the step by step instructions on how to execute the program. So basically starting with the generation of the .eap file to running it on a device:

#### Build the application
Standing in your working directory run the following commands:

> [!IMPORTANT]
> *Depending on the network you are connected to,
The file that needs those settings is: *~/.docker/config.json.*
For reference please see: https://docs.docker.com/network/proxy/ and a
[script for Axis device here](../FAQs.md#HowcanIset-upnetworkproxysettingsontheAxisdevice?).*

```bash
docker build --tag <APP_IMAGE> .
```

<APP_IMAGE> is the name to tag the image with, e.g., hello_world_dynamic_user:1.0

Copy the result from the container image to a local directory build:

```bash
docker cp $(docker create <APP_IMAGE>):/opt/app ./build
```

The working dir now contains a build folder with the following files:

```bash
axoverlay
├── app
│   ├── hello_world_dynamic_user.c
│   ├── LICENSE
│   └── Makefile
├── build
│   ├── hello_world_dynamic_user*
│   ├── hello_world_dynamic_user_1_0_0_armv7hf.eap
│   ├── hello_world_dynamic_user_1_0_0_LICENSE.txt
│   ├── hello_world_dynamic_user.c
│   ├── LICENSE
│   ├── Makefile
│   ├── package.conf
│   ├── package.conf.orig
│   └── param.conf
├── Dockerfile
└── README.md
```

* **build/hello_world_dynamic_user*** - Application executable binary file.
* **build/hello_world_dynamic_user_1_0_0_armv7hf.eap** - Application package .eap file.
* **build/hello_world_dynamic_user_1_0_0_LICENSE.txt** - Copy of LICENSE file.
* **build/package.conf** - Defines the application and its configuration.
* **build/package.conf.orig** - Defines the application and its configuration, original file.
* **build/param.conf** - File containing application parameters.

#### Install your application
Installing your application on an Axis video device is as simple as:

Browse to the following page (replace <axis_device_ip> with the IP number of your Axis video device)

```bash
http://<axis_device_ip>/#settings/apps
```

*Goto your device web page above > Click on the tab **App** in the device GUI > Add **(+)** sign and browse to
the newly built **hello_world_dynamic_user_1_0_0_armv7hf.eap** > Click **Install** > Run the application by enabling the **Start** switch*

#### The expected output
Application log can be found directly at:

```
http://<axis_device_ip>/axis-cgi/admin/systemlog.cgi?appname=hello_world_dynamic_user
```

or by clicking on the "**App log**" link in the device GUI or by extracting the logs using following commands in the terminal.

>[!IMPORTANT]
*> Please make sure SSH is enabled on the device to run the following commands.*

```bash
tail -f /var/log/info.log | grep hello_world_dynamic_user
```

```
----- Contents of SYSTEM_LOG for 'hello_world_dynamic_user' -----

14:13:07.412 [ INFO ] hello_world_dynamic_user[6425]: Hello World!

```

## License
**[Apache License 2.0](../LICENSE)**
