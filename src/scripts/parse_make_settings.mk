include $(CURDIR)/src/scripts/internal_make_funcs.mk

SETTING_VARS := \
    BUILD_TYPE \
    PLATFORM_CC \
    PLATFORM_AR \
    PLATFORM_OS \

SWITCH_VARS := \
    FEATURE_MQTT_COMM_ENABLED \
    FEATURE_COAP_COMM_ENABLED \
    FEATURE_MQTT_DEVICE_SHADOW \
    FEATURE_SDKTESTS_ENABLED \
    FEATURE_NOTLS_ENABLED \
    FEATURE_MQTT_RMDUP_MSG_ENABLED \

$(foreach v, \
    $(SETTING_VARS) $(SWITCH_VARS), \
    $(eval export $(v)=$($(v))) \
)

$(foreach v, \
    $(SWITCH_VARS), \
    $(if $(filter y,$($(v))), \
        $(eval CFLAGS += -D$(subst FEATURE_,,$(v)))) \
)

include $(CURDIR)/src/configs/settings.mk

ifeq (debug,$(strip $(BUILD_TYPE)))
CFLAGS  += -DIOT_DEBUG
endif

ifneq (y,$(strip $(FEATURE_MQTT_COMM_ENABLED)))
$(error MQTT required to be y!)
endif