 *Copyright (C) 2021, Axis Communications AB, Lund, Sweden. All Rights Reserved.*

# A combined vdo stream and larod based ACAP3 application running inference on an edge device
This README file explains how to build an ACAP3 application that uses:
- vdo to fetch frames from e.g. a camera
- larod API to load a graph model and run preprocessing and classification inferences

It is achieved by using the containerized Axis API and toolchain docker images.

Together with this README file you should be able to find a directory called app. That directory contains the "vdo_larod_preprocessing" application source code, which can easily be compiled and run with the help of the tools and step by step below.

## Detailed outline of example application
This application opens a client to vdo and starts fetching frames (in a new thread) in the yuv format. It tries to find the smallest VDO stream resolution that fits the WIDTH and HEIGHT required by the neural network. The thread fetching frames is written so that it always tries to provide a frame as new as possible even if not all previous frames have been processed by larod.

This is a simple example found in vdo_larod_preprocessing.c that utilizes VDO, larod and larod-vdo-utils libs to:
1. Fetch image data from VDO.
2. Preprocess the images (crop, scale and color convert) using larod with
   either the libyuv or VProc backend (depending on platform).
3. Run MobileNetV2 inferences with the preprocessing output as input on a larod
   backend specified by a command line argument.
4. Sort the classification scores output by the MobileNetV2 inferences and
   print the top scores along with corresponding indices and [ImageNet][1]
   labels to stdout.

It will run steps 1-4 `NUM_ROUNDS` (defaulted to 5) times.

## Which backends and models are supported?

Unless you modify the app to your own needs you should only use a MobileNetV2
model that takes 224x224 RGB images as input, and that outputs an array of
1001 classification scores of type UINT8 or FLOAT32.

You can run the example with any inference backend as long as you can provide it
with a model as described above.

## Getting started
These instructions will guide you on how to execute the code. Below is the structure and scripts used in the example:

```bash
vdo-larod-preprocessing
├── app
│   ├── imagenet-labels.h
│   ├── larod-vdo-utils.c
│   ├── larod-vdo-utils.h
│   ├── LICENSE
│   ├── Makefile
│   ├── package.conf.cpu
│   ├── package.conf.edgetpu
│   └── vdo_larod_preprocessing.c
├── Dockerfile
└── README.md
```

* **app/imagenet-labels.h** - List of model labels.
* **app/larod-vdo-utils.h/.c** - Larod VDO support files.
* **app/LICENSE** - Text file which lists all open source licensed source code distributed with the application.
* **app/Makefile** - Makefile containing the build and link instructions for building the ACAP3 application.
* **app/package.conf.cpu** - Defines the application and its configuration when building for CPU with TensorFlow Lite.
* **app/package.conf.edgetpu** - Defines the application and its configuration when building chip and model for Google TPU.
* **app/vdo_larod_preprocessing.c** - Application using larod, written in C.
* **Dockerfile** - Docker file with the specified Axis toolchain and API container to build the example specified.
* **README.md** - Step by step instructions on how to run the example.

## Limitations
* ARTPEC-7 based device with edge TPU.
* This application was not written to optimize performance.
* MobileNet is good for classification, but it requires that the object you want to classify should cover almost all the frame.

## How to run the code
Below is the step by step instructions on how to execute the program. So basically starting with the generation of the .eap file to running it on a device:

## Build the application

