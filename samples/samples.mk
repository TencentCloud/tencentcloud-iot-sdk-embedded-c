DEPENDS             := src/platform
LDFLAGS             := $(FINAL_DIR)/lib/libiot_sdk.a
LDFLAGS             += $(FINAL_DIR)/lib/libiot_platform.a
LDFLAGS             += $(FINAL_DIR)/lib/libmbedtls.a $(FINAL_DIR)/lib/libmbedx509.a $(FINAL_DIR)/lib/libmbedcrypto.a
CFLAGS              := $(filter-out -ansi,$(CFLAGS))

ifneq (,$(filter -DMQTT_COMM_ENABLED,$(CFLAGS)))
mqtt_sample:
	$(eval CFLAGS := $(filter-out $(IOTSDK_INCLUDE_FILES),$(CFLAGS)) \
		-I$(TOP_DIR)/src/sdk-impl -I$(TOP_DIR)/src/sdk-impl/exports)

	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/mqtt/$@.c $(LDFLAGS) -o $@

	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/scenarized/door_$@.c $(LDFLAGS) -o door_$@

	$(TOP_Q) \
	mv door_$@ $(FINAL_DIR)/bin && \
	mv $@ $(FINAL_DIR)/bin
endif

ifneq (,$(filter -DMQTT_DEVICE_SHADOW,$(CFLAGS)))
shadow_sample:
	$(eval CFLAGS := $(filter-out $(IOTSDK_INCLUDE_FILES),$(CFLAGS)) \
		-I$(TOP_DIR)/src/sdk-impl -I$(TOP_DIR)/src/sdk-impl/exports)

	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/shadow/$@.c $(LDFLAGS) -o $@
	
	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/scenarized/aircond_$@.c $(LDFLAGS) -o aircond_$@

	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/scenarized/aircond_$@_v2.c $(LDFLAGS) -o aircond_$@_v2

	$(TOP_Q) \
	mv $@ $(FINAL_DIR)/bin && \
	mv aircond_$@ $(FINAL_DIR)/bin && \
	mv aircond_$@_v2 $(FINAL_DIR)/bin
endif

ifneq (,$(filter -DCOAP_COMM_ENABLED,$(CFLAGS)))
coap_sample:
	$(eval CFLAGS := $(filter-out $(IOTSDK_INCLUDE_FILES),$(CFLAGS)) \
		-I$(TOP_DIR)/src/sdk-impl -I$(TOP_DIR)/src/sdk-impl/exports)

	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/coap/$@.c $(LDFLAGS) -o $@

	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/scenarized/door_$@.c $(LDFLAGS) -o door_$@

	$(TOP_Q) \
	mv $@ $(FINAL_DIR)/bin

	$(TOP_Q) \
	mv door_$@ $(FINAL_DIR)/bin
endif

samples_final:
	$(eval CFLAGS := $(filter-out -I$(TOP_DIR)/src/sdk-impl,$(CFLAGS)) \
		 $(IOTSDK_INCLUDE_FILES))
	$(TOP_Q) \
	cp -rf $(TOP_DIR)/src/sdk-impl/qcloud_iot_*port.h $(FINAL_DIR)/include/

	$(TOP_Q) \
	cp -rf $(TOP_DIR)/src/sdk-impl/exports $(FINAL_DIR)/include/

	$(TOP_Q) \
	cp -rf $(TOP_DIR)/certs $(FINAL_DIR)/bin/