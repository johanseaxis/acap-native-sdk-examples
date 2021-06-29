ARG ARCH=armv7hf
ARG VERSION=3.4
ARG UBUNTU_VERSION=20.04
ARG REPO=axisecp
ARG SDK=acap-sdk

FROM ${REPO}/${SDK}:${VERSION}-${ARCH}-ubuntu${UBUNTU_VERSION}

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
	autoconf \
	libtool \
	automake

ENV ARCHDIR=arm-linux-gnueabihf

WORKDIR /opt/build
RUN git clone https://github.com/curl/curl.git
WORKDIR /opt/build/curl

# Curl lib generate
RUN autoreconf -fi
RUN ./configure --prefix=${PWD}/build --host=arm-linux-gnueabihf --without-ssl && make \
       && make install

# Delete manual directory from curl built result
RUN rm -rf /opt/build/curl/build/share/man

# Copy the library to application folder
ARG BUILDDIR=/opt/build/curl
COPY ./app /opt/app/
WORKDIR /opt/app
RUN mkdir -p lib && \
	cp -r ${BUILDDIR}/build lib

# Building the ACAP application
RUN . /opt/axis/acapsdk/environment-setup* && create-package.sh
