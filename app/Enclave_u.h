#ifndef ENCLAVE_U_H__
#define ENCLAVE_U_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include <string.h>
#include "sgx_edger8r.h" /* for sgx_status_t etc. */


#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif


sgx_status_t jwt_to_salt(sgx_enclave_id_t eid, sgx_status_t* retval, const uint8_t* some_string, size_t len, uint8_t* output_salt);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif