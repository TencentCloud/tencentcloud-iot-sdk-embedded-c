

#include <CppUTest/TestHarness_c.h>
#include <CppUTest/UtestMacros.h>
#include <CppUTest/Utest.h>

TEST_GROUP_C_WRAPPER(DisconnectTests) {
    TEST_GROUP_C_SETUP_WRAPPER(DisconnectTests);
    TEST_GROUP_C_TEARDOWN_WRAPPER(DisconnectTests);
};

TEST_C_WRAPPER(DisconnectTests, NullClientDisconnect)
TEST_C_WRAPPER(DisconnectTests, NullClientSetDisconnectHandler)
TEST_C_WRAPPER(DisconnectTests, SetDisconnectHandlerNullHandler)
TEST_C_WRAPPER(DisconnectTests, disconnectNotConnected)
TEST_C_WRAPPER(DisconnectTests, disconnectNoAckSuccess)
TEST_C_WRAPPER(DisconnectTests, HandlerInvokedOnDisconnect)
TEST_C_WRAPPER(DisconnectTests, SetHandlerAndInvokedOnDisconnect)
