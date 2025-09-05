#ifndef _ENCLAVE_H_
#define _ENCLAVE_H_

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "sgx_edger8r.h"  // for sgx_status_t
#include "sgx_eid.h"      // for sgx_enclave_id_t
#include "sgx_error.h"    // for sgx_status_t

#include "Enclave_t.h"    // for trusted function call

#define printf_s printf

#endif