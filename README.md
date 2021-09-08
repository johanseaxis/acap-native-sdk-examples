# What is Axis Camera Application Platform?
[AXIS Camera Application Platform (ACAP)](https://www.axis.com/support/developer-support/axis-camera-application-platform) is an open application platform that enables members of [Axis Application Development Partner](https://www.axis.com/partners/adp-partner-program) (ADP) Program to develop applications that can be downloaded and installed on Axis network cameras and video encoders. ACAP makes it possible to develop applications for a wide range of use cases:
* Security applications that improve surveillance systems and facilitate investigation.
* Business intelligence applications that improve business efficiency.
* Camera feature plug-ins that add value beyond the Axis product's core functionality

# Prerequisites for ACAP development
ACAP is Axis own open platform for applications that run on-board an Axis product. If you are new to ACAP, start with learning more about the platform, [prerequisites](https://www.axis.com/developer-community/acap-fundamentals), [compatible architectures](https://www.axis.com/developer-community/acap-sdk) and [SDK user manual](https://help.axis.com/acap-3-developer-guide).

## Getting started with the repo
This repository contains a set of application examples which aims to enrich the
developers analytics experience. All examples are using Docker framework and has a
README file in its directory which shows overview, example directory structure and
step-by-step instructions on how to run applications on the camera.

## Example applications
Below is the list of examples available in the repository.

* [axevent](./axevent/)
  * The example code is written in C which illustrates both how to subscribe to different events and how to send an event.
* [axoverlay](./axoverlay/)
  * The example code is written in C which illustrates how to draw plain boxes and text as overlays in a stream.
* [hello-world](./hello-world/)
  * The example code is written in C and shows how to build a simple hello world application.
* [larod](./larod/)
  * The example code is written in C which connects to [larod](./FAQs.md#WhatisLarod?) and loads a model, runs inference on it and then finally deletes the loaded model from [larod](./FAQs.md#WhatisLarod?).
* [licensekey](./licensekey/)
  * The example code is written in C which illustrates how to check the licensekey status.
* [reproducible-package](./reproducible-package/)
  * An example of how to create a reproducible application package.
* [subscribe-to-object-detection](./subscribe-to-object-detection/)
  * The example code is written in C and exemplifies how to subscribe to video object detection, filter detections based on calibration data and points out ways to optimize object detection.
* [tensorflow-to-larod](./tensorflow-to-larod/)
  * This example covers model conversion, model quantization, image formats and custom models in
greater depth than the [larod](./larod)
and [vdo-larod](./vdo-larod) examples.
* [using-opencv](./using-opencv/)
  * This example covers how to build, bundle and use OpenCV with ACAP4 Native SDK.
* [vdostream](./vdostream/)
  * The example code is written in C which starts a vdo stream and then illustrates how to continuously capture frames from the vdo service, access the received buffer contents as well as the frame metadata.
* [vdo-larod](./vdo-larod/)
  * The example code is written in C and loads an image classification model to [larod](./FAQs.md#WhatisLarod?) and then uses vdo to fetch frames of size WIDTH x HEIGHT in yuv format which are converted to interleaved rgb format and then sent to larod for inference on MODEL.
* [vdo-opencl-filtering](./vdo-opencl-filtering/)
  * This example illustrates how to capture frames from the vdo service, access the received buffer, and finally perform a GPU accelerated Sobel filtering with OpenCL.
* [web-server](./web-server/)
  * The example code is written in C which runs a Monkey web server on the camera.


### DockerHub Image
The ACAP SDK image can be used as a basis for custom built images to run your application or as a developer environment inside the container. The image is public and free to use for anyone.

* [ACAP SDK](https://hub.docker.com/r/axisecp/acap-sdk) This image is based on Ubuntu and contains the environment needed for building an AXIS Camera Application Platform (ACAP) application. This includes all tools for building and packaging an ACAP 3 application as well as API components (header and library files) needed for accessing different parts of the camera firmware.

# Issues
If you encounter issues with the examples, make sure your product is running the latest firmware version or one that is compatible with the ACAP SDK used.
The examples use the ACAP SDK during the build process, of which each version is compatible with a set of firmware versions.
The specific SDK version that each example is based on is specified in the Dockerfile used to build the application, through the `VERSION` variable.
The full compatibility schema for ACAP SDK version and firmware version is available at [Find the right SDK for software compatibility](https://help.axis.com/acap-3-developer-guide#find-the-right-sdk-for-software-compatibility).

If the issue persists with a compatible firmware, please create an issue containing the information specified in the template below.

### Issue template
*Axis product/device (e.g. Q1615 Mk III):*

*Device firmware version:*

*Issue description:*

# Frequently asked questions
Please visit [FAQs page](FAQs.md) for frequently asked questions.

# License
[Apache 2.0](LICENSE)

