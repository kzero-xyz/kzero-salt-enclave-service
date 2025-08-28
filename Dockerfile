# Stage 1: Base Environment
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
    python3 \
    python3-pip \
    python-is-python3 \
    ocaml \
    ocamlbuild \
    automake \
    autoconf \
    libtool \
    unzip \
    && rm -rf /var/lib/apt/lists/*

# Stage 2: Intel SGX SDK Build
FROM base AS sgx-builder

# Install Rust nightly version (using 2024-10-17 version)
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --default-toolchain nightly-2024-10-17
ENV PATH="/root/.cargo/bin:${PATH}"

# Install rust-src component (required by xargo)
RUN rustup component add rust-src

# Install xargo master branch (using clone and build method)
RUN git clone --depth 1 --branch master https://github.com/japaric/xargo.git /opt/xargo && \
    cd /opt/xargo && \
    cargo build --release && \
    cp target/release/xargo /root/.cargo/bin/ && \
    rm -rf /opt/xargo

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

# Stage 3: Application Compilation
FROM sgx-builder AS app

# Set working directory to project root
WORKDIR /opt/test-enclave

# Copy project files
COPY . .

# Clone Teaclave SGX SDK
RUN cd .. && \
    git clone --depth 1 --branch main https://github.com/2nado/incubator-teaclave-sgx-sdk.git && \
    mv incubator-teaclave-sgx-sdk teaclave-sgx-sdk

# Create necessary directory structure
RUN mkdir -p ../../edl ../../common/inc && \
    cp buildenv.mk ../../buildenv.mk

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
RUN XARGO_SGX=1 make

# Expose port
EXPOSE 8080

# Start command
CMD ["./bin/app"]
