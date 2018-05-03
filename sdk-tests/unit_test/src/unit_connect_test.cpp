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
#include "unit_helper_functions.h"
#include "qcloud_iot_export_mqtt.h"
#include "mqtt_client.h"
#include "iot_unit_config.h"

static bool isMqttConnected = false;

static Qcloud_IoT_Client 	*client;
static MQTTInitParams 		initParams;
static MQTTConnectParams 	connectParams = DEFAULT_MQTTCONNECT_PARAMS;

PublishParams testPubMsgParams;

SubscribeParams testTopicParams;

#define NO_MSG_XXXX "XXXX"
// static char CallbackMsgStringclean[100] = NO_MSG_XXXX;

using namespace std;

class MQTTConnectTests : public testing::Test
{
protected:
    virtual void SetUp()
    {
        IOT_Log_Set_Level(DEBUG);
        std::cout << "MQTTConnectTests Test Begin \n";
        isMqttConnected = false;
    }
    virtual void TearDown()
    {
        sleep(1);
        std::cout << "MQTTConnectTests Test End \n";
//        IOT_MQTT_Destroy(&client);
    } 
	
};

/**
 * 1. Connect success
 */
TEST_F(MQTTConnectTests, ConnectSuccess) {
    SetupMQTTConnectInitParams(&initParams);
    void * client = IOT_MQTT_Construct(&initParams);
    ASSERT_TRUE(client != NULL);
}

/**
 * 2. Init with NULL MQTTInitParams Struct
 */
TEST_F(MQTTConnectTests, nullParamsInit) {
    Log_d("2. Init with NULL MQTTInitParams Struct");
    void *client_ptr = IOT_MQTT_Construct(NULL);
    ASSERT_TRUE(NULL == client_ptr);
}

/**
 * 3. Init with NULL Cert File Path
 */
TEST_F(MQTTConnectTests, nullCert) {
    Log_d("4. Init with NULL Cert File Path");
    SetupMQTTConnectInitParams(&initParams);
#ifndef AUTH_WITH_NOTLS
#ifdef AUTH_MODE_CERT
    initParams.cert_file = NULL;
#else

#endif
#endif
    void *client_ptr = IOT_MQTT_Construct(&initParams);
    ASSERT_TRUE(NULL == client_ptr);
}

/**
 * 4. Init with NULL Key File Path
 */
TEST_F(MQTTConnectTests, nullKeyFile) {
    SetupMQTTConnectInitParams(&initParams);
#ifndef AUTH_WITH_NOTLS
#ifdef AUTH_MODE_CERT
    initParams.key_file = NULL;
#else

#endif
#endif
    void *client_ptr = IOT_MQTT_Construct(&initParams);
    ASSERT_TRUE(NULL == client_ptr);
}

/**
 * 5. Connect with NULL/Empty Client ID
 */
TEST_F(MQTTConnectTests, nullClientID) {

    // if NULL Client ID
    SetupMQTTConnectInitParams(&initParams);
    void *client_ptr = IOT_MQTT_Construct(&initParams);
    ASSERT_TRUE(NULL == client_ptr);

    // if Empty Client ID
    SetupMQTTConnectInitParams(&initParams);
    client_ptr = IOT_MQTT_Construct(&initParams);
    ASSERT_TRUE(NULL == client_ptr);
}

/**
 * 6. Connect with Invalid Cert
 */
TEST_F(MQTTConnectTests, InvalidCert) {
    char invalidCert[20];
    snprintf(invalidCert, 20, "invalid");

    SetupMQTTConnectInitParams(&initParams);
#ifndef AUTH_WITH_NOTLS
#ifdef AUTH_MODE_CERT
    initParams.cert_file = invalidCert;
#else

#endif
#endif
    void *client_ptr = IOT_MQTT_Construct(&initParams);
    ASSERT_TRUE(NULL == client_ptr);
}

/**
 * 7. Connect with Invalid Key
 */
TEST_F(MQTTConnectTests, InvalidKey) {
    char invalidKey[20];
    snprintf(invalidKey, 20, "invalid");

    SetupMQTTConnectInitParams(&initParams);
#ifndef AUTH_WITH_NOTLS
#ifdef AUTH_MODE_CERT
    initParams.key_file = invalidKey;
#else

#endif
#endif
    void *client_ptr = IOT_MQTT_Construct(&initParams);
    ASSERT_TRUE(NULL == client_ptr);
}

/**
 * 8. Connect, with no Response timeout
 */
TEST_F(MQTTConnectTests, NoResponseTimeout) {
    int rc;

    MQTTInitParamsSetup(&initParams, false);
    rc = qcloud_iot_mqtt_init(client, &initParams);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);

    ConnectParamsSetup(&connectParams, &initParams);
    rc = qcloud_iot_mqtt_connect(client, &connectParams);
    ASSERT_TRUE(rc != QCLOUD_ERR_SUCCESS);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

