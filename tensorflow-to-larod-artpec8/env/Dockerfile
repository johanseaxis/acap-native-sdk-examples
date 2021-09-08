ARG http_proxy
ARG https_proxy
ARG ARCH=aarch64
ARG VERSION=1.0_beta1
ARG UBUNTU_VERSION=20.04
ARG REPO=axisecp
ARG SDK=acap-native-sdk

FROM ${REPO}/${SDK}:${VERSION}-${ARCH}-ubuntu${UBUNTU_VERSION}
WORKDIR /opt/app
COPY ./app .
RUN . /opt/axis/acapsdk/environment-setup* && create-package.sh
