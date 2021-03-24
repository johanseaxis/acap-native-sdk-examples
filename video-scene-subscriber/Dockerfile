# Specify the architecture at build time: armv7hf/aarch64
# Should be used for getting API image
# Currently, only armv7hf is supported
ARG ARCH=armv7hf
ARG VERSION=3.2
ARG UBUNTU_VERSION=20.04
ARG REPO=axisecp
ARG SDK=acap-sdk

FROM ${REPO}/${SDK}:${VERSION}-${ARCH}-ubuntu${UBUNTU_VERSION}

# Get the host protobuf code compiler with the same version as the target protobuf libraries
# Currently this is v3.11.4
RUN curl -L https://repo1.maven.org/maven2/com/google/protobuf/protoc/3.11.4/protoc-3.11.4-linux-x86_64.exe -o /usr/bin/protoc && chmod +x /usr/bin/protoc

# Building the ACAP application
COPY ./app /opt/app/
WORKDIR /opt/app
RUN . /opt/axis/acapsdk/environment-setup* && create-package.sh
