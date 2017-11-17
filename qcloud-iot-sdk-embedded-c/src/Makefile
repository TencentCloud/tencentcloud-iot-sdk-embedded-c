include ../config.mk

PROJECT_ROOT_PATH = ../

BUILD_OUT_PATH = $(PROJECT_ROOT_PATH)/build/
LIB_STATIC = $(IOT_SDK_LIB_NAME).a
CC = $(PLATFORM_CC)
AR = $(PLATFORM_AR)

#path of include files
INCLUDE_DIRS += -I./utils/include/
INCLUDE_DIRS += -I./http/include/
INCLUDE_DIRS += -I./mqtt/include/
INCLUDE_DIRS += -I./device/include/
INCLUDE_DIRS += -I./platform/$(PLATFORM)/$(TLS_LIB)
INCLUDE_DIRS += -I./sdk-impl/
INCLUDE_DIRS += -I./sdk-impl/exports/
INCLUDE_DIRS += -I./sdk-impl/imports/

#path of src files
CLOUD_SRC_PATH = ./shadow/src/
MQTT_SRC_PATH = ./mqtt/src/
DEVICE_SRC_PATH = ./device/src/
UTILS_SRC_PATH = ./utils/src/
HTTP_SRC_PATH = ./http/src/
TLS_SRC_PATH = ./platform/$(PLATFORM)/$(TLS_LIB)/
OS_SRC_PATH = ./platform/$(PLATFORM)/os/
JSMN_SRC_PATH = ../libs/jsmn/
IMPL_SRC_PATH = ./sdk-impl/

CLOUD_SRC = $(wildcard $(CLOUD_SRC_PATH))
MQTT_SRC = $(wildcard $(MQTT_SRC_PATH))
DEVICE_SRC = $(wildcard $(DEVICE_SRC_PATH))
TLS_SRC = $(wildcard $(TLS_SRC_PATH))
OS_SRC = $(wildcard $(OS_SRC_PATH))
JSMN_SRC = $(wildcard $(JSMN_SRC_PATH))
UTILS_SRC = $(wildcard $(UTILS_SRC_PATH))
HTTP_SRC = $(wildcard $(HTTP_SRC_PATH))
IMPL_SRC = $(wildcard $(IMPL_SRC_PATH))

SRC_FILES = $(wildcard $(MQTT_SRC)*.c)
SRC_FILES += $(wildcard $(DEVICE_SRC)*.c)
SRC_FILES += $(wildcard $(IMPL_SRC)*.c)
SRC_FILES += $(wildcard $(TLS_SRC)*.c)
SRC_FILES += $(wildcard $(OS_SRC)*.c)
SRC_FILES += $(wildcard $(UTILS_SRC)*.c)
SRC_FILES += $(wildcard $(HTTP_SRC)*.c)

ifeq ($(ENABLE_SHADOW),y)
INCLUDE_DIRS += -I./shadow/include/
INCLUDE_DIRS += -I../libs/jsmn/

SRC_FILES += $(wildcard $(CLOUD_SRC)*.c)
SRC_FILES += $(wildcard $(JSMN_SRC)*.c)
endif

objects = $(patsubst %.c,%.o, $(SRC_FILES))

ifeq ($(TLS_LIB),mbedtls)
    MBEDTLS_PATH = ../libs/mbedtls/
    TLS_LIB_DIR = ../libs/mbedtls/library/
    INCLUDE_DIRS += -I../libs/mbedtls/include/
else
    TLS_LIB_DIR = /usr/lib/
    INCLUDE_DIRS += -I /usr/include/openssl
endif

ifndef VERBOSE
.SILENT:
endif

.PHONY: clean libs

all: ${LIB_STATIC}

${LIB_STATIC}: libs ${objects}
	$(AR) rcs $@ $(objects) $(BUILD_OUT_PATH)*.o
	-rm ${objects}
	-rm -rf ${BUILD_OUT_PATH}

libs:
	mkdir -p ${BUILD_OUT_PATH}
ifeq ($(TLS_LIB),mbedtls)
	make -C $(MBEDTLS_PATH) lib -e CC=$(PLATFORM_CC) AR=$(PLATFORM_AR)
	cp -RP $(TLS_LIB_DIR)libmbedtls.*   $(BUILD_OUT_PATH)
	cp -RP $(TLS_LIB_DIR)libmbedx509.*  $(BUILD_OUT_PATH)
	cp -RP $(TLS_LIB_DIR)libmbedcrypto.* $(BUILD_OUT_PATH)
	cd $(BUILD_OUT_PATH) && $(AR) x libmbedtls.a
	cd $(BUILD_OUT_PATH) && $(AR) x libmbedx509.a
	cd $(BUILD_OUT_PATH) && $(AR) x libmbedcrypto.a
else
	cp -RP $(TLS_LIB_DIR)libcrypto.*    $(BUILD_OUT_PATH)
	cp -RP $(TLS_LIB_DIR)libssl.*   $(BUILD_OUT_PATH)
	cd $(BUID_OUT_PATH) && $(AR) x libcrypto.a
	cd $(BUILD_OUT_PATH) && $(AR) x libssl.a
endif

CFLAGS += -Wall -O2 ${INCLUDE_DIRS}
CFLAGS += -DSTD_OUT

ifeq ($(ENABLE_TRACE),y)
CFLAGS += -DIOT_SDK_TRACE
endif

${objects}:%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	-rm -rf ${BUILD_OUT_PATH}
	-rm ${LIB_STATIC}
ifeq ($(TLS_LIB),mbedtls)
	-make -C ${MBEDTLS_PATH} clean
endif
