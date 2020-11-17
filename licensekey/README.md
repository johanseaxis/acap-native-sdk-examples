 *Copyright (C) 2020, Axis Communications AB, Lund, Sweden. All Rights Reserved.*

# A licenskey handler based ACAP3 application on an edge device
This README file explains how to build an ACAP3 application that uses the licensekey API. It is achieved by using the containerized Axis API and toolchain images.

Together with this README file, you should be able to find a directory called app. That directory contains the "licensekey_handler" application source code which can easily be compiled and run with the help of the tools and step by step below.

This example illustrates how to check the licensekey status. A licensekey is a signed file which has been generate for a specific device ID. Licensekey status i.e. valid or invalid is logged in the Application log.

## Getting started
These instructions will guide you on how to execute the code. Below is the structure and scripts used in the example:

```bash
vdostream
├── app
│   ├── LICENSE
│   ├── Makefile
│   ├── licensekey_handler.c
├── Dockerfile
└── README.md
```

* **Dockerfile** - Docker file with the specified Axis toolchain and API container to build the example specified.
* **app/LICENSE** - Text file which lists all open source licensed source code distributed with the application.
* **app/Makefile** - Makefile containing the build and link instructions for building the ACAP3 application.
* **app/licensekey_handler.c** - Application to check licensekey status in C.
* **README.md** - Step by step instructions on how to run the example.

### Limitations
* The example is done for the armv7hf architecture, but it is possible to update to aarch64 architecture.

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

<APP_IMAGE> is the name to tag the image with, e.g., licensekey_handler:1.0

Copy the result from the container image to a local directory build:

```bash
docker cp $(docker create <APP_IMAGE>):/opt/app ./build
```

The working dir now contains a build folder with the following files:

```bash
vdostream
├── app
│   ├── LICENSE
│   ├── Makefile
│   ├── licensekey_handler.c
├── Dockerfile
└── README.md
├── build
│   ├── LICENSE
│   ├── Makefile
│   ├── package.conf
│   ├── package.conf.orig
│   ├── param.conf
│   ├── licensekey_handler*
│   ├── licensekey_handler_1_0_0_armv7hf.eap
│   ├── licensekey_handler_1_0_0_LICENSE.txt
│   ├── licensekey_handler.c
```

* **build/package.conf** - Defines the application and its configuration.
* **build/package.conf.orig** - Defines the application and its configuration, original file.
* **build/param.conf** - File containing application parameters.
* **build/licensekey_handler*** - Application executable binary file.
* **build/licensekey_handler_1_0_0_armv7hf.eap** - Application package .eap file.
* **build/licensekey_handler_1_0_0_LICENSE.txt** - Copy of LICENSE file.

#### Install your application
Installing your application on an Axis video device is as simple as:

Browse to the following page (replace <axis_device_ip> with the IP number of your Axis video device)

```bash
http://<axis_device_ip>/#settings/apps
```

*Goto your device web page above > Click on the tab **App** in the device GUI > Add **(+)** sign and browse to
the newly built **licensekey_handler_1_0_0_armv7hf.eap** > Click **Install** > Run the application by enabling the **Start** switch*

Application will run with default video compression format h264.

#### The expected output
Application log can be found directly at:

```
http://<axis_device_ip>/axis-cgi/admin/systemlog.cgi?appname=licensekey_handler
```

or by clicking on the "**App log**" link in the device GUI.

```
----- Contents of SYSTEM_LOG for 'licensekey_handler' -----


2020-11-17T10:26:42.499+01:00 axis-accc8e691c41 [ INFO    ] licensekey_handler[0]: starting licensekey_handler
2020-11-17T10:26:42.539+01:00 axis-accc8e691c41 [ INFO    ] licensekey_handler[14660]: Licensekey is invalid
2020-11-17T10:31:43.058+01:00 axis-accc8e691c41 [ INFO    ] licensekey_handler[14660]: Licensekey is invalid
```

## License
**[Apache License 2.0](../LICENSE)**
