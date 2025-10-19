# SGX Enclave Hardware Environment Guide

A comprehensive guide for running Intel SGX enclave applications on Alibaba Cloud vSGX instances with actual SGX hardware support.

## Overview

This project provides a JWT-to-salt conversion service running inside an Intel SGX enclave on Alibaba Cloud vSGX instances. The service processes JWT tokens and returns corresponding salt information using hardware-based confidential computing.

## Prerequisites

- **Cloud Provider**: Alibaba Cloud ECS
- **Instance Type**: vSGX instances (g7t, c7t, or r7t families)
- **Operating System**: Ubuntu 22.04 UEFI (recommended)
- **Architecture**: x86_64
- **Storage**: At least 8GB free disk space

## Salt Generation Principles
![salt_server_workflow](./assets/salt_workflow.png)
The salt server plays an important part in maintaining privacy and security for users' Web2 credentials when using Kzero. Using a secret master seed and the user's JWT, the salt server produces a salt value that is unique to that user for that app, but hides the connection from the user's identity to their Polkadot activity, cryptographically ensuring privacy. The salt value is required before generating a zkLogin proof and therefore before issuing transactions onchain.

When someone uses an app backed by the Kzero salt server, they enter their Web2 credentials and the application requests a JWT from the auth provider. The app then sends the JWT to the salt server to get the salt value. Each time the Polkadot address is derived from the user's identity, the salt is used to ensure that the user's address can always deterministically be computed from their token without revealing the binding between the two.

