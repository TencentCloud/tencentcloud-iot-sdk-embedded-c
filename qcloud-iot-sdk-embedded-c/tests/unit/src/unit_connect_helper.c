#include <CppUTest/TestHarness_c.h>
#include <qcloud_iot_mqtt_client_interface.h>
#include <unit_mock_tls_params.h>
#include <string.h>
#include "unit_helper_functions.h"

static bool isMqttConnected = false;

static Qcloud_IoT_Client client;
static MQTTConnectParams connectParams;

static ConnectBufferProofread connectBufferProofread;

static char subTopic1[12] = "sdk/Topic1";
static char subTopic2[12] = "sdk/Topic2";

PublishParams testPubMsgParams;

SubscribeParams testTopicParams;

#define NO_MSG_XXXX "XXXX"
static char CallbackMsgStringclean[100] = NO_MSG_XXXX;

static void
iot_subscribe_callback_handler(char *topicName, size_t topicNameLen, MQTTMessage *iotMessage, void *pUserdata) {
    char *tmp = iotMessage->payload;
    unsigned int i;

    for (i = 0; i < iotMessage->payload_len; i++) {
        CallbackMsgStringclean[i] = tmp[i];
    }
}


TEST_GROUP_C_SETUP(ConnectTests) {
    isMqttConnected = false;
    qcloud_iot_mqtt_disconnect(&client);
}

TEST_GROUP_C_TEARDOWN(ConnectTests) {
    qcloud_iot_mqtt_disconnect(&client);
}

/**
 * 1. Init with NULL Client Struct
 */
