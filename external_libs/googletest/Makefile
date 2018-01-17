include ../../make.settings

CXX = g++
AR = $(PLATFORM_AR)

TARGET = libgtest.a
GOOGLE_TEST_DIR = .


.PHONY: clean lib

all: lib

lib: 
	@ \
	$(CXX) -I$(GOOGLE_TEST_DIR)/include -I$(GOOGLE_TEST_DIR)/ -c $(GOOGLE_TEST_DIR)/src/gtest-all.cc
	@ \
	$(CXX) -I$(GOOGLE_TEST_DIR)/include -I$(GOOGLE_TEST_DIR)/ -c $(GOOGLE_TEST_DIR)/src/gtest_main.cc
	
	@ \
	$(AR) -rc $(GOOGLE_TEST_DIR)/${TARGET} $(GOOGLE_TEST_DIR)/gtest-all.o $(GOOGLE_TEST_DIR)/gtest_main.o
	
	@ \
	$(AR) -s $(GOOGLE_TEST_DIR)/${TARGET}
	
	@rm -f $(GOOGLE_TEST_DIR)/gtest-all.o $(GOOGLE_TEST_DIR)/gtest_main.o

clean:
	@rm -rf $(GOOGLE_TEST_DIR)/$(TARGET)