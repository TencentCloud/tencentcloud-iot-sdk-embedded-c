# Basic Settings
#
SHELL           := /bin/bash
TOP_DIR         ?= $(CURDIR)

# Settings of input directory
#
CONFIG_DIR      := $(TOP_DIR)/src/configs
SCRIPT_DIR      := $(TOP_DIR)/src/scripts

# Settings of output directory
#
TEMP_DIR   		:= $(TOP_DIR)/tmp
DIST_DIR        := $(TOP_DIR)/output
FINAL_DIR       := $(DIST_DIR)/release

# Thirdparty libs directory
THIRD_PARTY_PATH 	:= $(TOP_DIR)/external_libs

# Sample directory
SAMPLE_DIR		:= $(TOP_DIR)/samples

# Test directory
TESTS_DIR		:= $(TOP_DIR)/sdk-tests

# Settings of makefile echo
#
Q               := @
TOP_Q           := @