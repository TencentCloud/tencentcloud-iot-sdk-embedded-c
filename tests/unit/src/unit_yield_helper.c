#include <stdio.h>
#include <unistd.h>
#include <CppUTest/TestHarness_c.h>
#include <qcloud_iot_mqtt_client_interface.h>
#include <unit_helper_functions.h>
#include <unit_mock_tls_params.h>
#include <string.h>

static MQTTConnectParams connectParams;
static Qcloud_IoT_Client iotClient;
static PublishParams testPubMsgParams;
static SubscribeParams testSubParams;

static ConnectBufferProofread prfrdParams;
static char CallbackMsgString[100];
static char subTopic[10] = "sdk/Test";
static uint16_t subTopicLen = 8;

static bool dcHandlerInvoked = false;

static void iot_tests_unit_acr_subscribe_callback_handler(char *topicName, size_t topicNameLen, MQTTMessage *params, void *pUserdata) {
    char *tmp = params->payload;
    unsigned int i;

    for (i = 0; i < params->payload_len; i++) {
        CallbackMsgString[i] = tmp[i];
    }
}

static void iot_tests_unit_yield_test_subscribe_callback_handler(char *topicName, size_t topicNameLen, MQTTMessage *params, void *pUserdata) {
    char *tmp = params->payload;
    unsigned int i;

    for (i = 0; i < params->payload_len; i++) {
        CallbackMsgString[i] = tmp[i];
    }
}

void iot_tests_unit_disconnect_handler() {
    dcHandlerInvoked = true;
}


TEST_GROUP_C_SETUP(YieldTests) {
    int rc = QCLOUD_ERR_SUCCESS;
    dcHandlerInvoked = false;

    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false,
                          iot_tests_unit_disconnect_handler);
    rc = qcloud_iot_mqtt_init(&iotClient, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);


    ConnectMQTTParamsSetup_Detailed(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID,
                                    QOS1, false, true, "willTopicName", "willMsg", NULL, NULL);
    connectParams.keep_alive_interval = 5;
    setTLSRxBufferForConnack(&connectParams, 0, 0);
    rc = qcloud_iot_mqtt_connect(&iotClient, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    rc = qcloud_iot_mqtt_set_autoreconnect(&iotClient, false);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    resetTlsBuffer();
}

TEST_GROUP_C_TEARDOWN(YieldTests) {
    /* Clean up. Not checking return code here because this is common to all tests.
     * A test might have already caused a disconnect by this point.
     */
    qcloud_iot_mqtt_disconnect(&iotClient);
}

/* G:1 - Yield with Null/empty Client Instance */
TEST_C(YieldTests, NullClientYield) {
    int rc = qcloud_iot_mqtt_yield(NULL, 1000);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

/* G:2 - Yield with zero yield timeout */

TEST_C(YieldTests, ZeroTimeoutYield) {
    int rc = qcloud_iot_mqtt_yield(&iotClient, 0);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}
/* G:3 - Yield, network disconnected, never connected */

TEST_C(YieldTests, YieldNetworkDisconnectedNeverConnected) {
    Qcloud_IoT_Client tempIotClient;
    int rc;

    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, iot_tests_unit_disconnect_handler);
    rc = qcloud_iot_mqtt_init(&tempIotClient, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    rc = qcloud_iot_mqtt_yield(&tempIotClient, 1000);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_NO_CONN, rc);
}

/* G:4 - Yield, network disconnected, disconnected manually */
TEST_C(YieldTests, YieldNetworkDisconnectedDisconnectedManually) {
    int rc = qcloud_iot_mqtt_disconnect(&iotClient);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    rc = qcloud_iot_mqtt_yield(&iotClient, 1000);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_MANUALLY_DISCONNECTED, rc);
}

/* G:5 - Yield, network connected, yield called while in subscribe application callback */
TEST_C(YieldTests, YieldInSubscribeCallback) {
    int rc = QCLOUD_ERR_SUCCESS;
    char expectedCallbackString[] = "0xA5A5A3";


    setTLSRxBufferForSuback(QOS1);
    testSubParams.qos = QOS1;
    testSubParams.on_message_handler = iot_tests_unit_yield_test_subscribe_callback_handler;
    rc = qcloud_iot_mqtt_subscribe(&iotClient, subTopic, &testSubParams);
    if (QCLOUD_ERR_SUCCESS == rc) {
        testPubMsgParams.qos = QOS1;
        setTLSRxBufferWithMsgOnSubscribedTopic(subTopic, strlen(subTopic), QOS1, expectedCallbackString);
        rc = qcloud_iot_mqtt_yield(&iotClient, 1000);
        if (QCLOUD_ERR_SUCCESS == rc) {
            CHECK_EQUAL_C_STRING(expectedCallbackString, CallbackMsgString);
        }
        CHECK_EQUAL_C_INT(1, isLastTLSTxMessagePuback());
    }

}

