ARG ARCH=armv7hf
ARG VERSION=3.4
ARG UBUNTU_VERSION=20.04
ARG REPO=axisecp
ARG SDK=acap-sdk

FROM ${REPO}/${SDK}:${VERSION}-${ARCH}-ubuntu${UBUNTU_VERSION}
# Building the ACAP application
COPY ./app /opt/app/
WORKDIR /opt/app
RUN . /opt/axis/acapsdk/environment-setup* && acap-build . \
    -a 'input/veiltail-11457_640_RGB_224x224.bin' \
    -a 'model/mobilenet_v1_1.0_224_quant.larod' \
    -a 'model/labels_mobilenet_quant_v1_224.txt'