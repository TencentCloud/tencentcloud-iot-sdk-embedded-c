#include <CppUTest/TestHarness_c.h>
#include <qcloud_iot_mqtt_client_interface.h>
#include <unit_helper_functions.h>
#include <string.h>

static MQTTConnectParams connectParams;
static PublishParams testPubMsgParams;
static SubscribeParams testSubParams;
static Qcloud_IoT_Client client;
static char subTopic[10] = "sub/Test";
static char CallbackMsgString[100];
char cPayload[100];

static char CallbackMsgString1[100] = {"XXXX"};
static char CallbackMsgString2[100] = {"XXXX"};
static char CallbackMsgString3[100] = {"XXXX"};
static char CallbackMsgString4[100] = {"XXXX"};
static char CallbackMsgString5[100] = {"XXXX"};
static char CallbackMsgString6[100] = {"XXXX"};

static void iot_subscribe_callback_handler(char *topicName, size_t topicNameLen, MQTTMessage *params, void *pUserdata) {

    if (params == NULL) {
        return;
    }

    char *tmp = params->payload;
    unsigned int i;

    for (i = 0; i < (params->payload_len); i++) {
        CallbackMsgString[i] = tmp[i];
    }
}

static void
iot_subscribe_callback_handler1(char *topicName, size_t topicNameLen, MQTTMessage *params, void *pUserdata) {
    if (params == NULL) {
        return;
    }

    char *tmp = params->payload;
    unsigned int i;

    printf("callback topic %.*s\n", (int) topicNameLen, topicName);
    for (i = 0; i < (params->payload_len); i++) {
        CallbackMsgString1[i] = tmp[i];
    }
}

static void
iot_subscribe_callback_handler2(char *topicName, size_t topicNameLen, MQTTMessage *params, void *pUserdata) {
    if (params == NULL) {
        return;
    }

    char *tmp = params->payload;
    unsigned int i;

    for (i = 0; i < (params->payload_len); i++) {
        CallbackMsgString2[i] = tmp[i];
    }
}

static void
iot_subscribe_callback_handler3(char *topicName, size_t topicNameLen, MQTTMessage *params, void *pUserdata) {
    if (params == NULL) {
        return;
    }

    char *tmp = params->payload;
    unsigned int i;

    for (i = 0; i < (params->payload_len); i++) {
        CallbackMsgString3[i] = tmp[i];
    }
}

static void
iot_subscribe_callback_handler4(char *topicName, size_t topicNameLen, MQTTMessage *params, void *pUserdata) {
    if (params == NULL) {
        return;
    }

    char *tmp = params->payload;
    unsigned int i;

    for (i = 0; i < (params->payload_len); i++) {
        CallbackMsgString4[i] = tmp[i];
    }
}

static void
iot_subscribe_callback_handler5(char *topicName, size_t topicNameLen, MQTTMessage *params, void *pUserdata) {
    if (params == NULL) {
        return;
    }

    char *tmp = params->payload;
    unsigned int i;

    for (i = 0; i < (params->payload_len); i++) {
        CallbackMsgString5[i] = tmp[i];
    }
}

static void
iot_subscribe_callback_handler6(char *topicName, size_t topicNameLen, MQTTMessage *params, void *pUserdata) {
    if (params == NULL) {
        return;
    }

    char *tmp = params->payload;
    unsigned int i;

    for (i = 0; i < (params->payload_len); i++) {
        CallbackMsgString6[i] = tmp[i];
    }
}

TEST_GROUP_C_SETUP(SubscribeTests) {
    int rc;
    resetTlsBuffer();
    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
    setTLSRxBufferForConnack(&connectParams, 0, 0);
    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    testPubMsgParams.qos = QOS1;
    testPubMsgParams.retained = 0;
    snprintf(cPayload, 100, "%s : %d ", "hello from SDK", 0);
    testPubMsgParams.payload = (void *) cPayload;
    testPubMsgParams.payload_len = strlen(cPayload);

    testSubParams.qos = QOS1;
    testSubParams.on_message_handler = iot_subscribe_callback_handler;

    resetTlsBuffer();
}

