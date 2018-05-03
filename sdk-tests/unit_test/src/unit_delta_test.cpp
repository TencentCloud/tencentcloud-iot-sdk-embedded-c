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

#include <string.h>
#include <stdio.h>

#include <gtest/gtest.h>
#include "mqtt_client.h"
#include <iot_unit_config.h>
#include <unit_helper_functions.h>
#include "shadow_client.h"

static Qcloud_IoT_Shadow 	*client;
static ShadowInitParams     initParams;

static char receivedNestedObject[100] = "";
static char sentNestedObjectData[100] = "{\"sensor1\":23}";
static char shadowDeltaTopic[MAX_SIZE_OF_CLOUD_TOPIC];

#define SHADOW_DELTA_UPDATE "$shadow/update/delta/%s/%s"

#undef QCLOUD_IOT_MY_DEVICE_NAME
#define QCLOUD_IOT_MY_DEVICE_NAME "QCLOUD-IoT-C-SDK"

class ShadowDeltaTest : public testing::Test
{
protected:
    virtual void SetUp()
    {
        std::cout << "ShadowDeltaTest Test Begin \n";
        SetupShadowConnectInitParams(&initParams);
    	client = (Qcloud_IoT_Shadow *)IOT_Shadow_Construct(&initParams);
    	snprintf(shadowDeltaTopic, MAX_SIZE_OF_CLOUD_TOPIC, (char *)SHADOW_DELTA_UPDATE, (char *)QCLOUD_IOT_MY_PRODUCT_ID, (char *)QCLOUD_IOT_MY_DEVICE_NAME);
    }
    virtual void TearDown()
    {
        std::cout << "ShadowDeltaTest Test End \n";
        IOT_Shadow_Destroy(client);
    } 
	
};

void genericCallback(void *pClient, const char *pJsonStringData, uint32_t JsonStringDataLen, DeviceProperty *pContext) {
    printf("======================\nkey[%s]==Data[%.*s]\n======================\n", pContext->key, JsonStringDataLen, pJsonStringData);
}

void nestedObjectCallback(void *pClient, const char *pJsonStringData, uint32_t JsonStringDataLen, DeviceProperty *pContext) {
    printf("======================\nkey[%s]==Data[%.*s]\n======================\n", pContext->key, JsonStringDataLen, pJsonStringData);
    snprintf(receivedNestedObject, 100, "%.*s", JsonStringDataLen, pJsonStringData);
    printf("receivedNestedObject: %s", receivedNestedObject);
}


TEST_F(ShadowDeltaTest, registerDeltaSuccess) {
    DeviceProperty windowHandler;
    char deltaJSONString[SIZE_OF_JSON_BUFFER] = {0};//"{\"state\":{\"delta\":{\"window\":true}},\"version\":1}";
    bool windowOpenData = false;
    int ret_val;

    windowHandler.key = (char*)"window";
    windowHandler.type = JBOOL;
    windowHandler.data = &windowOpenData;

    ret_val = IOT_Shadow_Register_Property(client, &windowHandler, genericCallback);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);

    windowOpenData = true;
    ret_val = IOT_Shadow_JSON_ConstructDesireAllNull(client, deltaJSONString, SIZE_OF_JSON_BUFFER);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);

    windowOpenData = false;
    ret_val = IOT_Shadow_Update(client, deltaJSONString, SIZE_OF_JSON_BUFFER, NULL, NULL, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);

    ret_val = IOT_Shadow_Yield(client, 3000);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);

    ASSERT_EQ(true, windowOpenData);
}


TEST_F(ShadowDeltaTest, registerDeltaInt) {
    int ret_val;
    DeviceProperty intHandler;
    int intData = 0;
    char deltaJSONString[SIZE_OF_JSON_BUFFER] = {0};//"{\"state\":{\"delta\":{\"length\":23}},\"version\":1}";

    intHandler.key = (char*)"length";
    intHandler.type = JINT32;
    intHandler.data = &intData;

    ret_val = IOT_Shadow_Register_Property(client, &intHandler, genericCallback);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);

    intData = 23;
    ret_val = IOT_Shadow_JSON_ConstructDesireAllNull(client, deltaJSONString, SIZE_OF_JSON_BUFFER);
    intData = 0;
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);

    ret_val = IOT_Shadow_Update(client, deltaJSONString, SIZE_OF_JSON_BUFFER, NULL, NULL, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);

    IOT_Shadow_Yield(client, 3000);
    ASSERT_EQ(23, intData);
}

TEST_F(ShadowDeltaTest, registerDeltaIntNoCallback) {
    int ret_val;
    DeviceProperty intHandler;
    int intData = 0;
    intHandler.key = (char*)"length_nocb";
    intHandler.type = JINT32;
    intHandler.data = &intData;

    ret_val = IOT_Shadow_Register_Property(client, &intHandler, NULL);
    ASSERT_EQ(QCLOUD_ERR_INVAL, ret_val);
}

TEST_F(ShadowDeltaTest, DeltaNestedObject) {
    int ret_val;

    DeviceProperty nestedObjectHandler;
    char *nestedObject = sentNestedObjectData;
    char deltaJSONString[SIZE_OF_JSON_BUFFER] = {0};

    nestedObjectHandler.key = (char*)"sensors";
    nestedObjectHandler.type = JOBJECT;
    nestedObjectHandler.data = nestedObject;

    ret_val = IOT_Shadow_Register_Property(client, &nestedObjectHandler, nestedObjectCallback);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);

    ret_val = IOT_Shadow_JSON_ConstructDesireAllNull(client, deltaJSONString, SIZE_OF_JSON_BUFFER);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);

    ret_val = IOT_Shadow_Update(client, deltaJSONString, SIZE_OF_JSON_BUFFER, NULL, NULL, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, ret_val);

    IOT_Shadow_Yield(client, 3000);
    ASSERT_TRUE( strstr(receivedNestedObject, sentNestedObjectData) != NULL );
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
