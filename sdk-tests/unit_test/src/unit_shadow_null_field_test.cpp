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
#include <iostream>
#include <unistd.h>
#include <limits.h>

#include <gtest/gtest.h>
#include <unit_helper_functions.h>
#include "qcloud_iot_export_shadow.h"
#include "qcloud_iot_export_error.h"


static Qcloud_IoT_Client    *g_client_ptr;
static ShadowInitParams     g_initParams;

static RequestAck           ackStatusRx;
static Method               actionRx;
static char                 jsonFullDocument[200];


static void onRequestCallback(void *pClient, Method method, RequestAck requestAck, const char *pReceivedJsonDocument, void *pUserdata)
{
    actionRx = method;
    ackStatusRx = requestAck;
    if (ACK_TIMEOUT != requestAck) {
        strcpy(jsonFullDocument, pReceivedJsonDocument);
    }
}

using namespace std;

class ShadowNullFieldsTest : public testing::Test
{
protected:
    virtual void SetUp()
    {
        std::cout << "ShadowNullFieldsTest Test Begin \n";
    }
    virtual void TearDown()
    {
        std::cout << "ShadowNullFieldsTest Test End \n";
    }
    
    
};

TEST_F(ShadowNullFieldsTest, NullClientConnect) 
{
    void *client = IOT_Shadow_Construct(NULL);
    ASSERT_TRUE(client == NULL);
}

TEST_F(ShadowNullFieldsTest, NullUpdateDocument) 
{
    SetupShadowConnectInitParams(&g_initParams);
    g_client_ptr = (Qcloud_IoT_Client *)IOT_Shadow_Construct(&g_initParams);
    int rc = IOT_Shadow_Update(g_client_ptr, NULL, 0, onRequestCallback, NULL, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);
    ASSERT_EQ(QCLOUD_ERR_INVAL, rc);
    IOT_Shadow_Destroy(g_client_ptr);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
}

TEST_F(ShadowNullFieldsTest, NullClientYield) 
{
    int rc = IOT_Shadow_Yield(NULL, 1000);
    ASSERT_EQ(QCLOUD_ERR_INVAL, rc);
}

TEST_F(ShadowNullFieldsTest, NullClientDisconnect) 
{
    int rc = IOT_Shadow_Destroy(NULL);
    ASSERT_EQ(QCLOUD_ERR_INVAL, rc);
}

TEST_F(ShadowNullFieldsTest, NullClientShadowGet) 
{
    int rc = IOT_Shadow_Get(NULL, onRequestCallback, NULL, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);
    ASSERT_EQ(QCLOUD_ERR_INVAL, rc);
}

TEST_F(ShadowNullFieldsTest, NullClientShadowUpdate) 
{
    int rc = IOT_Shadow_Update(NULL, jsonFullDocument, 200, onRequestCallback, NULL, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);
    ASSERT_EQ(QCLOUD_ERR_INVAL, rc);
}

TEST_F(ShadowNullFieldsTest, NullClientShadowDelete) 
{
//    int rc = IOT_Shadow_Delete(NULL, onRequestCallback, NULL, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);
//    ASSERT_EQ(QCLOUD_ERR_INVAL, rc);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
