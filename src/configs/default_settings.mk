SUBDIRS                := directory-not-exist-actually

PLATFORM_CC            ?= gcc
PLATFORM_AR            ?= ar
PLATFORM_OS            ?= linux

FEATURE_MQTT_COMM_ENABLED   		?= y
FEATURE_MQTT_DEVICE_SHADOW  		?= $(FEATURE_MQTT_COMM_ENABLED)
FEATURE_COAP_COMM_ENABLED			?= n
FEATURE_OTA_COMM_ENABLED			?= n

FEATURE_SDKTESTS_ENABLED			?= n	#是否开启单元测试编译	仅支持linux
FEATURE_MQTT_RMDUP_MSG_ENABLED		?= n	#是否开启MQTT消息去重能力

CFLAGS  += -DFORCE_SSL_VERIFY

DEBUG_MAKEFILE         = n