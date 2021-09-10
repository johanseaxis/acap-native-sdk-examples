# Put a proxy URL in the docker_proxy environment variable if needed
ARG DOCKER_PROXY
ARG http_proxy=$DOCKER_PROXY
ARG https_proxy=$DOCKER_PROXY

ARG ARCH=armv7hf
ARG VERSION=3.4
ARG UBUNTU_VERSION=20.04
ARG REPO=axisecp
ARG SDK=acap-sdk

FROM ${REPO}/${SDK}:${VERSION}-${ARCH}-ubuntu${UBUNTU_VERSION}

# Copy the library to application folder
WORKDIR /opt/app
COPY ./app /opt/app/

# Setup model and label directories
RUN mkdir -p model
RUN curl -o model/mobilenet_v2_1.0_224_quant.tgz http://download.tensorflow.org/models/tflite_11_05_08/mobilenet_v2_1.0_224_quant.tgz
RUN tar -xvf model/mobilenet_v2_1.0_224_quant.tgz -C model
RUN rm -f model/*.tgz  model/*.pb* model/*.ckpt* model/*.meta model/*.txt

RUN mkdir -p model
RUN curl -L -o model/mobilenet_v2_1.0_224_quant_edgetpu.tflite https://github.com/google-coral/edgetpu/raw/master/test_data/mobilenet_v2_1.0_224_quant_edgetpu.tflite

RUN mkdir -p label
RUN curl -L -o label/imagenet_labels.txt https://github.com/google-coral/edgetpu/raw/master/test_data/imagenet_labels.txt

ARG CHIP=CPU
# Building the ACAP application
RUN if [ "$CHIP" = CPU ]; then \
    . /opt/axis/acapsdk/environment-setup* && acap-build . \
    -a 'label/imagenet_labels.txt' -a 'model/mobilenet_v2_1.0_224_quant.tflite'; \
    elif [ "$CHIP" = EDGETPU ]; then \
    . /opt/axis/acapsdk/environment-setup* && acap-build . \
    -a 'label/imagenet_labels.txt' -a 'model/mobilenet_v2_1.0_224_quant_edgetpu.tflite'; \
    else \
    echo 'Error:' $CHIP 'is not a valid value for the CHIP variable'; \
    exit 1; \
    fi
