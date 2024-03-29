# Basic Settings
SHELL           := /bin/bash
TOP_DIR         ?= $(CURDIR)
SUBDIRS         := directory-not-exist-actually

# Settings of input directory
SCRIPT_DIR      := $(TOP_DIR)/tools/build_scripts
SRC_DIR         := sdk_src
PLATFORM_DIR    := platform
EXP_INC_DIR     := include include/exports
INT_INC_DIR     := sdk_src/internal_inc

# Thirdparty libs directory
THIRD_PARTY_PATH 	:= $(TOP_DIR)/external_libs
TEST_LIB_DIR        := $(THIRD_PARTY_PATH)/googletest

# Sample directory
SAMPLE_DIR		:= $(TOP_DIR)/samples

# Test directory
TESTS_DIR		:= $(TOP_DIR)/sdk-tests

include make.settings
include $(SCRIPT_DIR)/parse_make_settings.mk

# Settings of output directory
TEMP_DIR   		:= $(TOP_DIR)/tmp
DIST_DIR        := $(TOP_DIR)/output
FINAL_DIR       := $(DIST_DIR)/$(BUILD_TYPE)

# Makefile echo
ifeq ($(DEBUG_MAKEFILE),n)
    Q               := @
    TOP_Q           := @
else
    Q               :=
    TOP_Q           :=
endif

# IoT SDK sources files defination
COMP_LIB            := libiot_sdk.a
COMP_LIB_COMPONENTS := \
    $(SRC_DIR)/utils \
    $(SRC_DIR)/network \
	$(SRC_DIR)/network/socket \
    $(SRC_DIR)/protocol/http \

ifeq (,$(filter -DAUTH_WITH_NOTLS,$(CFLAGS)))
	COMP_LIB_COMPONENTS += \
    $(SRC_DIR)/network/tls
endif

ifneq (,$(filter -DWEBSOCKET_CLIENT,$(CFLAGS)))
	COMP_LIB_COMPONENTS += \
    $(SRC_DIR)/protocol/websocket
endif

$(call CompLib_Map, MQTT_COMM_ENABLED, \
    $(SRC_DIR)/protocol/mqtt \
)

$(call CompLib_Map, COAP_COMM_ENABLED, \
	$(SRC_DIR)/protocol/coap \
)

$(call CompLib_Map, MQTT_DEVICE_SHADOW, \
	$(SRC_DIR)/services/shadow \
)

$(call CompLib_Map, SYSTEM_COMM_ENABLED, \
    $(SRC_DIR)/services/system \
)

$(call CompLib_Map, SYSTEM_COMM_ENABLED, \
    $(SRC_DIR)/services/ssh \
)

$(call CompLib_Map, LOG_UPLOAD_ENABLED, \
    $(SRC_DIR)/services/log \
)

$(call CompLib_Map, OTA_COMM_ENABLED, \
	$(SRC_DIR)/services/ota \
)

$(call CompLib_Map, GATEWAY_ENABLED, \
	$(SRC_DIR)/services/gateway \
)

$(call CompLib_Map, DEV_DYN_REG_ENABLED, \
	$(SRC_DIR)/services/dynreg \
)

$(call CompLib_Map, EVENT_POST_ENABLED, \
	$(SRC_DIR)/services/event \
)

$(call CompLib_Map, AT_TCP_ENABLED, \
	$(SRC_DIR)/network/at_socket \
)

$(call CompLib_Map, BROADCAST_ENABLED, \
	$(SRC_DIR)/services/broadcast \
)

$(call CompLib_Map, RRPC_ENABLED, \
	$(SRC_DIR)/services/rrpc \
)

$(call CompLib_Map, REMOTE_CONFIG_MQTT_ENABLED, \
	$(SRC_DIR)/services/config \
)

