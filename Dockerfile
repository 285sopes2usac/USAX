FROM ubuntu:24.04 as BuildBuilder

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
   gawk parted qemu-system-x86 qemu-system-gui \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /work

COPY . .

RUN scripts/build_toolchain

#---------------------- Final Builder Image ----------------------
FROM ubuntu:24.04 as Builder

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
   gawk parted qemu-system-x86 qemu-system-gui \
 && rm -rf /var/lib/apt/lists/*

RUN mkdir /work
COPY --from=BuildBuilder /work/toolchain3 /work/toolchain3

ENV PATH=/home/builder/.local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

CMD ["/bin/bash"]
