include make.settings
include src/configs/default_settings.mk
include src/scripts/parse_make_settings.mk

# IoT SDK sources files defination
COMP_LIB            := libiot_sdk.a
COMP_LIB_COMPONENTS := \
    src/utils/digest \
    src/utils/farra \
    src/utils/lite \
    src/device/src \
    src/sdk-impl \

$(call CompLib_Map, MQTT_COMM_ENABLED, \
    src/mqtt/src \
)

$(call CompLib_Map, MQTT_COMM_ENABLED, \
    src/system/src \
)

$(call CompLib_Map, MQTT_DEVICE_SHADOW, \
	src/shadow/src \
)

$(call CompLib_Map, COAP_COMM_ENABLED, \
	src/coap/src \
)

$(call CompLib_Map, OTA_COMM_ENABLED, \
	src/ota/src \
)

$(call CompLib_Map, NBIOT_COMM_ENABLED, \
	src/nbiot/src \
)

$(call CompLib_Map, GATEWAY_ENABLED, \
	src/gateway/src \
)

$(call CompLib_Map, DEV_DYN_REG_ENABLED, \
	src/dynreg/src \
)

$(call CompLib_Map, EVENT_POST_ENABLED, \
	src/event/src \
)

IOTSDK_SRC_FILES := \

$(foreach v, \
    $(COMP_LIB_COMPONENTS), \
    $(eval \
    	export IOTSDK_SRC_FILES += \
    	$(wildcard $(TOP_DIR)/$(v)/*.c) \
    ) \
)

EXCLUDE_SRC_FILES := \

ifeq (,$(filter -DOTA_COAP_CHANNEL, $(CFLAGS)))
EXCLUDE_SRC_FILES += $(TOP_DIR)/src/ota/src/ota_coap.c
IOTSDK_SRC_FILES := $(filter-out $(EXCLUDE_SRC_FILES),$(IOTSDK_SRC_FILES))
else
EXCLUDE_SRC_FILES += $(TOP_DIR)/src/ota/src/ota_mqtt.c
IOTSDK_SRC_FILES := $(filter-out $(EXCLUDE_SRC_FILES),$(IOTSDK_SRC_FILES))
endif

# IoT Platform sources files defination
PLATFORM_LIB		:= libiot_platform.a
PLATFORM_LIB_COMPONENTS := \
    src/platform/os/$(PLATFORM_OS) \
    
ifeq (,$(filter -DAUTH_WITH_NOTLS,$(CFLAGS)))
	PLATFORM_LIB_COMPONENTS += \
    src/platform/ssl/$(PLATFORM_SSL)
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
    src/utils/digest \
    src/utils/farra \
    src/utils/lite \
    src/device/include \
    src/sdk-impl \
    src/sdk-impl/exports \
    external_libs/mbedtls/include \
    src/system/include \
    
$(call CompInc_Map, MQTT_COMM_ENABLED, \
    src/mqtt/include \
)

$(call CompInc_Map, MQTT_DEVICE_SHADOW, \
	src/shadow/include \
)

$(call CompInc_Map, COAP_COMM_ENABLED, \
	src/coap/include \
)

$(call CompInc_Map, OTA_COMM_ENABLED, \
	src/ota/include \
)

$(call CompInc_Map, NBIOT_COMM_ENABLED, \
	src/nbiot/include \
)

$(call CompInc_Map, GATEWAY_ENABLED, \
	src/gateway/include \
)

$(call CompInc_Map, DEV_DYN_REG_ENABLED, \
	src/dynreg/include \
)

$(call CompInc_Map, EVENT_POST_ENABLED, \
	src/event/include \
)
    
IOTSDK_INCLUDE_FILES := \

$(foreach v, \
    $(COMP_LIB_COMPONENTS_INCLUDES), \
    $(eval \
    	export IOTSDK_INCLUDE_FILES += \
    	-I$(TOP_DIR)/$(v) \
    ) \
)

CFLAGS += -Werror -Wall -Wno-error=sign-compare -Wno-error=format -Os ${IOTSDK_INCLUDE_FILES} -pthread

include src/scripts/rules.mk
include samples/samples.mk

ifneq (,$(filter -DSDKTESTS_ENABLED, $(CFLAGS)))
include sdk-tests/unit_test/unit_test.mk
include sdk-tests/multi_thread_test/multi_thread_test.mk
endif
