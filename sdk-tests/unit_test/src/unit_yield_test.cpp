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
#include <unistd.h>
#include <string.h>

#include <gtest/gtest.h>
#include <unit_helper_functions.h>
#include "iot_unit_config.h"


static MQTTInitParams       initParams;
static MQTTConnectParams 	connectParams;

static Qcloud_IoT_Client 	iotClient;

static PublishParams 		testPubMsgParams;
static SubscribeParams 		testSubParams;

static char CallbackMsgString[100];
static char subTopic[SIZE_OF_JSON_BUFFER] = "sdk/Test";
static uint16_t subTopicLen = SIZE_OF_JSON_BUFFER;

static bool dcHandlerInvoked = false;

static void iot_tests_unit_acr_subscribe_callback_handler(void *pClient, MQTTMessage *params, void *pUserdata) {
    char *tmp = (char *)params->payload;
    unsigned int i;
    subTopicLen = 9;
    for (i = 0; i < params->payload_len; i++) {
        CallbackMsgString[i] = tmp[i];
    }
}

static void iot_tests_unit_yield_test_subscribe_callback_handler(void *pClient, MQTTMessage *params, void *pUserdata) 
{
    char *tmp = (char *)params->payload;
    unsigned int i;

    for (i = 0; i < params->payload_len; i++) {
        CallbackMsgString[i] = tmp[i];
    }
}

void iot_tests_unit_disconnect_handler(void) 
{
    dcHandlerInvoked = true;
}

void event_handle(void *pcontext, void *pclient, MQTTEventMsg *msg)
{
    switch (msg->event_type) {
        case MQTT_EVENT_UNDEF:
            Log_d("MQTT_EVENT_UNDEF|undefined event occur.");
            break;

        case MQTT_EVENT_DISCONNECT:
            Log_d("MQTT_EVENT_DISCONNECT|MQTT disconnect.");
			iot_tests_unit_disconnect_handler();
            break;

        default:
            Log_d("Should NOT arrive here.");
            break;
    }
}

class YieldTests : public testing::Test
{
protected:
    virtual void SetUp()
    {
        std::cout << "YieldTests Test Begin \n";

        int rc;
	    dcHandlerInvoked = false;

	    MQTTInitParamsSetup(&initParams, false);
        initParams.event_handle.h_fp = event_handle;
	    rc = qcloud_iot_mqtt_init(&iotClient, &initParams);
	    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);

	    ConnectMQTTParamsSetup_Detailed(&connectParams, &initParams, QOS1, false, true, (char *)"willTopicName", (char *)"willMsg", NULL);
	    initParams.keep_alive_interval_ms = 5000;

        HAL_Snprintf(subTopic, subTopicLen, "%s/%s/get", QCLOUD_IOT_MY_PRODUCT_ID, QCLOUD_IOT_MY_DEVICE_NAME);

	    // void * client = IOT_MQTT_Construct(&initParams);
        // iotClient = *((Qcloud_IoT_Client *)client);
	    // ASSERT_TRUE(client != NULL);
        rc = qcloud_iot_mqtt_connect(&iotClient, &connectParams);
        ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);

	    rc = qcloud_iot_mqtt_set_autoreconnect(&iotClient, false);
	    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    }
    virtual void TearDown()
    {
        std::cout << "YieldTests Test End \n";
        qcloud_iot_mqtt_disconnect(&iotClient);
        sleep(1);
    } 
	
};


/* G:1 - Yield with Null/empty Client Instance */
TEST_F(YieldTests, NullClientYield) {
    int rc = qcloud_iot_mqtt_yield(NULL, 1000);
    ASSERT_EQ(QCLOUD_ERR_INVAL, rc);
}

/* G:2 - Yield with zero yield timeout */
TEST_F(YieldTests, ZeroTimeoutYield) {
    int rc = qcloud_iot_mqtt_yield(&iotClient, 0);
    ASSERT_EQ(QCLOUD_ERR_INVAL, rc);
}

/* G:3 - Yield, network disconnected, never connected */
TEST_F(YieldTests, YieldNetworkDisconnectedNeverConnected) {
    Qcloud_IoT_Client tempIotClient;
    int rc;

    MQTTInitParamsSetup(&initParams, false);
    rc = qcloud_iot_mqtt_init(&tempIotClient, &initParams);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);

    rc = qcloud_iot_mqtt_yield(&tempIotClient, 1000);
    ASSERT_EQ(QCLOUD_ERR_MQTT_NO_CONN, rc);
}

