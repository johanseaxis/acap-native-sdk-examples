ARG ARCH=armv7hf
ARG VERSION=3.4_beta1
ARG UBUNTU_VERSION=20.04
ARG REPO=axisecp
ARG SDK=acap-sdk

FROM ${REPO}/${SDK}:${VERSION}-${ARCH}-ubuntu${UBUNTU_VERSION}

# Get the host protobuf code compiler with the same version as the target protobuf libraries
# Currently this is v3.13.0
RUN curl -L https://repo1.maven.org/maven2/com/google/protobuf/protoc/3.13.0/protoc-3.13.0-linux-x86_64.exe -o /usr/bin/protoc && chmod +x /usr/bin/protoc

# Building the ACAP application
COPY ./app /opt/app/
WORKDIR /opt/app
RUN . /opt/axis/acapsdk/environment-setup* && create-package.sh
