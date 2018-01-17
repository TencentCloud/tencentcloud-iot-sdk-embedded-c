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

#include <gtest/gtest.h>
#include <unit_helper_functions.h>
#include "qcloud_iot_export.h"

static MQTTConnectParams    connectParams;
static MQTTInitParams       initParams;

static PublishParams testPubMsgParams;
static SubscribeParams testSubParams;

static Qcloud_IoT_Client client;

static char subTopic[32] = "A58FGENPD7/Air_0/Test0";
static char CallbackMsgString[100];
char cPayload[100];

static char CallbackMsgString1[100] = {"XXXX"};
static char CallbackMsgString2[100] = {"XXXX"};
static char CallbackMsgString3[100] = {"XXXX"};
static char CallbackMsgString4[100] = {"XXXX"};
static char CallbackMsgString5[100] = {"XXXX"};
static char CallbackMsgString6[100] = {"XXXX"};

static bool sg_callback_status1 = false;
static bool sg_callback_status2 = false;
static bool sg_callback_status3 = false;
static bool sg_callback_status4 = false;
static bool sg_callback_status5 = false;
static bool sg_callback_status6 = false;


static void iot_subscribe_callback_handler(void *pClient, MQTTMessage *params, void *pUserdata) {

    if (params == NULL) {
        return;
    }

    char *tmp = (char *)params->payload;
    unsigned int i;

    for (i = 0; i < (params->payload_len); i++) {
        CallbackMsgString[i] = tmp[i];
    }
}

static void
iot_subscribe_callback_handler1(void *pClient, MQTTMessage *params, void *pUserdata) {
    if (params == NULL) {
        return;
    }

    char *tmp = (char *)params->payload;
    unsigned int i;

    printf("callback topic %.*s\n", (int) params->topic_len, params->ptopic);
    for (i = 0; i < (params->payload_len); i++) {
        CallbackMsgString1[i] = tmp[i];
    }
    sg_callback_status1 = true;
}

static void
iot_subscribe_callback_handler2(void *pClient, MQTTMessage *params, void *pUserdata) {
    if (params == NULL) {
        return;
    }

    char *tmp = (char *)params->payload;
    unsigned int i;

    for (i = 0; i < (params->payload_len); i++) {
        CallbackMsgString2[i] = tmp[i];
    }
    sg_callback_status2 = true;
}

static void
iot_subscribe_callback_handler3(void *pClient, MQTTMessage *params, void *pUserdata) {
    if (params == NULL) {
        return;
    }

    char *tmp = (char *)params->payload;
    unsigned int i;

    for (i = 0; i < (params->payload_len); i++) {
        CallbackMsgString3[i] = tmp[i];
    }
    sg_callback_status3 = true;
}

static void
iot_subscribe_callback_handler4(void *pClient, MQTTMessage *params, void *pUserdata) {
    if (params == NULL) {
        return;
    }

    char *tmp = (char *)params->payload;
    unsigned int i;

    for (i = 0; i < (params->payload_len); i++) {
        CallbackMsgString4[i] = tmp[i];
    }
    sg_callback_status4 = true;
}

static void
iot_subscribe_callback_handler5(void *pClient, MQTTMessage *params, void *pUserdata) {
    if (params == NULL) {
        return;
    }

    char *tmp = (char *)params->payload;
    unsigned int i;

    for (i = 0; i < (params->payload_len); i++) {
        CallbackMsgString5[i] = tmp[i];
    }
    sg_callback_status5 = true;
}

static void
iot_subscribe_callback_handler6(void *pClient, MQTTMessage *params, void *pUserdata) {
    if (params == NULL) {
        return;
    }

    char *tmp = (char *)params->payload;
    unsigned int i;

    for (i = 0; i < (params->payload_len); i++) {
        CallbackMsgString6[i] = tmp[i];
    }
    sg_callback_status6 = true;
}

