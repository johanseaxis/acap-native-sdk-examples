*Copyright (C) 2020, Axis Communications AB, Lund, Sweden. All Rights Reserved.*

# From Tensorflow model to larod inference on camera
## Overview
This model is modified based on [tensorflow-to-larod](https://github.com/AxisCommunications/acap3-examples-staging/tree/master/tensorflow-to-larod) for object detection.

## Prerequisites
- Axis camera equipped with an [Edge TPU](https://coral.ai/docs/edgetpu/faq/)
- [Docker](https://docs.docker.com/get-docker/)

## Quickstart
The following instructions can be executed to simply run the example.

1. Compile the ACAP:
```sh
./build_acap.sh tensorflow_to_larod_acap:1.0
```
2. Find the ACAP `.eap` file 
```sh
/env/build/tensorflow_to_larod_app_1_0_0_armv7hf.eap
```
3. Install and start the ACAP on your camera through the GUI

4. SSH to the camera

5. View its log to see the ACAP output:
```sh
tail -f /var/volatile/log/info.log | grep tensorflow_to_larod
```

## Output
There are four outputs from MobileNet SSD v1(2) (COCO) model, and the Number of detections, CLasses, Scores, and Locations are shown as below. The location are in the form of [top, left, bottom, right]. 

```sh
[ INFO    ] tensorflow_to_larod[645]: There are 3 objects
[ INFO    ] tensorflow_to_larod[645]: Object 1: Classes: car - Scores: 0.769531 - Locations: [0.750146,0.086451,0.894765,0.299347]
[ INFO    ] tensorflow_to_larod[645]: Object 2: Classes: car - Scores: 0.335938 - Locations: [0.005453,0.101417,0.045346,0.144171]
[ INFO    ] tensorflow_to_larod[645]: Object 3: Classes: car - Scores: 0.308594 - Locations: [0.109673,0.005128,0.162298,0.050947]
```
## License
**[Apache License 2.0](../LICENSE)**


