CXX 				:= g++
LDFLAGS             := $(FINAL_DIR)/lib/libiot_sdk.a
LDFLAGS             += $(FINAL_DIR)/lib/libiot_platform.a
LDFLAGS             += $(FINAL_DIR)/lib/libmbedtls.a $(FINAL_DIR)/lib/libmbedx509.a $(FINAL_DIR)/lib/libmbedcrypto.a
GOOGLE_TEST_DIRS 	:= $(THIRD_PARTY_PATH)/googletest

UNIT_SRC_DIR = ${TESTS_DIR}/unit_test/src
UNIT_SRC_FILES = $(wildcard $(UNIT_SRC_DIR)/*.cpp)
UNIT_SRC_FILES := $(filter-out $(UNIT_SRC_DIR)/unit_util.cpp, $(UNIT_SRC_FILES))

HELPER_C_FILES = $(wildcard $(UNIT_SRC_DIR)/*.c)
TLS_C_FILES = $(wildcard ${TESTS_DIR}/unit_test/tls_mock/*.c)

unit_objects = $(patsubst %.cpp,%, $(UNIT_SRC_FILES))
	
ifneq (,$(filter -DSDKTESTS_ENABLED,$(CFLAGS)))
run_unit_test: ${unit_objects}
	
${unit_objects}:%:%.cpp

	$(TOP_Q) \
	make -s -C $(GOOGLE_TEST_DIRS)
	
	$(TOP_Q) \
	$(CXX) $(CFLAGS) -I$(GOOGLE_TEST_DIRS)/include -I$(TESTS_DIR)/unit_test/include \
	-I$(TESTS_DIR)/unit_test/tls_mock -pthread \
	$^ $(HELPER_C_FILES) $(TLS_C_FILES) $(TESTS_DIR)/unit_test/src/unit_util.cpp $(LDFLAGS) ${GOOGLE_TEST_DIRS}/libgtest.a \
	-o $@
	
	$(TOP_Q) \
	mv $@ $(FINAL_DIR)/unittest
	
endif
