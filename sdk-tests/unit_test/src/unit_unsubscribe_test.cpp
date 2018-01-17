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
#include <unit_helper_functions.h>

static MQTTInitParams initParams;
static PublishParams testPubMsgParams;
static SubscribeParams testSubParams;

static Qcloud_IoT_Client iotClient;
static char CallbackMsgString[100];
char cPayload[100];

static char *sg_topic0 = (char *)"A58FGENPD7/Air_0/Test0";

static MQTTEventType sg_callback_status0 = MQTT_EVENT_UNDEF;
static MQTTEventType sg_callback_status1 = MQTT_EVENT_UNDEF;
static MQTTEventType sg_callback_status2 = MQTT_EVENT_UNDEF;
static MQTTEventType sg_callback_status3 = MQTT_EVENT_UNDEF;
static MQTTEventType sg_callback_status4 = MQTT_EVENT_UNDEF;
static MQTTEventType sg_callback_status5 = MQTT_EVENT_UNDEF;
static MQTTEventType sg_callback_status6 = MQTT_EVENT_UNDEF;

static bool sg_time_out_status = false;

static int sg_packet_id_0 = 0;
static int sg_packet_id_1 = 0;
static int sg_packet_id_2 = 0;
static int sg_packet_id_3 = 0;
static int sg_packet_id_4 = 0;
static int sg_packet_id_5 = 0;
static int sg_packet_id_6 = 0;

static bool sg_has_call_sub_handler = false;

static void handle_callback_status_in_event(int packetId, MQTTEventType type)
{
    if (packetId == sg_packet_id_0) {
        sg_callback_status0 = type;
    } else if (packetId == sg_packet_id_1) {
        sg_callback_status1 = type;
    } else if (packetId == sg_packet_id_2) {
        sg_callback_status2 = type;
    } else if (packetId == sg_packet_id_3) {
        sg_callback_status3 = type;
    } else if (packetId == sg_packet_id_4) {
        sg_callback_status4 = type;
    } else if (packetId == sg_packet_id_5) {
        sg_callback_status5 = type;
    } else if (packetId == sg_packet_id_6) {
        sg_callback_status6 = type;
    }
}

static void reset_all_status(void)
{
    sg_packet_id_0 = 0;
    sg_packet_id_1 = 0;
    sg_packet_id_2 = 0;
    sg_packet_id_3 = 0;
    sg_packet_id_4 = 0;
    sg_packet_id_5 = 0;
    sg_packet_id_6 = 0;

    sg_callback_status0 = MQTT_EVENT_UNDEF;
    sg_callback_status1 = MQTT_EVENT_UNDEF;
    sg_callback_status2 = MQTT_EVENT_UNDEF;
    sg_callback_status3 = MQTT_EVENT_UNDEF;
    sg_callback_status4 = MQTT_EVENT_UNDEF;
    sg_callback_status5 = MQTT_EVENT_UNDEF;
    sg_callback_status6 = MQTT_EVENT_UNDEF;
}

static void iot_subscribe_callback_handler(void *pClient, MQTTMessage *params, void *pUserdata) 
{
    sg_has_call_sub_handler = true;
    char *tmp = (char *)params->payload;
    unsigned int i;

    for (i = 0; i < (params->payload_len); i++) {
        CallbackMsgString[i] = tmp[i];
    }
}

static void iot_subscribe_callback_handler1(void *pClient, MQTTMessage *params, void *pUserdata) {

    char *tmp = (char *)params->payload;
    unsigned int i;

    for (i = 0; i < (params->payload_len); i++) {
        CallbackMsgString[i] = tmp[i];
    }
    Log_d("tmp=%s|payload_len=%u", tmp, params->payload_len);
}

