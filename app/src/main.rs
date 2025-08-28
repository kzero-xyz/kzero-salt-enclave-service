// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License..

use actix_web::{web, App, HttpServer, Responder};
use serde::{Deserialize};
use lazy_static::lazy_static;
use std::sync::Arc;
use jsonwebtoken::{decode, decode_header, DecodingKey, Validation, Algorithm};
use serde_json::Value;
use reqwest;


extern crate sgx_types;
extern crate sgx_urts;
use sgx_types::types::EnclaveId;
use sgx_types::error::SgxStatus;
use sgx_urts::enclave::SgxEnclave;

static ENCLAVE_FILE: &'static str = "/opt/test-enclave/bin/enclave.signed.so";
const GOOGLE_JWKS_URL: &str = "https://www.googleapis.com/oauth2/v3/certs";
const PUBLIC_KEY: &str = "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAxj0X1nJS8kgmFZiQA59e\nZ9TsWntpmXAir0olAqJ6+QFCL7HFwFyWFkwGm46rtCSL/Rtk6LZkkZYbd6v2kBGm\nOnfF4UDXRyf55gFMkkofTiM7cmulhGHXwvGhbMAVKtbrehHmp488eIJGbP7w0IukU9\nZFvH2PJYNOeJHsowhRTkzGxZ3Y4i9FBvWheJLMNqklYBaWLirjf/cKHfs5mCoK6W/\nxJh6hPiq0mwUf/vtZkwICzL/Gyt/YIPqI58fDwcp4RloqIcr4C6Z6VbYvMu+RSeW8\nxxIFl98cgU5s4bKjbVU8Pxd1RARi5mJST5pDn0dycU5AiRe6xmWbxTOnR7sQpQIDAQAB\n-----END PUBLIC KEY-----";

#[link(name = "Enclave_u")]
unsafe extern {
    fn jwt_to_salt(eid: EnclaveId, retval: *mut SgxStatus,
                    some_string: *const u8, some_len: usize,
                     output_salt: *mut u8) -> SgxStatus;
}

fn init_enclave() -> Result<SgxEnclave, SgxStatus> {
    // Use Teaclave SGX SDK create method (only needs 2 parameters)
    SgxEnclave::create(ENCLAVE_FILE, true)
        .map_err(|e| e.into())
}

#[derive(Deserialize)]
struct GreetQuery {
    message: String,
}

#[derive(Deserialize)]
struct JwksResponse {
    keys: Vec<JwkKey>,
}

#[derive(Deserialize)]
struct JwkKey {
    kid: String,
    n: String,
    e: String,
    alg: String,
    #[serde(rename = "use")]
    usage: String,
    kty: String,
}

lazy_static! {
    static ref ENCLAVE: Arc<SgxEnclave> = {
        match init_enclave() {
            Ok(r) => {
                println!("[+] Init Enclave Successful {}!", r.eid());
                Arc::new(r)
            },
            Err(x) => {
                println!("[-] Init Enclave failed {}!", x);
                panic!("Failed to initialize enclave");
            }
        }
    };
}

async fn verify_custom_jwt(token: &str) -> Result<Value, String> {
    // 1. Extract header from JWT to get kid
    let header = decode_header(token).map_err(|e| format!("Failed to decode JWT header: {}", e))?;
    let kid = header.kid.ok_or("No 'kid' in JWT header")?;

    // 2. Verify kid matches expected value
    if kid != "grFgslma1FTW28ZMuObNCad44n-_P-ibhnYzrTvjBFg" {
        return Err("Invalid kid".to_string());
    }

    // 3. Verify JWT
    let mut validation = Validation::new(Algorithm::RS256);
    validation.set_audience(&["560629365517-mt9j9arflcgi35i8hpoptr66qgo1lmfm.apps.googleusercontent.com"]);
    validation.set_issuer(&["https://accounts.google.com"]);
    // Skip time validation (for testing)
    validation.validate_exp = false;
    validation.validate_nbf = false;

    let key = DecodingKey::from_rsa_pem(PUBLIC_KEY.as_bytes())
        .map_err(|e| format!("Failed to create decoding key: {}", e))?;

    let token_data = decode::<Value>(token, &key, &validation)
        .map_err(|e| format!("JWT verification failed: {}", e))?;

    Ok(token_data.claims)
}

