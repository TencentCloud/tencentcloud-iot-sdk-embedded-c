CXX 				:= g++
LDFLAGS             := $(FINAL_DIR)/lib/libiot_sdk.a
LDFLAGS             += $(FINAL_DIR)/lib/libiot_platform.a
ifeq (,$(filter -DAUTH_WITH_NOTLS,$(CFLAGS)))
LDFLAGS             += $(FINAL_DIR)/lib/libmbedtls.a $(FINAL_DIR)/lib/libmbedx509.a $(FINAL_DIR)/lib/libmbedcrypto.a
endif

UNIT_SRC_DIR = ${TESTS_DIR}/unit_test/src
UNIT_SRC_FILES = $(wildcard $(UNIT_SRC_DIR)/*.cpp)

HELPER_C_FILES = $(wildcard $(UNIT_SRC_DIR)/*.c)
TLS_C_FILES = $(wildcard ${TESTS_DIR}/unit_test/tls_mock/*.c)

unit_objects = $(patsubst %.cpp,%, $(UNIT_SRC_FILES))

ifneq (,$(filter -DSDKTESTS_ENABLED,$(CFLAGS)))
run_unit_test: update gtest ${unit_objects}

update:
	$(TOP_Q) \
	chmod a+x $(SCRIPT_DIR)/update_gtest.sh
	$(TOP_Q) \
	$(SCRIPT_DIR)/update_gtest.sh > /dev/null

gtest:
	$(TOP_Q) \
	make -s -C $(TEST_LIB_DIR)

${unit_objects}:%:%.cpp
	$(call Brief_Log,"LD")
	$(TOP_Q) \
	$(CXX) $(CFLAGS) -I$(TEST_LIB_DIR)/include -I$(TESTS_DIR)/unit_test/include \
	-I$(TESTS_DIR)/unit_test/tls_mock -pthread \
	$^ $(HELPER_C_FILES) $(TLS_C_FILES) $(LDFLAGS) ${TEST_LIB_DIR}/libgtest.a \
	-o $@

	$(TOP_Q) \
	mv $@ $(FINAL_DIR)/unittest

endif
