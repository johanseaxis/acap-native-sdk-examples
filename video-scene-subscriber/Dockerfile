ARG ARCH=armv7hf
ARG VERSION=3.4_rc1
ARG UBUNTU_VERSION=20.04
ARG REPO=docker-prod.se.axis.com/axisecp
ARG SDK=acap-sdk

FROM ${REPO}/${SDK}:${VERSION}-${ARCH}-ubuntu${UBUNTU_VERSION}

# Building the ACAP application
COPY ./app /opt/app/
WORKDIR /opt/app
RUN . /opt/axis/acapsdk/environment-setup* && create-package.sh
