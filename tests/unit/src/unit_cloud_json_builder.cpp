
#include <CppUTest/TestHarness_c.h>
#include <CppUTest/UtestMacros.h>
#include <CppUTest/Utest.h>
#include <CppUTest/CommandLineTestRunner.h>


TEST_GROUP_C_WRAPPER(CloudJsonBuilderTests) {
    TEST_GROUP_C_SETUP_WRAPPER(CloudJsonBuilderTests);
    TEST_GROUP_C_TEARDOWN_WRAPPER(CloudJsonBuilderTests);
};

TEST_C_WRAPPER(CloudJsonBuilderTests, BuildEmptyJson)

TEST_C_WRAPPER(CloudJsonBuilderTests, UpdateTheJSONDocumentBuilder)

TEST_C_WRAPPER(CloudJsonBuilderTests, PassingNullValue)

TEST_C_WRAPPER(CloudJsonBuilderTests, SmallBuffer)

TEST_C_WRAPPER(CloudJsonBuilderTests, CheckParseJson)

TEST_C_WRAPPER(CloudJsonBuilderTests, InvalidCheckParseJson)

TEST_C_WRAPPER(CloudJsonBuilderTests, UpdateValueIfKeyMatch)

TEST_C_WRAPPER(CloudJsonBuilderTests, ParseClientToken)

TEST_C_WRAPPER(CloudJsonBuilderTests, ParseVersionNum)

TEST_C_WRAPPER(CloudJsonBuilderTests, ParseErrorCode)

TEST_C_WRAPPER(CloudJsonBuilderTests, ParseMessage)


