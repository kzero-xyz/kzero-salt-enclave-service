#ifndef ENCLAVE_T_H__
#define ENCLAVE_T_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include "sgx_edger8r.h" /* for sgx_ocall etc. */


#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

sgx_status_t jwt_to_salt(const uint8_t* some_string, size_t len, uint8_t* output_salt);

sgx_status_t SGX_CDECL ocall_print_message(const char* message);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
