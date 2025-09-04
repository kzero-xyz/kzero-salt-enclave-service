# Base Environment
FROM ubuntu:22.04 AS base

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    wget \
    curl \
    git \
    pkg-config \
    libssl-dev \
    libcurl4-openssl-dev \
    ca-certificates \
    cmake \
    libjson-c-dev \
    libjwt-dev \
    libmicrohttpd-dev \
    python3 \
    python3-pip \
    python-is-python3 \
    ocaml \
    ocamlbuild \
    automake \
    autoconf \
    libtool \
    unzip \
    protobuf-compiler \
    libprotobuf-dev \
    && rm -rf /var/lib/apt/lists/*

FROM base AS sgx-builder

WORKDIR /opt
RUN git clone https://github.com/Thalhammer/jwt-cpp.git
WORKDIR /opt/jwt-cpp
RUN mkdir build && cd build && cmake .. && make -j4
RUN cd build && make install


# Clone and build Intel SGX SDK 2.25 version
WORKDIR /opt
RUN git clone --depth 1 --branch sgx_2.25 https://github.com/intel/linux-sgx.git intel-sgx


# Build Intel SGX SDK
WORKDIR /opt/intel-sgx
RUN make preparation
RUN make sdk
RUN make sdk_install_pkg

# Install Intel SGX SDK
WORKDIR /opt/intel-sgx/linux/installer/bin
RUN echo "yes" | ./sgx_linux_x64_sdk_*.bin --prefix=/opt/intel


# Application Compilation
FROM sgx-builder AS app

# Set working directory to project root
WORKDIR /opt/test-enclave

# Copy project files
COPY . .

# Set Intel SGX SDK environment variables
ENV SGX_SDK=/opt/intel/sgxsdk
ENV PATH=/opt/intel/sgxsdk/bin/x64:$PATH
ENV LD_LIBRARY_PATH=/opt/intel/sgxsdk/lib64:$LD_LIBRARY_PATH

# Set SGX simulator mode
ENV SGX_MODE=SIM

# Automatically set SGX simulator symlinks
RUN cd /opt/intel/sgxsdk/lib64 && \
   rm -f libsgx_urts.so.2 && \
   ln -s libsgx_urts_sim.so libsgx_urts.so.2

# Build entire project using XARGO_SGX=1 make
RUN make

# Expose port
EXPOSE 8080

# Start command
CMD ["./bin/app"]
