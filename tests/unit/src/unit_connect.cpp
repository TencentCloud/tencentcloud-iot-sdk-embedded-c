#include <CppUTest/TestHarness_c.h>
#include <CppUTest/UtestMacros.h>
#include <CppUTest/Utest.h>
#include <CppUTest/CommandLineTestRunner.h>


TEST_GROUP_C_WRAPPER(ConnectTests) {
    TEST_GROUP_C_SETUP_WRAPPER(ConnectTests);
    TEST_GROUP_C_TEARDOWN_WRAPPER(ConnectTests);
};

TEST_C_WRAPPER(ConnectTests, nullClientInit);

TEST_C_WRAPPER(ConnectTests, nullParamsInit);

TEST_C_WRAPPER(ConnectTests, nullHost);

TEST_C_WRAPPER(ConnectTests, nullPort);

TEST_C_WRAPPER(ConnectTests, nullCA);

TEST_C_WRAPPER(ConnectTests, nullCert);

TEST_C_WRAPPER(ConnectTests, nullKeyFile);

TEST_C_WRAPPER(ConnectTests, nullClientConnect);

TEST_C_WRAPPER(ConnectTests, nullClientID);

TEST_C_WRAPPER(ConnectTests, InvalidEndpoint);

TEST_C_WRAPPER(ConnectTests, InvalidPort);

TEST_C_WRAPPER(ConnectTests, InvalidCA);

TEST_C_WRAPPER(ConnectTests, InvalidCert);

TEST_C_WRAPPER(ConnectTests, InvalidKey);

TEST_C_WRAPPER(ConnectTests, NoResponseTimeout);

TEST_C_WRAPPER(ConnectTests, ConnackTooLarge);

TEST_C_WRAPPER(ConnectTests, FixedHeaderCorrupted);

TEST_C_WRAPPER(ConnectTests, InvalidRemainLength);

TEST_C_WRAPPER(ConnectTests, UnacceptableProtocolVersion);

TEST_C_WRAPPER(ConnectTests, IndentifierRejected);

TEST_C_WRAPPER(ConnectTests, ServerUnavailable);

TEST_C_WRAPPER(ConnectTests, BadUserNameOrPassword);

TEST_C_WRAPPER(ConnectTests, NotAuthorized);

TEST_C_WRAPPER(ConnectTests, InvalidConnackReturnCode);

TEST_C_WRAPPER(ConnectTests, ConnectSuccess);

TEST_C_WRAPPER(ConnectTests, FlagSettingsAndParamsAreRecordedIntoBuf);

TEST_C_WRAPPER(ConnectTests, ConnectDisconnectConnect);

//TEST_C_WRAPPER(ConnectTests, cleanSessionInitSubscribers);