async fn verify_google_jwt(token: &str) -> Result<Value, String> {
    println!("[DEBUG] Starting Google JWT verification...");
    // 1. Extract header from JWT to get kid
    let header = decode_header(token).map_err(|e| format!("Failed to decode JWT header: {}", e))?;
    let kid = header.kid.ok_or("No 'kid' in JWT header")?;
    println!("[DEBUG] JWT kid: {}", kid);

    // 2. Fetch Google public keys
    println!("[DEBUG] Fetching JWKS from: {}", GOOGLE_JWKS_URL);
    let jwks: JwksResponse = reqwest::get(GOOGLE_JWKS_URL)
        .await
        .map_err(|e| format!("Failed to fetch JWKS: {}", e))?
        .json::<JwksResponse>()
        .await
        .map_err(|e| format!("Failed to parse JWKS: {}", e))?;
    println!("[DEBUG] JWKS fetched successfully, found {} keys", jwks.keys.len());

    // 3. Find matching key
    let matching_key = jwks.keys
        .iter()
        .find(|k| k.kid == kid)
        .ok_or("No matching key found")?;

    // 4. Verify JWT
    let mut validation = Validation::new(Algorithm::RS256);
    validation.set_audience(&["560629365517-mt9j9arflcgi35i8hpoptr66qgo1lmfm.apps.googleusercontent.com"]); // Set zkLogin Google Client ID
    validation.set_issuer(&["https://accounts.google.com"]);
    // Skip time validation (for testing)
    validation.validate_exp = false;
    validation.validate_nbf = false;

    let key = DecodingKey::from_rsa_components(&matching_key.n, &matching_key.e)
        .map_err(|e| format!("Failed to create decoding key: {}", e))?;

    let token_data = decode::<Value>(token, &key, &validation)
        .map_err(|e| format!("JWT verification failed: {}", e))?;

    Ok(token_data.claims)
}

async fn greet(query: web::Json<GreetQuery>) -> impl Responder {
    // First verify Google JWT
    match verify_google_jwt(&query.message).await {
        Err(err) => {
            // If Google JWT verification fails, try custom JWT
            match verify_custom_jwt(&query.message).await {
                Err(custom_err) => {
                    return format!("JWT verification failed: {} and Custom JWT failed: {}", err, custom_err);
                },
                Ok(_) => {
                    println!("[+] Custom JWT verification successful");
                }
            }
        },
        Ok(_) => {
            println!("[+] Google JWT verification successful");
        }
    }

    // After JWT verification passes, continue processing
    let mut retval = SgxStatus::Success;
    let mut output_salt = [0u8; 16];

    let result = unsafe {
        jwt_to_salt(ENCLAVE.eid(),
                    &mut retval,
                    query.message.as_ptr() as *const u8,
                    query.message.len(),
                    output_salt.as_mut_ptr())
    };
    
    match result {
        SgxStatus::Success => {
            println!("[+] jwt_to_salt success...");
            // Convert bytes to decimal number
            let decimal_value: u128 = output_salt.iter()
                .fold(0u128, |acc, &b| (acc << 8) | b as u128);
            format!("{}", decimal_value)
        },
        _ => {
            println!("[-] ECALL Enclave Failed {}!", result.as_str());
            format!("Error: ECALL Enclave Failed {}", result.as_str())
        }
    }
}

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    HttpServer::new(|| {
        App::new()
            .service(
                web::resource("/get_salt")
                    .route(web::post().to(greet))
                    .app_data(web::JsonConfig::default().limit(4096))
            )
    })
    .bind("0.0.0.0:8080")?
    .run()
    .await
}