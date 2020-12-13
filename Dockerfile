FROM ubuntu:18.04

ENV DEBIAN_FRONTEND=noninteractive

# Install qemu riscv-gnu-toolchain
ENV RISCV=/opt/riscv
ENV PATH=$RISCV/bin:$PATH
WORKDIR $RISCV

RUN apt update
RUN apt install -y autoconf automake autotools-dev curl libmpc-dev libmpfr-dev \
  libgmp-dev gawk build-essential bison flex texinfo gperf libtool patchutils \
  bc zlib1g-dev libexpat-dev
RUN apt install -y git
RUN git clone --recursive https://github.com/riscv/riscv-gnu-toolchain
RUN apt install -y python3
RUN cd riscv-gnu-toolchain && ./configure --prefix=/opt/riscv && make linux

# Install qemu
ENV QEMU=/opt/qemu-riscv
ENV PATH=$QEMU/bin:$PATH
WORKDIR $QEMU

RUN apt install -y libglib2.0-dev libfdt-dev libpixman-1-dev zlib1g-dev wget \
  python
RUN apt install -y ninja-build
RUN wget https://download.qemu.org/qemu-5.2.0.tar.xz && \
  tar xf qemu-5.2.0.tar.xz && cd qemu-5.2.0/ && mkdir build
RUN cd qemu-5.2.0/build/ && \
  ../configure --target-list=riscv32-softmmu,riscv64-softmmu,riscv64-linux-user,riscv32-linux-user --prefix=/opt/qemu-riscv && \
  make && make install
ENV QEMU_LD_PREFIX=$RISCV/sysroot

# Additional
RUN apt install -y sudo vim gcc make binutils libc6-dev gdb

RUN adduser --disabled-password --gecos '' user
RUN echo 'user ALL=(root) NOPASSWD:ALL' > /etc/sudoers.d/user
USER user
WORKDIR /home/user
