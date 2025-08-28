#![no_std]
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
// under the License.

#![crate_name = "helloworldsampleenclave"]
#![crate_type = "staticlib"]

#![cfg_attr(not(target_env = "sgx"), no_std)]
#![cfg_attr(target_env = "sgx", feature(rustc_private))]

extern crate sgx_types;

extern crate sgx_tstd as std;

use sgx_types::*;
use sgx_types::error::SgxStatus;

type sgx_status_t = SgxStatus;

use std::string::String;
use std::vec::Vec;
use std::io::{self, Write};
use std::slice;
use std::string::ToString;
extern crate hkdf;
extern crate sha2;  
extern crate hex_literal;
extern crate base64;

use hkdf::Hkdf;
use sha2::Sha256;
use hex_literal::hex;

const ikm: [u8; 22] = hex!("0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b");

fn parse_jwt(jwt: &str) -> Result<serde_json::Value, sgx_status_t> {
    // Split JWT into parts
    let parts: Vec<&str> = jwt.split('.').collect();
    if parts.len() != 3 {
//        println!("Invalid JWT format");
        return Err(sgx_status_t::InvalidParameter);
    }

    // Decode JWT payload (second part)
    let payload = match base64::decode(parts[1]) {
        Ok(p) => p,
        Err(_) => return Err(sgx_status_t::InvalidParameter),
    };

    // Parse JSON payload
    let payload_str = String::from_utf8_lossy(&payload).to_string();

    match serde_json::from_str(&payload_str) {
        Ok(json) => Ok(json),
        Err(_) => {
      //      println!("Failed to parse JWT payload as JSON");
            Err(sgx_status_t::InvalidParameter)
        }
    }
}

fn generate_hkdf(sub: &str, iss: &str, aud: &str) -> [u8; 16] {
    // Combine iss and aud as salt
    let salt = std::format!("{}{}", iss, aud);
    let salt_bytes = salt.as_bytes();

    // Use sub as info
    let info = sub.as_bytes();

    // Create HKDF
    let hk = Hkdf::<Sha256>::new(Some(salt_bytes), &ikm);
    let mut okm = [0u8; 42];
    hk.expand(info, &mut okm)
        .expect("42 is a valid length for Sha256 to output");

    let mut result = [0u8; 16];
    result.copy_from_slice(&okm[0..16]);
    result
}

#[no_mangle]
pub extern "C" fn jwt_to_salt(some_string: *const u8, some_len: usize, output_salt: *mut u8) -> sgx_status_t {
    let str_slice = unsafe { slice::from_raw_parts(some_string, some_len) };
    let input_string = String::from_utf8_lossy(str_slice);

    let json = match parse_jwt(&input_string) {
        Ok(j) => j,
        Err(e) => return e,
    };

    // Extract and print claims
    if let Some(sub) = json.get("sub") {
        // println!("Subject (sub): {}", sub);
    }
    if let Some(iss) = json.get("iss") {
     //   println!("Issuer (iss): {}", iss);
    }
    if let Some(aud) = json.get("aud") {
     //   println!("Audience (aud): {}", aud);
    }

    let sub = json.get("sub").unwrap().as_str().unwrap_or("").to_string();
    let iss = json.get("iss").unwrap().as_str().unwrap_or("").to_string();
    let aud = json.get("aud").unwrap().as_str().unwrap_or("").to_string();

    let okm = generate_hkdf(&sub, &iss, &aud);
  //  println!("{:?}", okm);

    // Copy okm to output buffer
    unsafe {
        std::ptr::copy_nonoverlapping(okm.as_ptr(), output_salt, 16);
    }

    sgx_status_t::Success
}