FROM ubuntu:25.04

# 1. Non-interactive install, and add necessary repositories and tools
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    software-properties-common \
    build-essential \
    cmake \
    git \
    libtool \
    autoconf \
    automake \
    clang \
    iproute2 \
    iputils-ping \
RUN apt install -y libboost-filesystem-dev libboost-thread-dev libboost-regex-dev libtool
RUN mkdir delayed-resharing
WORKDIR /delayed-resharing/
COPY . .
RUN mkdir thirdparty
RUN bash setup_libote.sh
WORKDIR /delayed-resharing/delayedresharing
RUN bash build.sh
WORKDIR /delayed-resharing/ShareAssigner
RUN bash build.sh
WORKDIR /delayed-resharing/Protocols
RUN bash build.sh
WORKDIR /delayed-resharing
CMD ["/bin/bash"]
