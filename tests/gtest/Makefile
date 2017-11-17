TARGET = run_test
CXX = g++
CXXFLAGS = -g -L/opt/gtest/lib -lgtest -lgtest_main -lpthread
GOOGLE_TEST_DIRS = ../../libs/googletest
SDK = ../..
SRC_DIR = ../../src

# 所有头文件目录
INCLUDE_DIRS = -I./
INCLUDE_DIRS += -I${SDK}/src/common/include
INCLUDE_DIRS += -I${SDK}/src/utils/include
INCLUDE_DIRS += -I${SDK}/src/http/include
INCLUDE_DIRS += -I${SDK}/src/shadow/include
INCLUDE_DIRS += -I${SDK}/src/mqtt/include
INCLUDE_DIRS += -I${SDK}/src/sdk-impl/
INCLUDE_DIRS += -I${SDK}/src/sdk-impl/exports/
INCLUDE_DIRS += -I${SDK}/src/sdk-impl/imports/
INCLUDE_DIRS += -I${SDK}/src/platform/$(PLATFORM)/os
INCLUDE_DIRS += -I${SDK}/src/platform/$(PLATFORM)/$(TLS_LIB)
INCLUDE_DIRS += -I${SDK}/libs/jsmn
INCLUDE_DIRS += -I./include/


.PHONY: clean lib

# 执行 make
all: ${TARGET}

${TARGET}:${TARGET}.cpp lib
	${CXX} $(INCLUDE_DIRS) -isystem  ${GOOGLE_TEST_DIRS}/include -pthread ${TARGET}.cpp ./src/util.cpp ${GOOGLE_TEST_DIRS}/libgtest.a ${SRC_DIR}/libiotsdk.a -o ${TARGET}

lib:
	make -C $(SRC_DIR)
	${CXX} -I${GOOGLE_TEST_DIRS}/include -I${GOOGLE_TEST_DIRS}/ -c ${GOOGLE_TEST_DIRS}/src/gtest-all.cc
	${CXX} -I${GOOGLE_TEST_DIRS}/include -I${GOOGLE_TEST_DIRS}/ -c ${GOOGLE_TEST_DIRS}/src/gtest_main.cc
	ar -rv ${GOOGLE_TEST_DIRS}/libgtest.a ${GOOGLE_TEST_DIRS}/gtest-all.o ${GOOGLE_TEST_DIRS}/gtest_main.o

clean:
	rm -rf ${TARGET} *.xml
	rm -rf ${GOOGLE_TEST_DIRS}/libgtest.a
	rm -rf ${GOOGLE_TEST_DIRS}/make/gtest_main.a
	make -C $(SRC_DIR) clean
