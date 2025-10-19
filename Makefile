# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

######## SGX SDK Settings ########

SGX_SDK ?= /opt/intel/sgxsdk
SGX_MODE ?= HW
SGX_ARCH ?= x64
SGX_DEBUG ?= 1

include $(SGX_SDK)/buildenv.mk

ifeq ($(shell getconf LONG_BIT), 32)
    SGX_ARCH := x86
else ifeq ($(findstring -m32, $(CXXFLAGS)), -m32)
    SGX_ARCH := x86
endif

ifeq ($(SGX_ARCH), x86)
    SGX_COMMON_FLAGS := -m32
    SGX_LIBRARY_PATH := $(SGX_SDK)/lib
    SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x86/sgx_sign
    SGX_EDGER8R := $(SGX_SDK)/bin/x86/sgx_edger8r
else
    SGX_COMMON_FLAGS := -m64
    SGX_LIBRARY_PATH := $(SGX_SDK)/lib64
    SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x64/sgx_sign
    SGX_EDGER8R := $(SGX_SDK)/bin/x64/sgx_edger8r
endif

ifeq ($(SGX_DEBUG), 1)
    SGX_COMMON_FLAGS += -O0 -g
else
    SGX_COMMON_FLAGS += -O2
endif

SGX_COMMON_FLAGS += -Wall -Wextra -Winit-self -Wpointer-arith -Wreturn-type \
                    -Waddress -Wsequence-point -Wformat-security \
                    -Wmissing-include-dirs -Wfloat-equal -Wundef -Wshadow \
                    -Wcast-align -Wcast-qual -Wconversion -Wredundant-decls

SGX_COMMON_CFLAGS := $(SGX_COMMON_FLAGS) -Wjump-misses-init -Wstrict-prototypes -Wunsuffixed-float-constants
SGX_COMMON_CXXFLAGS := $(SGX_COMMON_FLAGS) -Wnon-virtual-dtor -std=c++11

ifneq ($(SGX_MODE), HW)
    Urts_Library_Name := sgx_urts_sim
else
    Urts_Library_Name := sgx_urts
endif

App_Cpp_Files := App/App.cpp
Test_Cpp_Files := tests/test_app.cpp
App_Include_Paths := -IInclude -IApp -I$(SGX_SDK)/include -I/usr/local/include
App_C_Flags := -fPIC -Wno-attributes $(App_Include_Paths)

ifeq ($(SGX_DEBUG), 1)
    App_C_Flags += -DDEBUG -UNDEBUG -UEDEBUG
else
    App_C_Flags += -DNDEBUG -UEDEBUG -UDEBUG
endif

ifdef TEST_MODE
    App_C_Flags += -DTEST_MODE
endif

App_Cpp_Flags := $(App_C_Flags)
App_Link_Flags := -L$(SGX_LIBRARY_PATH) -Wl,-rpath,$(SGX_LIBRARY_PATH) -l$(Urts_Library_Name) -lpthread -lcurl -ljson-c -lssl -lcrypto -lmicrohttpd

# Link SGX uae service library depending on mode
ifneq ($(SGX_MODE), HW)
    App_Link_Flags += -lsgx_uae_service_sim
else
    App_Link_Flags += -lsgx_uae_service
endif

ifeq ($(COVERAGE), 1)
    App_Cpp_Flags += --coverage
    App_Link_Flags += --coverage
endif

App_Cpp_Objects := $(App_Cpp_Files:.cpp=.o)
Test_Cpp_Objects := $(Test_Cpp_Files:.cpp=.o)
App_Name := app
Test_Name := bin/test_app

ifneq ($(SGX_MODE), HW)
    Trts_Library_Name := sgx_trts_sim
    Service_Library_Name := sgx_tservice_sim
else
    Trts_Library_Name := sgx_trts
    Service_Library_Name := sgx_tservice
endif

Crypto_Library_Name := sgx_tcrypto

Enclave_Cpp_Files := Enclave/Enclave.cpp
Enclave_Include_Paths := -IInclude -IEnclave -I$(SGX_SDK)/include -I$(SGX_SDK)/include/tlibc -I$(SGX_SDK)/include/libcxx

Enclave_C_Flags := $(Enclave_Include_Paths) -nostdinc -fvisibility=hidden -fpie -ffunction-sections -fdata-sections
Enclave_Cpp_Flags := $(Enclave_C_Flags) -nostdinc++

