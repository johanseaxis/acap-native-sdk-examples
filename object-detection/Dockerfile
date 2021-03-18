FROM tensorflow/tensorflow:latest-gpu-py3

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

# Get pretrained models
# WORKDIR /env
ADD https://github.com/google-coral/test_data/raw/master/ssd_mobilenet_v1_coco_quant_postprocess_edgetpu.tflite /env/app/
ADD https://github.com/google-coral/test_data/raw/master/ssd_mobilenet_v2_coco_quant_postprocess_edgetpu.tflite /env/app/

ARG DEBIAN_FRONTEND=noninteractive

ENV NVIDIA_VISIBLE_DEVICES all
ENV NVIDIA_DRIVER_CAPABILITIES all

# Copy the rest of the local files
ADD env/ /env/
