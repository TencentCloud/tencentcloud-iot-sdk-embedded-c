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

#include <stdbool.h>
#include <iostream>

#include <gtest/gtest.h>

#include "unit_helper_functions.h"
#include "qcloud_iot_export_shadow.h"
#include "qcloud_iot_export_error.h"
#include "qcloud_iot_import.h"
#include "shadow_client.h"


#define SIZE_OF_UPDATE_DOCUMENT 200
#define TEST_JSON_RESPONSE_FULL_DOCUMENT    "{\"state\":{\"reported\":{\"sensor1\":98}}, \"clientToken\":\"" QCLOUD_IOT_MY_PRODUCT_ID "-1\"}"
#define TEST_JSON_RESPONSE_UPDATE_DOCUMENT  "{\"state\":{\"reported\":{\"doubleData\":4.090800,\"floatData\":3.445000}}, \"clientToken\":\"" QCLOUD_IOT_MY_PRODUCT_ID "-1\"}"


static Qcloud_IoT_Shadow    *sg_pshadow;
static ShadowInitParams     sg_initParams;

static RequestAck           sg_ack_status_rx;
static Method               sg_method_rx;
static char                 sg_json_full_document[200];

using namespace std;

static void on_request_callback(void *pClient, Method method, RequestAck status, const char *pReceivedJsonDocument, void *pUserdata)
{
    sg_method_rx = method;
    sg_ack_status_rx = status;
    if (ACK_NONE != status) {
        char* clientTokenString;
        char* stateString;

        char* pReceivedJsonDocument_bak = (char*)HAL_Malloc(strlen(pReceivedJsonDocument));
        if (pReceivedJsonDocument_bak == NULL) {
        	return;
        }
        strcpy(pReceivedJsonDocument_bak, pReceivedJsonDocument);

        if (parse_shadow_state(pReceivedJsonDocument_bak, &stateString) &&
            parse_client_token(pReceivedJsonDocument_bak, &clientTokenString)) {
            HAL_Snprintf(sg_json_full_document, SIZE_OF_JSON_BUFFER, "{\"state\":%s, \"clientToken\":\"%s\"}", stateString, clientTokenString);
        }
    }
}

static void get_the_full_document_callback(void *pClient, Method method, RequestAck status, const char *pReceivedJsonDocument, void *pUserdata)
{
    sg_method_rx = method;
    sg_ack_status_rx = status;
    Log_d("method = %d|status = %d", sg_method_rx, sg_ack_status_rx);
}

class ShadowTest : public testing::Test
{
protected:
    virtual void SetUp() 
    {
        IOT_Log_Set_Level(DEBUG);
        sg_ack_status_rx = ACK_NONE;
        
        SetupShadowConnectInitParams(&sg_initParams);

        void *client = IOT_Shadow_Construct(&sg_initParams);
        sg_pshadow = (Qcloud_IoT_Shadow *)client;
    }

    virtual void TearDown()
    {
        if (NULL != sg_pshadow)
	        IOT_Shadow_Destroy(sg_pshadow);
    }
};


TEST_F(ShadowTest, ShadowIsConnected)
{
    bool isConnected = IOT_Shadow_IsConnected(sg_pshadow);
    
    ASSERT_EQ(true, isConnected);
}

TEST_F(ShadowTest, ShadowDestroy)
{    
    bool isConnected = IOT_Shadow_IsConnected((void *)sg_pshadow);
	ASSERT_EQ(true, isConnected);

    int rc = IOT_Shadow_Destroy(sg_pshadow);
    sg_pshadow = NULL;
    ASSERT_TRUE(rc == 0);
}

TEST_F(ShadowTest, GetTheFullJSONDocument)
{
	int ret_val;

    char deltaJSONString[SIZE_OF_JSON_BUFFER] = {0};
    DeviceProperty property;
    int intData = 98;
    property.key = (char*)"sensor1";
    property.type = JINT16;
    property.data = &intData;
    ret_val = IOT_Shadow_JSON_ConstructReport(sg_pshadow, deltaJSONString, SIZE_OF_JSON_BUFFER, 1, &property);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);

    ret_val = IOT_Shadow_Update(sg_pshadow, deltaJSONString, SIZE_OF_JSON_BUFFER, get_the_full_document_callback, NULL, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);
    while (sg_ack_status_rx == ACK_NONE) {
        Log_d("wait for sub ack!");
        ret_val = IOT_Shadow_Yield(sg_pshadow, 200);
        ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);
        sleep(1);
    }
    sg_ack_status_rx = ACK_NONE;

    ret_val = IOT_Shadow_Get(sg_pshadow, on_request_callback, NULL, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);
    while (sg_ack_status_rx == ACK_NONE) {
        ret_val = IOT_Shadow_Yield(sg_pshadow, 200);
        ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);
        sleep(1);
    }

    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);
    ASSERT_STREQ(TEST_JSON_RESPONSE_FULL_DOCUMENT, sg_json_full_document);
    ASSERT_EQ(GET, sg_method_rx);
    ASSERT_EQ(ACK_ACCEPTED, sg_ack_status_rx);
}

