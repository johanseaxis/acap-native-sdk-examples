ARG ARCH=armv7hf
ARG VERSION=3.2
ARG UBUNTU_VERSION=20.04
ARG REPO=axisecp
ARG SDK=acap-sdk

FROM ${REPO}/${SDK}:${VERSION}-${ARCH}-ubuntu${UBUNTU_VERSION}

# Install the tools needed for building the protobuf-c dependency and C code generation from .proto file
RUN apt-get update && apt-get install -y --no-install-recommends \
    autoconf automake libtool \
    protobuf-compiler \
    protobuf-c-compiler

# Copy the app source files
COPY ./app /opt/app/

# Clone and build the protobuf-c shared library dependency
WORKDIR /opt/build
ARG PROTOBUF_C_VERSION=v1.3.3
RUN git clone -n https://github.com/protobuf-c/protobuf-c.git
RUN cd protobuf-c && \
    git checkout ${PROTOBUF_C_VERSION}
WORKDIR /opt/build/protobuf-c
RUN ./autogen.sh &&\
    ./configure --host=arm-linux-gnueabihf --disable-protoc &&\
    make
# Copy the needed shared library and header files into app dir
RUN mkdir -p /opt/app/lib &&\
    mkdir -p /opt/app/include &&\
    cp protobuf-c/.libs/libprotobuf-c.so* /opt/app/lib &&\
    cp protobuf-c/protobuf-c.h /opt/app/include

# Building the ACAP application
WORKDIR /opt/app
RUN . /opt/axis/acapsdk/environment-setup* && create-package.sh
