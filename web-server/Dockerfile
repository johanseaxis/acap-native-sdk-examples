ARG http_proxy
ARG https_proxy
ARG ARCH=armv7hf
ARG REPO=axisecp
ARG SDK=acap-sdk
ARG UBUNTU_VERSION=20.04
ARG VERSION=3.3

FROM ${REPO}/${SDK}:${VERSION}-${ARCH}-ubuntu${UBUNTU_VERSION}

# Add a sources.list file that contains the armhf repositories
COPY sources.list /etc/apt
RUN apt-get update

ENV DEBIAN_FRONTEND=noninteractive

# Get crosscompilation toolchain
RUN dpkg --add-architecture armhf
RUN apt-get update && apt-get install -yf --no-install-recommends \
        ca-certificates \
        crossbuild-essential-armhf \
        cmake \
        curl \
        pkg-config \
 && update-ca-certificates \
 && apt-get clean

WORKDIR /opt/monkey

# Build server and library
RUN git clone -b v1.5.6 https://github.com/monkey/monkey . &&\
    . /opt/axis/acapsdk/environment-setup* &&\
    ./configure \
        --enable-shared \
        --malloc-libc \
        --prefix=/usr/local \
        --plugdir=/usr/local/lib \
        --pidfile=/usr/local/logs/monkey.pid \
        --incdir=/usr/local/include/monkey \
        --systemddir=/usr/lib/systemd/system &&\
    make &&\
    make install

WORKDIR /opt/monkey/examples
COPY ./app .

# Build examples
RUN sed -i 's/LDFLAGS/LDLIBS/g' Makefile
RUN cp /opt/axis/acapsdk/sysroots/x86_64-pokysdk-linux/usr/bin/create-package.sh .
RUN . /opt/axis/acapsdk/environment-setup* &&\
    mkdir -p $SDKTARGETSYSROOT/usr/local &&\
    cp -r /usr/local $SDKTARGETSYSROOT/usr &&\
    mkdir -p lib &&\
    cp /usr/local/lib/libmonkey.so.1.5 /usr/local/lib/*.so -t lib/ &&\
    create-package.sh