> [!IMPORTANT]
> *Depending on the network you are connected to,
The file that needs those settings is: *~/.docker/config.json.*
For reference please see: https://docs.docker.com/network/proxy/ and a
[script for Axis device here](../FAQs.md#HowcanIset-upnetworkproxysettingsontheAxisdevice?).*

Depending on selected chip, different model can be used for running larod. Label file is used for identifying objects in the video stream.

Model and label files are downloaded from https://coral.ai/models/, when building the application.

Which model that is used is configured through attributes in package.conf:
- APPOPTS, which contains the application command line options.
- OTHERFILES, shows files to be included in the package e.g. model. Files listed here are copied to the application directory during installation.
- PACKAGENAME, a user friendly package name which is also part of the .eap file name.

Different devices support different chips and models.

Select one of the chip alternatives, CPU or Google TPU, to build an application for that chip:

## Alternative Chip 2 - CPU with TensorFlow Lite
Standing in your working directory run the following command, to copy configuration for CPU with TensorFlow Lite:

```
cp app/package.conf.cpu app/package.conf
```

## Alternative Chip 4 - Google TPU
Standing in your working directory run the following command, to copy configuration for Google TPU:

```
cp app/package.conf.edgetpu app/package.conf
```

## Build steps
```bash
docker build --tag <APP_IMAGE> .
```

<APP_IMAGE> is the name to tag the image with, e.g., vdo_larod_preprocessing:1.0

Copy the result from the container image to a local directory build:

```bash
docker cp $(docker create <APP_IMAGE>):/opt/app ./build
```

The working dir now also contains a build folder with the following content:

```bash
vdo_larod_preprocessing
├── build
│   ├── label
|   │   └── imagenet_labels.txt
│   ├── model
|   │   ├── mobilenet_v2_1.9_224_quant_edgetpu.larod
|   │   └── mobilenet_v2_1.9_224_quant.larod
│   ├── imagenet-labels.conf
│   ├── larod-vdo-utils.c
│   ├── larod-vod-utils.h
│   ├── LICENSE
│   ├── Makefile
│   ├── package.conf
│   ├── package.conf.cpu
│   ├── package.conf.edgetpu
│   ├── package.conf.orig
│   ├── param.conf
│   ├── vdo_larod_preprocessing
│   ├── vdo_larod_preprocessing_cpu_1_0_0_armv7hf.eap / vdo_larod_preprocessing_edgetpu_1_0_0_armv7hf.eap
│   └── vdo_larod_preprocessing.c
```

Explanation of some files in build folder:
* **build/label/imagenet_labels.txt** - Label file for MobileNet V2 (ImageNet).
* **build/model/mobilenet_v2_1.9_224_quant_edgetpu.larod** - Model file for MobileNet V2 (ImageNet), used for Google TPU.
* **build/model/mobilenet_v2_1.9_224_quant.larod** - Model file for MobileNet V2 (ImageNet), used for CPU with TensorFlow Lite.
* **build/vdo_larod_preprocessing_cpu_1_0_0_armv7hf.eap** - Application package .eap file,
  if alternative chip 2 has been built.
* **build/vdo_larod_preprocessing_edgetpu_1_0_0_armv7hf.eap** - Application package .eap file,
  if alternative chip 4 has been built.

## Install your application
Installing your application on an Axis video device is as simple as:

Browse to the following page (replace <axis_device_ip> with the IP number of your Axis video device)

```bash
http://<axis_device_ip>/#settings/apps
```

*Go to your device web page above > Click on the tab **App** in the device GUI > Add **(+)** sign and browse to
the newly built **vdo_larod_preprocessing_cpu_1_0_0_armv7hf.eap** or **vdo_larod_preprocessing_edgetpu_1_0_0_armv7hf.eap** > Click **Install** > Run the application by enabling the **Start** switch*

Application vdo_larod_preprocessing is now available as an application on the device,
using the friendly name "vdo_larod_proprocessing_cpu" or "vdo_larod_preprocessing_edgetpu".

## The expected output
Application log can be found directly at:

```
http://<axis_device_ip>/axis-cgi/admin/systemlog.cgi?appname=vdo_larod
```

or by clicking on the "**App log**" link in the device GUI or by extracting the logs using following commands
in the terminal.
> [!IMPORTANT]
*> Please make sure SSH is enabled on the device to run the
following commands.*

```
ssh root@<axis_device_ip>
cd /var/log/
head -50 info.log
```

Depending on selected chip, different output is received. The label file is used for identifying objects.

## Output Alternative Chip 2 - CPU with TensorFlow Lite

```
----- Contents of SYSTEM_LOG for 'vdo_larod' -----

vdo_larod[4840]: Starting /usr/local/packages/vdo_larod/vdo_larod with larod preprocessing
vdo_larod[4840]: Creating VDO image provider and creating stream 320 x 240
vdo_larod[4840]: chooseStreamResolution: We select stream w/h=320 x 240 based on VDO channel info.
vdo_larod[4840]: Dump of vdo stream settings map =====
vdo_larod[4840]: Calculate crop image
vdo_larod[4840]: Create larod models
vdo_larod[4840]: Create preprocessing maps
vdo_larod[4840]: Crop VDO image X=40 Y=0 (240 x 240)
vdo_larod[4840]: Setting up larod connection with chip 2 and model /usr/local/packages/vdo_larod/model/mobilenet_v2_1.0_224_quant.larod
vdo_larod[4840]: Available chip ids:
vdo_larod[4840]: 2: CPU with TensorFlow Lite
vdo_larod[4840]: 4: Google TPU
vdo_larod[4840]: 11: CPU with libyuv
vdo_larod[4840]: Allocate memory for input/output buffers
vdo_larod[4840]: Create input/output tensors
vdo_larod[4840]: Determine tensor buffer sizes
vdo_larod[4840]: Connect tensors to file descriptors
vdo_larod[4840]: Create job requests
vdo_larod[4840]: Start fetching video frames from VDO
vdo_larod[4840]: createAndMapTmpFile: Setting up a temp fd with pattern /tmp/larod.in.test-XXXXXX and size 150528
vdo_larod[4840]: createAndMapTmpFile: Setting up a temp fd with pattern /tmp/larod.out.test-XXXXXX and size 1001
vdo_larod[4840]: createAndMapTmpFile: Setting up a temp fd with pattern /tmp/larod.pp.test-XXXXXX and size 115200
vdo_larod[4840]: Converted image in 5 ms
vdo_larod[4840]: Ran inference for 369 ms
vdo_larod[4840]: Top result:  566  freight car with score 58.40%
vdo_larod[4840]: Converted image in 4 ms
vdo_larod[4840]: Ran inference for 348 ms
vdo_larod[4840]: Top result:  566  freight car with score 58.40%
vdo_larod[4840]: Converted image in 4 ms
vdo_larod[4840]: Ran inference for 330 ms
vdo_larod[4840]: Top result:  566  freight car with score 59.20%
```

## Output Alternative Chip 4 - Google TPU

```
----- Contents of SYSTEM_LOG for 'vdo_larod' -----

vdo_larod[3440]: Starting /usr/local/packages/vdo_larod/vdo_larod with larod preprocessing
vdo_larod[3440]: Creating VDO image provider and creating stream 320 x 240
vdo_larod[3440]: chooseStreamResolution: We select stream w/h=320 x 240 based on VDO channel info.
vdo_larod[3440]: Dump of vdo stream settings map =====
vdo_larod[3440]: Calculate crop image
vdo_larod[3440]: Create larod models
vdo_larod[3440]: Create preprocessing maps
vdo_larod[3440]: Crop VDO image X=40 Y=0 (240 x 240)
vdo_larod[3440]: Setting up larod connection with chip 4 and model /usr/local/packages/vdo_larod/model/mobilenet_v2_1.0_224_quant_edgetpu.larod
vdo_larod[3440]: Available chip ids:
vdo_larod[3440]: 11: CPU with libyuv
vdo_larod[3440]: 2: CPU with TensorFlow Lite
vdo_larod[3440]: 4: Google TPU
vdo_larod[3440]: Allocate memory for input/output buffers
vdo_larod[3440]: Connect tensors to file descriptors
vdo_larod[3440]: Create input/output tensors
vdo_larod[3440]: Create job requests
vdo_larod[3440]: Determine tensor buffer sizes
vdo_larod[3440]: Start fetching video frames from VDO
vdo_larod[3440]: createAndMapTmpFile: Setting up a temp fd with pattern /tmp/larod.in.test-XXXXXX and size 150528
vdo_larod[3440]: createAndMapTmpFile: Setting up a temp fd with pattern /tmp/larod.out.test-XXXXXX and size 1001
vdo_larod[3440]: createAndMapTmpFile: Setting up a temp fd with pattern /tmp/larod.pp.test-XXXXXX and size 115200
vdo_larod[3440]: Converted image in 5 ms
vdo_larod[3440]: Ran inference for 11 ms
vdo_larod[3440]: Top result:  566  freight car with score 32.40%
vdo_larod[3440]: Converted image in 10 ms
vdo_larod[3440]: Ran inference for 5 ms
vdo_larod[3440]: Top result:  566  freight car with score 31.60%
vdo_larod[3440]: Converted image in 4 ms
vdo_larod[3440]: Ran inference for 5 ms
vdo_larod[3440]: Top result:  566  freight car with score 26.80%
```

## A note on performance

Buffers are allocated and tracked by VDO and larod. As such they will
automatically be handled as efficiently as possible. The libyuv backend will
map each buffer once and never copy. The VProc backend and any inference
backend that supports dma-bufs will use that to achieve both zero copy and zero
mapping. Inference backends not supporting dma-bufs will map each buffer once
and never copy just like libyuv. It should also be mentioned that the input
tensors of the inference model will be used as output tensors for the
preprocessing model to avoid copying data.

The application however does no pipelining of preprocessing and inferences, but
uses the synchronous liblarod API call `larodRunJob()` in the interest of
simplicity. One could implement pipelining using `larodRunJobAsync()` and thus
improve performance, but with some added complexity to the program.

## Conclusion
- This is an example of test data, which is dependent on selected device and chip.
- Running inference is much faster on chip Google TPU than CPU with TensorFlow Lite.
- Converting images takes almost the same time on both chips.
- Objects with score less than 60% are generally not good enough to be used as classification results.

## License
**[Apache License 2.0](../LICENSE)**

[1]: http://www.image-net.org/
