# version: v6
FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive

# Install the required system packages
RUN apt update && apt upgrade -y && \
apt install -y git-core gcc g++ gcc-multilib g++-multilib clang cmake \
python3-minimal python3-serial python3-pip python3-coverage gawk unzip \
parted wget curl qemu-system-riscv64 bison flex libssl-dev device-tree-compiler

# Copy usax's source to our container
COPY . /usax-tmp

# Setup the list of packages to download & build for all the compilers
ARG usax_PACKAGES="mtools zlib busybox dtc lcov acpica"

# -------------------------------------------------------------------------

RUN mkdir -p /tc
ARG TCROOT=/tc
ARG ARCH=riscv64

# -------------------------------------------------------------------------
ARG GCC_TC_VER=12.4.0
RUN /usax-tmp/scripts/build_toolchain && \
/usax-tmp/scripts/build_toolchain -s $usax_PACKAGES

# -------------------------------------------------------------------------
ARG GCC_TC_VER=13.3.0
RUN /usax-tmp/scripts/build_toolchain && \
/usax-tmp/scripts/build_toolchain -s $usax_PACKAGES


# Remove usax's source and the toolchain cache to save space
RUN rm -rf /usax-tmp && rm -rf /tc/cache
