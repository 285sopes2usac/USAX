FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
	 build-essential \
	 ca-certificates \
	 curl \
	 wget \
	 git \
	 python3 \
	 python3-pip \
	 python3-venv \
	 cmake \
	 ninja-build \
	 gcc \
	 g++ \
	 gcc-multilib \
	 g++-multilib \
	 clang \
	 lld \
	 flex \
	 bison \
	 xz-utils \
	 unzip \
	 file \
	 bc \
	 autoconf \
	 automake \
	 libtool \
	 pkg-config \
	 gettext \
	 nasm \
	 lcov \
	 libncurses-dev \
	 libssl-dev \
	 libelf-dev \
	 libgmp-dev \
 && rm -rf /var/lib/apt/lists/*

# Create an unprivileged user for builds
RUN useradd -m builder \
 && mkdir -p /work \
 && chown builder:builder /work

WORKDIR /work

ENV PATH=/home/builder/.local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

USER builder

CMD ["/bin/bash"]