static void
event_handler(void *pclient, void *handle_context, MQTTEventMsg *msg) 
{	
	uintptr_t packet_id = (uintptr_t)msg->msg;

	switch(msg->event_type) {
		case MQTT_EVENT_UNDEF:
			Log_i("undefined event occur.");
			break;

		case MQTT_EVENT_DISCONNECT:
			Log_i("MQTT disconnect.");
			break;

		case MQTT_EVENT_RECONNECT:
			Log_i("MQTT reconnect.");
			break;

		case MQTT_EVENT_PUBLISH_RECVEIVED:
			break;
		case MQTT_EVENT_SUBCRIBE_SUCCESS:
			Log_i("subscribe success, packet-id=%u", (unsigned int)packet_id);
			break;

		case MQTT_EVENT_SUBCRIBE_TIMEOUT:
			Log_i("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
			break;

		case MQTT_EVENT_SUBCRIBE_NACK:
			Log_i("subscribe nack, packet-id=%u", (unsigned int)packet_id);
            // if (strcmp(topic, sg_topic1) == 0) {
            //     sg_event_handler_status1 = true;
            // } else if (strcmp(topic, sg_topic2) == 0) {
            //     sg_event_handler_status2 = true;
            // } else if (strcmp(topic, sg_topic3) == 0) {
            //     sg_event_handler_status3 = true;
            // } else if (strcmp(topic, sg_topic4) == 0) {
            //     sg_event_handler_status4 = true;
            // } else if (strcmp(topic, sg_topic5) == 0) {
            //     sg_event_handler_status5 = true;
            // } else if (strcmp(topic, sg_topic6) == 0) {
            //     sg_event_handler_status6 = true;
            // }
			break;

		case MQTT_EVENT_UNSUBCRIBE_SUCCESS:
			Log_i("unsubscribe success, packet-id=%u", (unsigned int)packet_id);
			break;

		case MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
			Log_i("unsubscribe timeout, packet-id=%u", (unsigned int)packet_id);
			break;

		case MQTT_EVENT_UNSUBCRIBE_NACK:
			Log_i("unsubscribe nack, packet-id=%u", (unsigned int)packet_id);
			break;

		case MQTT_EVENT_PUBLISH_SUCCESS:
			Log_i("publish success, packet-id=%u", (unsigned int)packet_id);
			break;

		case MQTT_EVENT_PUBLISH_TIMEOUT:
			Log_i("publish timeout, packet-id=%u", (unsigned int)packet_id);
			break;

		case MQTT_EVENT_PUBLISH_NACK:
			Log_i("publish nack, packet-id=%u", (unsigned int)packet_id);
			break;
		default:
			Log_i("Should NOT arrive here.");
			break;
	}
}

class SubscribeTests : public testing::Test
{
protected:
    virtual void SetUp()
    {
        std::cout << "SubscribeTests Test Begin \n";

        IOT_Log_Set_Level(DEBUG);

	    int rc;
	    MQTTInitParamsSetup(&initParams, false);
        initParams.event_handle.h_fp = event_handler;
	    rc = qcloud_iot_mqtt_init(&client, &initParams);
		if (rc != QCLOUD_ERR_SUCCESS)
		{
			std::cout << "qcloud_iot_mqtt_init fail \n";
		}

	    ConnectParamsSetup(&connectParams, &initParams);

	    rc = qcloud_iot_mqtt_connect(&client, &connectParams);

		if (rc != QCLOUD_ERR_SUCCESS)
		{
			std::cout << "!qcloud_iot_mqtt_connect fail \n";
		} else {
            std::cout << "!qcloud_iot_mqtt_connect success \n";
        }

	    testPubMsgParams.qos = QOS1;
	    testPubMsgParams.retained = 0;
	    snprintf(cPayload, 100, "%s : %d ", (char *)"hello from SDK", 0);
	    testPubMsgParams.payload = (void *) cPayload;
	    testPubMsgParams.payload_len = strlen(cPayload);

	    testSubParams.qos = QOS1;
	    testSubParams.on_message_handler = iot_subscribe_callback_handler;

    }
    virtual void TearDown()
    {
        std::cout << "SubscribeTests Test End \n";
        int rc = qcloud_iot_mqtt_disconnect(&client);
        if (rc != QCLOUD_ERR_SUCCESS) {
            std::cout << "qcloud_iot_mqtt_disconnect fail \n";
        }
    } 
	
};

/* C:1 - Subscribe with Null/empty Client Instance */
TEST_F(SubscribeTests, SubscribeNullClient) {
    int rc = qcloud_iot_mqtt_subscribe(NULL, subTopic, &testSubParams);
    ASSERT_EQ(QCLOUD_ERR_INVAL, rc);
}

/* C:2 - Subscribe with Null/empty Topic Name */
TEST_F(SubscribeTests, SubscribeNullTopic) {
    int rc = qcloud_iot_mqtt_subscribe(&client, NULL, &testSubParams);
    ASSERT_EQ(QCLOUD_ERR_INVAL, rc);

    rc = qcloud_iot_mqtt_subscribe(&client, (char *)"", &testSubParams);
    ASSERT_EQ(QCLOUD_ERR_INVAL, rc);
}

/* C:3 - Subscribe with Null client callback */
// TEST_F(SubscribeTests, SubscribeNullSubscribeHandler) {
//     testSubParams.on_message_handler = NULL;
//     int rc = qcloud_iot_mqtt_subscribe(&client, subTopic, &testSubParams);
//     ASSERT_EQ(QCLOUD_ERR_INVAL, rc);
// }

/* C:4 - Subscribe with Null subParams*/
TEST_F(SubscribeTests, SubscribeNullParams) {
    int rc = qcloud_iot_mqtt_subscribe(&client, subTopic, NULL);
    ASSERT_EQ(QCLOUD_ERR_INVAL, rc);
}

/* C:5 - Subscribe with no connection */
TEST_F(SubscribeTests, SubscribeNoConnection) {
	/* Disconnect first */
	int rc = qcloud_iot_mqtt_disconnect(&client);
	ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);

	rc = qcloud_iot_mqtt_subscribe(&client, subTopic, &testSubParams);
	ASSERT_EQ(QCLOUD_ERR_MQTT_NO_CONN, rc);
}

/* C:6 - Subscribe, multiple topics, messages on each topic */
TEST_F(SubscribeTests, SubscribeToMultipleTopicsSuccess) {
    int rc;
    char expectedCallbackString[] = "";

    rc = qcloud_iot_mqtt_subscribe(&client, (char *)"A58FGENPD7/Air_0/Test1", &testSubParams);
    ASSERT_TRUE(0 < rc);

    testSubParams.on_message_handler = iot_subscribe_callback_handler2;
    rc = qcloud_iot_mqtt_subscribe(&client, (char *)"A58FGENPD7/Air_0/Test2", &testSubParams);
    
    ASSERT_TRUE(0 < rc);

    int count = 10;
    while (count) {
        rc = qcloud_iot_mqtt_yield(&client, 200);
        count--;
        sleep(0.5);
    }
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    ASSERT_STREQ(expectedCallbackString, CallbackMsgString);
}

/* C:7 - Subscribe, max topics, messages on each topic */
TEST_F(SubscribeTests, SubcribeToMaxAllowedTopicsSuccess) {
    int rc;
    char expectedCallbackString[] = "XXXX";
    char expectedCallbackString2[] = "XXXX";
    char expectedCallbackString3[] = "XXXX";
    char expectedCallbackString4[] = "XXXX";
    char expectedCallbackString5[] = "XXXX";

    testSubParams.on_message_handler = iot_subscribe_callback_handler1;
    rc = qcloud_iot_mqtt_subscribe(&client, (char *)"A58FGENPD7/Air_0/Test1", &testSubParams);
    ASSERT_TRUE(0 < rc);

    testSubParams.on_message_handler = iot_subscribe_callback_handler2;
    rc = qcloud_iot_mqtt_subscribe(&client, (char *)"A58FGENPD7/Air_0/Test2", &testSubParams);
    ASSERT_TRUE(0 < rc);

    testSubParams.on_message_handler = iot_subscribe_callback_handler3;
    rc = qcloud_iot_mqtt_subscribe(&client, (char *)"A58FGENPD7/Air_0/Test3", &testSubParams);
    ASSERT_TRUE(0 < rc);

    testSubParams.on_message_handler = iot_subscribe_callback_handler4;
    rc = qcloud_iot_mqtt_subscribe(&client, (char *)"A58FGENPD7/Air_0/Test4", &testSubParams);
    ASSERT_TRUE(0 < rc);

    testSubParams.on_message_handler = iot_subscribe_callback_handler5;
    rc = qcloud_iot_mqtt_subscribe(&client, (char *)"A58FGENPD7/Air_0/Test5", &testSubParams);
    ASSERT_TRUE(0 < rc);

    /*
    rc = qcloud_iot_mqtt_yield(&client, 1000);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);

    rc = qcloud_iot_mqtt_yield(&client, 1000);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);

    rc = qcloud_iot_mqtt_yield(&client, 1000);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);

    rc = qcloud_iot_mqtt_yield(&client, 1000);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);

    rc = qcloud_iot_mqtt_yield(&client, 1000);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
    */

    int count = 10;
    while (count) {
        rc = qcloud_iot_mqtt_yield(&client, 200);
        ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
        count--;
        sleep(0.2);
    }

    ASSERT_STREQ(expectedCallbackString, CallbackMsgString1);
    ASSERT_STREQ(expectedCallbackString2, CallbackMsgString2);
    ASSERT_STREQ(expectedCallbackString3, CallbackMsgString3);
    ASSERT_STREQ(expectedCallbackString4, CallbackMsgString4);
    ASSERT_STREQ(expectedCallbackString5, CallbackMsgString5);

}

/* C:8 - Subscribe, max topics, another subscribe */
TEST_F(SubscribeTests, SubcribeToMaxPlusOneAllowedTopicsFailure) {
    int rc;

    testSubParams.on_message_handler = iot_subscribe_callback_handler1;
    rc = qcloud_iot_mqtt_subscribe(&client, (char *)"A58FGENPD7/Air_0/Test1", &testSubParams);
    ASSERT_TRUE(0 < rc);

    testSubParams.on_message_handler = iot_subscribe_callback_handler2;
    rc = qcloud_iot_mqtt_subscribe(&client, (char *)"A58FGENPD7/Air_0/Test2", &testSubParams);
    ASSERT_TRUE(0 < rc);

    testSubParams.on_message_handler = iot_subscribe_callback_handler3;
    rc = qcloud_iot_mqtt_subscribe(&client, (char *)"A58FGENPD7/Air_0/Test3", &testSubParams);
    ASSERT_TRUE(0 < rc);

    testSubParams.on_message_handler = iot_subscribe_callback_handler4;
    rc = qcloud_iot_mqtt_subscribe(&client, (char *)"A58FGENPD7/Air_0/Test4", &testSubParams);
    ASSERT_TRUE(0 < rc);

    testSubParams.on_message_handler = iot_subscribe_callback_handler5;
    rc = qcloud_iot_mqtt_subscribe(&client, (char *)"A58FGENPD7/Air_0/Test5", &testSubParams);
    ASSERT_TRUE(0 < rc);

    testSubParams.on_message_handler = iot_subscribe_callback_handler6;
    rc = qcloud_iot_mqtt_subscribe(&client, (char *)"A58FGENPD7/Air_0/Test6", &testSubParams);
    ASSERT_EQ(QCLOUD_ERR_MQTT_MAX_SUBSCRIPTIONS, rc);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
