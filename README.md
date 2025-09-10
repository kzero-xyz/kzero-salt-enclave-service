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
- You can build it locally via this command, but we recommend you to directly pulling our docker online.
```bash
docker build -t test-enclave:latest .
```
- Pulling the docker online:
```bash
docker pull kzeroxyz/kzero-salt-enclave-service:v0.1.0
```

### 2. Run the Container
- If you build locally, run this command:
```bash
docker run -d -p 8080:8080 --name test-enclave-new -e SGX_MODE=SIM test-enclave:latest
```
- If you pull online, run this command:
```bash
docker run -d -p 8080:8080 --name test-enclave-new -e SGX_MODE=SIM kzeroxyz/kzero-salt-enclave-service:v0.1.0
```

### 3. Test the API

```bash
curl -X POST http://localhost:8080/get_salt   -H "Content-Type: application/json"   -d '{"message": "eyJhbGciOiJSUzI1NiIsImtpZCI6IjJkN2VkMzM4YzBmMTQ1N2IyMTRhMjc0YjVlMGU2NjdiNDRhNDJkZGUiLCJ0eXAiOiJKV1QifQ.eyJpc3MiOiJodHRwczovL2FjY291bnRzLmdvb2dsZS5jb20iLCJhenAiOiI1NjA2MjkzNjU1MTctbXQ5ajlhcmZsY2dpMzVpOGhwb3B0cjY2cWdvMWxtZm0uYXBwcy5nb29nbGV1c2VyY29udGVudC5jb20iLCJhdWQiOiI1NjA2MjkzNjU1MTctbXQ5ajlhcmZsY2dpMzVpOGhwb3B0cjY2cWdvMWxtZm0uYXBwcy5nb29nbGV1c2VyY29udGVudC5jb20iLCJzdWIiOiIxMDI2ODcyOTA4OTIwOTUyNDQwNTciLCJub25jZSI6IkhTcXdzb3k4a1Nwb3Q5LWNrRVVGUGItTGRHMCIsIm5iZiI6MTc1NzA2MjA5NiwiaWF0IjoxNzU3MDYyMzk2LCJleHAiOjE3NTcwNjU5OTYsImp0aSI6IjM3ODJiMDc3NmZkYzZlMzZlMWI1ZmM4YmY4N2JkMzExOTJmZmM4MTYifQ.Yl5OeWtcEuHfjn5mNRaI_nNTHtcmQ6Q41C85-0PHX8XytVBzZGaLXxuCEbDogSb4f9MHK56_Zn2kFThhYyZ7uIK9v_EyTf6_ZjJ3IN29ehNHNlvJToqzsCE9O0zQ5yzgIdHMRfg6l3wRGkWGX2ChEtdTy2zMFB6AZP-8nkf2CaKkD5O8aDYurVsew6tneORWE6OnNC6XEITKIU_JhrN-6zND0XirzMZyL-Ozn2U8i7_vMytuFZSjORUSgPglGCmpVOtwwr8CS4679ltqyXZxND4jbt6A3mfHWySxvNJ_AAQiM4rtExg_IQ58sriBBGUcCRL3YcRD8x9iFHGe1sOfzQ","provider": "google"}'
```

## API Endpoints

### POST /get_salt
Processes JWT tokens and returns salt information.

**Request Body:**
```json
{
  "message": "your_jwt_token_here",
  "provider": "your_jwk_provider_here"
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