Enclave_Security_Link_Flags := -Wl,-z,relro,-z,now,-z,noexecstack

Enclave_Link_Flags := $(Enclave_Security_Link_Flags) \
    -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L$(SGX_TRUSTED_LIBRARY_PATH) \
    -Wl,--whole-archive -l$(Trts_Library_Name) -Wl,--no-whole-archive \
    -Wl,--start-group -lsgx_tstdc -lsgx_tcxx -l$(Crypto_Library_Name) -l$(Service_Library_Name) -Wl,--end-group \
    -Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
    -Wl,-pie,-eenclave_entry -Wl,--export-dynamic \
    -Wl,--defsym,__ImageBase=0 -Wl,--gc-sections

Enclave_Cpp_Objects := $(sort $(Enclave_Cpp_Files:.cpp=.o))

Enclave_Name := enclave.so
Signed_Enclave_Name := enclave.signed.so
Enclave_Config_File := Enclave/Enclave.config.xml
Enclave_Test_Key := Enclave_private_test.pem

ifeq ($(SGX_MODE), HW)
ifeq ($(SGX_DEBUG), 1)
    Build_Mode = HW_DEBUG
else
    Build_Mode = HW_RELEASE
endif
else
ifeq ($(SGX_DEBUG), 1)
    Build_Mode = SIM_DEBUG
else
    Build_Mode = SIM_RELEASE
endif
endif

.PHONY: all target run clean test test-build test-coverage test-run test-run-coverage test-app test-coverage-app

all: .config_$(Build_Mode)_$(SGX_ARCH)
	@$(MAKE) target

ifeq ($(Build_Mode), HW_RELEASE)
target: $(App_Name) $(Enclave_Name)
	@echo "The project has been built in release hardware mode."
	@echo "Please sign the $(Enclave_Name) first with your signing key before you run the $(App_Name) to launch and access the enclave."
	@echo "To sign the enclave use the command:"
	@echo "   $(SGX_ENCLAVE_SIGNER) sign -key <your key> -enclave $(Enclave_Name) -out <$(Signed_Enclave_Name)> -config $(Enclave_Config_File)"
else
target: $(App_Name) $(Signed_Enclave_Name)
ifeq ($(Build_Mode), HW_DEBUG)
	@echo "The project has been built in debug hardware mode."
else ifeq ($(Build_Mode), SIM_DEBUG)
	@echo "The project has been built in debug simulation mode."
else
	@echo "The project has been built in release simulation mode."
endif
endif

# Build test target when COVERAGE is enabled
ifeq ($(COVERAGE), 1)
target: $(Test_Name)
	@echo "Test executable built with coverage support"
endif

run: all
ifneq ($(Build_Mode), HW_RELEASE)
	@$(CURDIR)/$(App_Name)
	@echo "RUN  =>  $(App_Name) [$(SGX_MODE)|$(SGX_ARCH), OK]"
endif

.config_$(Build_Mode)_$(SGX_ARCH):
	@rm -f .config_* $(App_Name) $(Enclave_Name) $(Signed_Enclave_Name) $(App_Cpp_Objects) Enclave_u.* $(Enclave_Cpp_Objects) Enclave_t.*
	@touch .config_$(Build_Mode)_$(SGX_ARCH)

App/Enclave_u.h: $(SGX_EDGER8R) Enclave/Enclave.edl
	@cd App && $(SGX_EDGER8R) --untrusted ../Enclave/Enclave.edl --search-path ../Enclave --search-path $(SGX_SDK)/include
	@echo "GEN  =>  $@"

App/Enclave_u.c: App/Enclave_u.h