TEST_F(ShadowTest, UpdateTheJSONDocument) 
{
    int ret_val;
    char updateRequestJson[SIZE_OF_UPDATE_DOCUMENT] = {0};
    char expectedUpdateRequestJson[SIZE_OF_UPDATE_DOCUMENT] = {0};

    double doubleData = 4.090800f;
    float floatData = 3.44500f;

    DeviceProperty dataFloatHandler;
    DeviceProperty dataDoubleHandler;

    dataFloatHandler.data = &floatData;
    dataFloatHandler.key = (char*)"floatData";
    dataFloatHandler.type = JFLOAT;

    dataDoubleHandler.data = &doubleData;
    dataDoubleHandler.key = (char*)"doubleData";
    dataDoubleHandler.type = JDOUBLE;

    ret_val = IOT_Shadow_Get_Sync(sg_pshadow, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);

    ret_val = IOT_Shadow_JSON_ConstructReport(sg_pshadow, updateRequestJson, SIZE_OF_UPDATE_DOCUMENT, 2, &dataDoubleHandler, &dataFloatHandler);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);

    snprintf(expectedUpdateRequestJson, SIZE_OF_UPDATE_DOCUMENT,
             "{\"version\":%d, \"state\":{\"reported\":{\"doubleData\":4.090800,\"floatData\":3.445000}}, \"clientToken\":\"%s-1\"}",
             sg_pshadow->inner_data.version, QCLOUD_IOT_MY_PRODUCT_ID);
    ASSERT_STREQ(expectedUpdateRequestJson, updateRequestJson);

    ret_val = IOT_Shadow_Update(sg_pshadow, updateRequestJson, SIZE_OF_UPDATE_DOCUMENT, on_request_callback, NULL, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);
    while (sg_ack_status_rx == ACK_NONE) {
        ret_val = IOT_Shadow_Yield(sg_pshadow, 200);
        ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);
        sleep(1);
    }

    ASSERT_STREQ(TEST_JSON_RESPONSE_UPDATE_DOCUMENT, sg_json_full_document);
    ASSERT_EQ(UPDATE, sg_method_rx);
    ASSERT_EQ(ACK_ACCEPTED, sg_ack_status_rx);
}

TEST_F(ShadowTest, ACKWaitingMoreThanAllowed)
{
    int ret_val;

    uint32_t time_out = 100 * 1000;

    // 1st
    ret_val = IOT_Shadow_Get(sg_pshadow, on_request_callback, NULL, time_out);// 100 sec to timeout
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);

    // 2nd
    ret_val = IOT_Shadow_Get(sg_pshadow, on_request_callback, NULL, time_out);// 100 sec to timeout
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);

    // 3rd
    ret_val = IOT_Shadow_Get(sg_pshadow, on_request_callback, NULL, time_out);// 100 sec to timeout
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);

    // 4th
    ret_val = IOT_Shadow_Get(sg_pshadow, on_request_callback, NULL, time_out);// 100 sec to timeout
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);

    // 5th
    ret_val = IOT_Shadow_Get(sg_pshadow, on_request_callback, NULL, time_out);// 100 sec to timeout
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);

    // 6th
    ret_val = IOT_Shadow_Get(sg_pshadow, on_request_callback, NULL, time_out);// 100 sec to timeout
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);

    // 7th
    ret_val = IOT_Shadow_Get(sg_pshadow, on_request_callback, NULL, time_out);// 100 sec to timeout
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);

    // 8th
    ret_val = IOT_Shadow_Get(sg_pshadow, on_request_callback, NULL, time_out);// 100 sec to timeout
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);

    // 9th
    ret_val = IOT_Shadow_Get(sg_pshadow, on_request_callback, NULL, time_out);// 100 sec to timeout
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);

    // 10th
    ret_val = IOT_Shadow_Get(sg_pshadow, on_request_callback, NULL, time_out);// 100 sec to timeout
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);

    // 11th
    // Should return some error code, since we are running out of ACK space
    ret_val = IOT_Shadow_Get(sg_pshadow, on_request_callback, NULL, time_out);// 100 sec to timeout
    ASSERT_EQ(QCLOUD_ERR_MAX_APPENDING_REQUEST, ret_val);
}

// TODO:
// 1.如果update的字符串不符合json格式是否能提示处理
// 2.当设备update的数据过长的时候是否能正确处理过长的数据
// 3.当设备shadow get回来的数据很大的时候是否能正确处理


int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