/* G:6 - Yield, network disconnected, ping timeout, auto-reconnect disabled */
TEST_C(YieldTests, disconnectNoAutoReconnect) {
    int rc = QCLOUD_ERR_FAILURE;

    rc = qcloud_iot_mqtt_set_autoreconnect(&iotClient, true);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    CHECK_EQUAL_C_INT(true, qcloud_iot_mqtt_is_connected(&iotClient));
    CHECK_EQUAL_C_INT(true, qcloud_iot_mqtt_is_autoreconnect_enabled(&iotClient));

    /* Disable Autoreconnect, then let ping request time out and call yield */
    qcloud_iot_mqtt_set_autoreconnect(&iotClient, false);
    sleep((uint16_t) (iotClient.options.keep_alive_interval));

    cpputest_malloc_count_reset();

    /* Sleep for keep alive interval to allow the first ping to be sent out */
    sleep(iotClient.options.keep_alive_interval);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    CHECK_EQUAL_C_INT(true, isLastTLSTxMessagePingreq());

    /* Let ping request time out and call yield */
    sleep(iotClient.options.keep_alive_interval + 1);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_NO_CONN, rc);
    CHECK_EQUAL_C_INT(1, isLastTLSTxMessageDisconnect());
    CHECK_EQUAL_C_INT(0, qcloud_iot_mqtt_is_connected(&iotClient));
    CHECK_EQUAL_C_INT(true, dcHandlerInvoked);

}

/* G:7 - Yield, network connected, no incoming messages */
TEST_C(YieldTests, YieldSuccessNoMessages) {
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

    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

}

/* G:8 - Yield, network connected, ping request/response */
TEST_C(YieldTests, PingRequestPingResponse) {
    int rc = QCLOUD_ERR_SUCCESS;
    int i = 0;
    int j = 0;
    int attempt = 3;

    for (i = 0; i < attempt; i++) {
        /* Set TLS buffer for ping response */
        resetTlsBuffer();
        setTLSRxBufferForPingresp();

        for (j = 0; j <= iotClient.options.keep_alive_interval; j++) {
            sleep(1);
            rc = qcloud_iot_mqtt_yield(&iotClient, 100);
            CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
        }

        /* Check whether ping was processed correctly and new Ping request was generated */
        CHECK_EQUAL_C_INT(1, isLastTLSTxMessagePingreq());
    }

}

/* G:9 - Yield, disconnected, Auto-reconnect timed-out */
TEST_C(YieldTests, disconnectAutoReconnectTimeout) {
    int rc = QCLOUD_ERR_FAILURE;

    rc = qcloud_iot_mqtt_set_autoreconnect(&iotClient, true);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    CHECK_EQUAL_C_INT(true, qcloud_iot_mqtt_is_connected(&iotClient));
    CHECK_EQUAL_C_INT(true, qcloud_iot_mqtt_is_autoreconnect_enabled(&iotClient));

    resetTlsBuffer();

    /* Sleep for keep alive interval to allow the first ping to be sent out */
    sleep(iotClient.options.keep_alive_interval);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    CHECK_EQUAL_C_INT(true, isLastTLSTxMessagePingreq());

    /* Let ping request time out and call yield */
    sleep(iotClient.options.keep_alive_interval + 1);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT, rc);
    CHECK_EQUAL_C_INT(0, qcloud_iot_mqtt_is_connected(&iotClient));
    CHECK_EQUAL_C_INT(true, dcHandlerInvoked);

}

/* G:10 - Yield, disconnected, Auto-reconnect successful */
TEST_C(YieldTests, disconnectAutoReconnectSuccess) {
    int rc = QCLOUD_ERR_FAILURE;
    unsigned char *currPayload = NULL;

    rc = qcloud_iot_mqtt_set_autoreconnect(&iotClient, true);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    CHECK_EQUAL_C_INT(true, qcloud_iot_mqtt_is_connected(&iotClient));
    CHECK_EQUAL_C_INT(true, qcloud_iot_mqtt_is_autoreconnect_enabled(&iotClient));

    /* Sleep for keep alive interval to allow the first ping to be sent out */
    sleep(iotClient.options.keep_alive_interval);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    CHECK_EQUAL_C_INT(true, isLastTLSTxMessagePingreq());

    /* Let ping request time out and call yield */
    sleep(iotClient.options.keep_alive_interval + 1);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT, rc);

    sleep(2); /* Default min reconnect delay is 1 sec */
    printf("\nWakeup");
    setTLSRxBufferForConnack(&connectParams, 0, 0);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    currPayload = connectTxBufferHeaderParser(&prfrdParams, TxBuffer.pBuffer);
    CHECK_C(true == isConnectTxBufFlagCorrect(&connectParams, &prfrdParams));
    CHECK_C(true == isConnectTxBufPayloadCorrect(&connectParams, currPayload));
    CHECK_EQUAL_C_INT(true, qcloud_iot_mqtt_is_connected(&iotClient));
    CHECK_EQUAL_C_INT(true, dcHandlerInvoked);

}

