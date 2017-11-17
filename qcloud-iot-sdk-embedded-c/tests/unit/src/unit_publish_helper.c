#include <stdio.h>
#include <CppUTest/TestHarness_c.h>
#include <qcloud_iot_mqtt_client_interface.h>
#include <unit_helper_functions.h>
#include <unit_mock_tls_params.h>
#include <string.h>

static MQTTConnectParams connectParams;
static PublishParams testPubMsgParams;
static char subTopic[10] = "sdk/Test";
static uint16_t subTopicLen = 8;

static Qcloud_IoT_Client iotClient;
char cPayload[100];

TEST_GROUP_C_SETUP(PublishTests) {
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
    sprintf(cPayload, "%s : %d ", "hello from SDK", 0);
    testPubMsgParams.payload = (void *) cPayload;
    testPubMsgParams.payload_len = strlen(cPayload);

    resetTlsBuffer();
}

TEST_GROUP_C_TEARDOWN(PublishTests) { }

/* E:1 - Publish with Null/empty client instance */
TEST_C(PublishTests, PublishNullClient) {
    int rc = QCLOUD_ERR_SUCCESS;

    testPubMsgParams.qos = QOS1;
    testPubMsgParams.retained = 0;
    testPubMsgParams.payload = "Message";
    testPubMsgParams.payload_len = 7;

    rc = qcloud_iot_mqtt_publish(NULL, subTopic, &testPubMsgParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);

}

/* E:2 - Publish with Null/empty Topic Name */
TEST_C(PublishTests, PublishNullTopic) {
    int rc = QCLOUD_ERR_SUCCESS;

    testPubMsgParams.qos = QOS1;
    testPubMsgParams.retained = 0;
    testPubMsgParams.payload = "Message";
    testPubMsgParams.payload_len = 7;

    rc = qcloud_iot_mqtt_publish(&iotClient, NULL, &testPubMsgParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);

    rc = qcloud_iot_mqtt_publish(&iotClient, "", &testPubMsgParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);

}

/* E:3 - Publish with Null/empty payload */
TEST_C(PublishTests, PublishNullParams) {
    int rc = QCLOUD_ERR_SUCCESS;

    rc = qcloud_iot_mqtt_publish(&iotClient, subTopic, NULL);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);

    testPubMsgParams.qos = QOS1;
    testPubMsgParams.retained = 0;
    testPubMsgParams.payload = NULL;
    testPubMsgParams.payload_len = 0;

    rc = qcloud_iot_mqtt_publish(&iotClient, subTopic, &testPubMsgParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);

}

/* E:4 - Publish with network disconnected */
TEST_C(PublishTests, PublishNetworkDisconnected) {
    int rc = QCLOUD_ERR_SUCCESS;

    /* Ensure network is disconnected */
    rc = qcloud_iot_mqtt_disconnect(&iotClient);

    testPubMsgParams.qos = QOS1;
    testPubMsgParams.retained = 0;
    testPubMsgParams.payload = "Message";
    testPubMsgParams.payload_len = 7;

    rc = qcloud_iot_mqtt_publish(&iotClient, subTopic, &testPubMsgParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_NO_CONN, rc);

}

/* E:6 - Publish with QoS1 send success, Puback not received */
TEST_C(PublishTests, publishQoS1FailureToReceivePuback) {
    int rc = QCLOUD_ERR_SUCCESS;

    rc = qcloud_iot_mqtt_publish(&iotClient, subTopic, &testPubMsgParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_REQUEST_TIMEOUT, rc);
}

/* E:7 - Publish with QoS1 send success, Delayed Puback received after command timeout */
TEST_C(PublishTests, publishQoS1FailureDelayedPuback) {
    int rc = QCLOUD_ERR_SUCCESS;

    setTLSRxBufferDelay(10, 0);
    setTLSRxBufferForPuback();
    rc = qcloud_iot_mqtt_publish(&iotClient, subTopic, &testPubMsgParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_REQUEST_TIMEOUT, rc);

}

/* E:8 - Publish with send success, Delayed Puback received before command timeout */
TEST_C(PublishTests, publishQoS1Success10msDelayedPuback) {
    int rc = QCLOUD_ERR_SUCCESS;
    char topicName[10];
    uint8_t firstByte, secondByte;
    uint16_t topicNameLen;

    resetTlsBuffer();
    setTLSRxBufferDelay(0, (int) iotClient.command_timeout_ms/2);
    setTLSRxBufferForPuback();
    rc = qcloud_iot_mqtt_publish(&iotClient, subTopic, &testPubMsgParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    firstByte = (uint8_t) (TxBuffer.pBuffer[2]);
    secondByte = (uint8_t) (TxBuffer.pBuffer[3]);
    topicNameLen = (uint16_t) (secondByte + (256 * firstByte));

    snprintf(topicName, topicNameLen + 1u, "%s", &(TxBuffer.pBuffer[4])); // Added one for null character

    // Verify publish happens
    CHECK_EQUAL_C_STRING(subTopic, topicName);

}

/* E:9 - Publish QoS0 success */
TEST_C(PublishTests, publishQoS0NoPubackSuccess) {
    int rc = QCLOUD_ERR_SUCCESS;

    testPubMsgParams.qos = QOS0; // switch to a Qos0 PUB
    rc = qcloud_iot_mqtt_publish(&iotClient, subTopic, &testPubMsgParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
}

/* E:10 - Publish with QoS1 send success, Puback received */
TEST_C(PublishTests, publishQoS1Success) {
    int rc = QCLOUD_ERR_SUCCESS;

    setTLSRxBufferForPuback();
    rc = qcloud_iot_mqtt_publish(&iotClient, subTopic, &testPubMsgParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
}
