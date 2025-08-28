# SGX Enclave Docker Environment Guide

A comprehensive guide for running Intel SGX enclave applications in a simulated SGX environment (SIM mode) using Docker.

## Overview

This project provides a Docker-based environment for developing and testing Intel SGX enclave applications. It uses the SGX simulation mode, which allows you to develop and test SGX applications without requiring actual SGX hardware.

## Prerequisites

- **Operating System**: Ubuntu 18.04+ (recommended)
- **Docker**: Version 20.10+ 
- **Storage**: At least 8GB free disk space

## Quick Start

### 1. Build the Docker Image

```bash
docker build -t test-enclave:latest .
```

### 2. Run the Container

```bash
docker run -d -p 8080:8080 --name test-enclave-new -e SGX_MODE=SIM test-enclave:latest
```

### 3. Test the API

```bash
curl -X POST -H "Content-Type: application/json" -d '{"message": "eyJhbGciOiJSUzI1NiIsImtpZCI6IjE0OTljMTU0Y2NjOGEyNWUyNGQ4ZGU4YjFhOWY4NDVhZWZiNmYzY2EiLCJ0eXAiOiJKV1QifQ.eyJpc3MiOiJodHRwczovL2FjY291bnRzLmdvb2dsZS5jb20iLCJhenAiOiI1NjA2MjkzNjU1MTctbXQ5ajlhcmZsY2dpMzVpOGhwb3B0cjY2cWdvMWxtZm0uYXBwcy5nb29nbGV1c2VyY29udGVudC5jb20iLCJhdWQiOiI1NjA2MjkzNjU1MTctbXQ5ajlhcmZsY2dpMzVpOGhwb3B0cjY2cWdvMWxtZm0uYXBwcy5nb29nbGV1c2VyY29udGVudC5jb20iLCJzdWIiOiIxMTExNDA0NjE1MzAyNDYxNjQ1MjYiLCJub25jZSI6IndsZXdXQndsSHhnaVJZbFdrRGwxbERVYWVERSIsIm5iZiI6MTc1NjQwNjUyMCwiaWF0IjoxNzU2NDA2ODIwLCJleHAiOjE3NTY0MTA0MjAsImp0aSI6Ijc4ZmVjMjI3NGRiNDE5ZWJkYjVhMmIzNzRhYjlhYzlkNzAxMjIyZGMifQ.VwnREcEhm0zJZqrLUGFxNLHQkHDCAzs1QsiSZSKV8BY6QVn0Km4xm6agNP1_KcRolE67uhoPtPSdxtHHRj9gad105sUwzjNE1Jl0polQR_0_M-1sEkJ5hnXD7bVjUPSglfUbD04Zkagu2h-imN-fy4w90dwHntVF6u7P3ttO8AJbtb4aSylgjCEkBNc-U2A90SMiTKsv7w0lD1qiOhTTxUnt6OEHmhv3DY3JCAdFjGDG08LTX08Nkj7m7Ox7YObVrykyXfr97b7pgR3rCXzBnxcU784EMkwIVBdqmDvSGdbFUxmoSRTvonPhbrBfLRnnv5O127xytZlodNrfCDb2pA"}' http://localhost:8080/get_salt
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
