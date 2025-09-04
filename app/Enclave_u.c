#include "Enclave_u.h"
#include <errno.h>

typedef struct ms_jwt_to_salt_t {
	sgx_status_t ms_retval;
	const uint8_t* ms_some_string;
	size_t ms_len;
	uint8_t* ms_output_salt;
} ms_jwt_to_salt_t;

typedef struct ms_ocall_print_message_t {
	const char* ms_message;
} ms_ocall_print_message_t;

static sgx_status_t SGX_CDECL Enclave_ocall_print_message(void* pms)
{
	ms_ocall_print_message_t* ms = SGX_CAST(ms_ocall_print_message_t*, pms);
	ocall_print_message(ms->ms_message);

	return SGX_SUCCESS;
}

static const struct {
	size_t nr_ocall;
	void * table[1];
} ocall_table_Enclave = {
	1,
	{
		(void*)Enclave_ocall_print_message,
	}
};
sgx_status_t jwt_to_salt(sgx_enclave_id_t eid, sgx_status_t* retval, const uint8_t* some_string, size_t len, uint8_t* output_salt)
{
	sgx_status_t status;
	ms_jwt_to_salt_t ms;
	ms.ms_some_string = some_string;
	ms.ms_len = len;
	ms.ms_output_salt = output_salt;
	status = sgx_ecall(eid, 0, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

