FROM acap3-sdk:latest

ARG VERSION=3.2
ARG PKG_ARCH=cortexa9hf-neon-poky-linux-gnueabi
ARG SYSROOT=/opt/axis/acapsdk/sysroots/cortexa9hf-neon-poky-linux-gnueabi
ARG libyuv_version=5b6042fa0d211ebbd8b477c7f3855977c7973048
ARG ARCH=armhf

   # TODO: Investigate why server certs can't be verified
WORKDIR /opt/build

RUN GIT_SSL_NO_VERIFY=1 git clone -n https://chromium.googlesource.com/libyuv/libyuv

WORKDIR /opt/build/libyuv

RUN  git checkout ${libyuv_version}

COPY yuv/*.patch /opt/build/libyuv

RUN git apply *.patch
 
 RUN CXXFLAGS=' -O2 -mthumb -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a9 -fomit-frame-pointer' \
    make -f linux.mk CXX=arm-linux-gnueabihf-g++ CC=arm-linux-gnueabihf-gcc

RUN arm-linux-gnueabihf-strip --strip-unneeded libyuv.so*

RUN cp libyuv.so* ${SYSROOT}/usr/lib/ &&\
    cp -a include/. ${SYSROOT}/usr/include &&\
    cd ${SYSROOT}/usr/lib/ &&\
    ln -s libyuv.so.1 libyuv.so &&\
    ln -s libyuv.so.1 libyuv.so.1.0

COPY ./app /opt/app/
WORKDIR /opt/app

# Creating package
RUN . /opt/axis/acapsdk/environment-setup* && create-package.sh