/* G:4 - Yield, network disconnected, disconnected manually */
TEST_F(YieldTests, YieldNetworkDisconnectedDisconnectedManually) {
    int rc = qcloud_iot_mqtt_disconnect(&iotClient);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);

    rc = qcloud_iot_mqtt_yield(&iotClient, 1000);
    ASSERT_EQ(QCLOUD_ERR_MQTT_MANUALLY_DISCONNECTED, rc);
}

/* G:5 - Yield, network connected, yield called while in subscribe application callback */
TEST_F(YieldTests, YieldInSubscribeCallback) {
    int rc;

    testSubParams.qos = QOS1;
    testSubParams.on_message_handler = iot_tests_unit_yield_test_subscribe_callback_handler;
    rc = qcloud_iot_mqtt_subscribe(&iotClient, subTopic, &testSubParams);
    if (0 < rc) {
        testPubMsgParams.qos = QOS1;
        rc = qcloud_iot_mqtt_yield(&iotClient, 1000);
        if (QCLOUD_ERR_SUCCESS == rc) {
            char expectedCallbackString[] = "0xA5A5A3";
            ASSERT_STREQ(expectedCallbackString, CallbackMsgString);
        }
    }

}

/* G:6 - Yield, network disconnected, ping timeout, auto-reconnect disabled */
TEST_F(YieldTests, disconnectNoAutoReconnect) {
    int rc;

    rc = qcloud_iot_mqtt_set_autoreconnect(&iotClient, true);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);

    ASSERT_EQ(true, IOT_MQTT_IsConnected(&iotClient));
    ASSERT_EQ(true, qcloud_iot_mqtt_is_autoreconnect_enabled(&iotClient));

    /* Disable Autoreconnect, then let ping request time out and call yield */
    qcloud_iot_mqtt_set_autoreconnect(&iotClient, false);
    sleep((uint16_t) (iotClient.options.keep_alive_interval - 1));

    /* Sleep for keep alive interval to allow the first ping to be sent out */
    // sleep(iotClient.options.keep_alive_interval);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);

    /* Let ping request time out and call yield */
    sleep(iotClient.options.keep_alive_interval + 1);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    ASSERT_EQ(QCLOUD_ERR_MQTT_NO_CONN, rc);
    ASSERT_EQ(0, IOT_MQTT_IsConnected(&iotClient));
    ASSERT_EQ(true, dcHandlerInvoked);
}

/* G:7 - Yield, network connected, no incoming messages */
TEST_F(YieldTests, YieldSuccessNoMessages) {
    int rc;
    int i;

    for (i = 0; i < 100; i++) {
        CallbackMsgString[i] = 'x';
    }

    rc = qcloud_iot_mqtt_yield(&iotClient, 1000);
    if (QCLOUD_ERR_SUCCESS == rc) {
        /* Check no messages were received */
        for (i = 0; i < 100; i++) {
            if ('x' != CallbackMsgString[i]) {
                rc = QCLOUD_ERR_FAILURE;
            }
        }
    }

    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
}

/* G:8 - Yield, network connected, ping request/response */
TEST_F(YieldTests, PingRequestPingResponse) {
    int rc;
    int i = 0;
    int j = 0;
    int attempt = 3;

    for (i = 0; i < attempt; i++) {

        for (j = 0; j <= iotClient.options.keep_alive_interval; j++) {
            sleep(1);
            rc = qcloud_iot_mqtt_yield(&iotClient, 100);
            ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
        }
    }

}

/* G:9 - Yield, disconnected, Auto-reconnect timed-out */
TEST_F(YieldTests, disconnectAutoReconnectTimeout) {
    int rc;

    rc = qcloud_iot_mqtt_set_autoreconnect(&iotClient, true);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);

    ASSERT_EQ(true, IOT_MQTT_IsConnected(&iotClient));
    ASSERT_EQ(true, qcloud_iot_mqtt_is_autoreconnect_enabled(&iotClient));

    /* Sleep for keep alive interval to allow the first ping to be sent out */
    sleep(iotClient.options.keep_alive_interval);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);

    /* Let ping request time out and call yield */
    sleep(iotClient.options.keep_alive_interval + 1);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    ASSERT_EQ(QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT, rc);
    ASSERT_EQ(0, IOT_MQTT_IsConnected(&iotClient));
    ASSERT_EQ(true, dcHandlerInvoked);

}

/* G:10 - Yield, disconnected, Auto-reconnect successful */
TEST_F(YieldTests, disconnectAutoReconnectSuccess) {
    int rc;

    rc = qcloud_iot_mqtt_set_autoreconnect(&iotClient, true);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);

    ASSERT_EQ(true, IOT_MQTT_IsConnected(&iotClient));
    ASSERT_EQ(true, qcloud_iot_mqtt_is_autoreconnect_enabled(&iotClient));

    /* Sleep for keep alive interval to allow the first ping to be sent out */
    sleep(iotClient.options.keep_alive_interval);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);

    /* Let ping request time out and call yield */
    sleep(iotClient.options.keep_alive_interval + 10);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    ASSERT_EQ(QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT, rc);

    sleep(2); /* Default min reconnect delay is 1 sec */
    printf("\nWakeup");
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);

    ASSERT_EQ(true, IOT_MQTT_IsConnected(&iotClient));
    ASSERT_EQ(true, dcHandlerInvoked);
}

/* G:11 - Yield, disconnected, Manual reconnect */
TEST_F(YieldTests, disconnectManualAutoReconnect) {
    int rc;

    ASSERT_TRUE(IOT_MQTT_IsConnected(&iotClient));

    /* Disable Autoreconnect, then let ping request time out and call yield */
    qcloud_iot_mqtt_set_autoreconnect(&iotClient, false);
    ASSERT_TRUE(!qcloud_iot_mqtt_is_autoreconnect_enabled(&iotClient));

    /* Sleep for keep alive interval to allow the first ping to be sent out */
    sleep(iotClient.options.keep_alive_interval);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);

    /* Let ping request time out and call yield */
    sleep(iotClient.options.keep_alive_interval + 5);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    ASSERT_EQ(QCLOUD_ERR_MQTT_NO_CONN, rc);
    ASSERT_TRUE(!IOT_MQTT_IsConnected(&iotClient));
    ASSERT_EQ(true, dcHandlerInvoked);

    dcHandlerInvoked = false;
    rc = qcloud_iot_mqtt_attempt_reconnect(&iotClient);
    ASSERT_EQ(QCLOUD_ERR_MQTT_RECONNECTED, rc);

    ASSERT_TRUE(IOT_MQTT_IsConnected(&iotClient));
    ASSERT_TRUE(false == dcHandlerInvoked);
}

/* G:12 - Yield, resubscribe to all topics on reconnect */
TEST_F(YieldTests, resubscribeSuccessfulReconnect) {
    int rc;
    bool connected;
    bool autoReconnectEnabled = false;
    char expectedCallbackString[100];

    rc = qcloud_iot_mqtt_set_autoreconnect(&iotClient, true);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);

    snprintf(CallbackMsgString, 100, "NOT_VISITED");

    connected = IOT_MQTT_IsConnected(&iotClient);
    ASSERT_EQ(1, connected);

    /* Subscribe to a topic */
    testSubParams.qos = QOS1;
    testSubParams.on_message_handler = iot_tests_unit_acr_subscribe_callback_handler;
    rc = qcloud_iot_mqtt_subscribe(&iotClient, subTopic, &testSubParams);
    ASSERT_TRUE(0 < rc);

    /* Check subscribe */
    // snprintf(expectedCallbackString, 100, "Message for %s after resub", subTopic);
    snprintf(expectedCallbackString, 100, "%s", "{\"action\":\"come_home\",\"targetDevice\":\"air\"}");
    while(subTopicLen != 9){
        rc = qcloud_iot_mqtt_yield(&iotClient, 100);
        sleep(2);
    }
    
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_STREQ(expectedCallbackString, CallbackMsgString);

    autoReconnectEnabled = qcloud_iot_mqtt_is_autoreconnect_enabled(&iotClient);
    ASSERT_EQ(1, autoReconnectEnabled);

    /* Sleep for keep alive interval to allow the first ping to be sent out */
    sleep(iotClient.options.keep_alive_interval);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);

    /* Let ping request time out and call yield */
    sleep(iotClient.options.keep_alive_interval + 5);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    ASSERT_EQ(QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT, rc);

    sleep(2); /* Default min reconnect delay is 1 sec */

    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    ASSERT_EQ(QCLOUD_ERR_MQTT_RECONNECTED, rc);

    /* Test if reconnect worked */
    connected = IOT_MQTT_IsConnected(&iotClient);
    ASSERT_EQ(true, connected);

    /* Check subscribe */
    // snprintf(expectedCallbackString, 100, "Message for %s after resub", subTopic);
    snprintf(expectedCallbackString, 100, "%s", "{\"action\":\"come_home\",\"targetDevice\":\"air\"}");
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_STREQ(expectedCallbackString, CallbackMsgString);
    ASSERT_EQ(true, dcHandlerInvoked);

}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
