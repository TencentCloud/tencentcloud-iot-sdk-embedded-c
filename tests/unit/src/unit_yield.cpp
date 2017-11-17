
#include <CppUTest/TestHarness_c.h>
#include <CppUTest/UtestMacros.h>
#include <CppUTest/Utest.h>

TEST_GROUP_C_WRAPPER(YieldTests) {
    TEST_GROUP_C_SETUP_WRAPPER(YieldTests);
    TEST_GROUP_C_TEARDOWN_WRAPPER(YieldTests);
};

TEST_C_WRAPPER(YieldTests, NullClientYield);
TEST_C_WRAPPER(YieldTests, ZeroTimeoutYield);
TEST_C_WRAPPER(YieldTests, YieldNetworkDisconnectedNeverConnected);
TEST_C_WRAPPER(YieldTests, YieldNetworkDisconnectedDisconnectedManually);
TEST_C_WRAPPER(YieldTests, YieldInSubscribeCallback);
TEST_C_WRAPPER(YieldTests, disconnectNoAutoReconnect);
TEST_C_WRAPPER(YieldTests, YieldSuccessNoMessages);
TEST_C_WRAPPER(YieldTests, PingRequestPingResponse);
TEST_C_WRAPPER(YieldTests, disconnectAutoReconnectTimeout);
TEST_C_WRAPPER(YieldTests, disconnectAutoReconnectSuccess);
TEST_C_WRAPPER(YieldTests, disconnectManualAutoReconnect);
TEST_C_WRAPPER(YieldTests, resubscribeSuccessfulReconnect);
