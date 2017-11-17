
#include <CppUTest/TestHarness_c.h>
#include <CppUTest/UtestMacros.h>
#include <CppUTest/Utest.h>

TEST_GROUP_C_WRAPPER(UnsubscribeTests) {
        TEST_GROUP_C_SETUP_WRAPPER(UnsubscribeTests);
        TEST_GROUP_C_TEARDOWN_WRAPPER(UnsubscribeTests);
};

TEST_C_WRAPPER(UnsubscribeTests, UnsubscribeNullClient)
TEST_C_WRAPPER(UnsubscribeTests, UnsubscribeNullTopic)
TEST_C_WRAPPER(UnsubscribeTests, UnsubscribeNotSubscribed)
TEST_C_WRAPPER(UnsubscribeTests, unsubscribeQoS0FailureOnNoUnsuback)
TEST_C_WRAPPER(UnsubscribeTests, unsubscribeQoS1FailureOnNoUnsuback)
TEST_C_WRAPPER(UnsubscribeTests, unsubscribeQoS0WithUnsubackSuccess)
TEST_C_WRAPPER(UnsubscribeTests, unsubscribeQoS0WithDelayedUnsubackSuccess)
TEST_C_WRAPPER(UnsubscribeTests, unsubscribeQoS1WithUnsubackSuccess)
TEST_C_WRAPPER(UnsubscribeTests, unsubscribeQoS1WithDelayedUnsubackSuccess)
TEST_C_WRAPPER(UnsubscribeTests, MsgAfterUnsubscribe)
TEST_C_WRAPPER(UnsubscribeTests, MaxTopicsSubscription)
TEST_C_WRAPPER(UnsubscribeTests, RepeatedSubUnSub)

