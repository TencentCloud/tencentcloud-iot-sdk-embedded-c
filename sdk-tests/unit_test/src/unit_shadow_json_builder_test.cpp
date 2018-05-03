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
#include <string.h>

#include <gtest/gtest.h>

#include "shadow_client.h"
#include "shadow_client_json.h"
#include "unit_helper_functions.h"
#include "lite-utils.h"

static DeviceProperty dataFloatHandler;
static DeviceProperty dataDoubleHandler;
static DeviceProperty dataIntegerHandler;

static int integerData = 1;
static double doubleData = 4.0908f;
static float floatData = 3.445f;

static ShadowInitParams     g_initParams;
static Qcloud_IoT_Shadow 	*g_shadow_ptr;

using namespace std;

class ShadowJsonBuilderTests : public testing::Test
{
protected:
    virtual void SetUp()
    {
        std::cout << "ShadowJsonBuilderTests Test Begin \n";
        SetupShadowConnectInitParams(&g_initParams);
        void *client = IOT_Shadow_Construct(&g_initParams);
        g_shadow_ptr = (Qcloud_IoT_Shadow *)client;

        dataFloatHandler.data = &floatData;
	    dataFloatHandler.key = (char*)"floatData";
	    dataFloatHandler.type = JFLOAT;
	    
	    dataDoubleHandler.data = &doubleData;
	    dataDoubleHandler.key = (char*)"doubleData";
	    dataDoubleHandler.type = JDOUBLE;

	    dataIntegerHandler.data = &integerData;
	    dataIntegerHandler.key = (char*)"integerData";
	    dataIntegerHandler.type = JINT32;
    }
    virtual void TearDown()
    {
        std::cout << "ShadowJsonBuilderTests Test End \n";
        IOT_Shadow_Destroy(g_shadow_ptr);
    }  
};

#define TEST_JSON_UPDATE_DOCUMENT "{\"state\":{\"reported\":{\"doubleData\":4.090800,\"floatData\":3.445000}}, \"clientToken\":\"" QCLOUD_IOT_MY_PRODUCT_ID "-0\"}"
#define TEST_JSON_EMPTY_GET_DELETE_DOCUMENT "{\"clientToken\":\"" QCLOUD_IOT_MY_PRODUCT_ID "-0\"}"

#define TEST_JSON_RESPONSE_UPDATE_DOCUMENT "{\"state\":{\"reported\":{\"integerData\":24}}, \"clientToken\":\"" QCLOUD_IOT_MY_PRODUCT_ID "-0\", \"version\":1, \"code\": 200, \"message\": \"OK\"}"

#define SIZE_OF_UPFATE_BUF 200

TEST_F(ShadowJsonBuilderTests, BuildEmptyJson) 
{
    char getOrDeleteRequestJson[SIZE_OF_UPFATE_BUF] = {0};
    build_empty_json(&(g_shadow_ptr->inner_data.token_num), getOrDeleteRequestJson);
    ASSERT_STREQ(TEST_JSON_EMPTY_GET_DELETE_DOCUMENT, getOrDeleteRequestJson);
}

TEST_F(ShadowJsonBuilderTests, UpdateTheJSONDocumentBuilder) 
{
    int ret_val;
    char updateRequestJson[SIZE_OF_UPFATE_BUF];
    // size_t jsonBufSize = sizeof(updateRequestJson) / sizeof(updateRequestJson[0]);

    Log_d("\n-->Running Cloud Json Builder Tests - Update the Json document builder \n");

    ret_val = IOT_Shadow_JSON_ConstructReport(g_shadow_ptr, updateRequestJson, SIZE_OF_UPFATE_BUF, 2, &dataDoubleHandler, &dataFloatHandler);

    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);

    ASSERT_STREQ(TEST_JSON_UPDATE_DOCUMENT, updateRequestJson);
}

TEST_F(ShadowJsonBuilderTests, PassingNullValue) 
{
    int ret_val;

    Log_d("\n-->Running Cloud Json Builder Tests - Passing Null value to Shadow json builder \n");

    ret_val = IOT_Shadow_JSON_ConstructReport(g_shadow_ptr, NULL, SIZE_OF_UPFATE_BUF, 2, &dataDoubleHandler, &dataFloatHandler);
    ASSERT_EQ(QCLOUD_ERR_INVAL, ret_val);
    ret_val = IOT_Shadow_JSON_ConstructDesireAllNull(g_shadow_ptr, NULL, SIZE_OF_UPFATE_BUF);
    ASSERT_EQ(QCLOUD_ERR_INVAL, ret_val);
}

TEST_F(ShadowJsonBuilderTests, SmallBuffer) 
{
    int ret_val;
    char updateRequestJson[14];
    size_t jsonBufSize = sizeof(updateRequestJson) / sizeof(updateRequestJson[0]);

    Log_d("\n-->Running Shadow Json Builder Tests - Json Buffer is too small \n");

    ret_val = IOT_Shadow_JSON_ConstructReport(g_shadow_ptr, updateRequestJson, jsonBufSize, 2, &dataDoubleHandler, &dataFloatHandler);
    ASSERT_EQ(QCLOUD_ERR_JSON_BUFFER_TRUNCATED, ret_val);
    ret_val = IOT_Shadow_JSON_ConstructDesireAllNull(g_shadow_ptr, updateRequestJson, jsonBufSize);
    ASSERT_EQ(QCLOUD_ERR_JSON_BUFFER_TRUNCATED, ret_val);
}

TEST_F(ShadowJsonBuilderTests, UpdateValueIfKeyMatch) 
{
    bool ret_val;

    ret_val = update_value_if_key_match((char*)TEST_JSON_RESPONSE_UPDATE_DOCUMENT, &dataIntegerHandler);
    integerData = *(int *) dataIntegerHandler.data;
    ASSERT_EQ(1, (int) ret_val);
    ASSERT_EQ(24, integerData);

    ret_val = update_value_if_key_match((char*)TEST_JSON_RESPONSE_UPDATE_DOCUMENT, &dataDoubleHandler);
    ASSERT_EQ(0, (int) ret_val);
}

TEST_F(ShadowJsonBuilderTests, ParseClientToken) 
{
    bool ret_val;
    char* clientToken;
    char expected[100];
    strcpy(expected, QCLOUD_IOT_MY_PRODUCT_ID);
    strcat(expected, "-0");

    ret_val = parse_client_token((char*)TEST_JSON_RESPONSE_UPDATE_DOCUMENT, &clientToken);

    ASSERT_EQ(1, (int) ret_val);
    ASSERT_STREQ(expected, clientToken);
}

TEST_F(ShadowJsonBuilderTests, ParseVersionNum) 
{
    bool ret_val;
    uint32_t version;
    uint32_t expected = 1;

    ret_val = parse_version_num((char*)TEST_JSON_RESPONSE_UPDATE_DOCUMENT, &version);

    ASSERT_EQ(1, (int) ret_val);
    ASSERT_EQ(expected, version);
}