static void event_handler(void *pclient, void *handle_context, MQTTEventMsg *msg) 
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

		case MQTT_EVENT_SUBCRIBE_SUCCESS:
			Log_i("subscribe success, packet-id=%u", (unsigned int)packet_id);
			break;

		case MQTT_EVENT_SUBCRIBE_TIMEOUT:
			Log_i("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
			break;

		case MQTT_EVENT_SUBCRIBE_NACK:
			Log_i("subscribe nack, packet-id=%u", (unsigned int)packet_id);
			break;

		case MQTT_EVENT_UNSUBCRIBE_SUCCESS:
			Log_i("unsubscribe success, packet-id=%u", (unsigned int)packet_id);
			break;

		case MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
			Log_i("unsubscribe timeout, packet-id=%u", (unsigned int)packet_id);
            sg_time_out_status = true;
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
    handle_callback_status_in_event((int)packet_id, msg->event_type);
}

class UnsubscribeTests : public testing::Test
{
protected:
    virtual void SetUp()
    {
        std::cout << "UnsubscribeTests Test Begin \n";

        IOT_Log_Set_Level(DEBUG);
        reset_all_status();

        int rc;
	    MQTTInitParamsSetup(&initParams, false);
        initParams.event_handle.h_fp = event_handler;
	    rc = qcloud_iot_mqtt_init(&iotClient, &initParams);
        if (QCLOUD_ERR_SUCCESS != rc)
            Log_e("qcloud_iot_mqtt_init fail!!");

	    ConnectParamsSetup(&iotClient.options, &initParams);
	    rc = qcloud_iot_mqtt_connect(&iotClient, &iotClient.options);
        if (QCLOUD_ERR_SUCCESS != rc)
            Log_e("qcloud_iot_mqtt_connect fail!!");

	    testPubMsgParams.qos = QOS1;
	    testPubMsgParams.retained = 0;
	    snprintf(cPayload, 100, "%s : %d ", "hello from SDK", 0);
	    testPubMsgParams.payload = (void *) cPayload;
	    testPubMsgParams.payload_len = strlen(cPayload);

	    testSubParams.qos = QOS1;
	    testSubParams.on_message_handler = iot_subscribe_callback_handler;

    }
    virtual void TearDown()
    {
        std::cout << "UnsubscribeTests Test End \n";
    } 
	
};

/* D:1 - Unsubscribe with Null/empty client instance */
TEST_F(UnsubscribeTests, UnsubscribeNullClient) 
{
    int rc = qcloud_iot_mqtt_unsubscribe(NULL, sg_topic0);
    ASSERT_EQ(QCLOUD_ERR_INVAL, rc);
}

/* D:2 - Unsubscribe with Null/empty topic name */
TEST_F(UnsubscribeTests, UnsubscribeNullTopic) {
    int rc = qcloud_iot_mqtt_unsubscribe(&iotClient, NULL);
    ASSERT_EQ(QCLOUD_ERR_INVAL, rc);
}

/* D:3 - Unsubscribe, Not subscribed to topic */
TEST_F(UnsubscribeTests, UnsubscribeNotSubscribed) {
    int rc = qcloud_iot_mqtt_unsubscribe(&iotClient, sg_topic0);
    ASSERT_EQ(QCLOUD_ERR_FAILURE, rc);
}

/* D:4 - Unsubscribe, QoS0, No response, timeout */
TEST_F(UnsubscribeTests, UnsubscribeQoS0FailureOnNoUnsuback) {
    int rc;

    // First, subscribe to a topic
    testSubParams.qos = QOS0;
    rc = qcloud_iot_mqtt_subscribe(&iotClient, sg_topic0, &testSubParams);
    if (rc > 0) {
        // Then, unsubscribe
        rc = qcloud_iot_mqtt_unsubscribe(&iotClient, sg_topic0);
        ASSERT_EQ(QCLOUD_ERR_MQTT_REQUEST_TIMEOUT, rc);
    }
}

/* D:5 - Unsubscribe, QoS1, No response, timeout */
TEST_F(UnsubscribeTests, UnsubscribeQoS1FailureOnNoUnsuback) {
    int rc;

    // First, subscribe to a topic
    testSubParams.qos = QOS1;
    rc = qcloud_iot_mqtt_subscribe(&iotClient, sg_topic0, &testSubParams);
    if (rc > 0) {
        // Then, unsubscribe
        while (sg_packet_id_0 != rc) {
            rc = qcloud_iot_mqtt_yield(&iotClient, 200);
            sleep(0.2);
        }

        rc = qcloud_iot_mqtt_unsubscribe(&iotClient, sg_topic0);
        while (sg_time_out_status == false) {
            rc = qcloud_iot_mqtt_yield(&iotClient, 200);
            ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
            sleep(7);
        }
        ASSERT_TRUE(sg_time_out_status == true);
    }

}