![salt_derive](./assets/salt_derive.png)
This service implements a salt generation mechanism that keeps a master seed value and derives a user salt with key derivation by validating and parsing the JWT. For example, using `HKDF(ikm = seed, salt = iss || aud, info = sub)`(To know more about HKDF, please refer to the link [here](https://datatracker.ietf.org/doc/html/rfc5869)).
> For more details about the salt server, please check [here](https://github.com/kzero-xyz/kzero-grant-docs/blob/main/kzero-salt-service-spec.md)

## Server Setup

### Step 1: Create vSGX Instance

1. Log in to the [Alibaba Cloud ECS Console](https://ecs.console.aliyun.com/)
2. Create a new ECS instance with the following specifications:
   - **Instance Family**: g7t, c7t, or r7t (vSGX instances)
   - **Image**: Ubuntu 22.04 UEFI


### Step 2: Initialize SGX Environment

Follow the [Alibaba Cloud SGX Setup Guide](https://www.alibabacloud.com/help/en/ecs/user-guide/build-an-sgx-encrypted-computing-environment#03e6d895betxu) to initialize the SGX environment:

#### 2.1 Check SGX Status
```bash
# Install cpuid
sudo apt-get update && sudo apt-get install -y --no-install-recommends cpuid

# Check if SGX is enabled
cpuid -1 -l 0x7 |grep SGX
```

#### 2.2 Install SGX Driver & Build the SGX confidential computing environment
```bash
# Create installation script
cat <<'EOF' > install_sgx_dcap.sh
#!/bin/bash
version_id=$(cat /etc/os-release|grep "VERSION_ID"|cut -d"=" -f2|tr -d "\"")
version_codename=$(cat /etc/os-release|grep "VERSION_CODENAME"|cut -d"=" -f2)
apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential dkms curl wget
if [ ! -e /dev/sgx/enclave -a ! -e /dev/sgx_enclave ]; then
  dcap_version=$(curl -s https://download.01.org/intel-sgx/latest/version.xml |grep dcap| sed -r 's/.*>(.*)<.*/\1/')
  dcap_files=$(curl -s https://download.01.org/intel-sgx/latest/dcap-latest/linux/SHA256SUM_dcap_${dcap_version}.cfg)
  echo "${dcap_files}" | grep "ubuntu${version_id}-server" |grep "sgx_linux_x64_driver" | awk '{print $2}' | xargs -I{} curl -O -J https://download.01.org/intel-sgx/latest/dcap-latest/linux/{}
      
  bash sgx_linux_x64_driver*.bin
else
  echo "driver already installed"
fi
EOF

# Run the script
sudo bash ./install_sgx_dcap.sh

# Verify driver installation
ls -l /dev/{sgx_enclave,sgx_provision}


cat <<'EOF' > install_sgx_sdk.sh
#!/bin/bash

version_id=$(cat /etc/os-release|grep "VERSION_ID"|cut -d"=" -f2|tr -d "\"")
version_codename=$(cat /etc/os-release|grep "VERSION_CODENAME"|cut -d"=" -f2)
apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential dkms curl wget

dcap_version=$(curl -s https://download.01.org/intel-sgx/latest/version.xml |grep dcap| sed -r 's/.*>(.*)<.*/\1/')
linux_version=$(curl -s https://download.01.org/intel-sgx/latest/version.xml |grep linux| sed -r 's/.*>(.*)<.*/\1/')
dcap_files=$(curl -s https://download.01.org/intel-sgx/latest/dcap-latest/linux/SHA256SUM_dcap_${dcap_version}.cfg)
echo "${dcap_files}" | grep "ubuntu${version_id}-server" | awk '{print $2}' | xargs -I{} curl -O -J https://download.01.org/intel-sgx/latest/dcap-latest/linux/{}

# install sgx_sdk
bash sgx_linux_x64_sdk*.bin --prefix /opt/intel
source /opt/intel/sgxsdk/environment

# install psw
echo "deb [arch=amd64] https://download.01.org/intel-sgx/sgx_repo/ubuntu ${version_codename} main" |  tee /etc/apt/sources.list.d/intelsgx.list
wget -qO - https://download.01.org/intel-sgx/sgx_repo/ubuntu/intel-sgx-deb.key | apt-key add -
apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y libsgx-launch libsgx-urts libsgx-epid libsgx-quote-ex libsgx-dcap-ql libsgx-dcap-ql-dev
systemctl enable --now aesmd.service
EOF

sudo bash ./install_sgx_sdk.sh
```

#### 2.3 Install SGX SDK and Runtime
```bash
# Update package list
sudo apt update

# Install build tools
sudo apt install -y build-essential

# Install Intel SGX SDK (follow the official Intel installation guide)
# This step may vary based on the specific SGX SDK version
```

### Step 3: Install Additional Dependencies

#### 3.1 Install System Dependencies
```bash
# Update package list
sudo apt update

# Install required libraries
sudo apt install -y libcurl4-openssl-dev libjson-c-dev libjwt-dev libssl-dev libmicrohttpd-dev
```

#### 3.2 Install JWT-CPP Library
```bash
# Clone and build jwt-cpp
cd /tmp
git clone https://github.com/Thalhammer/jwt-cpp.git
cd jwt-cpp
mkdir build && cd build
cmake ..
make -j4
sudo make install

# Update library cache
sudo ldconfig
```

## Project Setup

### Step 1: Clone and Build

```bash
# Clone the project (replace with your repository URL)
git clone git@github.com:kzero-xyz/kzero-salt-enclave-service.git
cd kzero-salt-enclave-service
git checkout enclave-hw-mode
# Build the project
make COVERAGE=1 SGX_MODE=HW SGX_DEBUG=1
```

### Step 2: Run the Service

```bash
# Run the application
sudo ./app
```

The service will start on port 8080 and display startup messages.

## API Usage

### Step 3: Run the Test
```bash
# run test
sudo ./bin/test_app

# get the coverage report
sudo gcov -o tests/test_app.o App/App.cpp
```
You should see the following test result:
> Notice: In the test, we used a fixed JWK(which is pulled from google  'https://www.googleapis.com/oauth2/v3/certs' at 2025-9-13, and use a fixed JWT which is generated at 2025-9-13, to avoid JWK&JWT expired error). In the Unit Test, the Google JWK is fixed, the testing JWT is also fixed and matches the Google JWK, so the 'No matching key found for JWT' error won't be found.
```bash
=== Test Results ===
All tests PASSED!
=== Test Suite Complete ===
Generating coverage report...
File 'App/App.cpp'
Lines executed:93.49% of 568
```
### POST /get_salt

Processes JWT tokens and returns salt information.

**Request:**
```bash
curl -X POST -H "Content-Type: application/json" \
  -d '{"message": "your_jwt_token_here", "provider": "your_jwk_provider_here"}' \
  http://localhost:8080/get_salt
```

**Response:**
```json
{
  "salt": "generated_salt_value",
  "status": "success"
}
```

### Example with Google OAuth JWT

```bash
curl -s -X POST http://localhost:8080/get_salt \
  -H 'Content-Type: application/json' \
  -d '{"message":"eyJhbGciOiJSUzI1NiIsImtpZCI6IjA3ZjA3OGYyNjQ3ZThjZDAxOWM0MGRhOTU2OWU0ZjUyNDc5OTEwOTQiLCJ0eXAiOiJKV1QifQ.eyJpc3MiOiJodHRwczovL2FjY291bnRzLmdvb2dsZS5jb20iLCJhenAiOiI1NjA2MjkzNjU1MTctbXQ5ajlhcmZsY2dpMzVpOGhwb3B0cjY2cWdvMWxtZm0uYXBwcy5nb29nbGV1c2VyY29udGVudC5jb20iLCJhdWQiOiI1NjA2MjkzNjU1MTctbXQ5ajlhcmZsY2dpMzVpOGhwb3B0cjY2cWdvMWxtZm0uYXBwcy5nb29nbGV1c2VyY29udGVudC5jb20iLCJzdWIiOiIxMTExNDA0NjE1MzAyNDYxNjQ1MjYiLCJub25jZSI6InlwanZ6TXB6d09qelcycUlrVnBiQU9UTUZuVSIsIm5iZiI6MTc1Nzc1MjA2NCwiaWF0IjoxNzU3NzUyMzY0LCJleHAiOjE3NTc3NTU5NjQsImp0aSI6ImZkYzRmNTc3YWI0NWViZjhiMjU3NjkwMjQwZmUzMTYyOGFkOGI4ZmMifQ.D4NVKogzU76ZGV5HsUDTOHRwSSG1I3lgG4bUEWAeMW8G-QDnXBNY6QDFmYnVEWWx5VlejyQhvmdtJrXF2eDOMKGeOwnFlm1INQuneELbLz0sbKnDw62IKshgQGNP5jv5ij-HEKj3jkx8D1zof83duVDhFOUmDud0VZKPODfBRLbqoTJKz0cp0RwZ5k-SiT_aSeL-y_FodYcCt5VtXIZfvgWj_NbcscqPaIBMvjJ9-wFx8yD-6C5dIQDVgyhZGtLzwxRLZMr6yotBuz_49BlKquuPA6TgNdUvMRu35QRYEQYPx3RigYtKw_8GGW-LVbmZTKSBOKu8QMEweR9CCaBHvg","provider":"test_google"}'
```

## References

- [Alibaba Cloud SGX Setup Guide](https://www.alibabacloud.com/help/en/ecs/user-guide/build-an-sgx-encrypted-computing-environment)
- [Intel SGX Documentation](https://software.intel.com/content/www/us/en/develop/topics/software-guard-extensions.html)
- [JWT-CPP Library](https://github.com/Thalhammer/jwt-cpp)
- [RFC 5869 - HMAC-based Extract-and-Expand Key Derivation Function (HKDF)](https://datatracker.ietf.org/doc/html/rfc5869)

## Development Instance Info
```bash
root@iZt4n2unl8umxkfmr20i5yZ:~/kzero-salt-enclave-service# uname -a
Linux iZt4n2unl8umxkfmr20i5yZ 5.4.0-106-generic #120-Ubuntu SMP Fri Mar 18 12:42:08 UTC 2022 x86_64 x86_64 x86_64 GNU/Linux
root@iZt4n2unl8umxkfmr20i5yZ:~/kzero-salt-enclave-service# lscpu
Architecture:                    x86_64
CPU op-mode(s):                  32-bit, 64-bit
Byte Order:                      Little Endian
Address sizes:                   46 bits physical, 57 bits virtual
CPU(s):                          2
On-line CPU(s) list:             0,1
Thread(s) per core:              2
Core(s) per socket:              1
Socket(s):                       1
NUMA node(s):                    1
Vendor ID:                       GenuineIntel
CPU family:                      6
Model:                           106
Model name:                      Intel(R) Xeon(R) Platinum 8369B CPU @ 2.70GHz
Stepping:                        6
CPU MHz:                         3492.211
BogoMIPS:                        5399.99
Hypervisor vendor:               KVM
Virtualization type:             full
L1d cache:                       48 KiB
L1i cache:                       32 KiB
L2 cache:                        1.3 MiB
L3 cache:                        48 MiB
NUMA node0 CPU(s):               0,1
Vulnerability Itlb multihit:     Not affected
Vulnerability L1tf:              Not affected
Vulnerability Mds:               Not affected
Vulnerability Meltdown:          Not affected
Vulnerability Spec store bypass: Vulnerable
Vulnerability Spectre v1:        Mitigation; usercopy/swapgs barriers and __user pointer sanitizat
                                 ion
Vulnerability Spectre v2:        Mitigation; Enhanced IBRS, RSB filling
Vulnerability Srbds:             Not affected
Vulnerability Tsx async abort:   Not affected
Flags:                           fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat
                                  pse36 clflush mmx fxsr sse sse2 ss ht syscall nx pdpe1gb rdtscp 
                                 lm constant_tsc rep_good nopl nonstop_tsc cpuid aperfmperf tsc_kn
                                 own_freq pni pclmulqdq monitor ssse3 fma cx16 pcid sse4_1 sse4_2 
                                 x2apic movbe popcnt aes xsave avx f16c rdrand hypervisor lahf_lm 
                                 abm 3dnowprefetch cpuid_fault invpcid_single ibrs_enhanced fsgsba
                                 se tsc_adjust bmi1 avx2 smep bmi2 erms invpcid avx512f avx512dq r
                                 dseed adx smap avx512ifma clflushopt clwb avx512cd sha_ni avx512b
                                 w avx512vl xsaveopt xsavec xgetbv1 xsaves wbnoinvd arat avx512vbm
                                 i pku ospke avx512_vbmi2 gfni vaes vpclmulqdq avx512_vnni avx512_
                                 bitalg avx512_vpopcntdq rdpid arch_capabilities

```
---

**Note**: This service requires actual SGX hardware and should be deployed on Alibaba Cloud vSGX instances or other SGX hardware supported instances for production use.
