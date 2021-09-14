FROM tensorflow/tensorflow:1.15.5-gpu

ARG DEBIAN_FRONTEND=noninteractive
ARG http_proxy
ARG https_proxy

ENV HTTPS_PROXY ${https_proxy}
ENV HTTP_PROXY ${http_proxy}
ENV https_proxy ${https_proxy}
ENV http_proxy ${http_proxy}
ENV TF_FORCE_GPU_ALLOW_GROWTH true

# Get docker to be able to build ACAP inside this container
RUN apt-get update
RUN apt-get install -y apt-transport-https \
    ca-certificates \
    curl \
    software-properties-common
RUN curl -fsSL https://download.docker.com/linux/ubuntu/gpg | apt-key add -
RUN add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu bionic stable"
RUN apt-get update && apt-get install docker-ce -y

# Create skeleton
RUN mkdir -p /env/app

# Get dataset
RUN apt-get install -y wget
RUN mkdir -p /data/images && mkdir -p /data/annotations
RUN curl -o /data/images/val2017.zip http://images.cocodataset.org/zips/val2017.zip && \
    unzip -q /data/images/val2017.zip -d /data/images/ && \
    rm /data/images/val2017.zip
RUN curl -o /data/annotations_trainval2017.zip http://images.cocodataset.org/annotations/annotations_trainval2017.zip && \
    unzip -q /data/annotations_trainval2017.zip -d /data/ && \
    rm /data/annotations_trainval2017.zip

# Commands for getting the full COCO dataset for training
#RUN curl -o /data/images/train2017.zip http://images.cocodataset.org/zips/train2017.zip && \
#    unzip -q /data/images/train2017.zip -d /data/images/ && \
#    rm /data/images/train2017.zip

# Get pretrained models
WORKDIR /data
COPY models.sha512 /data/models.sha512
RUN mkdir -p /data/models

############### TODO: add models for A8
#ADD https://www.axis.com/techsup/developer_doc/acap3/examples/models.zip /data
# Temporary location for models before AWS
# cp /h/joha/models/a8/models.tar.xz /data/models.tar.xz
RUN sha512sum -c /data/models.sha512
RUN rm /data/models.sha512
RUN tar xf /data/models.tar.xz -C /data

RUN cp /data/models/converted_model.tflite /env/app/converted_model.tflite

# Get python packages
RUN pip3 install Pillow

ENV NVIDIA_VISIBLE_DEVICES all
ENV NVIDIA_DRIVER_CAPABILITIES all

WORKDIR /env
