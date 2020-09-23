# Specify the architecture at build time: armv7hf
# Should be used for getting API image
# Currently, only armv7hf is supported
ARG ARCH=armv7hf
ARG VERSION=3.1
ARG UBUNTU_VERSION=19.10

FROM axisecp/acap-api:${VERSION}-${ARCH}-ubuntu${UBUNTU_VERSION} as api
FROM axisecp/acap-toolchain:${VERSION}-${ARCH}-ubuntu${UBUNTU_VERSION} as toolchain

# Use PKG_ARCH=aarch64-poky-linux if ARCH=aarch64
ARG PKG_ARCH=cortexa9hf-neon-poky-linux-gnueabi

# Copy the lib, include and .pc files from the API container
COPY --from=api /opt/axis/sdk/temp/sysroots/${PKG_ARCH}/usr/lib/ /opt/axis/sdk/sysroots/${PKG_ARCH}/usr/lib/
COPY --from=api /opt/axis/sdk/temp/sysroots/${PKG_ARCH}/usr/include/ /opt/axis/sdk/sysroots/${PKG_ARCH}/usr/include/
COPY --from=api /opt/axis/sdk/temp/sysroots/${PKG_ARCH}/usr/lib/pkgconfig/ /opt/axis/sdk/sysroots/${PKG_ARCH}/usr/lib/pkgconfig/

# Copy the tools and scripts from the toolchain container
RUN cp -r /opt/axis/sdk/temp/* /opt/axis/sdk/

# Install the tools needed for building the protobuf-c dependency and C code generation from .proto file
RUN apt-get update && apt-get install -y --no-install-recommends \
    autoconf automake libtool \
    git \
    protobuf-compiler \
    protobuf-c-compiler

# Copy the app source files
COPY ./app /opt/app/

# Copy the .proto file from the API container
COPY --from=api /opt/axis/sdk/temp/sysroots/${PKG_ARCH}/usr/share/protobuf/video-object-detection.proto /opt/app/

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

# Building the ACAP
WORKDIR /opt/app
RUN . /opt/axis/sdk/environment-setup* && create-package.sh