/* D:6 - Unsubscribe, QoS0, success */
TEST_F(UnsubscribeTests, UnsubscribeQoS0WithUnsubackSuccess) {
    int rc;

    // First, subscribe to a topic
    testSubParams.qos = QOS0;
    rc = qcloud_iot_mqtt_subscribe(&iotClient, sg_topic0, &testSubParams);
    if (rc > 0) {
        sg_packet_id_0 = rc;
        // Then, unsubscribe
        while (sg_callback_status0 == MQTT_EVENT_UNDEF) {
            rc = qcloud_iot_mqtt_yield(&iotClient, 200);
            ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
            sleep(1);
        }
        sg_callback_status0 = MQTT_EVENT_UNDEF;

        sg_packet_id_0 = qcloud_iot_mqtt_unsubscribe(&iotClient, sg_topic0);
        while(sg_callback_status0 == MQTT_EVENT_UNDEF) {
            rc = qcloud_iot_mqtt_yield(&iotClient, 200);
            ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
            sleep(1);
        }
        ASSERT_EQ(sg_callback_status0, MQTT_EVENT_UNSUBCRIBE_SUCCESS);
    }
}

// /* D:7 - Unsubscribe, QoS0, half command timeout delayed unsuback, success */
// TEST_F(UnsubscribeTests, UnsubscribeQoS0WithDelayedUnsubackSuccess) {
//     int rc;

//     // First, subscribe to a topic
//     testSubParams.qos = QOS0;
//     rc = qcloud_iot_mqtt_subscribe(&iotClient, sg_topic0, &testSubParams);
//     if (rc > 0) {
//         rc = qcloud_iot_mqtt_unsubscribe(&iotClient, sg_topic0);
//         ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
//     }

// }

/* D:8 - Unsubscribe, QoS1, success */
TEST_F(UnsubscribeTests, UnsubscribeQoS1WithUnsubackSuccess) {
    int rc;

    // First, subscribe to a topic
    testSubParams.qos = QOS1;
    rc = qcloud_iot_mqtt_subscribe(&iotClient, sg_topic0, &testSubParams);
    if (rc > 0) {
        // Then, unsubscribe
        sg_packet_id_0 = rc;
        while (sg_callback_status0 == MQTT_EVENT_UNDEF) {
            rc = qcloud_iot_mqtt_yield(&iotClient, 200);
            ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
            sleep(1);
        }
        sg_callback_status0 = MQTT_EVENT_UNDEF;

        sg_packet_id_0 = qcloud_iot_mqtt_unsubscribe(&iotClient, sg_topic0);
        while(sg_callback_status0 == MQTT_EVENT_UNDEF) {
            rc = qcloud_iot_mqtt_yield(&iotClient, 200);
            ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
            sleep(1);
        }
        ASSERT_EQ(sg_callback_status0, MQTT_EVENT_UNSUBCRIBE_SUCCESS);
    }
}

/* D:9 - Unsubscribe, QoS1, half command timeout delayed unsuback, success */
// TEST_F(UnsubscribeTests, UnsubscribeQoS1WithDelayedUnsubackSuccess) {
//     int rc;

//     // First, subscribe to a topic
//     testSubParams.qos = QOS1;
//     rc = qcloud_iot_mqtt_subscribe(&iotClient, sg_topic0, &testSubParams);
//     if (rc > 0) {
//         rc = qcloud_iot_mqtt_unsubscribe(&iotClient, sg_topic0);
//         ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
//     }

// }

/* D:10 - Unsubscribe, success, message on topic ignored
 * 1. Subscribe to topic 1
 * 2. Send message and receive it
 * 3. Unsubscribe to topic 1
 * 4. Should not receive message
 */
