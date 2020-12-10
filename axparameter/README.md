 *Copyright (C) 2020, Axis Communications AB, Lund, Sweden. All Rights Reserved.*

# A parameter handler based ACAP3 application on an edge device
This README file explains how to build an ACAP3 application that uses the axparameter API. It is achieved by using the containerized Axis API and toolchain images.

Together with this README file, you should be able to find a directory called app. That directory contains the application source code which can easily be compiled and run with the help of the tools and step by step below.

This example illustrates how to handle application defined parameters. It is possible to add, remove, set and get parameters for an application as well as register callback functions when a parameter value is updated.

This example have three parameters "ParameterOne", "ParameterTwo" and "StaticParam1" which are visible in device GUI.
It is also possible to have hidden parameters, in this case "HiddenParam1".

## Getting started
These instructions will guide you on how to execute the code. Below is the structure and scripts used in the example:

```bash
axparameter
├── app
│   ├── axparameter.c
│   ├── LICENSE
│   ├── Makefile
|   └── param.conf
├── Dockerfile
└── README.md
```

* **app/axparameter.c** - Application to defines parameters in C.
* **app/LICENSE** - Text file which lists all open source licensed source code distributed with the application.
* **app/Makefile** - Makefile containing the build and link instructions for building the ACAP3 application.
* **app/param.conf** - File containing additional application parameters.
* **Dockerfile** - Docker file with the specified Axis toolchain and API container to build the example specified.
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

<APP_IMAGE> is the name to tag the image with, e.g., axparameter:1.0

Copy the result from the container image to a local directory build:

```bash
docker cp $(docker create <APP_IMAGE>):/opt/app ./build
```

The working dir now contains a build folder with the following files:

```bash
axparameter
├── app
│   ├── axparameter.c
│   ├── LICENSE
│   ├── Makefile
|   └── param.conf
├── build
│   ├── axparameter*
│   ├── axparameter_1_0_0_armv7hf.eap
│   ├── axparameter_1_0_0_LICENSE.txt
│   ├── axparameter.c
│   ├── LICENSE
│   ├── Makefile
│   ├── package.conf
│   ├── package.conf.orig
│   └── param.conf
├── Dockerfile
└── README.md
```

* **build/axparameter*** - Application executable binary file.
* **build/axparameter_1_0_0_armv7hf.eap** - Application package .eap file.
* **build/axparameter_1_0_0_LICENSE.txt** - Copy of LICENSE file.
* **build/package.conf** - Defines the application and its configuration.
* **build/package.conf.orig** - Defines the application and its configuration, original file.

#### Install your application
Installing your application on an Axis video device is as simple as:

Browse to the following page (replace <axis_device_ip> with the IP number of your Axis video device)

```bash
http://<axis_device_ip>/#settings/apps
```

*Goto your device web page above > Click on the tab **Apps** in the device GUI > Add **(+)** sign and browse to
the newly built **axparameter_1_0_0_armv7hf.eap** > Click **Install** > Run the application by enabling the **Start** switch*

#### The expected output
Application log can be found directly at:

```
http://<axis_device_ip>/axis-cgi/admin/systemlog.cgi?appname=axparameter
```

or by clicking on the "**App log**" link in the device GUI.

The log contains a list of available parameters, both visible and hidden, which have been defined in **app/axparameter.c** and **app/param.conf**.

```
----- Contents of SYSTEM_LOG for 'axparameter' -----
17:05:16.849 [ INFO ] axparameter[0]: starting axparameter
17:05:16.930 [ INFO ] axparameter[2602]: The value of "ParameterOne" is "param_default"
17:05:16.932 [ INFO ] axparameter[2602]: The value of "ParameterTwo" is "param_two"
17:05:17.087 [ INFO ] axparameter[2602]: Parameter in list: "StaticParam1"
17:05:17.092 [ INFO ] axparameter[2602]: Parameter in list: "HiddenParam1"
17:05:17.092 [ INFO ] axparameter[2602]: Parameter in list: "ParameterOne"
17:05:17.092 [ INFO ] axparameter[2602]: Parameter in list: "ParameterTwo"
17:05:17.092 [ INFO ] axparameter[2602]: The value of "ParameterOne" is "param_default" or "param_one"
17:05:26.918 [ INFO ] axparameter[2602]: The value of "ParameterOne" is "param_default" or "param_one"
```

In this example, a parameter callback function is called if the parameters "ParameterOne" or "ParameterTwo" changes. Every 10th second another callback function is called, which checks the values of the parameters.

Update "**Parameter one**" to "param_one" in the in the device GUI. This will log "ParameterOne". The parameter callback function is called twice for both "ParameterOne" and "ParameterTwo", since this is the first time a parameter value is updated.

```
----- Contents of SYSTEM_LOG for 'axparameter' -----
17:06:00.424 [ INFO ] axparameter[2602]: In callback, value of "ParameterOne" is "param_one"
17:06:00.426 [ INFO ] axparameter[2602]: In callback, value of "ParameterTwo" is "param_two"
17:06:06.911 [ INFO ] axparameter[2602]: The value of "ParameterOne" is "param_default" or "param_one"
17:06:16.919 [ INFO ] axparameter[2602]: The value of "ParameterOne" is "param_default" or "param_one"
```

Update "**Parameter one**" again to "something" in the in the device GUI. This will instead log "ParameterTwo".

```
----- Contents of SYSTEM_LOG for 'axparameter' -----
17:06:40.277 [ INFO ] axparameter[2602]: In callback, value of "ParameterOne" is "something"
17:06:46.910 [ INFO ] axparameter[2602]: The value of "ParameterTwo" is "param_two"
17:06:56.918 [ INFO ] axparameter[2602]: The value of "ParameterTwo" is "param_two"
```

Parameter values are preserved if the application stopped and then started, which is visible by disabling and enabling the **Start** switch* in the device GUI.

## License
**[Apache License 2.0](../LICENSE)**
