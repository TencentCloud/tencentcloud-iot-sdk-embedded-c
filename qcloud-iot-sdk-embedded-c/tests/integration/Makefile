include ../../config.mk

CC = $(PLATFORM_CC)
CFLAGS = -Wall -g -DSTD_OUT

SDK_DIR = ../../src

SAMPLE_DIR = ./src/
SAMPLE_NAME=iot_test_integration_runner

# 所有头文件目录
INCLUDE_DIRS = -I./include
INCLUDE_DIRS += -I../../src/common/include
INCLUDE_DIRS += -I../../src/utils/include
INCLUDE_DIRS += -I../../src/http/include
INCLUDE_DIRS += -I../../src/mqtt/include
INCLUDE_DIRS += -I../../src/mqtt_wrapper/include
INCLUDE_DIRS += -I../../src/platform/$(PLATFORM)/common
INCLUDE_DIRS += -I../../src/platform/$(PLATFORM)/$(TLS_LIB)
INCLUDE_DIRS += -I../../src/shadow/include
INCLUDE_DIRS += -I../../libs/jsmn

ifeq ($(TLS_LIB),mbedtls)
    MBEDTLS_PATH = ../../libs/mbedtls/
    TLS_LIB_DIR = ../../libs/mbedtls/library/
    INCLUDE_DIRS += -I../../libs/mbedtls/include/
else
    TLS_LIB_DIR = /usr/lib/
    INCLUDE_DIRS += -I /usr/include/openssl
endif


SRC_FILES += $(shell find $(SAMPLE_DIR) -name '*.c')

.PHONY: clean lib

all: ${SAMPLE_NAME}

${SAMPLE_NAME}: lib
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -o $(SAMPLE_NAME) $(SRC_FILES) $(SDK_DIR)/libiotsdk.a

lib:
	make -C $(SDK_DIR)

clean:
	rm -f $(SAMPLE_NAME)
	make -C $(SDK_DIR) clean
