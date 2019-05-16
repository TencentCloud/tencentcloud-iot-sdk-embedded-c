DEPENDS             := src/platform
LDFLAGS             := $(FINAL_DIR)/lib/libiot_sdk.a
LDFLAGS             += $(FINAL_DIR)/lib/libiot_platform.a
ifeq (,$(filter -DAUTH_WITH_NOTLS,$(CFLAGS)))
LDFLAGS             += $(FINAL_DIR)/lib/libmbedtls.a $(FINAL_DIR)/lib/libmbedx509.a $(FINAL_DIR)/lib/libmbedcrypto.a
endif
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

ifneq (,$(filter -DMULTITHREAD_TEST_ENABLED,$(CFLAGS)))
multi_thread_mqtt_sample:
	$(eval CFLAGS := $(filter-out $(IOTSDK_INCLUDE_FILES),$(CFLAGS)) \
		-I$(TOP_DIR)/src/sdk-impl -I$(TOP_DIR)/src/sdk-impl/exports -pthread)

	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/mqtt/$@.c $(LDFLAGS) -o $@

	$(TOP_Q) \
	mv $@ $(FINAL_DIR)/bin
endif

ifneq (,$(filter -DDEV_DYN_REG_ENABLED,$(CFLAGS)))
dynamic_reg_dev_sample:
	$(eval CFLAGS := $(filter-out $(IOTSDK_INCLUDE_FILES),$(CFLAGS)) \
		-I$(TOP_DIR)/src/sdk-impl -I$(TOP_DIR)/src/sdk-impl/exports)

	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/dynamic_reg_dev/$@.c $(LDFLAGS) -o $@

	$(TOP_Q) \
	mv $@ $(FINAL_DIR)/bin
endif

ifneq (,$(filter -DOTA_COMM_ENABLED,$(CFLAGS)))
ifneq (,$(filter -DOTA_MQTT_CHANNEL,$(CFLAGS)))
ota_mqtt_sample:
	$(eval CFLAGS := $(filter-out $(IOTSDK_INCLUDE_FILES),$(CFLAGS)) \
		-I$(TOP_DIR)/src/sdk-impl -I$(TOP_DIR)/src/sdk-impl/exports)

	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/ota/$@.c $(LDFLAGS) -o $@

	$(TOP_Q) \
	mv $@ $(FINAL_DIR)/bin
endif
endif
endif

ifneq (,$(filter -DMQTT_DEVICE_SHADOW,$(CFLAGS)))
shadow_sample:
	$(eval CFLAGS := $(filter-out $(IOTSDK_INCLUDE_FILES),$(CFLAGS)) \
		-I$(TOP_DIR)/src/sdk-impl -I$(TOP_DIR)/src/sdk-impl/exports -I$(TOP_DIR)/src/utils/lite -I$(TOP_DIR)/src/utils/farra)

	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/shadow/$@.c $(LDFLAGS) -o $@
	
	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/data_template/data_template_sample.c $(LDFLAGS) -o data_template_sample
	
	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/scenarized/light_data_template_sample.c $(LDFLAGS) -o light_data_template_sample
	
	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/scenarized/aircond_$@.c $(LDFLAGS) -o aircond_$@

	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/scenarized/aircond_$@_v2.c $(LDFLAGS) -o aircond_$@_v2
	
	$(TOP_Q) \
	mv $@ $(FINAL_DIR)/bin && \
	mv data_template_sample $(FINAL_DIR)/bin && \
	mv light_data_template_sample $(FINAL_DIR)/bin && \
	mv aircond_$@ $(FINAL_DIR)/bin && \
	mv aircond_$@_v2 $(FINAL_DIR)/bin
endif

ifneq (,$(filter -DEVENT_POST_ENABLED,$(CFLAGS)))
event_sample:
	$(eval CFLAGS := $(filter-out $(IOTSDK_INCLUDE_FILES),$(CFLAGS)) \
		-I$(TOP_DIR)/src/sdk-impl -I$(TOP_DIR)/src/sdk-impl/exports)
		
	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/event/$@.c $(LDFLAGS) -o $@	

	
	$(TOP_Q) \
	mv $@ $(FINAL_DIR)/bin
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
ifneq (,$(filter -DOTA_COMM_ENABLED,$(CFLAGS)))
ifneq (,$(filter -DOTA_COAP_CHANNEL,$(CFLAGS)))
ota_coap_sample:
	$(eval CFLAGS := $(filter-out $(IOTSDK_INCLUDE_FILES),$(CFLAGS)) \
		-I$(TOP_DIR)/src/sdk-impl -I$(TOP_DIR)/src/sdk-impl/exports)

	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/ota/$@.c $(LDFLAGS) -o $@

	$(TOP_Q) \
	mv $@ $(FINAL_DIR)/bin
endif
endif
endif

ifneq (,$(filter -DNBIOT_COMM_ENABLED,$(CFLAGS)))
nbiot_sample:
	$(eval CFLAGS := $(filter-out $(IOTSDK_INCLUDE_FILES),$(CFLAGS)) \
		-I$(TOP_DIR)/src/sdk-impl -I$(TOP_DIR)/src/sdk-impl/exports)

	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/nbiot/$@.c $(LDFLAGS) -o $@

	$(TOP_Q) \
	mv $@ $(FINAL_DIR)/bin
endif

ifneq (,$(filter -DGATEWAY_ENABLED,$(CFLAGS)))
gateway_sample:
	$(eval CFLAGS := $(filter-out $(IOTSDK_INCLUDE_FILES),$(CFLAGS)) \
		-I$(TOP_DIR)/src/sdk-impl -I$(TOP_DIR)/src/sdk-impl/exports)

	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/gateway/$@.c $(LDFLAGS) -o $@

	$(TOP_Q) \
	mv $@ $(FINAL_DIR)/bin
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
