 *Copyright (C) 2021, Axis Communications AB, Lund, Sweden. All Rights Reserved.*

# A hello-world ACAP application using manifest
This README file explains how to build a simple Hello World manifest ACAP application. It is achieved by using the containerized API and toolchain images.

Together with this README file, you should be able to find a directory called app. That directory contains the "hello-world" application source code which can easily be compiled and run with the help of the tools and step by step below.

## Getting started
These instructions will guide you on how to execute the code. Below is the structure and scripts used in the example:

```bash
web-server
├── app
│   ├── LICENSE
│   ├── Makefile
│   └── manifest.json
├── Dockerfile
└── README.md
```

* **app/LICENSE** - Text file which lists all open source licensed source code distributed with the application.
* **app/Makefile** - Makefile containing the build and link instructions for building the ACAP application.
* **app/manifest.json** - Defines the application and its configuration.
* **Dockerfile** - Docker file with the specified Axis toolchain and API container to build the example specified.
* **README.md** - Step by step instructions on how to run the example.

### How to run the code
Below is the step by step instructions on how to execute the program. So basically starting with the generation of the .eap file to running it on a device:

#### Build the application
Standing in your working directory run the following commands:

> [!IMPORTANT]
> *Depending on the network you are connected to,
The file that needs those settings is: *~/.docker/config.json.*
For reference please see: https://docs.docker.com/network/proxy/ and a
[script for Axis device here](../FAQs.md#HowcanIset-upnetworkproxysettingsontheAxisdevice?).*

## Instructions
### Quick start
1. Run the `build.sh` script with a container name as argument:
```sh
./build.sh monkey
```

Default architecture is **armv7hf**. To build for **aarch64** it's possible to
update the *ARCH* variable in the Dockerfile or to set it in the docker build
command via build argument:
```bash
docker build . --build-arg ARCH=armv7hf --build-arg http_proxy --build-arg https_proxy -t monkey
```

Copy the result from the container image to a local directory build:

```bash
docker cp $(docker create monkey):/opt/monkey/examples ./build
```

The working dir now contains a build folder with the following files:

```bash
monkey
├── app
│   ├── LICENSE
│   ├── Makefile
│   └── manifest.json
├── build
│   ├── monkey*
│   ├── monkey_1_0_0_armv7hf.eap
│   ├── monkey_1_0_0_LICENSE.txt
│   ├── LICENSE
│   ├── Makefile
│   ├── manifest.json
│   ├── package.conf
│   ├── package.conf.orig
│   └── param.conf
├── Dockerfile
└── README.md
```

* **build/monkey*** - Application executable binary file.
* **build/monkey_1_0_0_armv7hf.eap** - Application package .eap file.
* **build/monkey_1_0_0_LICENSE.txt** - Copy of LICENSE file.
* **build/manifest.json** - Defines the application and its configuration.
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
the newly built **monkey_1_0_0_armv7hf.eap** > Click **Install** > Run the application by enabling the **Start** switch*

#### The expected output
Application log can be found directly at:

```
http://<axis_device_ip>/axis-cgi/admin/systemlog.cgi?appname=hello_world
```

or by clicking on the "**App log**" link in the device GUI or by extracting the logs using following commands in the terminal.

>[!IMPORTANT]
*> Please make sure SSH is enabled on the device to run the following commands.*

```bash
tail -f /var/log/info.log | grep hello_world
```

```
----- Contents of SYSTEM_LOG for 'hello_world' -----

14:13:07.412 [ INFO ] hello_world[6425]: Hello World!

```

## License
**[Apache License 2.0](../LICENSE)**











# How to use Web Server in ACAP 4
This document explains briefly how to build and use [Monkey Web Server](https://github.com/monkey/monkey) in ACAP4. Monkey is a fast and lightweight Web Server for Linux. It has been designed to be very scalable with low memory and CPU consumption, the perfect solution for Embedded Linux and high end production environments. Besides the common features as HTTP server, it expose a flexible C API which aims to behave as a fully HTTP development framework, so it can be extended as desired through the plugins interface. The Monkey Web Server [documentation](http://monkey-project.com/documentation/1.5) describes the configuration in detail.

## Structure of this application
Below is the structure of the application with a brief description of its files.
```sh
web-server
├── Dockerfile - Specifications on how to build the camera docker image
└── README.md - Instructions on how to build and run the application
```

## Prerequisites
The following items are required to run this example:
* Camera: Q1615-MkIII
* ACAP4 Docker version 19.03.5 or higher
* Camera Firmware: 10.2

## Configure Camera Apache Web Server to forward web requests
The Web Server can be accessed from a Web Browser eighter directly using a port number (i.e. http://mycamera:8080) or through the Apache Server in the camera using an extension to the camera web URL (i.e http://mycamera/monkey). To configure the Apache Server as a Reverse Proxy Server, use the procedure shown below.
```sh
# Do ssh login to the camera
ssh root@<CAMERA_IP>

# Add Reverse Proxy configuration to the Apache Server, example:
cat >> /etc/apache2/httpd.conf <<EOF
ProxyPass "/monkey/demo" "http://localhost:2001"
ProxyPassReverse "/monkey/demo" "http://localhost:2001"
ProxyPass "/monkey" "http://localhost:8080"
ProxyPassReverse "/monkey" "http://localhost:8080"
EOF

# Restart the Apache Server
systemctl restart httpd
```

## Build and run the Web Server
Start by building the image containing the Web Server code with examples. This will compile the code to an executable and create an armv7hf container containing the executable, which can be uploaded to and run on the camera. After the Web Server is started it can be accessed from a web browser by specifying the web address: http://mycamera/monkey or http://mycamera:8080
```sh
# Set your camera IP address
export AXIS_TARGET_IP=<actual camera IP address>

# To allow retrieval of the image from the cloud
# this should be a repository that you can push to
# and that your camera can pull from, i.e., substitute
# axisecp for your own repository
export APP=monkey

docker build . --build-arg http_proxy --build-arg https_proxy -t $APP

```

### The expected output
Running the application on the camera should output information similar to what's shown below.
```sh
Monkey HTTP Server v1.5.6
Built : Jun 18 2021 11:05:42 (arm-linux-gnueabihf-gcc -mthumb -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a9 9.3.0)
Home  : http://monkey-project.com
[+] Process ID is 6
[+] Server socket listening on Port 80
[+] 2 threads, 253 client connections per thread, total 506
[+] Transport layer by liana in http mode
[+] Linux Features: TCP_FASTOPEN SO_REUSEPORT
```

## C API Examples
Some C API examples are included in the Web Server container that has been built. The commands below show how to run the examples on the camera. To see the result, use a web browser and web address: http://mycamera/monkey/demo or http://mycamera:2001
```sh
# Run the hello example
docker -H tcp://$AXIS_TARGET_IP run --rm -p 2001:2001 -it $APP hello

# Run the list directory example
docker -H tcp://$AXIS_TARGET_IP run --rm -p 2001:2001 -it $APP list

# Run the quiz example
docker -H tcp://$AXIS_TARGET_IP run --rm -p 2001:2001 -it $APP quiz
```


## Proxy settings
Depending on the network, you might need proxy settings in the following file: *~/.docker/config.json.*

For reference please see: https://docs.docker.com/network/proxy/.

*Proxy settings can also be added to the edge device:*
```sh
ssh root@<CAMERA_IP>
```