/* G:11 - Yield, disconnected, Manual reconnect */
TEST_C(YieldTests, disconnectManualAutoReconnect) {
    int rc = QCLOUD_ERR_FAILURE;
    unsigned char *currPayload = NULL;

    CHECK_C(qcloud_iot_mqtt_is_connected(&iotClient));

    /* Disable Autoreconnect, then let ping request time out and call yield */
    qcloud_iot_mqtt_set_autoreconnect(&iotClient, false);
    CHECK_C(!qcloud_iot_mqtt_is_autoreconnect_enabled(&iotClient));

    /* Sleep for keep alive interval to allow the first ping to be sent out */
    sleep(iotClient.options.keep_alive_interval);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    CHECK_EQUAL_C_INT(true, isLastTLSTxMessagePingreq());

    /* Let ping request time out and call yield */
    sleep(iotClient.options.keep_alive_interval + 1);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_NO_CONN, rc);
    CHECK_EQUAL_C_INT(1, isLastTLSTxMessageDisconnect());
    CHECK_C(!qcloud_iot_mqtt_is_connected(&iotClient));
    CHECK_EQUAL_C_INT(true, dcHandlerInvoked);

    dcHandlerInvoked = false;
    setTLSRxBufferForConnack(&connectParams, 0, 0);
    rc = qcloud_iot_mqtt_attempt_reconnect(&iotClient);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_RECONNECTED, rc);

    currPayload = connectTxBufferHeaderParser(&prfrdParams, TxBuffer.pBuffer);
    CHECK_C(true == isConnectTxBufFlagCorrect(&connectParams, &prfrdParams));
    CHECK_C(true == isConnectTxBufPayloadCorrect(&connectParams, currPayload));
    CHECK_C(qcloud_iot_mqtt_is_connected(&iotClient));
    CHECK_EQUAL_C_INT(false, dcHandlerInvoked);

}

/* G:12 - Yield, resubscribe to all topics on reconnect */
TEST_C(YieldTests, resubscribeSuccessfulReconnect) {
    int rc = QCLOUD_ERR_FAILURE;
    char cPayload[100];
    bool connected = false;
    bool autoReconnectEnabled = false;
    char expectedCallbackString[100];

    rc = qcloud_iot_mqtt_set_autoreconnect(&iotClient, true);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    snprintf(CallbackMsgString, 100, "NOT_VISITED");

    testPubMsgParams.qos = QOS1;
    testPubMsgParams.retained = 0;
    snprintf(cPayload, 100, "%s : %d ", "hello from SDK", 0);
    testPubMsgParams.payload = (void *) cPayload;
    testPubMsgParams.payload_len = strlen(cPayload);

    connected = qcloud_iot_mqtt_is_connected(&iotClient);
    CHECK_EQUAL_C_INT(1, connected);

    resetTlsBuffer();

    /* Subscribe to a topic */
    testSubParams.qos = QOS1;
    testSubParams.on_message_handler = iot_tests_unit_acr_subscribe_callback_handler;
    setTLSRxBufferForSuback(QOS1);
    rc = qcloud_iot_mqtt_subscribe(&iotClient, subTopic, &testSubParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    resetTlsBuffer();

    /* Check subscribe */
    snprintf(expectedCallbackString, 100, "Message for %s", subTopic);
    testPubMsgParams.qos = QOS1;
    setTLSRxBufferWithMsgOnSubscribedTopic(subTopic, strlen(subTopic), QOS1, expectedCallbackString);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    CHECK_EQUAL_C_STRING(expectedCallbackString, CallbackMsgString);

    resetTlsBuffer();

    autoReconnectEnabled = qcloud_iot_mqtt_is_autoreconnect_enabled(&iotClient);
    CHECK_EQUAL_C_INT(1, autoReconnectEnabled);

    /* Sleep for keep alive interval to allow the first ping to be sent out */
    sleep(iotClient.options.keep_alive_interval);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    CHECK_EQUAL_C_INT(true, isLastTLSTxMessagePingreq());

    /* Let ping request time out and call yield */
    sleep(iotClient.options.keep_alive_interval + 1);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT, rc);

    sleep(2); /* Default min reconnect delay is 1 sec */

    resetTlsBuffer();
    setTLSRxBufferForConnackAndSuback(&connectParams, 0, QOS1);

    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    /* Test if reconnect worked */
    connected = qcloud_iot_mqtt_is_connected(&iotClient);
    CHECK_EQUAL_C_INT(true, connected);

    resetTlsBuffer();

    /* Check subscribe */
    snprintf(expectedCallbackString, 100, "Message for %s after resub", subTopic);
    testPubMsgParams.qos = QOS1;
    setTLSRxBufferWithMsgOnSubscribedTopic(subTopic,strlen(subTopic), QOS1, expectedCallbackString);
    rc = qcloud_iot_mqtt_yield(&iotClient, 100);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    CHECK_EQUAL_C_STRING(expectedCallbackString, CallbackMsgString);
    CHECK_EQUAL_C_INT(true, dcHandlerInvoked);

}