TEST_F(UnsubscribeTests, MsgAfterUnsubscribe) {
    int rc;
    char expectedCallbackString[100];

    // 1.
    testSubParams.qos = QOS0;
    testSubParams.on_message_handler = iot_subscribe_callback_handler;
    rc = qcloud_iot_mqtt_subscribe(&iotClient, sg_topic0, &testSubParams);
    ASSERT_TRUE(rc > 0);
    sg_packet_id_0 = rc;
    while (sg_callback_status0 == MQTT_EVENT_UNDEF) {
        // 2.
        rc = qcloud_iot_mqtt_yield(&iotClient, 200);
        ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
        sleep(1);
    }
    snprintf(expectedCallbackString, 100, "%s", (char *)"");
    ASSERT_STREQ(expectedCallbackString, CallbackMsgString);
    sg_callback_status0 = MQTT_EVENT_UNDEF;

    //3.
    rc = qcloud_iot_mqtt_unsubscribe(&iotClient, sg_topic0);
    ASSERT_TRUE(rc > 0);
    sg_packet_id_0 = rc;
    while (sg_callback_status0 == MQTT_EVENT_UNDEF) {
        rc = qcloud_iot_mqtt_yield(&iotClient, 200);
        ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
        sleep(1);
    }
    sg_callback_status0 = MQTT_EVENT_UNDEF;
    //reset the string
    snprintf(CallbackMsgString, 100, "%s", " ");

    // 4.
    // Have a new message published to that topic coming in
    snprintf(expectedCallbackString, 100, "%s", (char *)"");
    
    char *pub_payload = (char *)"Have a new message published to that topic coming in";
    PublishParams pub_msg_params;
    pub_msg_params.qos = QOS1;
    pub_msg_params.retained = 0;
    pub_msg_params.payload = (void *)pub_payload;
    pub_msg_params.payload_len = strlen(pub_payload);
    rc = qcloud_iot_mqtt_publish(&iotClient, sg_topic0, &pub_msg_params);
    sg_packet_id_0 = rc;
    ASSERT_TRUE(rc > 0);
    while (sg_callback_status0 == MQTT_EVENT_UNDEF) {
        rc = qcloud_iot_mqtt_yield(&iotClient, 200);
        ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
        sleep(1);
    }
    // No new msg was received
    ASSERT_STREQ(" ", CallbackMsgString);
}

/* D:11 - Unsubscribe after max topics reached
 * 1. Subscribe to max topics + 1 fail for last subscription
 * 2. Unsubscribe from one topic
 * 3. Subscribe again and should have no error
 * 4. Receive msg test - last subscribed topic
 */
TEST_F(UnsubscribeTests, MaxTopicsSubscription) {
    int rc;
    int i;
    char topics[MAX_MESSAGE_HANDLERS + 1][100];
    char expectedCallbackString[] = "MaxTopicsSubscription";

    // 1.
    for (i = 0; i < MAX_MESSAGE_HANDLERS; i++) {
        snprintf(topics[i], 100, "A58FGENPD7/Air_0/Test%d", i);
        testSubParams.qos = QOS0;
        testSubParams.on_message_handler = iot_subscribe_callback_handler1;
        rc = qcloud_iot_mqtt_subscribe(&iotClient, topics[i], &testSubParams);
        if (i == 0) sg_packet_id_0 = rc;
        sleep(0.5);
        ASSERT_TRUE(0 < rc);
    }

    snprintf(topics[i], 100, "A58FGENPD7/Air_0/Test%d", i);
    testSubParams.qos = QOS0;
    testSubParams.on_message_handler = iot_subscribe_callback_handler1;
    rc = qcloud_iot_mqtt_subscribe(&iotClient, topics[i], &testSubParams);
    ASSERT_EQ(QCLOUD_ERR_MQTT_MAX_SUBSCRIPTIONS, rc);
    
    while (sg_callback_status0 == MQTT_EVENT_UNDEF) {
        rc = qcloud_iot_mqtt_yield(&iotClient, 100);
        ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
        sleep(1);
    }
    sg_callback_status0 = MQTT_EVENT_UNDEF;

    // 2.
    rc = qcloud_iot_mqtt_unsubscribe(&iotClient, topics[0]);
    ASSERT_TRUE(0 < rc);
    unsigned int count = 0;
    while (count < 5) {
        rc = qcloud_iot_mqtt_yield(&iotClient, 100);
        ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
        count++;
        sleep(0.5);
    }

    //3.
    rc = qcloud_iot_mqtt_subscribe(&iotClient, topics[0], &testSubParams);
    ASSERT_TRUE(0 < rc);

    count = 0;
    while (count < 5) {
        rc = qcloud_iot_mqtt_yield(&iotClient, 100);
        ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
        count++;
        sleep(0.5);
    }

    char *pub_payload = expectedCallbackString;
    PublishParams pub_msg_params;
    pub_msg_params.qos = QOS1;
    pub_msg_params.retained = 0;
    pub_msg_params.payload = (void *)pub_payload;
    pub_msg_params.payload_len = strlen(expectedCallbackString);
    rc = qcloud_iot_mqtt_publish(&iotClient, sg_topic0, &pub_msg_params);
    sg_packet_id_0 = rc;
    ASSERT_TRUE(rc > 0);
    count = 0;
    while (count < 5) {
        rc = qcloud_iot_mqtt_yield(&iotClient, 100);
        ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
        count++;
        sleep(0.5);
    }

    //4.
    ASSERT_STREQ(expectedCallbackString, CallbackMsgString);
}

