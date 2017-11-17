include ../../config.mk

CC = $(PLATFORM_CC)
CFLAGS = -Wall -g -DSTD_OUT

SDK_DIR = ../../src

# sample目录
SAMPLE_DIR = .
SAMPLE_NAME=airconditioner
SAMPLE_NAME_V2=airconditioner_v2

# 所有头文件目录
INCLUDE_DIRS = -I./
INCLUDE_DIRS += -I../../src/utils/include
INCLUDE_DIRS += -I../../src/http/include
INCLUDE_DIRS += -I../../src/shadow/include
INCLUDE_DIRS += -I../../src/mqtt/include
INCLUDE_DIRS += -I../../device/include/
INCLUDE_DIRS += -I../../src/sdk-impl/
INCLUDE_DIRS += -I../../src/sdk-impl/exports/
INCLUDE_DIRS += -I../../src/sdk-impl/imports/
INCLUDE_DIRS += -I../../src/platform/$(PLATFORM)/os
INCLUDE_DIRS += -I../../src/platform/$(PLATFORM)/$(TLS_LIB)
INCLUDE_DIRS += -I../../libs/jsmn

ifeq ($(TLS_LIB),mbedtls)
    MBEDTLS_PATH = ../../libs/mbedtls/
    TLS_LIB_DIR = ../../libs/mbedtls/library/
    INCLUDE_DIRS += -I../../libs/mbedtls/include/
else
    TLS_LIB_DIR = /usr/lib/
    INCLUDE_DIRS += -I /usr/include/openssl
endif

ifndef VERBOSE
.SILENT:
endif

.PHONY: clean lib

all: ${SAMPLE_NAME} ${SAMPLE_NAME_V2}

${SAMPLE_NAME}: lib
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -o $(SAMPLE_NAME) $(SAMPLE_NAME).c $(SDK_DIR)/libiotsdk.a
${SAMPLE_NAME_V2}: lib
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -o $(SAMPLE_NAME_V2) $(SAMPLE_NAME_V2).c $(SDK_DIR)/libiotsdk.a

lib:
	make -C $(SDK_DIR)

clean:
	rm -f $(SAMPLE_DIR)/$(SAMPLE_NAME)
	rm -f $(SAMPLE_DIR)/$(SAMPLE_NAME_V2)
	make -C $(SDK_DIR) clean
