# Specify the architecture at build time: armv7hf/aarch64
# Should be used for getting API image
# Currently, only armv7hf is supported
ARG ARCH=armv7hf
ARG VERSION=master
ARG UBUNTU_VERSION=19.10
ARG REPO=docker-prod.se.axis.com/axisecp

FROM ${REPO}/acap-api:${VERSION}-${ARCH}-ubuntu${UBUNTU_VERSION} as api
FROM ${REPO}/acap-toolchain:${VERSION}-${ARCH}-ubuntu${UBUNTU_VERSION} as toolchain

# Use PKG_ARCH=aarch64-poky-linux if ARCH=aarch64
ARG PKG_ARCH=cortexa9hf-neon-poky-linux-gnueabi

# Copy the lib, include, .proto and .pc files from the API container
COPY --from=api /opt/axis/sdk/temp/sysroots/${PKG_ARCH}/usr/lib/ /opt/axis/sdk/sysroots/${PKG_ARCH}/usr/lib/
COPY --from=api /opt/axis/sdk/temp/sysroots/${PKG_ARCH}/usr/include/ /opt/axis/sdk/sysroots/${PKG_ARCH}/usr/include/
COPY --from=api /opt/axis/sdk/temp/sysroots/${PKG_ARCH}/usr/lib/pkgconfig/ /opt/axis/sdk/sysroots/${PKG_ARCH}/usr/lib/pkgconfig/
COPY --from=api /opt/axis/sdk/temp/sysroots/${PKG_ARCH}/usr/share/protobuf/ /opt/axis/sdk/sysroots/${PKG_ARCH}/usr/share/protobuf/

# Copy the tools and scripts from the toolchain container
RUN cp -r /opt/axis/sdk/temp/* /opt/axis/sdk/

# Get the host protobuf code compiler with the same version as the target protobuf libraries
# Currently this is v3.11.4
RUN apt-get update && apt-get install -y --no-install-recommends curl
RUN curl -L https://repo1.maven.org/maven2/com/google/protobuf/protoc/3.11.4/protoc-3.11.4-linux-x86_64.exe -o /usr/bin/protoc && chmod +x /usr/bin/protoc

# Building the ACAP
COPY app/ /opt/app
WORKDIR /opt/app
RUN . /opt/axis/sdk/environment-setup* && create-package.sh
