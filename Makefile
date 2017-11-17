#This target is to ensure accidental execution of Makefile as a bash script will not execute commands like rm in unexpected directories and exit gracefully.
.prevent_execution:
	exit 0

#Set this to @ to keep the makefile quiet
ifndef SILENCE
	SILENCE = @ 
endif

CC = gcc
RM = rm

DEBUG =

#--- Inputs ----#
COMPONENT_NAME = IotSdkC

ALL_TARGETS := build-cpputest
ALL_TARGETS_CLEAN :=

CPPUTEST_USE_EXTENSIONS = Y
CPP_PLATFORM = Gcc
CPPUTEST_CFLAGS += -std=gnu99
CPPUTEST_CFLAGS += -D__USE_BSD
CPPUTEST_USE_GCOV = Y

#IoT client directory
PROJECT_ROOT_DIR = .

APP_DIR = $(PROJECT_ROOT_DIR)/tests/unit
APP_NAME = qcloud_iot_sdk_unit_tests
APP_SRC_FILES = $(shell find $(APP_DIR)/src -name '*.cpp')
APP_SRC_FILES = $(shell find $(APP_DIR)/src -name '*.c')
APP_INCLUDE_DIRS = -I $(APP_DIR)/include

CPPUTEST_DIR = $(PROJECT_ROOT_DIR)/libs/CppUTest

# Provide paths for CppUTest to run Unit Tests otherwise build will fail
ifndef CPPUTEST_INCLUDE
    CPPUTEST_INCLUDE = $(CPPUTEST_DIR)/include
endif

ifndef CPPUTEST_BUILD_LIB
    CPPUTEST_BUILD_LIB = $(CPPUTEST_DIR)/lib
endif

CPPUTEST_LDFLAGS += -ldl $(CPPUTEST_BUILD_LIB)/libCppUTest.a

COMPILER_FLAGS += -DSTD_OUT

EXTERNAL_LIBS += -L$(CPPUTEST_BUILD_LIB)

#IoT client directory
COMMON_DIR = $(PROJECT_ROOT_DIR)/common

IOT_INCLUDE_DIRS = -I $(PROJECT_ROOT_DIR)/src/utils/include
IOT_INCLUDE_DIRS += -I $(PROJECT_ROOT_DIR)/src/http/include
IOT_INCLUDE_DIRS += -I $(PROJECT_ROOT_DIR)/src/shadow/include
IOT_INCLUDE_DIRS += -I $(PROJECT_ROOT_DIR)/src/mqtt/include
IOT_INCLUDE_DIRS += -I $(PROJECT_ROOT_DIR)/src/sdk-impl/
IOT_INCLUDE_DIRS += -I $(PROJECT_ROOT_DIR)/src/sdk-impl/imports/
IOT_INCLUDE_DIRS += -I $(PROJECT_ROOT_DIR)/src/sdk-impl/exports/
IOT_INCLUDE_DIRS += -I $(PROJECT_ROOT_DIR)/tests/unit/tls_mock
IOT_INCLUDE_DIRS += -I $(PROJECT_ROOT_DIR)/libs/jsmn
IOT_INCLUDE_DIRS += -I $(PROJECT_ROOT_DIR)/libs/hostmatch

IOT_SRC_FILES += $(shell find ./src/shadow/src/ -name '*.c')
IOT_SRC_FILES += $(shell find ./src/utils/src/ -name '*.c')
IOT_SRC_FILES += $(shell find ./src/http/src/ -name '*.c')
IOT_SRC_FILES += $(shell find ./src/mqtt/src/ -name '*.c')
IOT_SRC_FILES += $(shell find ./src/sdk-impl/ -name '*.c')
IOT_SRC_FILES += $(shell find ./src/common/src/ -name '*.c')
IOT_SRC_FILES += $(shell find ./libs/jsmn/ -name '*.c')
IOT_SRC_FILES += $(shell find ./libs/hostmatch/ -name '*.c')
IOT_SRC_FILES += $(shell find ./tests/unit/tls_mock/ -name '*.c')

#Aggregate all include and src directories
INCLUDE_DIRS += $(IOT_INCLUDE_DIRS)
INCLUDE_DIRS += $(APP_INCLUDE_DIRS)
INCLUDE_DIRS += $(CPPUTEST_INCLUDE)

TEST_SRC_DIRS = $(APP_DIR)/src

SRC_FILES += $(APP_SRC_FILES)
SRC_FILES += $(IOT_SRC_FILES)

COMPILER_FLAGS += -g
COMPILER_FLAGS += $(LOG_FLAGS)
PRE_MAKE_CMDS = cd $(CPPUTEST_DIR) &&
PRE_MAKE_CMDS += ./configure &&
PRE_MAKE_CMDS += make &&
PRE_MAKE_CMDS += cd - &&
PRE_MAKE_CMDS += pwd &&
PRE_MAKE_CMDS += cp -f $(CPPUTEST_DIR)/lib/libCppUTest.a $(CPPUTEST_DIR)/libCppUTest.a &&
PRE_MAKE_CMDS += cp -f $(CPPUTEST_DIR)/lib/libCppUTestExt.a $(CPPUTEST_DIR)/libCppUTestExt.a

ISYSTEM_HEADERS += $(IOT_ISYSTEM_HEADERS)
CPPUTEST_CPPFLAGS +=  $(ISYSTEM_HEADERS)
CPPUTEST_CPPFLAGS +=  $(LOG_FLAGS)

LCOV_EXCLUDE_PATTERN = "tests/unit/*"
LCOV_EXCLUDE_PATTERN += "tests/integration/*"
LCOV_EXCLUDE_PATTERN += "libs/*"

#use this section for running a specific group of tests, comment this to run all
#ONLY FOR TESTING PURPOSE
#COMMAND_LINE_ARGUMENTS += -g CommonTests
#COMMAND_LINE_ARGUMENTS += -v

build-cpputest:
	$(PRE_MAKE_CMDS)

include CppUTestMakefileWorker.mk

.PHONY: run-unit-tests
run-unit-tests: $(ALL_TARGETS)
	@echo $(ALL_TARGETS)

.PHONY: clean
clean:
	$(MAKE) -C $(CPPUTEST_DIR) clean
	$(RM) -rf build_output
	$(RM) -rf gcov
	$(RM) -rf objs
	$(RM) -rf testLibs