App/Enclave_u.o: App/Enclave_u.c
	@$(CC) $(SGX_COMMON_CFLAGS) $(App_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"

App/App.o: App/App.cpp App/Enclave_u.h
	@$(CXX) $(SGX_COMMON_CXXFLAGS) $(App_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

# Special rule for App.o when building tests (excludes main function)
App/App_test.o: App/App.cpp App/Enclave_u.h
	@$(CXX) $(SGX_COMMON_CXXFLAGS) $(App_Cpp_Flags) -DTEST_BINARY --coverage -c $< -o $@
	@echo "CXX  <=  $< (test version)"


$(App_Name): App/Enclave_u.o App/App.o
	@$(CXX) $^ -o $@ $(App_Link_Flags)
	@echo "LINK =>  $@"

Enclave/Enclave_t.h: $(SGX_EDGER8R) Enclave/Enclave.edl
	@cd Enclave && $(SGX_EDGER8R) --trusted ../Enclave/Enclave.edl --search-path ../Enclave --search-path $(SGX_SDK)/include
	@echo "GEN  =>  $@"

Enclave/Enclave_t.c: Enclave/Enclave_t.h

Enclave/Enclave_t.o: Enclave/Enclave_t.c
	@$(CC) $(SGX_COMMON_CFLAGS) $(Enclave_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"

Enclave/Enclave.o: Enclave/Enclave.cpp Enclave/Enclave_t.h
	@$(CXX) $(SGX_COMMON_CXXFLAGS) $(Enclave_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

$(Enclave_Name): Enclave/Enclave_t.o Enclave/Enclave.o
	@$(CXX) $^ -o $@ $(Enclave_Link_Flags)
	@echo "LINK =>  $@"

$(Signed_Enclave_Name): $(Enclave_Name)
ifeq ($(wildcard $(Enclave_Test_Key)),)
	@echo "There is no enclave test key<Enclave_private_test.pem>."
	@echo "The project will generate a key<Enclave_private_test.pem> for test."
	@openssl genrsa -out $(Enclave_Test_Key) -3 3072
endif
	@$(SGX_ENCLAVE_SIGNER) sign -key $(Enclave_Test_Key) -enclave $(Enclave_Name) -out $@ -config $(Enclave_Config_File)
	@echo "SIGN =>  $@"

# Test targets
test: $(Test_Name)
	@echo "Running tests..."
	@$(CURDIR)/$(Test_Name)

test-build: $(Test_Name)
	@echo "Test executable built successfully: $(Test_Name)"

test-coverage: COVERAGE=1
test-coverage: test-build
	@echo "Test executable built with coverage support"

test-run: $(Test_Name)
	@echo "Running tests..."
	@$(CURDIR)/$(Test_Name)

test-run-coverage: COVERAGE=1
test-run-coverage: $(Test_Name)
	@echo "Running tests with coverage..."
	@$(CURDIR)/$(Test_Name)
	@echo "Generating coverage report..."
	@gcov -o App/App_test.o App/App.cpp
	@echo "Coverage report generated"

coverage: test-run-coverage
	@echo "Coverage: $(shell gcov -o App/App_test.o App/App.cpp 2>&1 | grep 'File.*App.cpp' | sed 's/.*Lines executed://' | sed 's/%.*//')%"

# Test command similar to docker run with --test flag
test-app: COVERAGE=1
test-app: $(Test_Name)
	@echo "Running test application..."
	@$(CURDIR)/$(Test_Name)

# Coverage test command
test-coverage-app: COVERAGE=1
test-coverage-app: $(Test_Name)
	@echo "Running test application with coverage..."
	@$(CURDIR)/$(Test_Name)
	@echo "Generating coverage report..."
	@gcov -o App/App_test App/App.cpp
	@echo ""
	@echo "=== Coverage Summary ==="
	@grep "Lines executed" App.cpp.gcov | head -1
	@echo "========================"

$(Test_Name): App/Enclave_u.o App/App_test.o $(Test_Cpp_Objects)
	@$(CXX) $^ -o $@ $(App_Link_Flags) --coverage
	@echo "LINK =>  $@"

tests/test_app.o: tests/test_app.cpp App/Enclave_u.h
	@$(CXX) $(SGX_COMMON_CXXFLAGS) $(App_Cpp_Flags) -DTEST_BINARY --coverage -c $< -o $@
	@echo "CXX  <=  $<"

clean:
	@rm -f .config_* $(App_Name) $(Enclave_Name) $(Signed_Enclave_Name) $(App_Cpp_Objects) App/App_test.o App/Enclave_u.* $(Enclave_Cpp_Objects) Enclave/Enclave_t.* $(Enclave_Test_Key)
	@rm -f $(Test_Name) $(Test_Cpp_Objects)
	@rm -f *.gcda *.gcno *.gcov coverage.info
	@rm -rf coverage_html