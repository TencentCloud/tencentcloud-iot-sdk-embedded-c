#include <CppUTest/TestHarness_c.h>
#include <CppUTest/UtestMacros.h>
#include <CppUTest/Utest.h>

TEST_GROUP_C_WRAPPER(CloudDeltaTest) {
    TEST_GROUP_C_SETUP_WRAPPER(CloudDeltaTest);
    TEST_GROUP_C_TEARDOWN_WRAPPER(CloudDeltaTest);
};

TEST_C_WRAPPER(CloudDeltaTest, registerDeltaSuccess)
TEST_C_WRAPPER(CloudDeltaTest, registerDeltaInt)
TEST_C_WRAPPER(CloudDeltaTest, registerDeltaIntNoCallback)
TEST_C_WRAPPER(CloudDeltaTest, DeltaNestedObject)
TEST_C_WRAPPER(CloudDeltaTest, DeltaVersionIgnoreOldVersion)
