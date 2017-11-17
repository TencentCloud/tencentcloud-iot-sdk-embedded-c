#include <CppUTest/TestHarness_c.h>
#include <CppUTest/UtestMacros.h>
#include <CppUTest/Utest.h>
#include <CppUTest/CommandLineTestRunner.h>

TEST_GROUP_C_WRAPPER(PublishTests) {
    TEST_GROUP_C_SETUP_WRAPPER(PublishTests);
    TEST_GROUP_C_TEARDOWN_WRAPPER(PublishTests);
};

TEST_C_WRAPPER(PublishTests, PublishNullClient)
TEST_C_WRAPPER(PublishTests, PublishNullTopic)
TEST_C_WRAPPER(PublishTests, PublishNullParams)
TEST_C_WRAPPER(PublishTests, PublishNetworkDisconnected)
TEST_C_WRAPPER(PublishTests, publishQoS1FailureToReceivePuback)
TEST_C_WRAPPER(PublishTests, publishQoS1FailureDelayedPuback)
TEST_C_WRAPPER(PublishTests, publishQoS1Success10msDelayedPuback)
TEST_C_WRAPPER(PublishTests, publishQoS0NoPubackSuccess)
TEST_C_WRAPPER(PublishTests, publishQoS1Success)
