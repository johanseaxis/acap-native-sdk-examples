# Specify the architecture at build time: aarch64
# Should be used for getting API and toolchain images
# Currently, only aarch64 is supported
ARG ARCH=aarch64
ARG VERSION=master
ARG UBUNTU_VERSION=19.10
ARG REPO=docker-prod.se.axis.com/axisecp

FROM ${REPO}/acap-api:${VERSION}-${ARCH}-ubuntu${UBUNTU_VERSION} as api
FROM ${REPO}/acap-toolchain:${VERSION}-${ARCH}-ubuntu${UBUNTU_VERSION} as toolchain

ARG PKG_ARCH=aarch64-poky-linux

# Copy the lib, include and .pc files from the API container
COPY --from=api /opt/axis/sdk/temp/sysroots/${PKG_ARCH}/usr/lib/ /opt/axis/sdk/sysroots/${PKG_ARCH}/usr/lib/
COPY --from=api /opt/axis/sdk/temp/sysroots/${PKG_ARCH}/usr/include/ /opt/axis/sdk/sysroots/${PKG_ARCH}/usr/include/
COPY --from=api /opt/axis/sdk/temp/sysroots/${PKG_ARCH}/usr/lib/pkgconfig/ /opt/axis/sdk/sysroots/${PKG_ARCH}/usr/lib/pkgconfig/

# Copy the tools and scripts from the toolchain container
RUN cp -r /opt/axis/sdk/temp/* /opt/axis/sdk/

COPY app/ /build/src/
WORKDIR /build/src
RUN . /opt/axis/sdk/environment-setup* && create-package.sh