/* D:12 - Repeated Subscribe and Unsubscribe
 * 1. subscribe and unsubscribe for more than the max subscribed topic
 * 2. ensure every time the subscribed topic msg is received
 */
TEST_F(UnsubscribeTests, RepeatedSubUnSub) {
    int i;
    char expectedCallbackString[100];
    char topics[MAX_MESSAGE_HANDLERS][100];

    for (i = 0; i < MAX_MESSAGE_HANDLERS; i++) {
        //1.
        int rc;
        snprintf(topics[i], 100, "A58FGENPD7/Air_0/Test%d", i);
        testSubParams.qos = QOS0;
        testSubParams.on_message_handler = iot_subscribe_callback_handler;
        rc = qcloud_iot_mqtt_subscribe(&iotClient, topics[i], &testSubParams);
        ASSERT_TRUE(0 < rc);

        sg_packet_id_0 = rc;
        sg_callback_status0 = MQTT_EVENT_UNDEF;
        while (sg_callback_status0 != MQTT_EVENT_SUBCRIBE_SUCCESS) {
            rc = qcloud_iot_mqtt_yield(&iotClient, 100);
            ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
            sleep(1);
        }
        
        sprintf(expectedCallbackString, "%s%d", "expectedCallbackString", i);
        char *pub_payload = expectedCallbackString;
        PublishParams pub_msg_params;
        pub_msg_params.qos = QOS1;
        pub_msg_params.retained = 0;
        pub_msg_params.payload = (void *)pub_payload;
        pub_msg_params.payload_len = strlen(expectedCallbackString);
        rc = qcloud_iot_mqtt_publish(&iotClient, topics[i], &pub_msg_params);
        ASSERT_TRUE(0 < rc);
        sg_packet_id_0 = rc;
        sg_callback_status0 = MQTT_EVENT_UNDEF;
        while (sg_has_call_sub_handler == false) {
            rc = qcloud_iot_mqtt_yield(&iotClient, 100);
            ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
            sleep(1);
        }
        ASSERT_STREQ(expectedCallbackString, CallbackMsgString);
        memset(CallbackMsgString, 0x0, 100);

        //2.
        rc = qcloud_iot_mqtt_unsubscribe(&iotClient, topics[i]);
        ASSERT_TRUE(0 < rc);
        sg_packet_id_0 = rc;
        sg_callback_status0 = MQTT_EVENT_UNDEF;
        while (sg_callback_status0 != MQTT_EVENT_UNSUBCRIBE_SUCCESS) {
            rc = qcloud_iot_mqtt_yield(&iotClient, 100);
            ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
            sleep(1);
        }
                
        sg_has_call_sub_handler = false;
        ASSERT_TRUE(sg_callback_status0 == MQTT_EVENT_UNSUBCRIBE_SUCCESS);
    }

}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