TEST_C(ConnectTests, nullClientInit) {
    int rc = QCLOUD_ERR_SUCCESS;
    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    rc = qcloud_iot_mqtt_init(NULL, &connectParams);

    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

/**
 * 2. Init with NULL MQTTInitParams Struct
 */
TEST_C(ConnectTests, nullParamsInit) {
    int rc = QCLOUD_ERR_SUCCESS;
    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    rc = qcloud_iot_mqtt_init(&client, NULL);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

/**
 * 3. Init with NULL host Param
 */
TEST_C(ConnectTests, nullHost) {
    int rc = QCLOUD_ERR_SUCCESS;
    MQTTInitParamsSetup(&connectParams, NULL, QCLOUD_IOT_MQTT_PORT, false, NULL);
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

/**
 * 4. Init with NULL port Param
 */
TEST_C(ConnectTests, nullPort) {
    int rc = QCLOUD_ERR_SUCCESS;
    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, 0, false, NULL);
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

/**
 * 5. Init with NULL CA File Path
 */
TEST_C(ConnectTests, nullCA) {
    int rc = QCLOUD_ERR_SUCCESS;
    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    connectParams.ca_file = NULL;
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

/**
 * 6. Init with NULL Cert File Path
 */
TEST_C(ConnectTests, nullCert) {
    int rc = QCLOUD_ERR_SUCCESS;
    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    connectParams.cert_file = NULL;
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

/**
 * 7. Init with NULL Key File Path
 */
TEST_C(ConnectTests, nullKeyFile) {
    int rc = QCLOUD_ERR_SUCCESS;
    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    connectParams.key_file = NULL;
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

/**
 * 8. Connect with NULL Client Struct
 */
TEST_C(ConnectTests, nullClientConnect) {
    int rc = QCLOUD_ERR_SUCCESS;

    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
    setTLSRxBufferForConnack(&connectParams, 0, 0);
    rc = qcloud_iot_mqtt_connect(NULL, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

/**
 * 9. Connect with NULL/Empty Client ID
 */
TEST_C(ConnectTests, nullClientID) {
    int rc = QCLOUD_ERR_SUCCESS;

    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    setTLSRxBufferForConnack(&connectParams, 0, 0);

    // if NULL Client ID
    ConnectParamsSetup(&connectParams, NULL);
    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);

    // if Empty Client ID
    ConnectParamsSetup(&connectParams, "");
    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);

    ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
    setTLSRxBufferForConnack(&connectParams, 0, 0);
    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
}

/**
 * 10. Connect with Invalid Endpoint
 */
TEST_C(ConnectTests, InvalidEndpoint) {
    int rc = QCLOUD_ERR_SUCCESS;

    char invalidEndpoint[20];
    snprintf(invalidEndpoint, 20, "invalid");

    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    invalidEndpointFilter = invalidEndpoint;
    connectParams.host = invalidEndpoint;
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
    setTLSRxBufferForConnack(&connectParams, 0, 0);
    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_C(rc != QCLOUD_ERR_SUCCESS);
}

/**
 * 11. Connect with Invalid Port
 */
TEST_C(ConnectTests, InvalidPort) {
    int rc = QCLOUD_ERR_SUCCESS;

    invalidPortFilter = 1234;

    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    connectParams.port = invalidPortFilter;
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
    setTLSRxBufferForConnack(&connectParams, 0, 0);
    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_C(rc != QCLOUD_ERR_SUCCESS);
}

/**
 * 12. Connect with Invalid CA Path
 */
TEST_C(ConnectTests, InvalidCA) {
    int rc = QCLOUD_ERR_SUCCESS;

    char invalidCaPath[20];
    snprintf(invalidCaPath, 20, "invalid");

    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    invalidRootCAPathFilter = invalidCaPath;
    connectParams.ca_file = invalidRootCAPathFilter;
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
    setTLSRxBufferForConnack(&connectParams, 0, 0);
    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_C(rc != QCLOUD_ERR_SUCCESS);
}

/**
 * 13. Connect with Invalid Cert
 */
TEST_C(ConnectTests, InvalidCert) {
    int rc = QCLOUD_ERR_SUCCESS;

    char invalidCert[20];
    snprintf(invalidCert, 20, "invalid");

    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    invalidCertPathFilter = invalidCert;
    connectParams.cert_file = invalidCertPathFilter;
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
    setTLSRxBufferForConnack(&connectParams, 0, 0);
    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_C(rc != QCLOUD_ERR_SUCCESS);
}

/**
 * 14. Connect with Invalid Key
 */
TEST_C(ConnectTests, InvalidKey) {
    int rc = QCLOUD_ERR_SUCCESS;

    char invalidKey[20];
    snprintf(invalidKey, 20, "invalid");

    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    invalidPrivKeyPathFilter = invalidKey;
    connectParams.key_file = invalidPrivKeyPathFilter;
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
    setTLSRxBufferForConnack(&connectParams, 0, 0);
    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_C(rc != QCLOUD_ERR_SUCCESS);
}

/**
 * 15. Connect, with no Response timeout
 */
TEST_C(ConnectTests, NoResponseTimeout) {
    int rc = QCLOUD_ERR_SUCCESS;

    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    resetTlsBuffer();
    ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_C(rc != QCLOUD_ERR_SUCCESS);
}

/**
 * 16. Connect, connack malformed, too large
 */
TEST_C(ConnectTests, ConnackTooLarge) {
    int rc = QCLOUD_ERR_SUCCESS;

    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
    setTLSRxBufferForConnack(&connectParams, 0, 0);
    RxBuffer.pBuffer[1] = (char) 0x15;  // set remain length large than expected
    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_C(rc != QCLOUD_ERR_SUCCESS);
}

/**
 * 17. Connect, connack malformed, fix header corrupted
 */
TEST_C(ConnectTests, FixedHeaderCorrupted) {
    int rc = QCLOUD_ERR_SUCCESS;

    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
    setTLSRxBufferForConnack(&connectParams, 0, 0);
    RxBuffer.pBuffer[0] = (char) 0x00;  // set fix header
    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_C(rc != QCLOUD_ERR_SUCCESS);
}

/**
 * 18. Connect, connack malformed, invalid remaining length
 */
TEST_C(ConnectTests, InvalidRemainLength) {
    int rc = QCLOUD_ERR_SUCCESS;

    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
    setTLSRxBufferForConnack(&connectParams, 0, 0);
    RxBuffer.pBuffer[1] = (char) 0x00;   // set remain length
    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_C(rc != QCLOUD_ERR_SUCCESS);
}

/**
 * 19. Connect, connack returned error, unacceptable protocol version
 */
TEST_C(ConnectTests, UnacceptableProtocolVersion) {
    int rc = QCLOUD_ERR_SUCCESS;

    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
    connectParams.MQTTVersion = 7;
    setTLSRxBufferForConnack(&connectParams, 0, 1);
    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_CONANCK_UNACCEPTABLE_PROTOCOL_VERSION, rc);
}

/**
 * 20. Connect, connack returned error, identifier rejected
 */
TEST_C(ConnectTests, IndentifierRejected) {
    int rc = QCLOUD_ERR_SUCCESS;

    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
    setTLSRxBufferForConnack(&connectParams, 0, 2);
    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_CONNACK_IDENTIFIER_REJECTED, rc);
}

/**
 * 21. Connect, connack returned error, server unavailable
 */
TEST_C(ConnectTests, ServerUnavailable) {
    int rc = QCLOUD_ERR_SUCCESS;

    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
    setTLSRxBufferForConnack(&connectParams, 0, 3);
    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_CONNACK_SERVER_UNAVAILABLE, rc);
}

/**
 * 22. Connect, connack returned error, bad user name or password
 */
TEST_C(ConnectTests, BadUserNameOrPassword) {
    int rc = QCLOUD_ERR_SUCCESS;

    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
    setTLSRxBufferForConnack(&connectParams, 0, 4);
    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_CONNACK_BAD_USERDATA, rc);
}

/**
 * 23. Connect, connack returned error, not authorized
 */
TEST_C(ConnectTests, NotAuthorized) {
    int rc = QCLOUD_ERR_SUCCESS;

    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
    setTLSRxBufferForConnack(&connectParams, 0, 5);
    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_CONNACK_NOT_AUTHORIZED, rc);
}


/**
 * 24. Connect, connack returned error, Invalid connack return code
 */
TEST_C(ConnectTests, InvalidConnackReturnCode) {
    int rc = QCLOUD_ERR_SUCCESS;

    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
    setTLSRxBufferForConnack(&connectParams, 0, 6);
    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_C(rc != QCLOUD_ERR_SUCCESS);
}

/**
 * 25. Connect success
 */
TEST_C(ConnectTests, ConnectSuccess) {
    int rc = QCLOUD_ERR_SUCCESS;

    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
    setTLSRxBufferForConnack(&connectParams, 0, 0);
    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
}

/**
 * 26. Connect, flag settings and parameters are recorded in buffer
 */
TEST_C(ConnectTests, FlagSettingsAndParamsAreRecordedIntoBuf) {
    int rc = QCLOUD_ERR_SUCCESS;
    unsigned char *currPayload = NULL;

    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    resetTlsBuffer();
    ConnectMQTTParamsSetup_Detailed(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID, QOS2, false, true, "willTopicName",
                                    "willMsg", NULL, NULL);
    connectParams.keep_alive_interval = (1 << 16) - 1;
    setTLSRxBufferForConnack(&connectParams, 0, 0);

    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    currPayload = connectTxBufferHeaderParser(&connectBufferProofread, TxBuffer.pBuffer);
    CHECK_C(true == isConnectTxBufFlagCorrect(&connectParams, &connectBufferProofread));
    CHECK_C(true == isConnectTxBufPayloadCorrect(&connectParams, currPayload));

}

/**
 * 27. Connect  Disconnect Manually reconnect
 */
TEST_C(ConnectTests, ConnectDisconnectConnect) {
    int rc = QCLOUD_ERR_SUCCESS;

    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);


    // connect
    ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
    setTLSRxBufferForConnack(&connectParams, 0, 0);
    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    isMqttConnected = qcloud_iot_mqtt_is_connected(&client);
    CHECK_EQUAL_C_INT(true, isMqttConnected);

    // disconnect
    rc = qcloud_iot_mqtt_disconnect(&client);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    isMqttConnected = qcloud_iot_mqtt_is_connected(&client);
    CHECK_EQUAL_C_INT(false, isMqttConnected);

    // reconnect
    resetTlsBuffer();
    setTLSRxBufferForConnack(&connectParams, 0, 0);
    rc = qcloud_iot_mqtt_attempt_reconnect(&client);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_RECONNECTED, rc);
    isMqttConnected = qcloud_iot_mqtt_is_connected(&client);
    CHECK_EQUAL_C_INT(true, isMqttConnected);
}

