# Specify the architecture at build time: armv7hf
# Should be used for getting API and toolchain image
# Currently, only armv7hf is supported
ARG http_proxy
ARG https_proxy
ARG ARCH=armv7hf
ARG VERSION=3.2
ARG UBUNTU_VERSION=19.10
ARG REPO=axisecp

FROM builder-yuv:1.0-ubuntu${UBUNTU_VERSION} as yuv
FROM ${REPO}/acap-sdk:${VERSION}-${ARCH}-ubuntu${UBUNTU_VERSION}

# Copy needed files from the yuv container
ARG PKG_ARCH=cortexa9hf-neon-poky-linux-gnueabi
COPY --from=yuv /yuv/lib /opt/axis/acapsdk/sysroots/${PKG_ARCH}/usr/lib/
COPY --from=yuv /yuv/include /opt/axis/acapsdk/sysroots/${PKG_ARCH}/usr/include/

# Building the ACAP application
COPY ./app /opt/app/
WORKDIR /opt/app
RUN . /opt/axis/acapsdk/environment-setup* && create-package.sh
