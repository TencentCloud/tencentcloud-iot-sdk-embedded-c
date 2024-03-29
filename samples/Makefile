# Basic Settings
SHELL           := /bin/bash
TOP_DIR         ?= $(CURDIR)/../
SUBDIRS         := directory-not-exist-actually

# Settings of input directory
SCRIPT_DIR      := $(TOP_DIR)/tools/build_scripts

include $(TOP_DIR)/make.settings
include $(SCRIPT_DIR)/parse_make_settings.mk

# Makefile echo
ifeq ($(DEBUG_MAKEFILE),n)
    Q               := @
    TOP_Q           := @
else
    Q               :=
    TOP_Q           :=
endif

# Settings of output directory
SAMPLE_DIR		:= $(CURDIR)
FINAL_DIR       := $(CURDIR)/../output/$(BUILD_TYPE)

IOT_LIB_DIR = $(FINAL_DIR)/lib
IOT_INC_CFLAGS = -I$(FINAL_DIR)/include -I$(FINAL_DIR)/include/exports  -I$(CURDIR)/../sdk_src/internal_inc

LDFLAGS             := -Wl,--start-group $(IOT_LIB_DIR)/libiot_sdk.a

ifneq (,$(filter -DWEBSOCKET_CLIENT,$(CFLAGS)))
LDFLAGS             += $(IOT_LIB_DIR)/libwslay.a
endif

ifeq ($(FEATURE_AUTH_WITH_NOTLS),n)
LDFLAGS             += $(IOT_LIB_DIR)/libmbedtls.a $(IOT_LIB_DIR)/libmbedx509.a $(IOT_LIB_DIR)/libmbedcrypto.a
endif
LDFLAGS             += $(IOT_LIB_DIR)/libiot_platform.a -Wl,--end-group


CFLAGS += -Wall -Wno-error=sign-compare -Wno-error=format -Os -pthread -std=gnu99 -DFORCE_SSL_VERIFY
CFLAGS += ${IOT_INC_CFLAGS}

ifeq ($(FEATURE_AUTH_MODE),CERT)
CFLAGS += -DAUTH_MODE_CERT
endif

.PHONY: mqtt_sample ota_mqtt_sample ota_coap_sample shadow_sample coap_sample gateway_sample multi_thread_mqtt_sample \
			dynreg_dev_sample multi_client broadcast_sample rrpc_sample remote_config_mqtt_sample ota_mqtt_subdev_sample \
			resource_mqtt_sample

all: mqtt_sample ota_mqtt_sample ota_coap_sample shadow_sample coap_sample gateway_sample multi_thread_mqtt_sample \
			dynreg_dev_sample multi_client broadcast_sample rrpc_sample remote_config_mqtt_sample ota_mqtt_subdev_sample \
			resource_mqtt_sample

ifneq (,$(filter -DMQTT_COMM_ENABLED,$(CFLAGS)))
mqtt_sample:
	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/mqtt/$@.c $(LDFLAGS) -o $@

	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/scenarized/door_$@.c $(LDFLAGS) -o door_$@

	$(TOP_Q) \
	mv door_$@ $(FINAL_DIR)/bin && \
	mv $@ $(FINAL_DIR)/bin

# Multi-Thread Multi-Client sample
ifneq (,$(filter -DMULTITHREAD_ENABLED,$(CFLAGS)))
multi_thread_mqtt_sample:
	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/mqtt/$@.c $(LDFLAGS) -o $@

	$(TOP_Q) \
	mv $@ $(FINAL_DIR)/bin

ifeq (,$(filter -DDEBUG_DEV_INFO_USED,$(CFLAGS)))
multi_client:
	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/multi_client/$@_mqtt_sample.c $(LDFLAGS) -o $@_mqtt_sample

	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/multi_client/$@_shadow_sample.c $(LDFLAGS) -o $@_shadow_sample

	$(TOP_Q) \
	mv $@_mqtt_sample $(FINAL_DIR)/bin && \
    mv $@_shadow_sample $(FINAL_DIR)/bin
endif
endif

ifneq (,$(filter -DDEV_DYN_REG_ENABLED,$(CFLAGS)))
dynreg_dev_sample:
	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/dynreg_dev/$@.c $(LDFLAGS) -o $@

	$(TOP_Q) \
	mv $@ $(FINAL_DIR)/bin
endif

ifneq (,$(filter -DOTA_COMM_ENABLED,$(CFLAGS)))
ifneq (,$(filter -DOTA_MQTT_CHANNEL,$(CFLAGS)))
ota_mqtt_sample:
	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/ota/$@.c $(LDFLAGS) -o $@

	$(TOP_Q) \
	mv $@ $(FINAL_DIR)/bin
endif
endif

ifeq (y, $(strip $(FEATURE_RESOURCE_COMM_ENABLED)))
ifeq (MQTT, $(strip $(FEATURE_RESOURCE_SIGNAL_CHANNEL)))
resource_mqtt_sample:
	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/resource/$@.c $(LDFLAGS) -o $@

	$(TOP_Q) \
	mv $@ $(FINAL_DIR)/bin
endif
endif

ifneq (,$(filter -DOTA_COMM_ENABLED,$(CFLAGS)))
ifneq (,$(filter -DOTA_MQTT_CHANNEL,$(CFLAGS)))
ifneq (,$(filter -DGATEWAY_ENABLED,$(CFLAGS)))
ota_mqtt_subdev_sample:
	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/ota/$@.c $(LDFLAGS) -o $@

	$(TOP_Q) \
	mv $@ $(FINAL_DIR)/bin
endif
endif
endif

ifneq (,$(filter -DMQTT_DEVICE_SHADOW,$(CFLAGS)))
shadow_sample:
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


ifneq (,$(filter -DGATEWAY_ENABLED,$(CFLAGS)))
gateway_sample:
	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/gateway/$@.c $(LDFLAGS) -o $@

	$(TOP_Q) \
	mv $@ $(FINAL_DIR)/bin
endif
endif

ifneq (,$(filter -DCOAP_COMM_ENABLED,$(CFLAGS)))
coap_sample:
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
	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/ota/$@.c $(LDFLAGS) -o $@

	$(TOP_Q) \
	mv $@ $(FINAL_DIR)/bin
endif
endif

endif

ifneq (,$(filter -DBROADCAST_ENABLED,$(CFLAGS)))
broadcast_sample:
	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/broadcast/$@.c $(LDFLAGS) -o $@

	$(TOP_Q) \
	mv $@ $(FINAL_DIR)/bin
endif

ifneq (,$(filter -DRRPC_ENABLED,$(CFLAGS)))
rrpc_sample:
	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/rrpc/$@.c $(LDFLAGS) -o $@

	$(TOP_Q) \
	mv $@ $(FINAL_DIR)/bin
endif

ifneq (,$(filter -DREMOTE_CONFIG_MQTT_ENABLED,$(CFLAGS)))
remote_config_mqtt_sample:
	$(TOP_Q) \
	$(PLATFORM_CC) $(CFLAGS) $(SAMPLE_DIR)/config/$@.c $(LDFLAGS) -o $@

	$(TOP_Q) \
	mv $@ $(FINAL_DIR)/bin
endif

clean:
	rm -rf $(FINAL_DIR)/bin/*