/**
 * 28. - Connect attempt, Clean session, Subscribe
 * connect with clean session true and subscribe to a topic1, set msg to topic1 and ensure it is received
 * connect cs false, set msg to topic1 and nothing should come in, Sub to topic2 and check if msg is received
 * connect cs false and send msg to topic2 and should be received
 * cs true and everything should be clean again
 */
TEST_C(ConnectTests, cleanSessionInitSubscribers) {
    int rc = QCLOUD_ERR_SUCCESS;
    char expectedCallbackString[] = "msg topic";


    MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, false, NULL);
    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    resetTlsBuffer();

    //1. connect with clean session true and
    ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
    connectParams.clean_session= true;
    setTLSRxBufferForConnack(&connectParams, 0, 0);
    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    //1. subscribe to a topic1 and
    testPubMsgParams.payload = expectedCallbackString;
    testPubMsgParams.qos = QOS0;
    testPubMsgParams.payload_len = (uint16_t) strlen(expectedCallbackString);

    testTopicParams.qos = QOS0;
    testTopicParams.on_message_handler = iot_subscribe_callback_handler;
    setTLSRxBufferForSuback(QOS0);

    rc = qcloud_iot_mqtt_subscribe(&client, subTopic1, &testTopicParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    resetTlsBuffer();
    //1. receive message
    setTLSRxBufferWithMsgOnSubscribedTopic(subTopic1, strlen(subTopic1), QOS0, expectedCallbackString);
    rc = qcloud_iot_mqtt_yield(&client, 1000);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    CHECK_EQUAL_C_STRING(expectedCallbackString, CallbackMsgStringclean);

    resetTlsBuffer();
    rc = qcloud_iot_mqtt_disconnect(&client);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    //2. connect cs false and
    ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
    connectParams.clean_session = false;
    setTLSRxBufferForConnack(&connectParams, 0, 0);
    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    resetTlsBuffer();
    //3. set msg to topic1 and should receive the topic1 message
    snprintf(CallbackMsgStringclean, 100, NO_MSG_XXXX);
    setTLSRxBufferWithMsgOnSubscribedTopic(subTopic1, strlen(subTopic1), QOS0, expectedCallbackString);
    rc = qcloud_iot_mqtt_yield(&client, 1000);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    CHECK_EQUAL_C_STRING(expectedCallbackString, CallbackMsgStringclean);

    resetTlsBuffer();
    //4. ,sub to topic2
    snprintf(CallbackMsgStringclean, 100, NO_MSG_XXXX);
    setTLSRxBufferForSuback(QOS0);

    rc = qcloud_iot_mqtt_subscribe(&client, subTopic2, &testTopicParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    resetTlsBuffer();
    //5. and check if topic 2 msg is received
    setTLSRxBufferWithMsgOnSubscribedTopic(subTopic2, strlen(subTopic2), QOS0, expectedCallbackString);
    rc = qcloud_iot_mqtt_yield(&client, 1000);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    CHECK_EQUAL_C_STRING(expectedCallbackString, CallbackMsgStringclean);

    rc = qcloud_iot_mqtt_disconnect(&client);

    //6. connect cs false and
    ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
    connectParams.clean_session = false;
    setTLSRxBufferForConnack(&connectParams, 0, 0);
    rc = qcloud_iot_mqtt_connect(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    //7. set msg to topic2 and
    snprintf(CallbackMsgStringclean, 100, NO_MSG_XXXX);
    setTLSRxBufferWithMsgOnSubscribedTopic(subTopic2, strlen(subTopic2), QOS0, expectedCallbackString);
    //8. should be received
    rc = qcloud_iot_mqtt_yield(&client, 1000);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
    CHECK_EQUAL_C_STRING(expectedCallbackString, CallbackMsgStringclean);
}


