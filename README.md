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
make TEST_MODE=1
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
sudo ./app --test
```
You should see the following test result:
> Notice: In the test, we used a fixed JWK(which is pulled from google  'https://www.googleapis.com/oauth2/v3/certs' at 2025-9-13, and use a fixed JWT which is generated at 2025-9-13, to avoid JWK&JWT expired error)
```bash
=== Running Unit Tests ===

=== Testing get_provider_type ===
PASS: get_provider_type tests

=== Testing get_provider_config ===
PASS: get_provider_config tests

=== Testing get_jwt_error_message ===
PASS: get_jwt_error_message tests

=== Testing base64url_decode ===
PASS: base64url_decode tests

=== Testing JWT Decode ===
[TEST] Decoding JWT token...
[TEST] JWT Header - kid: 07f078f2647e8cd019c40da9569e4f5247991094, alg: RS256, typ: JWT
[TEST] JWT Payload information:
  iss: https://accounts.google.com
  sub: 111140461530246164526
[TEST] Converting JWK to PEM format...
[TEST] JWK converted to PEM successfully
[TEST] Verifying JWT with manual JWK...
[TEST] JWT signature verification successful!
[TEST] Testing custom JWKS structure...
[TEST] Found matching key in custom JWKS: kid=07f078f2647e8cd019c40da9569e4f5247991094, alg=RS256, kty=RSA
PASS: JWT decode tests

=== Test Results ===
All tests PASSED!
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
curl -X POST http://localhost:8080/get_salt   -H "Content-Type: application/json"   -d '{"message": "eyJhbGciOiJSUzI1NiIsImtpZCI6IjJkN2VkMzM4YzBmMTQ1N2IyMTRhMjc0YjVlMGU2NjdiNDRhNDJkZGUiLCJ0eXAiOiJKV1QifQ.eyJpc3MiOiJodHRwczovL2FjY291bnRzLmdvb2dsZS5jb20iLCJhenAiOiI1NjA2MjkzNjU1MTctbXQ5ajlhcmZsY2dpMzVpOGhwb3B0cjY2cWdvMWxtZm0uYXBwcy5nb29nbGV1c2VyY29udGVudC5jb20iLCJhdWQiOiI1NjA2MjkzNjU1MTctbXQ5ajlhcmZsY2dpMzVpOGhwb3B0cjY2cWdvMWxtZm0uYXBwcy5nb29nbGV1c2VyY29udGVudC5jb20iLCJzdWIiOiIxMDI2ODcyOTA4OTIwOTUyNDQwNTciLCJub25jZSI6IkhTcXdzb3k4a1Nwb3Q5LWNrRVVGUGItTGRHMCIsIm5iZiI6MTc1NzA3NjE2NiwiaWF0IjoxNzU3MDc2NDY2LCJleHAiOjE3NTcwODAwNjYsImp0aSI6ImNjZjgyM2FkYjlmNjBjMGFjZDNiNmFlMmFmZWQxMjkwNGRmNTdkZjMifQ.deCKin6mHw47yQ64YT_GZ74baXuOqSFdMhumgjjL2zKNO01P4HACW313a4eLpjEqSml2gFt1XR_StxU-wXCN6etMbGy-4rT88LZ9P5XqRhTexNwLZiY8r38N5mwakWrZYAfr2-jwW8eZ2AfIj8oI8iOqfhWmT-aSmpSGOnBcYqmo2rwhPM8PR-9ZSC3rRTbOhJJ0pkkB9JRGCRQa4dgIlIfbin7QIA4MzTqWsu7DikztaiDUqsnWF-MoUuaj1zuKAE-oT7Vg9fvRQbth-7N5WE6ZAlPlJE7LfCyAT6-2tsaCP_zK7s3X4eppj9Zzr-ZQFSOY_T2baoIV4R9-CisraA","provider": "google"}'
```

## References

- [Alibaba Cloud SGX Setup Guide](https://www.alibabacloud.com/help/en/ecs/user-guide/build-an-sgx-encrypted-computing-environment)
- [Intel SGX Documentation](https://software.intel.com/content/www/us/en/develop/topics/software-guard-extensions.html)
- [JWT-CPP Library](https://github.com/Thalhammer/jwt-cpp)

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
