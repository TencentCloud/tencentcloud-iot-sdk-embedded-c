#include <CppUTest/TestHarness_c.h>
#include <CppUTest/UtestMacros.h>
#include <CppUTest/Utest.h>

TEST_GROUP_C_WRAPPER(CloudNullFields) {
    TEST_GROUP_C_SETUP_WRAPPER(CloudNullFields);
    TEST_GROUP_C_TEARDOWN_WRAPPER(CloudNullFields);
};

TEST_C_WRAPPER(CloudNullFields, NullHost)
TEST_C_WRAPPER(CloudNullFields, NullPort)
TEST_C_WRAPPER(CloudNullFields, NullClientID)
TEST_C_WRAPPER(CloudNullFields, NullClientInit)
TEST_C_WRAPPER(CloudNullFields, NullClientConnect)
TEST_C_WRAPPER(CloudNullFields, NullUpdateDocument)
TEST_C_WRAPPER(CloudNullFields, NullClientYield)
TEST_C_WRAPPER(CloudNullFields, NullClientDisconnect)
TEST_C_WRAPPER(CloudNullFields, NullClientShadowGet)
TEST_C_WRAPPER(CloudNullFields, NullClientShadowUpdate)
TEST_C_WRAPPER(CloudNullFields, NullClientShadowDelete)