TEST_GROUP_C_TEARDOWN(SubscribeTests) {}

/* C:1 - Subscribe with Null/empty Client Instance */
TEST_C(SubscribeTests, SubscribeNullClient) {
    int rc = qcloud_iot_mqtt_subscribe(NULL, subTopic, &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

/* C:2 - Subscribe with Null/empty Topic Name */
TEST_C(SubscribeTests, SubscribeNullTopic) {
    int rc = qcloud_iot_mqtt_subscribe(&client, NULL, &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);

    rc = qcloud_iot_mqtt_subscribe(&client, "", &testSubParams);
}

/* C:3 - Subscribe with Null client callback */
TEST_C(SubscribeTests, SubscribeNullSubscribeHandler) {
    testSubParams.on_message_handler = NULL;
    int rc = qcloud_iot_mqtt_subscribe(&client, subTopic, &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}
/* C:4 - Subscribe with Null subParams*/
TEST_C(SubscribeTests, SubscribeNullParams) {
    int rc = qcloud_iot_mqtt_subscribe(&client, subTopic, NULL);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

/* C:5 - Subscribe with no connection */
TEST_C(SubscribeTests, SubscribeNoConnection) {
    /* Disconnect first */
    int rc = qcloud_iot_mqtt_disconnect(&client);

    rc = qcloud_iot_mqtt_subscribe(&client, subTopic, &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_NO_CONN, rc);
}
/* C:6 - Subscribe QoS2, error */
/* Not required, QoS enum doesn't have value for QoS2 */

/* C:7 - Subscribe attempt, QoS0, no response timeout */
TEST_C(SubscribeTests, subscribeQoS0FailureOnNoSuback) {
    int rc = QCLOUD_ERR_SUCCESS;
    testSubParams.qos = QOS0;
    rc = qcloud_iot_mqtt_subscribe(&client, subTopic, &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_REQUEST_TIMEOUT, rc);

}
/* C:8 - Subscribe attempt, QoS1, no response timeout */
TEST_C(SubscribeTests, subscribeQoS1FailureOnNoSuback) {
    int rc = QCLOUD_ERR_SUCCESS;

    rc = qcloud_iot_mqtt_subscribe(&client, subTopic, &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_REQUEST_TIMEOUT, rc);

}

/* 0x80 Error subscribe ack*/
TEST_C(SubscribeTests, subscribeFailed) {
    int rc = QCLOUD_ERR_SUCCESS;
    testSubParams.qos = QOS0;
    setTLSRxBufferForSuback(0x80);
    rc = qcloud_iot_mqtt_subscribe(&client, subTopic, &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_SUB, rc);
}

/* C:9 - Subscribe QoS0 success, suback received */
TEST_C(SubscribeTests, subscribeQoS0Success) {
    int rc = QCLOUD_ERR_SUCCESS;
    testSubParams.qos = QOS0;
    setTLSRxBufferForSuback(QOS0);
    rc = qcloud_iot_mqtt_subscribe(&client, subTopic, &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

}

/* C:10 - Subscribe QoS1 success, suback received */
TEST_C(SubscribeTests, subscribeQoS1Success) {
    int rc = QCLOUD_ERR_SUCCESS;

    setTLSRxBufferForSuback(QOS1);
    rc = qcloud_iot_mqtt_subscribe(&client, subTopic, &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

}

/* C:11 - Subscribe, QoS0, delayed suback, success */
TEST_C(SubscribeTests, subscribeQoS0WithDelayedSubackSuccess) {
    int rc = QCLOUD_ERR_SUCCESS;

    testSubParams.qos = QOS0;
    setTLSRxBufferForSuback(QOS0);
    setTLSRxBufferDelay(0, (int) client.command_timeout_ms / 2);
    rc = qcloud_iot_mqtt_subscribe(&client, subTopic, &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
}

/* C:12 - Subscribe, QoS1, delayed suback, success */
TEST_C(SubscribeTests, subscribeQoS1WithDelayedSubackSuccess) {
    int rc = QCLOUD_ERR_SUCCESS;

    setTLSRxBufferForSuback(QOS1);
    setTLSRxBufferDelay(0, (int) client.command_timeout_ms / 2);
    rc = qcloud_iot_mqtt_subscribe(&client, subTopic, &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
}

/* C:13 - Subscribe QoS0 success, no puback sent on message */
TEST_C(SubscribeTests, subscribeQoS0MsgReceivedAndNoPubackSent) {
    int rc = QCLOUD_ERR_SUCCESS;
    char expectedCallbackString[100] = "Test msg - unit test";

    testSubParams.qos = QOS0;
    resetTlsBuffer();
    setTLSRxBufferForSuback(QOS0);
    rc = qcloud_iot_mqtt_subscribe(&client, subTopic, &testSubParams);
    if (QCLOUD_ERR_SUCCESS == rc) {
        testPubMsgParams.qos = QOS0;
        setTLSRxBufferWithMsgOnSubscribedTopic(subTopic, strlen(subTopic), QOS0, expectedCallbackString);
        rc = qcloud_iot_mqtt_yield(&client, 1000);
        if (QCLOUD_ERR_SUCCESS == rc) {
            CHECK_EQUAL_C_STRING(expectedCallbackString, CallbackMsgString);
        }
    }
    CHECK_EQUAL_C_INT(0, isLastTLSTxMessagePuback());
}

/* C:14 - Subscribe QoS1 success, puback sent on message */
TEST_C(SubscribeTests, subscribeQoS1MsgReceivedAndSendPuback) {
    int rc = QCLOUD_ERR_SUCCESS;
    char expectedCallbackString[] = "0xA5A5A3";

    setTLSRxBufferForSuback(QOS1);
    rc = qcloud_iot_mqtt_subscribe(&client, subTopic, &testSubParams);
    if (QCLOUD_ERR_SUCCESS == rc) {
        setTLSRxBufferWithMsgOnSubscribedTopic(subTopic, strlen(subTopic), QOS1, expectedCallbackString);
        rc = qcloud_iot_mqtt_yield(&client, 1000);
        if (QCLOUD_ERR_SUCCESS == rc) {
            CHECK_EQUAL_C_STRING(expectedCallbackString, CallbackMsgString);
        }
        CHECK_EQUAL_C_INT(1, isLastTLSTxMessagePuback());
    }
}

/* C:15 - Subscribe, malformed response */
TEST_C(SubscribeTests, subscribeMalformedResponse) {}

/* C:16 - Subscribe, multiple topics, messages on each topic */
TEST_C(SubscribeTests, SubscribeToMultipleTopicsSuccess) {
    int rc = QCLOUD_ERR_SUCCESS;
    char expectedCallbackString[] = "0xA5A5A3";

    setTLSRxBufferForSuback(QOS1);
    rc = qcloud_iot_mqtt_subscribe(&client, "sdk/Test1", &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    testSubParams.on_message_handler = iot_subscribe_callback_handler2;
    setTLSRxBufferForSuback(QOS1);
    rc = qcloud_iot_mqtt_subscribe(&client, "sdk/Test2", &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    setTLSRxBufferWithMsgOnSubscribedTopic("sdk/Test1", strlen("sdk/Test1"), QOS1, expectedCallbackString);
    rc = qcloud_iot_mqtt_yield(&client, 1000);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    CHECK_EQUAL_C_STRING(expectedCallbackString, CallbackMsgString);

}
/* C:17 - Subscribe, max topics, messages on each topic */
TEST_C(SubscribeTests, SubcribeToMaxAllowedTopicsSuccess) {
    int rc = QCLOUD_ERR_SUCCESS;
    char expectedCallbackString[] = "topics sdk/Test1";
    char expectedCallbackString2[] = "topics sdk/Test2";
    char expectedCallbackString3[] = "topics sdk/Test3";
    char expectedCallbackString4[] = "topics sdk/Test4";
    char expectedCallbackString5[] = "topics sdk/Test5";

    testSubParams.on_message_handler = iot_subscribe_callback_handler1;
    setTLSRxBufferForSuback(QOS1);
    rc = qcloud_iot_mqtt_subscribe(&client, "sdk/Test1", &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    testSubParams.on_message_handler = iot_subscribe_callback_handler2;
    setTLSRxBufferForSuback(QOS1);
    rc = qcloud_iot_mqtt_subscribe(&client, "sdk/Test2", &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    testSubParams.on_message_handler = iot_subscribe_callback_handler3;
    setTLSRxBufferForSuback(QOS1);
    rc = qcloud_iot_mqtt_subscribe(&client, "sdk/Test3", &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    testSubParams.on_message_handler = iot_subscribe_callback_handler4;
    setTLSRxBufferForSuback(QOS1);
    rc = qcloud_iot_mqtt_subscribe(&client, "sdk/Test4", &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    testSubParams.on_message_handler = iot_subscribe_callback_handler5;
    setTLSRxBufferForSuback(QOS1);
    rc = qcloud_iot_mqtt_subscribe(&client, "sdk/Test5", &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    setTLSRxBufferWithMsgOnSubscribedTopic("sdk/Test1", strlen("sdk/Test1"), QOS1, expectedCallbackString);
    rc = qcloud_iot_mqtt_yield(&client, 1000);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    setTLSRxBufferWithMsgOnSubscribedTopic("sdk/Test2", strlen("sdk/Test2"), QOS1, expectedCallbackString2);
    rc = qcloud_iot_mqtt_yield(&client, 1000);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    setTLSRxBufferWithMsgOnSubscribedTopic("sdk/Test3", strlen("sdk/Test3"), QOS1, expectedCallbackString3);
    rc = qcloud_iot_mqtt_yield(&client, 1000);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    setTLSRxBufferWithMsgOnSubscribedTopic("sdk/Test4", strlen("sdk/Test4"), QOS1, expectedCallbackString4);
    rc = qcloud_iot_mqtt_yield(&client, 1000);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    setTLSRxBufferWithMsgOnSubscribedTopic("sdk/Test5", strlen("sdk/Test5"), QOS1, expectedCallbackString5);
    rc = qcloud_iot_mqtt_yield(&client, 1000);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    CHECK_EQUAL_C_STRING(expectedCallbackString, CallbackMsgString1);
    CHECK_EQUAL_C_STRING(expectedCallbackString2, CallbackMsgString2);
    CHECK_EQUAL_C_STRING(expectedCallbackString3, CallbackMsgString3);
    CHECK_EQUAL_C_STRING(expectedCallbackString4, CallbackMsgString4);
    CHECK_EQUAL_C_STRING(expectedCallbackString5, CallbackMsgString5);

}
/* C:18 - Subscribe, max topics, another subscribe */
TEST_C(SubscribeTests, SubcribeToMaxPlusOneAllowedTopicsFailure) {
    int rc = QCLOUD_ERR_SUCCESS;

    testSubParams.on_message_handler = iot_subscribe_callback_handler1;
    setTLSRxBufferForSuback(QOS1);
    rc = qcloud_iot_mqtt_subscribe(&client, "sdk/Test1", &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    testSubParams.on_message_handler = iot_subscribe_callback_handler2;
    setTLSRxBufferForSuback(QOS1);
    rc = qcloud_iot_mqtt_subscribe(&client, "sdk/Test2", &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    testSubParams.on_message_handler = iot_subscribe_callback_handler3;
    setTLSRxBufferForSuback(QOS1);
    rc = qcloud_iot_mqtt_subscribe(&client, "sdk/Test3", &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    testSubParams.on_message_handler = iot_subscribe_callback_handler4;
    setTLSRxBufferForSuback(QOS1);
    rc = qcloud_iot_mqtt_subscribe(&client, "sdk/Test4", &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    testSubParams.on_message_handler = iot_subscribe_callback_handler5;
    setTLSRxBufferForSuback(QOS1);
    rc = qcloud_iot_mqtt_subscribe(&client, "sdk/Test5", &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    testSubParams.on_message_handler = iot_subscribe_callback_handler6;
    setTLSRxBufferForSuback(QOS1);
    rc = qcloud_iot_mqtt_subscribe(&client, "sdk/Test6", &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_MAX_SUBSCRIPTIONS, rc);
}

/* C:19 - Subscribe, '#' not last character in topic name, Failure */
TEST_C(SubscribeTests, subscribeTopicWithHashkeyAllSubTopicSuccess) {
    int rc = QCLOUD_ERR_SUCCESS;
    char expectedCallbackString[100] = "New message: sub1, Hashkey";

    // Set up the subscribed topic, including '#'
    setTLSRxBufferForSuback(QOS1);
    rc = qcloud_iot_mqtt_subscribe(&client, "sdk/Test/#", &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    // Now provide a published message from a sub topic
    setTLSRxBufferWithMsgOnSubscribedTopic("sdk/Test", strlen("sdk/Test"), QOS1, expectedCallbackString);
    snprintf(CallbackMsgString, 100, "NOT_VISITED");

    rc = qcloud_iot_mqtt_yield(&client, 1000);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    CHECK_EQUAL_C_STRING(expectedCallbackString, CallbackMsgString);
    CHECK_EQUAL_C_INT(1, isLastTLSTxMessagePuback());

    // Re-initialize Rx Tx Buffer
    resetTlsBuffer();

    // Now provide another message from a different sub topic
    snprintf(expectedCallbackString, 100, "New message: sub2, Hashkey");
    setTLSRxBufferWithMsgOnSubscribedTopic("sdk/Test/", strlen("sdk/Test/"), QOS1, expectedCallbackString);
    snprintf(CallbackMsgString, 100, "NOT_VISITED");

    rc = qcloud_iot_mqtt_yield(&client, 1000);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    CHECK_EQUAL_C_STRING(expectedCallbackString, CallbackMsgString);
    CHECK_EQUAL_C_INT(1, isLastTLSTxMessagePuback());

    resetTlsBuffer();

    // Now provide another message from a different sub topic
    snprintf(expectedCallbackString, 100, "New message: sub3, Hashkey");
    setTLSRxBufferWithMsgOnSubscribedTopic("sdk/Test/sub", strlen("sdk/Test/sub"), QOS1, expectedCallbackString);
    snprintf(CallbackMsgString, 100, "NOT_VISITED");

    rc = qcloud_iot_mqtt_yield(&client, 1000);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    CHECK_EQUAL_C_STRING(expectedCallbackString, CallbackMsgString);
    CHECK_EQUAL_C_INT(1, isLastTLSTxMessagePuback());

    // Now provide another message from a different sub topic
    snprintf(expectedCallbackString, 100, "New message: sub4, Hashkey");
    setTLSRxBufferWithMsgOnSubscribedTopic("sdk/Test/sub/sub4", strlen("sdk/Test/sub/sub4"), QOS1,
                                           expectedCallbackString);
    snprintf(CallbackMsgString, 100, "NOT_VISITED");

    rc = qcloud_iot_mqtt_yield(&client, 1000);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    CHECK_EQUAL_C_STRING(expectedCallbackString, CallbackMsgString);
    CHECK_EQUAL_C_INT(1, isLastTLSTxMessagePuback());

}
/* C:20 - Subscribe with '#', subscribed to all subtopics */
TEST_C(SubscribeTests, subscribeTopicHashkeyMustBeTheLastFail) {
    int rc = QCLOUD_ERR_SUCCESS;
    char expectedCallbackString[100] = "New message: foo1/sub, Hashkey";

    // Set up the subscribed topic, with '#' in the middle
    // Topic directory not permitted, should fail
    setTLSRxBufferForSuback(QOS1);
    rc = qcloud_iot_mqtt_subscribe(&client, "sdk/#/sub", &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    resetTlsBuffer();
    // Now provide a published message from a sub directoy with this sub topic
    setTLSRxBufferWithMsgOnSubscribedTopic("sdk/Test/foo1/sub", strlen("sdk/Test/foo1/sub"), QOS1,
                                           expectedCallbackString);
    snprintf(CallbackMsgString, 100, "NOT_VISITED");

    rc = qcloud_iot_mqtt_yield(&client, 1000);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    CHECK_EQUAL_C_STRING("NOT_VISITED", CallbackMsgString);

}
/* C:21 - Subscribe with '+' as wildcard success */
TEST_C(SubscribeTests, subscribeTopicWithPluskeySuccess) {
    int rc = QCLOUD_ERR_SUCCESS;
    char expectedCallbackString[100] = "New message: 1/sub, Pluskey";

    // Set up the subscribed topic, including '+'
    setTLSRxBufferForSuback(QOS1);
    rc = qcloud_iot_mqtt_subscribe(&client, "sdk/Test/+/sub", &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    // Now provide a published message from a sub topic
    setTLSRxBufferWithMsgOnSubscribedTopic("sdk/Test/1/sub", strlen("sdk/Test/1/sub"), QOS1, expectedCallbackString);
    snprintf(CallbackMsgString, 100, "NOT_VISITED");

    rc = qcloud_iot_mqtt_yield(&client, 1000);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    CHECK_EQUAL_C_STRING(expectedCallbackString, CallbackMsgString);
    CHECK_EQUAL_C_INT(1, isLastTLSTxMessagePuback());

    // Re-initialize Rx Tx Buffer
    resetTlsBuffer();

    // Now provide another message from a different sub topic
    snprintf(expectedCallbackString, 100, "New message: 2/sub, Pluskey");
    setTLSRxBufferWithMsgOnSubscribedTopic("sdk/Test/2/sub", strlen("sdk/Test/2/sub"), QOS1, expectedCallbackString);
    snprintf(CallbackMsgString, 100, "NOT_VISITED");

    rc = qcloud_iot_mqtt_yield(&client, 1000);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    CHECK_EQUAL_C_STRING(expectedCallbackString, CallbackMsgString);
    CHECK_EQUAL_C_INT(1, isLastTLSTxMessagePuback());
}
/* C:22 - Subscribe with '+' as last character in topic name, Success */
TEST_C(SubscribeTests, subscribeTopicPluskeyComesLastSuccess) {
    int rc = QCLOUD_ERR_SUCCESS;
    char expectedCallbackString[100] = "New message: foo1, Pluskey";

    // Set up the subscribed topic, with '+' comes the last
    setTLSRxBufferForSuback(QOS1);
    rc = qcloud_iot_mqtt_subscribe(&client, "sdk/Test/+", &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    // Now provide a published message from a single layer of sub directroy
    setTLSRxBufferWithMsgOnSubscribedTopic("sdk/Test/foo1", strlen("sdk/Test/foo1"), QOS1, expectedCallbackString);
    snprintf(CallbackMsgString, 100, "NOT_VISITED");

    rc = qcloud_iot_mqtt_yield(&client, 1000);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    CHECK_EQUAL_C_STRING(expectedCallbackString, CallbackMsgString);
    CHECK_EQUAL_C_INT(1, isLastTLSTxMessagePuback());

    // Re-initialize Rx Tx Buffer
    resetTlsBuffer();

    // Now provide a published message from another single layer of sub directroy
    snprintf(expectedCallbackString, 100, "New message: foo2, Pluskey");
    setTLSRxBufferWithMsgOnSubscribedTopic("sdk/Test/foo2", strlen("sdk/Test/foo2"), QOS1, expectedCallbackString);
    snprintf(CallbackMsgString, 100, "NOT_VISITED");

    rc = qcloud_iot_mqtt_yield(&client, 1000);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    CHECK_EQUAL_C_STRING(expectedCallbackString, CallbackMsgString);
    CHECK_EQUAL_C_INT(1, isLastTLSTxMessagePuback());

}


