# SGX Enclave Docker Environment Guide

A comprehensive guide for running Intel SGX enclave applications in a simulated SGX environment (SIM mode) using Docker.

## Overview

This project provides a Docker-based environment for developing and testing Intel SGX enclave applications. It uses the SGX simulation mode, which allows you to develop and test SGX applications without requiring actual SGX hardware.

## Prerequisites

- **Operating System**: Ubuntu 20.04+ (recommended)
- **Architecture**: x86_64
- **Docker**: Version 20.10+ 
- **Storage**: At least 8GB free disk space

## Quick Start

### 1. Build the Docker Image
> **Note:** Build time depends on your machine's performance. For reference, on a 4-CPU Intel(R) Xeon(R) CPU E5-2680 v2 @ 2.80GHz (CPU MHz: 2792.998), the build process takes approximately 50 minutes.

```bash
docker build -t test-enclave:latest .
```

### 2. Run the Container

```bash
docker run -d -p 8080:8080 --name test-enclave-new -e SGX_MODE=SIM test-enclave:latest
```

### 3. Test the API

```bash
curl -X POST -H "Content-Type: application/json" -d '{"message": "eyJhbGciOiJSUzI1NiIsImtpZCI6IjJkN2VkMzM4YzBmMTQ1N2IyMTRhMjc0YjVlMGU2NjdiNDRhNDJkZGUiLCJ0eXAiOiJKV1QifQ.eyJpc3MiOiJodHRwczovL2FjY291bnRzLmdvb2dsZS5jb20iLCJhenAiOiI1NjA2MjkzNjU1MTctbXQ5ajlhcmZsY2dpMzVpOGhwb3B0cjY2cWdvMWxtZm0uYXBwcy5nb29nbGV1c2VyY29udGVudC5jb20iLCJhdWQiOiI1NjA2MjkzNjU1MTctbXQ5ajlhcmZsY2dpMzVpOGhwb3B0cjY2cWdvMWxtZm0uYXBwcy5nb29nbGV1c2VyY29udGVudC5jb20iLCJzdWIiOiIxMDI2ODcyOTA4OTIwOTUyNDQwNTciLCJub25jZSI6IjJBM0dOTjllZFdjOXhpdWlFTFowZE9VMVJOYyIsIm5iZiI6MTc1NzAwNjA3MCwiaWF0IjoxNzU3MDA2MzcwLCJleHAiOjE3NTcwMDk5NzAsImp0aSI6IjVkZWJmNTgxMjY2ZTVhYTBhNDFkMmMwZWNhZWQwOTlmZTc4MGNjNjMifQ.o65jbFRLTaU2jukoWfX9fh_EMJMrDXp-6GYNueHCzXKno2a_7L0e46ASnA4B9_2p0FU2un-doS9jB4ajR12fx9xaWOHPUYbSJzETXl0S7T73L_1IsaK8YWlW4LiRmjHtK3t2QujKYjH4S8UCEsMNEoRLYvESPO48uK4tARLN3hlopocm3Flap7SqNjF9FER1Hiect75qwWlpncKVZC0xba-NGrKAN6Jm16iJ9zTeoWHZkdHLUtnE-bmRm0Lqtvj2Lp9-zb7vWEvPWLvfHxSGXZ3bbrik0VO8seR2XXZ0DsEGtq7q3i8cJWHYfipUKRuGXSuKhAANS6cm0iYVNVhhCg"}' http://localhost:8080/get_salt
```

## API Endpoints

### POST /get_salt
Processes JWT tokens and returns salt information.

**Request Body:**
```json
{
  "message": "your_jwt_token_here"
}
```

## Environment Variables

| Variable | Description | Default | Required |
|----------|-------------|---------|----------|
| `SGX_MODE` | SGX operation mode | `SIM` | Yes |

### Logs

View container logs:
```bash
docker logs test-enclave-new
```

---

**Note**: This environment is designed for development and testing purposes. For production deployments, ensure proper security configurations and consider using actual SGX hardware when available.
