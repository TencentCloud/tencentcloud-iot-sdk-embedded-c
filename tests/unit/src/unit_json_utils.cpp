#include <CppUTest/TestHarness_c.h>
#include <CppUTest/UtestMacros.h>
#include <CppUTest/Utest.h>
#include <CppUTest/CommandLineTestRunner.h>

TEST_GROUP_C_WRAPPER(JsonUtils) {
        TEST_GROUP_C_SETUP_WRAPPER(JsonUtils);
        TEST_GROUP_C_TEARDOWN_WRAPPER(JsonUtils);
};

TEST_C_WRAPPER(JsonUtils, ParseStringBasic)
TEST_C_WRAPPER(JsonUtils, ParseStringLongerStringIsValid)
TEST_C_WRAPPER(JsonUtils, ParseStringEmptyStringIsValid)
TEST_C_WRAPPER(JsonUtils, ParseStringErrorOnInteger)
TEST_C_WRAPPER(JsonUtils, ParseStringErrorOnBoolean)

TEST_C_WRAPPER(JsonUtils, ParseBooleanTrue)
TEST_C_WRAPPER(JsonUtils, ParseBooleanFalse)
TEST_C_WRAPPER(JsonUtils, ParseBooleanErrorOnString)
TEST_C_WRAPPER(JsonUtils, ParseBooleanErrorOnInvalidJson)

TEST_C_WRAPPER(JsonUtils, ParseDoubleBasic)
TEST_C_WRAPPER(JsonUtils, ParseDoubleNumberWithNoDecimal)
TEST_C_WRAPPER(JsonUtils, ParseDoubleSmallDouble)
TEST_C_WRAPPER(JsonUtils, ParseDoubleErrorOnString)
TEST_C_WRAPPER(JsonUtils, ParseDoubleErrorOnBoolean)
TEST_C_WRAPPER(JsonUtils, ParseDoubleErrorOnJsonObject)
TEST_C_WRAPPER(JsonUtils, ParseDoubleNegativeNumber)

TEST_C_WRAPPER(JsonUtils, ParseFloatBasic)
TEST_C_WRAPPER(JsonUtils, ParseFloatNumberWithNoDecimal)
TEST_C_WRAPPER(JsonUtils, ParseFloatSmallFloat)
TEST_C_WRAPPER(JsonUtils, ParseFloatErrorOnString)
TEST_C_WRAPPER(JsonUtils, ParseFloatErrorOnBoolean)
TEST_C_WRAPPER(JsonUtils, ParseFloatErrorOnJsonObject)
TEST_C_WRAPPER(JsonUtils, ParseFloatNegativeNumber)

TEST_C_WRAPPER(JsonUtils, ParseIntegerBasic)
TEST_C_WRAPPER(JsonUtils, ParseIntegerLargeInteger)
TEST_C_WRAPPER(JsonUtils, ParseIntegerNegativeInteger)
TEST_C_WRAPPER(JsonUtils, ParseIntegerErrorOnBoolean)
TEST_C_WRAPPER(JsonUtils, ParseIntegerErrorOnString)

TEST_C_WRAPPER(JsonUtils, ParseInteger16bitBasic)
TEST_C_WRAPPER(JsonUtils, ParseInteger16bitLargeInteger)
TEST_C_WRAPPER(JsonUtils, ParseInteger16bitNegativeInteger)
TEST_C_WRAPPER(JsonUtils, ParseInteger16bitErrorOnBoolean)
TEST_C_WRAPPER(JsonUtils, ParseInteger16bitErrorOnString)

TEST_C_WRAPPER(JsonUtils, ParseInteger8bitBasic)
TEST_C_WRAPPER(JsonUtils, ParseInteger8bitLargeInteger)
TEST_C_WRAPPER(JsonUtils, ParseInteger8bitNegativeInteger)
TEST_C_WRAPPER(JsonUtils, ParseInteger8bitErrorOnBoolean)
TEST_C_WRAPPER(JsonUtils, ParseInteger8bitErrorOnString)

TEST_C_WRAPPER(JsonUtils, ParseUnsignedIntegerBasic)
TEST_C_WRAPPER(JsonUtils, ParseUnsignedIntegerLargeInteger)
TEST_C_WRAPPER(JsonUtils, ParseUnsignedIntegerErrorOnNegativeInteger)
TEST_C_WRAPPER(JsonUtils, ParseUnsignedIntegerErrorOnBoolean)
TEST_C_WRAPPER(JsonUtils, ParseUnsignedIntegerErrorOnString)

TEST_C_WRAPPER(JsonUtils, ParseUnsignedInteger16bitBasic)
TEST_C_WRAPPER(JsonUtils, ParseUnsignedInteger16bitLargeInteger)
TEST_C_WRAPPER(JsonUtils, ParseUnsignedInteger16bitErrorOnNegativeInteger)
TEST_C_WRAPPER(JsonUtils, ParseUnsignedInteger16bitErrorOnBoolean)
TEST_C_WRAPPER(JsonUtils, ParseUnsignedInteger16bitErrorOnString)

TEST_C_WRAPPER(JsonUtils, ParseUnsignedInteger8bitBasic)
TEST_C_WRAPPER(JsonUtils, ParseUnsignedInteger8bitLargeInteger)
TEST_C_WRAPPER(JsonUtils, ParseUnsignedInteger8bitErrorOnNegativeInteger)
TEST_C_WRAPPER(JsonUtils, ParseUnsignedInteger8bitErrorOnBoolean)
TEST_C_WRAPPER(JsonUtils, ParseUnsignedInteger8bitErrorOnString)