ifeq (y, $(strip $(FEATURE_RESOURCE_COMM_ENABLED)))
ifeq (MQTT, $(strip $(FEATURE_RESOURCE_SIGNAL_CHANNEL)))
	COMP_LIB_COMPONENTS += \
    $(SRC_DIR)/services/resource
endif
endif


IOTSDK_SRC_FILES := \

$(foreach v, \
    $(COMP_LIB_COMPONENTS), \
    $(eval \
    	export IOTSDK_SRC_FILES += \
    	$(wildcard $(TOP_DIR)/$(v)/*.c) \
    ) \
)

ifeq (y, $(strip $(FEATURE_RESOURCE_COMM_ENABLED)))
ifeq (MQTT, $(strip $(FEATURE_RESOURCE_SIGNAL_CHANNEL)))
ifneq (y, $(strip $(FEATURE_OTA_COMM_ENABLED)))
	IOTSDK_SRC_FILES += $(SRC_DIR)/services/ota/ota_fetch.c
endif
endif
endif

EXCLUDE_SRC_FILES := \

ifeq (,$(filter -DOTA_COAP_CHANNEL, $(CFLAGS)))
EXCLUDE_SRC_FILES += $(TOP_DIR)/$(SRC_DIR)/services/ota/ota_coap.c
IOTSDK_SRC_FILES := $(filter-out $(EXCLUDE_SRC_FILES),$(IOTSDK_SRC_FILES))
else
EXCLUDE_SRC_FILES += $(TOP_DIR)/$(SRC_DIR)/services/ota/ota_mqtt.c
IOTSDK_SRC_FILES := $(filter-out $(EXCLUDE_SRC_FILES),$(IOTSDK_SRC_FILES))
endif

# IoT Platform sources files defination
PLATFORM_LIB		:= libiot_platform.a
PLATFORM_LIB_COMPONENTS := \
    $(PLATFORM_DIR)/os/$(PLATFORM_OS) \

ifeq (,$(filter -DAUTH_WITH_NOTLS,$(CFLAGS)))
	PLATFORM_LIB_COMPONENTS += \
    $(PLATFORM_DIR)/tls/$(PLATFORM_SSL)
endif

ifneq (,$(filter -DAT_TCP_ENABLED,$(CFLAGS)))
	PLATFORM_LIB_COMPONENTS += \
    $(PLATFORM_DIR)/at_device/$(PLATFORM_AT_DEV)
endif
   
IOTPLATFORM_SRC_FILES := \

$(foreach v, \
    $(PLATFORM_LIB_COMPONENTS), \
    $(eval \
    	export IOTPLATFORM_SRC_FILES += \
    	$(wildcard $(TOP_DIR)/$(v)/*.c) \
    ) \
)


# IoT Include files defination
COMP_LIB_COMPONENTS_INCLUDES := \
    $(EXP_INC_DIR) \
    $(INT_INC_DIR) \
    external_libs/mbedtls/include \
    external_libs/wslay/lib/includes \
    
IOTSDK_INCLUDE_FILES := \

$(foreach v, \
    $(COMP_LIB_COMPONENTS_INCLUDES), \
    $(eval \
    	export IOTSDK_INCLUDE_FILES += \
    	-I$(TOP_DIR)/$(v) \
    ) \
)

$(call CompInc_Map, AT_TCP_ENABLED, \
	 $(PLATFORM_DIR)/at_device/$(PLATFORM_AT_DEV) \
)

CFLAGS += -Werror -Wall -fPIC -Wno-error=sign-compare -Wno-error=format -Os ${IOTSDK_INCLUDE_FILES}  -std=gnu99 -pthread -DFORCE_SSL_VERIFY

#CFLAGS += -DMQTT_RMDUP_MSG_ENABLED

include $(SCRIPT_DIR)/rules.mk

ifneq (,$(filter -DSDKTESTS_ENABLED, $(CFLAGS)))
include sdk-tests/unit_test/unit_test.mk
include sdk-tests/multi_thread_test/multi_thread_test.mk
endif
