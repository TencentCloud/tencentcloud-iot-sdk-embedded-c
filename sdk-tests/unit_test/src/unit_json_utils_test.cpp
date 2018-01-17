/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
 


#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <gtest/gtest.h>
#include <jsmn.h>
#include "qcloud_iot_export.h"
#include "qcloud_iot_utils_json.h"
#include "shadow_client_json.h"

static int rc;

static jsmn_parser test_parser;
static jsmntok_t t[128];


class JsonUtils : public testing::Test
{
protected:
    virtual void SetUp()
    {
        std::cout << "JsonUtils Test Begin \n";

        jsmn_init(&test_parser);
    }
    virtual void TearDown()
    {
        std::cout << "JsonUtils Test End \n";
    } 
	
};


TEST_F(JsonUtils, ParseStringBasic) {
    int r;
    const char *json = "{\"x\":\"test1\"}";
    char parsedString[50];

    Log_d("-->Running Json Utils Tests -  Basic String Parsing");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_string(parsedString, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_STREQ("test1", parsedString);
}

TEST_F(JsonUtils, ParseStringLongerStringIsValid) {
    int r;
    const char *json = "{\"x\":\"this is a longer string for test 2\"}";
    char parsedString[50];

    Log_d("\n-->Running Json Utils Tests - Parse long string \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_string(parsedString, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_STREQ("this is a longer string for test 2", parsedString);
}

TEST_F(JsonUtils, ParseStringEmptyStringIsValid) {
    int r;
    const char *json = "{\"x\":\"\"}";
    char parsedString[50];

    Log_d("\n-->Running Json Utils Tests - Parse empty string \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_string(parsedString, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_STREQ("", parsedString);
}

TEST_F(JsonUtils, ParseStringErrorOnInteger) {
    int r;
    const char *json = "{\"x\":3}";
    char parsedString[50];

    Log_d("\n-->Running Json Utils Tests - parse integer as string returns error \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_string(parsedString, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseStringErrorOnBoolean) {
    int r;
    const char *json = "{\"x\":true}";
    char parsedString[50];

    Log_d("\n-->Running Json Utils Tests - parse boolean as string returns error \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_string(parsedString, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseBooleanTrue) {
    int r;
    const char *json = "{\"x\":true}";
    bool parsedBool;

    Log_d("\n-->Running Json Utils Tests - parse boolean with true value \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_boolean(&parsedBool, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_EQ(1, (int) parsedBool);
}

TEST_F(JsonUtils, ParseBooleanFalse) {
    int r;
    const char *json = "{\"x\":false}";
    bool parsedBool;

    Log_d("\n-->Running Json Utils Tests - parse boolean with false value \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_boolean(&parsedBool, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_EQ(0, (int) parsedBool);
}

TEST_F(JsonUtils, ParseBooleanErrorOnString) {
    int r;
    const char *json = "{\"x\":\"not a bool\"}";
    bool parsedBool;

    Log_d("\n-->Running Json Utils Tests - parse string as boolean returns error \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_boolean(&parsedBool, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseBooleanErrorOnInvalidJson) {
    int r;
    const char *json = "{\"x\":frue}"; // Invalid
    bool parsedBool;

    Log_d("\n-->Running Json Utils Tests - parse boolean returns error with invalid json \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_boolean(&parsedBool, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseDoubleBasic) {
    int r;
    const char *json = "{\"x\":20.5}";
    double parsedDouble;

    Log_d("\n-->Running Json Utils Tests - Parse double test \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_double(&parsedDouble, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_NEAR(20.5, parsedDouble, 0.0);
}

TEST_F(JsonUtils, ParseDoubleNumberWithNoDecimal) {
    int r;
    const char *json = "{\"x\":100}";
    double parsedDouble;

    Log_d("\n-->Running Json Utils Tests - Parse double number with no decimal \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_double(&parsedDouble, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_NEAR(100, parsedDouble, 0.0);
}

TEST_F(JsonUtils, ParseDoubleSmallDouble) {
    int r;
    const char *json = "{\"x\":0.000004}";
    double parsedDouble;

    Log_d("\n-->Running Json Utils Tests - Parse small double value \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_double(&parsedDouble, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_NEAR(0.000004, parsedDouble, 0.0);
}

TEST_F(JsonUtils, ParseDoubleErrorOnString) {
    int r;
    const char *json = "{\"x\":\"20.5\"}";
    double parsedDouble;

    Log_d("\n-->Running Json Utils Tests - Parse string as double returns error \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_double(&parsedDouble, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseDoubleErrorOnBoolean) {
    int r;
    const char *json = "{\"x\":true}";
    double parsedDouble;

    Log_d("\n-->Running Json Utils Tests - Parse boolean as double returns error \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_double(&parsedDouble, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseDoubleErrorOnJsonObject) {
    int r;
    const char *json = "{\"x\":{}}";
    double parsedDouble;

    Log_d("\n-->Running Json Utils Tests - Parse json object as double returns error \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_double(&parsedDouble, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseDoubleNegativeNumber) {
    int r;
    const char *json = "{\"x\":-56.78}";
    double parsedDouble;

    Log_d("\n-->Running Json Utils Tests - Parse negative double value \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_double(&parsedDouble, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_NEAR(-56.78, parsedDouble, 0.0);
}

TEST_F(JsonUtils, ParseFloatBasic) {
    int r;
    const char *json = "{\"x\":20.5}";
    float parsedFloat;

    Log_d("\n-->Running Json Utils Tests - Parse float test \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_float(&parsedFloat, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_NEAR(20.5, parsedFloat, 0.0);
}

TEST_F(JsonUtils, ParseFloatNumberWithNoDecimal) {
    int r;
    const char *json = "{\"x\":100}";
    float parsedFloat;

    Log_d("\n-->Running Json Utils Tests - Parse float number with no decimal \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_float(&parsedFloat, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_NEAR(100, parsedFloat, 0.0);
}

TEST_F(JsonUtils, ParseFloatSmallFloat) {
    int r;
    const char *json = "{\"x\":0.0004}";
    float parsedFloat;

    Log_d("\n-->Running Json Utils Tests - Parse small float value \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_float(&parsedFloat, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_TRUE(0.0004f == parsedFloat);
}

TEST_F(JsonUtils, ParseFloatErrorOnString) {
    int r;
    const char *json = "{\"x\":\"20.5\"}";
    float parsedFloat;

    Log_d("\n-->Running Json Utils Tests - Parse string as float returns error \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_float(&parsedFloat, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseFloatErrorOnBoolean) {
    int r;
    const char *json = "{\"x\":true}";
    float parsedFloat;

    Log_d("\n-->Running Json Utils Tests - Parse boolean as float returns error \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_float(&parsedFloat, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseFloatErrorOnJsonObject) {
    int r;
    const char *json = "{\"x\":{}}";
    float parsedDouble;

    Log_d("\n-->Running Json Utils Tests - Parse json object as float returns error \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_float(&parsedDouble, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseFloatNegativeNumber) {
    int r;
    const char *json = "{\"x\":-56.78}";
    float parsedFloat;

    Log_d("\n-->Running Json Utils Tests - Parse negative float value \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_float(&parsedFloat, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_TRUE(-56.78f == parsedFloat);
}

TEST_F(JsonUtils, ParseIntegerBasic) {
    int r;
    const char *json = "{\"x\":1}";
    int32_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse 32 bit integer \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_int32(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_EQ(1, parsedInteger);
}

TEST_F(JsonUtils, ParseIntegerLargeInteger) {
    int r;
    const char *json = "{\"x\":2147483647}";
    int32_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse large 32 bit integer \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_int32(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_EQ(2147483647, parsedInteger);
}

TEST_F(JsonUtils, ParseIntegerNegativeInteger) {
    int r;
    const char *json = "{\"x\":-308}";
    int32_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse negative 32 bit integer \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_int32(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_EQ(-308, parsedInteger);
}

TEST_F(JsonUtils, ParseIntegerErrorOnBoolean) {
    int r;
    const char *json = "{\"x\":true}";
    int32_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse 32 bit integer returns error with boolean \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_int32(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseIntegerErrorOnString) {
    int r;
    const char *json = "{\"x\":\"45\"}";
    int32_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse 32 bit integer returns error on string \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_int32(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseInteger16bitBasic) {
    int r;
    const char *json = "{\"x\":1}";
    int16_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse 16 bit integer \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_int16(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_EQ(1, parsedInteger);
}

TEST_F(JsonUtils, ParseInteger16bitLargeInteger) {
    int r;
    const char *json = "{\"x\":32767}";
    int16_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse large 16 bit integer \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_int16(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_EQ(32767, parsedInteger);
}

TEST_F(JsonUtils, ParseInteger16bitNegativeInteger) {
    int r;
    const char *json = "{\"x\":-308}";
    int16_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse negative 16 bit integer \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_int16(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_EQ(-308, parsedInteger);
}

TEST_F(JsonUtils, ParseInteger16bitErrorOnBoolean) {
    int r;
    const char *json = "{\"x\":true}";
    int16_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse 16 bit integer returns error with boolean \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_int16(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseInteger16bitErrorOnString) {
    int r;
    const char *json = "{\"x\":\"45\"}";
    int16_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse 16 bit integer returns error on string \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_int16(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseInteger8bitBasic) {
    int r;
    const char *json = "{\"x\":1}";
    int8_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse 8 bit integer \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_int8(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_EQ(1, parsedInteger);
}

TEST_F(JsonUtils, ParseInteger8bitLargeInteger) {
    int r;
    const char *json = "{\"x\":127}";
    int8_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse large 8 bit integer \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_int8(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_EQ(127, parsedInteger);
}

TEST_F(JsonUtils, ParseInteger8bitNegativeInteger) {
    int r;
    const char *json = "{\"x\":-30}";
    int8_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse negative 8 bit integer \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_int8(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_EQ(-30, parsedInteger);
}

TEST_F(JsonUtils, ParseInteger8bitErrorOnBoolean) {
    int r;
    const char *json = "{\"x\":true}";
    int8_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse 8 bit integer returns error with boolean \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_int8(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseInteger8bitErrorOnString) {
    int r;
    const char *json = "{\"x\":\"45\"}";
    int8_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse 8 bit integer returns error on string \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_int8(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseUnsignedIntegerBasic) {
    int r;
    const char *json = "{\"x\":1}";
    uint32_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse unsigned 32 bit integer \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_uint32(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_TRUE(1 == parsedInteger);
}

TEST_F(JsonUtils, ParseUnsignedIntegerLargeInteger) {
    int r;
    const char *json = "{\"x\":2147483647}";
    uint32_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse large unsigned 32 bit integer \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_uint32(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_TRUE(2147483647 == parsedInteger);
}

TEST_F(JsonUtils, ParseUnsignedIntegerErrorOnNegativeInteger) {
    int r;
    const char *json = "{\"x\":-308}";
    uint32_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse unsigned 32 bit integer returns error with negative value \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_uint32(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseUnsignedIntegerErrorOnBoolean) {
    int r;
    const char *json = "{\"x\":true}";
    uint32_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse unsigned 32 bit integer returns error with boolean \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_uint32(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseUnsignedIntegerErrorOnString) {
    int r;
    const char *json = "{\"x\":\"45\"}";
    uint32_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse unsigned 32 bit integer returns error on string \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_uint32(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseUnsignedInteger16bitBasic) {
    int r;
    const char *json = "{\"x\":1}";
    uint16_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse unsigned 16 bit integer \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_uint16(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_EQ(1, parsedInteger);
}

TEST_F(JsonUtils, ParseUnsignedInteger16bitLargeInteger) {
    int r;
    const char *json = "{\"x\":65535}";
    uint16_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse large unsigned 16 bit integer \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_uint16(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_EQ(65535, parsedInteger);
}

TEST_F(JsonUtils, ParseUnsignedInteger16bitErrorOnNegativeInteger) {
    int r;
    const char *json = "{\"x\":-308}";
    uint16_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse unsigned 16 bit integer returns error on negative value \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_uint16(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseUnsignedInteger16bitErrorOnBoolean) {
    int r;
    const char *json = "{\"x\":true}";
    uint16_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse unsigned 16 bit integer returns error with boolean \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_uint16(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseUnsignedInteger16bitErrorOnString) {
    int r;
    const char *json = "{\"x\":\"45\"}";
    uint16_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse unsigned 16 bit integer returns error on string \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_uint16(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseUnsignedInteger8bitBasic) {
    int r;
    const char *json = "{\"x\":1}";
    uint8_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse unsigned 8 bit integer \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_uint8(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_EQ(1, parsedInteger);
}

TEST_F(JsonUtils, ParseUnsignedInteger8bitLargeInteger) {
    int r;
    const char *json = "{\"x\":255}";
    uint8_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse large unsigned 8 bit integer \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_uint8(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_EQ(255, parsedInteger);
}

TEST_F(JsonUtils, ParseUnsignedInteger8bitErrorOnNegativeInteger) {
    int r;
    const char *json = "{\"x\":-30}";
    uint8_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse unsigned 8 bit integer returns error on negative value \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_uint8(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseUnsignedInteger8bitErrorOnBoolean) {
    int r;
    const char *json = "{\"x\":true}";
    uint8_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse unsigned 8 bit integer returns error with boolean \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_uint8(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseUnsignedInteger8bitErrorOnString) {
    int r;
    const char *json = "{\"x\":\"45\"}";
    uint8_t parsedInteger;

    Log_d("\n-->Running Json Utils Tests - Parse unsigned 8 bit integer returns error on string \n");

    r = jsmn_parse(&test_parser, json, strlen(json), t, sizeof(t) / sizeof(t[0]));
    rc = get_uint8(&parsedInteger, json, t + 2);

    ASSERT_EQ(3, r);
    ASSERT_EQ(QCLOUD_ERR_JSON_PARSE, rc);
}

TEST_F(JsonUtils, ParseShadowOperation) {
    int rc;
    char type_str[20] = {0};
    char delta_str[100] = {0};
    int16_t result_code = 0;
    int32_t token_count = 0;
    const char *json = "{\"type\": \"get\",\"result\": 0,\"timestamp\": \"xxxxx\",\"payload\": {\"state\": {\"reported\": {\"attr_name1\": \"reported_name\"},\"desired\": {\"attr_name1\": \"desired_name\"},\"delta\":{\"attr_name1\": \"delta_name\"}}}}";

    rc = check_and_parse_json(json, &token_count, NULL);
    ASSERT_EQ(true, rc);

    rc = parse_shadow_operation_result_code(json, token_count, &result_code);
    ASSERT_EQ(true, rc);
    ASSERT_EQ(0, result_code);

    rc = parse_shadow_operation_type(json, token_count, type_str);
    ASSERT_EQ(true, rc);
    ASSERT_STREQ("get", type_str);

    rc = parse_shadow_operation_delta(json, token_count, delta_str);
    ASSERT_EQ(true, rc);
    ASSERT_STREQ("{\"attr_name1\": \"delta_name\"}", delta_str);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
