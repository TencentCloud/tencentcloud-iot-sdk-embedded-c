#include <stdio.h>
#include <string.h>
#include <CppUTest/TestHarness_c.h>
#include <qcloud_iot_mqtt_client_interface.h>
#include <unit_helper_functions.h>

static MQTTConnectParams connectParams;
static PublishParams testPubMsgParams;
static SubscribeParams testSubParams;
static char subTopic[10] = "sdk/Test";

static Qcloud_IoT_Client iotClient;
static char CallbackMsgString[100];
char cPayload[100];

static void iot_subscribe_callback_handler(char *topicName, size_t topicNameLen, MQTTMessage *params, void *pUserdata) {


    char *tmp = params->payload;
    unsigned int i;

    for (i = 0; i < (params->payload_len); i++) {
        CallbackMsgString[i] = tmp[i];
    }
}

static void iot_subscribe_callback_handler1(char *topicName, size_t topicNameLen, MQTTMessage *params, void *pUserdata) {

    char *tmp = params->payload;
    unsigned int i;

    for (i = 0; i < (params->payload_len); i++) {
        CallbackMsgString[i] = tmp[i];
    }
}

TEST_GROUP_C_SETUP(UnsubscribeTests) {
    int rc = QCLOUD_ERR_SUCCESS;
    resetTlsBuffer();
    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    rc = qcloud_iot_mqtt_init(&iotClient, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
    setTLSRxBufferForConnack(&connectParams, 0, 0);
    rc = qcloud_iot_mqtt_connect(&iotClient, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    testPubMsgParams.qos = QOS1;
    testPubMsgParams.retained = 0;
    snprintf(cPayload, 100, "%s : %d ", "hello from SDK", 0);
    testPubMsgParams.payload = (void *) cPayload;
    testPubMsgParams.payload_len = strlen(cPayload);

    resetTlsBuffer();
}

TEST_GROUP_C_TEARDOWN(UnsubscribeTests) {}

/* D:1 - Unsubscribe with Null/empty client instance */
TEST_C(UnsubscribeTests, UnsubscribeNullClient) {
    int rc = qcloud_iot_mqtt_unsubscribe(NULL, "sdkTest/Sub");
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

/* D:2 - Unsubscribe with Null/empty topic name */
TEST_C(UnsubscribeTests, UnsubscribeNullTopic) {
    int rc = qcloud_iot_mqtt_unsubscribe(&iotClient, NULL);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

/* D:3 - Unsubscribe, Not subscribed to topic */
TEST_C(UnsubscribeTests, UnsubscribeNotSubscribed) {
    int rc = qcloud_iot_mqtt_unsubscribe(&iotClient, "sdkTest/Sub");
    CHECK_EQUAL_C_INT(QCLOUD_ERR_FAILURE, rc);
}

/* D:4 - Unsubscribe, QoS0, No response, timeout */
TEST_C(UnsubscribeTests, unsubscribeQoS0FailureOnNoUnsuback) {
    int rc = QCLOUD_ERR_SUCCESS;

    // First, subscribe to a topic
    testSubParams.qos = QOS0;
    setTLSRxBufferForSuback(QOS0);
    rc = qcloud_iot_mqtt_subscribe(&iotClient, subTopic, &testSubParams);
    if (QCLOUD_ERR_SUCCESS == rc) {
        // Then, unsubscribe
        rc = qcloud_iot_mqtt_unsubscribe(&iotClient, subTopic);
        CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_REQUEST_TIMEOUT, rc);
    }
}

/* D:5 - Unsubscribe, QoS1, No response, timeout */
TEST_C(UnsubscribeTests, unsubscribeQoS1FailureOnNoUnsuback) {
    int rc = QCLOUD_ERR_SUCCESS;

    // First, subscribe to a topic
    testSubParams.qos = QOS1;
    setTLSRxBufferForSuback(QOS1);
    rc = qcloud_iot_mqtt_subscribe(&iotClient, subTopic, &testSubParams);
    if (QCLOUD_ERR_SUCCESS == rc) {
        // Then, unsubscribe
        rc = qcloud_iot_mqtt_unsubscribe(&iotClient, subTopic);
        CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_REQUEST_TIMEOUT, rc);
    }

}

/* D:6 - Unsubscribe, QoS0, success */
TEST_C(UnsubscribeTests, unsubscribeQoS0WithUnsubackSuccess) {
    int rc = QCLOUD_ERR_SUCCESS;

    // First, subscribe to a topic
    testSubParams.qos = QOS0;
    setTLSRxBufferForSuback(QOS0);
    rc = qcloud_iot_mqtt_subscribe(&iotClient, subTopic, &testSubParams);
    if (QCLOUD_ERR_SUCCESS == rc) {
        // Then, unsubscribe
        setTLSRxBufferForUnsuback();
        rc = qcloud_iot_mqtt_unsubscribe(&iotClient, subTopic);
        CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    }

}

/* D:7 - Unsubscribe, QoS0, half command timeout delayed unsuback, success */
TEST_C(UnsubscribeTests, unsubscribeQoS0WithDelayedUnsubackSuccess) {
    int rc = QCLOUD_ERR_SUCCESS;

    // First, subscribe to a topic
    testSubParams.qos = QOS0;
    setTLSRxBufferForSuback(QOS0);
    rc = qcloud_iot_mqtt_subscribe(&iotClient, subTopic, &testSubParams);
    if (QCLOUD_ERR_SUCCESS == rc) {
        // Then, unsubscribe
        setTLSRxBufferForUnsuback();
        setTLSRxBufferDelay(0, (int) iotClient.command_timeout_ms / 2);
        rc = qcloud_iot_mqtt_unsubscribe(&iotClient, subTopic);
        CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    }

}

/* D:8 - Unsubscribe, QoS1, success */
TEST_C(UnsubscribeTests, unsubscribeQoS1WithUnsubackSuccess) {
    int rc = QCLOUD_ERR_SUCCESS;

    // First, subscribe to a topic
    testSubParams.qos = QOS1;
    setTLSRxBufferForSuback(QOS1);
    rc = qcloud_iot_mqtt_subscribe(&iotClient, subTopic, &testSubParams);
    if (QCLOUD_ERR_SUCCESS == rc) {
        // Then, unsubscribe
        setTLSRxBufferForUnsuback();
        rc = qcloud_iot_mqtt_unsubscribe(&iotClient, subTopic);
        CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    }

}

/* D:9 - Unsubscribe, QoS1, half command timeout delayed unsuback, success */
TEST_C(UnsubscribeTests, unsubscribeQoS1WithDelayedUnsubackSuccess) {
    int rc = QCLOUD_ERR_SUCCESS;

    // First, subscribe to a topic
    testSubParams.qos = QOS1;
    setTLSRxBufferForSuback(QOS1);
    rc = qcloud_iot_mqtt_subscribe(&iotClient, subTopic, &testSubParams);
    if (QCLOUD_ERR_SUCCESS == rc) {
        // Then, unsubscribe
        setTLSRxBufferForUnsuback();
        setTLSRxBufferDelay(0, (int) iotClient.command_timeout_ms / 2);
        rc = qcloud_iot_mqtt_unsubscribe(&iotClient, subTopic);
        CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    }

}

/* D:10 - Unsubscribe, success, message on topic ignored
 * 1. Subscribe to topic 1
 * 2. Send message and receive it
 * 3. Unsubscribe to topic 1
 * 4. Should not receive message
 */
TEST_C(UnsubscribeTests, MsgAfterUnsubscribe) {
    int rc = QCLOUD_ERR_SUCCESS;
    char expectedCallbackString[100];

    // 1.
    testSubParams.qos = QOS0;
    testSubParams.on_message_handler = iot_subscribe_callback_handler;
    setTLSRxBufferForSuback(QOS0);
    rc = qcloud_iot_mqtt_subscribe(&iotClient, "topic1", &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    // 2.
    snprintf(expectedCallbackString, 100, "Message for topic1");
    testPubMsgParams.qos = QOS0;
    setTLSRxBufferWithMsgOnSubscribedTopic("topic1", strlen("topic1"), QOS0, expectedCallbackString);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    CHECK_EQUAL_C_STRING(expectedCallbackString, CallbackMsgString);

    //3.
    setTLSRxBufferForUnsuback();
    rc = qcloud_iot_mqtt_unsubscribe(&iotClient, "topic1");
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    //reset the string
    snprintf(CallbackMsgString, 100, " ");

    // 4.
    // Have a new message published to that topic coming in
    snprintf(expectedCallbackString, 100, "Message after unsubscribe");
    testPubMsgParams.qos = QOS0;
    setTLSRxBufferWithMsgOnSubscribedTopic("topic1", strlen("topic1"), QOS0, expectedCallbackString);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    // No new msg was received
    CHECK_EQUAL_C_STRING(" ", CallbackMsgString);
}

/* D:11 - Unsubscribe after max topics reached
 * 1. Subscribe to max topics + 1 fail for last subscription
 * 2. Unsubscribe from one topic
 * 3. Subscribe again and should have no error
 * 4. Receive msg test - last subscribed topic
 */
TEST_C(UnsubscribeTests, MaxTopicsSubscription) {
    int rc = QCLOUD_ERR_SUCCESS;
    int i = 0;
    char topics[MAX_MESSAGE_HANDLERS + 1][10];
    char expectedCallbackString[] = "Message after subscribe - topic[i]";

    // 1.
    for (i = 0; i < MAX_MESSAGE_HANDLERS; i++) {
        snprintf(topics[i], 10, "topic-%d", i);
        testSubParams.qos = QOS0;
        testSubParams.on_message_handler = iot_subscribe_callback_handler1;
        setTLSRxBufferForSuback(QOS0);
        rc = qcloud_iot_mqtt_subscribe(&iotClient, topics[i], &testSubParams);
        CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    }
    snprintf(topics[i], 10, "topic-%d", i);
    testSubParams.qos = QOS0;
    testSubParams.on_message_handler = iot_subscribe_callback_handler1;
    setTLSRxBufferForSuback(QOS0);
    rc = qcloud_iot_mqtt_subscribe(&iotClient, topics[i], &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_MAX_SUBSCRIPTIONS, rc);

    // 2.
    resetTlsBuffer();
    setTLSRxBufferForUnsuback();
    rc = qcloud_iot_mqtt_unsubscribe(&iotClient, topics[i - 1]);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    //3.
    resetTlsBuffer();
    setTLSRxBufferForSuback(QOS0);
    rc = qcloud_iot_mqtt_subscribe(&iotClient, topics[i], &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    //4.
    testPubMsgParams.qos = QOS0;
    setTLSRxBufferWithMsgOnSubscribedTopic(topics[i], strlen(topics[i]), QOS0, expectedCallbackString);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    CHECK_EQUAL_C_STRING(expectedCallbackString, CallbackMsgString);

}

/* D:12 - Repeated Subscribe and Unsubscribe
 * 1. subscribe and unsubscribe for more than the max subscribed topic
 * 2. ensure every time the subscribed topic msg is received
 */
TEST_C(UnsubscribeTests, RepeatedSubUnSub) {
    int rc = QCLOUD_ERR_SUCCESS;
    int i = 0;
    char expectedCallbackString[100];
    char topics[3 * MAX_MESSAGE_HANDLERS][10];


    for (i = 0; i < 3 * MAX_MESSAGE_HANDLERS; i++) {
        //1.
        snprintf(topics[i], 10, "topic-%d", i);
        testSubParams.qos = QOS0;
        testSubParams.on_message_handler = iot_subscribe_callback_handler;
        setTLSRxBufferForSuback(QOS0);
        rc = qcloud_iot_mqtt_subscribe(&iotClient, topics[i], &testSubParams);
        CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
        snprintf(expectedCallbackString, 10, "message##%d", i);
        testPubMsgParams.payload = (void *) expectedCallbackString;
        testPubMsgParams.payload_len = strlen(expectedCallbackString);
        setTLSRxBufferWithMsgOnSubscribedTopic(topics[i], strlen(topics[i]), QOS0, expectedCallbackString);
        rc = qcloud_iot_mqtt_yield(&iotClient, 100);
        CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
        CHECK_EQUAL_C_STRING(expectedCallbackString, CallbackMsgString);

        //2.
        setTLSRxBufferForUnsuback();
        rc = qcloud_iot_mqtt_unsubscribe(&iotClient, topics[i]);
        CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    }

}
