larod opencv example
====================

The idea of this example is to use larod to run preprocessing + inference on
a video stream from opencv.

## Todo

- [x] Display a video stream using opencv.
- [x] Configure video stream to be in YUV-format.
- [ ] Implement YUYV convertion in larod.
- [ ] Run preprocessing YUYV -> RGB + crop-scale.
- [ ] Run inference.
- [ ] Create a cv::Mat of the final RGB-image and display it with opencv.
- [ ] Do any postprocessing depending on the NN model.

## Video formats

The example retrieves a video stream, using opencv, which it requests to be in
YUYV format. If opencv uses the Video4Linux backend, the available formats on
your system can be listed with
```
    v4l2-ctl -d0 --list-formats-ext
```

## Build and run in docker

Build the docker image (including Google EdgeTPU USB support):
```
docker build -t larod-opencv-edgetpu --build-arg USE_TFLITE_EDGETPU=true .
```

Allow any user to connect to the X server:
```
    xhost +
```

Run docker image:
```
    docker run --volume <path to larod>:/larod \
        --volume /run/dbus/system_bus_socket:/run/dbus/system_bus_socket \
        --volume /tmp/.X11-unix:/tmp/.X11-unix \
        --device=/dev/video0 \
        --env DISPLAY=$DISPLAY \
        --privileged --rm -it larod
```
If one wants to use a USB EdgeTPU, one needs to mount in the relevant USB device
as well with `-v /dev:/dev`.

Build larod and run example with:
```
    cd larod
    meson build -Duse_tflite_cpu=true -Duse_tflite_tpu=true
    cd build
    ninja
    ./examples/opencv/larod-opencv-example
```
