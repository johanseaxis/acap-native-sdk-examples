# Put a proxy URL in the docker_proxy environment variable if needed
ARG DOCKER_PROXY
ARG http_proxy=$DOCKER_PROXY
ARG https_proxy=$DOCKER_PROXY

# Specify the architecture at build time: armv7hf
# Should be used for getting API and toolchain image
# Currently, only armv7hf is supported
ARG ARCH=armv7hf
ARG VERSION=rel-fw-10.4-maint
ARG UBUNTU_VERSION=20.04
ARG REPO=docker-prod.se.axis.com/axisecp
ARG SDK=acap-sdk

FROM ${REPO}/${SDK}:${VERSION}-${ARCH}-ubuntu${UBUNTU_VERSION}

# Copy the library to application folder
WORKDIR /opt/app
COPY ./app /opt/app/

# Building the ACAP application
RUN . /opt/axis/acapsdk/environment-setup* && create-package.sh
