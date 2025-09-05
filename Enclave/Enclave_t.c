#include "Enclave_t.h"

#include "sgx_trts.h" /* for sgx_ocalloc, sgx_is_outside_enclave */
#include "sgx_lfence.h" /* for sgx_lfence */

#include <errno.h>
#include <mbusafecrt.h> /* for memcpy_s etc */
#include <stdlib.h> /* for malloc/free etc */

#define CHECK_REF_POINTER(ptr, siz) do {	\
	if (!(ptr) || ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define CHECK_UNIQUE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define CHECK_ENCLAVE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_within_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define ADD_ASSIGN_OVERFLOW(a, b) (	\
	((a) += (b)) < (b)	\
)


typedef struct ms_jwt_to_salt_t {
	sgx_status_t ms_retval;
	const uint8_t* ms_some_string;
	size_t ms_len;
	uint8_t* ms_output_salt;
} ms_jwt_to_salt_t;

typedef struct ms_ocall_print_message_t {
	const char* ms_message;
} ms_ocall_print_message_t;

static sgx_status_t SGX_CDECL sgx_jwt_to_salt(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_jwt_to_salt_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_jwt_to_salt_t* ms = SGX_CAST(ms_jwt_to_salt_t*, pms);
	ms_jwt_to_salt_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_jwt_to_salt_t), ms, sizeof(ms_jwt_to_salt_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	const uint8_t* _tmp_some_string = __in_ms.ms_some_string;
	size_t _tmp_len = __in_ms.ms_len;
	size_t _len_some_string = _tmp_len;
	uint8_t* _in_some_string = NULL;
	uint8_t* _tmp_output_salt = __in_ms.ms_output_salt;
	size_t _len_output_salt = 16;
	uint8_t* _in_output_salt = NULL;
	sgx_status_t _in_retval;

	CHECK_UNIQUE_POINTER(_tmp_some_string, _len_some_string);
	CHECK_UNIQUE_POINTER(_tmp_output_salt, _len_output_salt);

	//
	// fence after pointer checks
	//
	sgx_lfence();

	if (_tmp_some_string != NULL && _len_some_string != 0) {
		if ( _len_some_string % sizeof(*_tmp_some_string) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_some_string = (uint8_t*)malloc(_len_some_string);
		if (_in_some_string == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_some_string, _len_some_string, _tmp_some_string, _len_some_string)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}
	if (_tmp_output_salt != NULL && _len_output_salt != 0) {
		if ( _len_output_salt % sizeof(*_tmp_output_salt) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		if ((_in_output_salt = (uint8_t*)malloc(_len_output_salt)) == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memset((void*)_in_output_salt, 0, _len_output_salt);
	}
	_in_retval = jwt_to_salt((const uint8_t*)_in_some_string, _tmp_len, _in_output_salt);
	if (memcpy_verw_s(&ms->ms_retval, sizeof(ms->ms_retval), &_in_retval, sizeof(_in_retval))) {
		status = SGX_ERROR_UNEXPECTED;
		goto err;
	}
	if (_in_output_salt) {
		if (memcpy_verw_s(_tmp_output_salt, _len_output_salt, _in_output_salt, _len_output_salt)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}

err:
	if (_in_some_string) free(_in_some_string);
	if (_in_output_salt) free(_in_output_salt);
	return status;
}

SGX_EXTERNC const struct {
	size_t nr_ecall;
	struct {void* ecall_addr; uint8_t is_priv; uint8_t is_switchless;} ecall_table[1];
} g_ecall_table = {
	1,
	{
		{(void*)(uintptr_t)sgx_jwt_to_salt, 0, 0},
	}
};

SGX_EXTERNC const struct {
	size_t nr_ocall;
	uint8_t entry_table[1][1];
} g_dyn_entry_table = {
	1,
	{
		{0, },
	}
};


sgx_status_t SGX_CDECL ocall_print_message(const char* message)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_message = message ? strlen(message) + 1 : 0;

	ms_ocall_print_message_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_print_message_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(message, _len_message);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (message != NULL) ? _len_message : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_print_message_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_print_message_t));
	ocalloc_size -= sizeof(ms_ocall_print_message_t);

	if (message != NULL) {
		if (memcpy_verw_s(&ms->ms_message, sizeof(const char*), &__tmp, sizeof(const char*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		if (_len_message % sizeof(*message) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_verw_s(__tmp, ocalloc_size, message, _len_message)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_message);
		ocalloc_size -= _len_message;
	} else {
		ms->ms_message = NULL;
	}

	status = sgx_ocall(0, ms);

	if (status == SGX_SUCCESS) {
	}
	sgx_ocfree();
	return status;
}

