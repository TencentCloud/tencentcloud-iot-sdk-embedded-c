SUBDIRS                := directory-not-exist-actually

PLATFORM_CC            ?= gcc
PLATFORM_AR            ?= ar
PLATFORM_OS            ?= linux

FEATURE_MQTT_COMM_ENABLED   		?= y
FEATURE_MQTT_DEVICE_SHADOW  		?= $(FEATURE_MQTT_COMM_ENABLED)
FEATURE_COAP_COMM_ENABLED			?= n
FEATURE_NBIOT_COMM_ENABLED			?= n
FEATURE_OTA_COMM_ENABLED			?= n
FEATURE_LOG_UPLOAD_ENABLED          ?= y     # 是否打开日志上报云端功能
FEATURE_SYSTEM_COMM_ENABLED         ?= y     # 是否打开系统通讯（获取iot后台时间等）功能

FEATURE_SDKTESTS_ENABLED			?= n	#是否开启单元测试编译	仅支持linux
FEATURE_MQTT_RMDUP_MSG_ENABLED		?= n	#是否开启MQTT消息去重能力
FEATURE_GATEWAY_ENABLED 		    ?= n    #是否打开网关功能
FEATURE_MULTITHREAD_TEST_ENABLED    ?= n    #是否编译多线程测试例程

CFLAGS  += -DFORCE_SSL_VERIFY

DEBUG_MAKEFILE         = n
