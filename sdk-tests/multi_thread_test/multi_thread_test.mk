DEPENDS             := src/platform
LDFLAGS             := $(FINAL_DIR)/lib/libiot_sdk.a
LDFLAGS             += $(FINAL_DIR)/lib/libiot_platform.a
ifeq (,$(filter -DAUTH_WITH_NOTLS,$(CFLAGS)))
LDFLAGS             += $(FINAL_DIR)/lib/libmbedtls.a $(FINAL_DIR)/lib/libmbedx509.a $(FINAL_DIR)/lib/libmbedcrypto.a
endif
CFLAGS              := $(filter-out -ansi,$(CFLAGS))
CFLAGS              += -pthread

ifneq (,$(filter -DSDKTESTS_ENABLED,$(CFLAGS)))

run_multi_thread_test:$(TESTS_DIR)/multi_thread_test/multi_thread_test.c
	$(eval CFLAGS := $(filter-out $(IOTSDK_INCLUDE_FILES),$(CFLAGS)) \
		-I$(TOP_DIR)/src/sdk-impl -I$(TOP_DIR)/src/sdk-impl/exports)

	$(call Brief_Log,"LD")
	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

	$(TOP_Q) \
	mv $@ $(FINAL_DIR)/unittest \

endif