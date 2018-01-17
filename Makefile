include make.settings
include src/configs/default_settings.mk
include src/scripts/parse_make_settings.mk

# IoT SDK sources files defination
COMP_LIB            := libiot_sdk.a
COMP_LIB_COMPONENTS := \
    src/utils/src \
    src/device/src \
    src/sdk-impl \

$(call CompLib_Map, MQTT_COMM_ENABLED, \
    src/mqtt/src \
    external_libs/jsmn \
)

$(call CompLib_Map, MQTT_DEVICE_SHADOW, \
	src/shadow/src \
)

$(call CompLib_Map, COAP_COMM_ENABLED, \
	src/coap/src \
)

IOTSDK_SRC_FILES := \

$(foreach v, \
    $(COMP_LIB_COMPONENTS), \
    $(eval \
    	export IOTSDK_SRC_FILES += \
    	$(wildcard $(TOP_DIR)/$(v)/*.c) \
    ) \
)

# IoT Platform sources files defination
PLATFORM_LIB		:= libiot_platform.a
PLATFORM_LIB_COMPONENTS := \
    src/platform/os/$(PLATFORM_OS) \
    src/platform/ssl/mbedtls
    
ifeq (,$(filter -DNOTLS_ENABLED,$(CFLAGS)))
	PLATFORM_LIB_COMPONENTS += \
    src/platform/$(PLATFORM_OS)/mbedtls
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
    src/utils/include \
    src/device/include \
    src/sdk-impl \
    src/sdk-impl/exports \
    external_libs/jsmn \
    external_libs/mbedtls/include \
    
$(call CompInc_Map, MQTT_COMM_ENABLED, \
    src/mqtt/include \
)
$(call CompInc_Map, MQTT_DEVICE_SHADOW, src/shadow/include)
$(call CompInc_Map, COAP_COMM_ENABLED, src/coap/include)
    
IOTSDK_INCLUDE_FILES := \

$(foreach v, \
    $(COMP_LIB_COMPONENTS_INCLUDES), \
    $(eval \
    	export IOTSDK_INCLUDE_FILES += \
    	-I$(TOP_DIR)/$(v) \
    ) \
)

CFLAGS += -Werror -Wall -Wno-error=sign-compare -Wno-error=format -Os ${IOTSDK_INCLUDE_FILES}

include src/scripts/rules.mk
include samples/samples.mk
include sdk-tests/unit_test/unit_test.mk
include sdk-tests/multi_thread_test/multi_thread_test.mk