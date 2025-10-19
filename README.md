# SGX Enclave Docker Environment Guide

A comprehensive guide for running Intel SGX enclave applications in a simulated SGX environment (SIM mode) using Docker.

## Overview

This project provides a Docker-based environment for developing and testing Intel SGX enclave applications. It uses the SGX simulation mode, which allows you to develop and test SGX applications without requiring actual SGX hardware.

## Prerequisites

- **Operating System**: Ubuntu 20.04+ (recommended)
- **Architecture**: x86_64
- **Docker**: Version 20.10+ 
- **Storage**: At least 8GB free disk space


## Salt Generation Principles
![salt_server_workflow](./assets/salt_workflow.png)
The salt server plays an important part in maintaining privacy and security for users' Web2 credentials when using Kzero. Using a secret master seed and the user's JWT, the salt server produces a salt value that is unique to that user for that app, but hides the connection from the user's identity to their Polkadot activity, cryptographically ensuring privacy. The salt value is required before generating a zkLogin proof and therefore before issuing transactions onchain.

When someone uses an app backed by the Kzero salt server, they enter their Web2 credentials and the application requests a JWT from the auth provider. The app then sends the JWT to the salt server to get the salt value. Each time the Polkadot address is derived from the user's identity, the salt is used to ensure that the user's address can always deterministically be computed from their token without revealing the binding between the two.

![salt_derive](./assets/salt_derive.png)
This service implements a salt generation mechanism that keeps a master seed value and derives a user salt with key derivation by validating and parsing the JWT. For example, using `HKDF(ikm = seed, salt = iss || aud, info = sub)`(To know more about HKDF, please refer to the link [here](https://datatracker.ietf.org/doc/html/rfc5869)).
> For more details about the salt server, please check [here](https://github.com/kzero-xyz/kzero-grant-docs/blob/main/kzero-salt-service-spec.md)


## Quick Start

### 1. Build the Docker Image
> **Note:** Build time depends on your machine's performance. For reference, on a 4-CPU Intel(R) Xeon(R) CPU E5-2680 v2 @ 2.80GHz (CPU MHz: 2792.998), the build process takes approximately 50 minutes.
- You can build it locally via this command, but we recommend you to directly pulling our docker online.
```bash
docker build --build-arg COVERAGE=1 -t test-enclave:coverage .
```
- Pulling the docker online:
```bash
docker pull kzeroxyz/kzero-salt-enclave-service:v0.1.1
```

### 2. Run the Container
- If you build locally, run this command:
```bash
docker run -d -p 8080:8080 --name test-enclave-new -e SGX_MODE=SIM test-enclave:coverage
```
- If you pull online, run this command:
```bash
docker run -d -p 8080:8080 --name test-enclave-new -e SGX_MODE=SIM kzeroxyz/kzero-salt-enclave-service:v0.1.1
```


### 3. Run Tests
```bash
docker run --rm test-enclave:coverage make test-app
```

To get the coverage report, run this command:
```bash
docker run --rm test-enclave:coverage make test-coverage-app
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

### 4. Test the API

```bash
curl -s -X POST http://localhost:8080/get_salt \
  -H 'Content-Type: application/json' \
  -d '{"message":"eyJhbGciOiJSUzI1NiIsImtpZCI6IjA3ZjA3OGYyNjQ3ZThjZDAxOWM0MGRhOTU2OWU0ZjUyNDc5OTEwOTQiLCJ0eXAiOiJKV1QifQ.eyJpc3MiOiJodHRwczovL2FjY291bnRzLmdvb2dsZS5jb20iLCJhenAiOiI1NjA2MjkzNjU1MTctbXQ5ajlhcmZsY2dpMzVpOGhwb3B0cjY2cWdvMWxtZm0uYXBwcy5nb29nbGV1c2VyY29udGVudC5jb20iLCJhdWQiOiI1NjA2MjkzNjU1MTctbXQ5ajlhcmZsY2dpMzVpOGhwb3B0cjY2cWdvMWxtZm0uYXBwcy5nb29nbGV1c2VyY29udGVudC5jb20iLCJzdWIiOiIxMTExNDA0NjE1MzAyNDYxNjQ1MjYiLCJub25jZSI6InlwanZ6TXB6d09qelcycUlrVnBiQU9UTUZuVSIsIm5iZiI6MTc1Nzc1MjA2NCwiaWF0IjoxNzU3NzUyMzY0LCJleHAiOjE3NTc3NTU5NjQsImp0aSI6ImZkYzRmNTc3YWI0NWViZjhiMjU3NjkwMjQwZmUzMTYyOGFkOGI4ZmMifQ.D4NVKogzU76ZGV5HsUDTOHRwSSG1I3lgG4bUEWAeMW8G-QDnXBNY6QDFmYnVEWWx5VlejyQhvmdtJrXF2eDOMKGeOwnFlm1INQuneELbLz0sbKnDw62IKshgQGNP5jv5ij-HEKj3jkx8D1zof83duVDhFOUmDud0VZKPODfBRLbqoTJKz0cp0RwZ5k-SiT_aSeL-y_FodYcCt5VtXIZfvgWj_NbcscqPaIBMvjJ9-wFx8yD-6C5dIQDVgyhZGtLzwxRLZMr6yotBuz_49BlKquuPA6TgNdUvMRu35QRYEQYPx3RigYtKw_8GGW-LVbmZTKSBOKu8QMEweR9CCaBHvg","provider":"test_google"}'
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
