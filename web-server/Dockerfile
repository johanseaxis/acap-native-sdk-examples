ARG ARCH=armv7hf
ARG REPO=axisecp
ARG SDK=acap-sdk
ARG UBUNTU_VERSION=20.04
ARG VERSION=3.3

FROM ${REPO}/${SDK}:${VERSION}-${ARCH}-ubuntu${UBUNTU_VERSION}

WORKDIR /opt/monkey

# Build server and library
RUN git clone -b v1.5.6 https://github.com/monkey/monkey . &&\
    . /opt/axis/acapsdk/environment-setup* &&\
    ./configure \
        --enable-shared \
        --malloc-libc \
        --prefix=/usr/local \
        --bindir=/usr/local/bin \
        --libdir=/usr/local/lib \
        --sysconfdir=/usr/local/packages/quiz/html \
        --datadir=/usr/local/packages/quiz/html \
        --mandir=/usr/local/man \
        --logdir=/tmp \
        --plugdir=/usr/local/packages/quiz/lib \
        --pidfile=/tmp/monkey.pid \
        --incdir=/usr/local/include/monkey \
        --systemddir=/usr/lib/systemd/system &&\
    make &&\
    make install

WORKDIR /opt/app
COPY ./app .

# Build examples
RUN . /opt/axis/acapsdk/environment-setup* &&\
    mkdir -p $SDKTARGETSYSROOT/usr/local &&\
    cp -r /usr/local $SDKTARGETSYSROOT/usr &&\
    mkdir -p lib &&\
    cp /usr/local/lib/libmonkey.so.1.5 /usr/local/packages/quiz/lib/*.so -t lib/ &&\
    cp -r /usr/local/packages/quiz/html . &&\
    acap-build ./
