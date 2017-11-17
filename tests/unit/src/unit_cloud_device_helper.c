#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <CppUTest/TestHarness_c.h>
#include <qcloud_iot_mqtt_client_interface.h>
#include <qcloud_iot_shadow_device.h>
#include <iot_unit_config.h>
#include <unit_helper_functions.h>
#include <unit_mock_tls_params.h>

#define SIZE_OF_UPDATE_DOCUMENT 200
#define TEST_JSON_RESPONSE_FULL_DOCUMENT "{\"state\":{\"reported\":{\"sensor1\":98}}, \"clientToken\":\"" QCLOUD_IOT_MQTT_CLIENT_ID "-0\"}"
#define TEST_JSON_RESPONSE_DELETE_DOCUMENT "{\"version\":2,\"timestamp\":1443473857,\"clientToken\":\"" QCLOUD_IOT_MQTT_CLIENT_ID "-0\"}"
#define TEST_JSON_RESPONSE_UPDATE_DOCUMENT "{\"state\":{\"reported\":{\"doubleData\":4.090800,\"floatData\":3.445000}}, \"clientToken\":\"" QCLOUD_IOT_MQTT_CLIENT_ID "-0\"}"

static Qcloud_IoT_Client client;
static PublishParams testPubMsgParams;
static ShadowConnectParams cloudConnectParams;

static RequestAck ackStatusRx;
static char jsonFullDocument[200];
static Method methodRx;

static void topicNameFromThingAndAction(char *pTopic, const char *pThingName, Method method) {
    char methodBuf[10];

    if (GET == method) {
        strcpy(methodBuf, "get");
    } else if (UPDATE == method) {
        strcpy(methodBuf, "update");
    } else if (DELETE == method) {
        strcpy(methodBuf, "delete");
    }

    snprintf(pTopic, 100, "$aws/things/%s/shadow/%s", pThingName, methodBuf);
}

TEST_GROUP_C_SETUP(CloudDeviceTests) {
    int ret_val = QCLOUD_ERR_SUCCESS;
    char cPayload[100];
    char topic[120];

    resetTlsBuffer();
    cloudConnectParams.mqtt.cert_file = QCLOUD_IOT_CERT_FILENAME;
    cloudConnectParams.mqtt.ca_file = QCLOUD_IOT_CA_FILENAME;
    cloudConnectParams.mqtt.key_file = QCLOUD_IOT_KEY_FILENAME;
    cloudConnectParams.mqtt.on_disconnect_handler = NULL;
    cloudConnectParams.mqtt.auto_connect_enable = false;
    ret_val = qcloud_iot_shadow_init(&client, &cloudConnectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    
    cloudConnectParams.mqtt.product_name = QCLOUD_IOT_MY_PRODUCT_NAME;
    cloudConnectParams.mqtt.device_name = QCLOUD_IOT_MY_DEVICE_NAME;

    ConnectParamsSetup(&cloudConnectParams->mqtt, QCLOUD_IOT_MQTT_CLIENT_ID);
    setTLSRxBufferForConnack(&cloudConnectParams->mqtt, 0, 0);
    ret_val = qcloud_iot_shadow_connect(&client, &cloudConnectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    setTLSRxBufferForPuback();
    testPubMsgParams.qos = QOS0;
    testPubMsgParams.retained = 0;
    snprintf(cPayload, 100, "%s : %d ", "hello from SDK", 0);
    testPubMsgParams.payload = (void *) cPayload;
    testPubMsgParams.payload_len = strlen(cPayload) + 1;
    topicNameFromThingAndAction(topic, QCLOUD_IOT_MY_DEVICE_NAME, GET);
    setTLSRxBufferForDoubleSuback(QOS0);
}

TEST_GROUP_C_TEARDOWN(CloudDeviceTests) {
    /* Clean up. Not checking return code here because this is common to all tests.
     * A test might have already caused a disconnect by this point.
     */
    qcloud_iot_shadow_disconnect(&client);
}

static void onRequestCallback(const char *pThingName, Method action, RequestAck status,
                              const char *pReceivedJsonDocument, void *pContextData) {

    methodRx = action;
    ackStatusRx = status;
    if (ACK_TIMEOUT != status) {
        strcpy(jsonFullDocument, pReceivedJsonDocument);
    }
}

// Happy path for Get, Update, Delete
TEST_C(CloudDeviceTests, GetTheFullJSONDocument) {
    int ret_val = QCLOUD_ERR_SUCCESS;
    PublishParams params;

    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback, NULL, 4, false);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    resetTlsBuffer();
    params.payload_len = strlen(TEST_JSON_RESPONSE_FULL_DOCUMENT);
    params.payload = TEST_JSON_RESPONSE_FULL_DOCUMENT;
    params.qos = QOS0;
    setTLSRxBufferWithMsgOnSubscribedTopic(GET_ACCEPTED_TOPIC, strlen(GET_ACCEPTED_TOPIC), QOS0, params.payload);
    ret_val = qcloud_iot_mqtt_yield(&client, 200);

    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    CHECK_EQUAL_C_STRING(TEST_JSON_RESPONSE_FULL_DOCUMENT, jsonFullDocument);
    CHECK_EQUAL_C_INT(GET, methodRx);
    CHECK_EQUAL_C_INT(ACK_ACCEPTED, ackStatusRx);
}

TEST_C(CloudDeviceTests, DeleteTheJSONDocument) {
    int ret_val = QCLOUD_ERR_SUCCESS;
    PublishParams params;

    ret_val = qcloud_iot_shadow_delete(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback, NULL, 4, false);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    params.payload_len = strlen(TEST_JSON_RESPONSE_DELETE_DOCUMENT);
    params.payload = TEST_JSON_RESPONSE_DELETE_DOCUMENT;
    params.qos = QOS0;
    setTLSRxBufferWithMsgOnSubscribedTopic(DELETE_ACCEPTED_TOPIC, strlen(DELETE_ACCEPTED_TOPIC), QOS0, params.payload);
    ret_val = qcloud_iot_shadow_yield(&client, 200);

    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    CHECK_EQUAL_C_STRING(TEST_JSON_RESPONSE_DELETE_DOCUMENT, jsonFullDocument);
    CHECK_EQUAL_C_INT(DELETE, methodRx);
    CHECK_EQUAL_C_INT(ACK_ACCEPTED, ackStatusRx);
}

TEST_C(CloudDeviceTests, UpdateTheJSONDocument) {
    int ret_val = QCLOUD_ERR_SUCCESS;
    char updateRequestJson[SIZE_OF_UPDATE_DOCUMENT];
    char expectedUpdateRequestJson[SIZE_OF_UPDATE_DOCUMENT];
    double doubleData = 4.0908f;
    float floatData = 3.445f;
    bool boolData = true;
    DeviceProperty dataFloatHandler;
    DeviceProperty dataDoubleHandler;
    DeviceProperty dataBoolHandler;
    PublishParams params;

    dataFloatHandler.data = &floatData;
    dataFloatHandler.key = "floatData";
    dataFloatHandler.type = FLOAT;

    dataDoubleHandler.data = &doubleData;
    dataDoubleHandler.key = "doubleData";
    dataDoubleHandler.type = DOUBLE;

    dataBoolHandler.data = &boolData;
    dataBoolHandler.key = "boolData";
    dataBoolHandler.type = BOOL;

    ret_val = json_document_init(updateRequestJson, SIZE_OF_UPDATE_DOCUMENT);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    ret_val = json_document_add_reported(updateRequestJson, SIZE_OF_UPDATE_DOCUMENT, 2, &dataDoubleHandler,
                                         &dataFloatHandler);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    ret_val = json_document_add_desired(updateRequestJson, SIZE_OF_UPDATE_DOCUMENT, 1, &dataBoolHandler);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    ret_val = json_document_finalize(updateRequestJson, SIZE_OF_UPDATE_DOCUMENT);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    snprintf(expectedUpdateRequestJson, SIZE_OF_UPDATE_DOCUMENT,
             "{\"state\":{\"reported\":{\"doubleData\":4.090800,\"floatData\":3.445000},\"desired\":{\"boolData\":true}}, \"clientToken\":\"%s-0\"}",
             QCLOUD_IOT_MQTT_CLIENT_ID);
    CHECK_EQUAL_C_STRING(expectedUpdateRequestJson, updateRequestJson);

    ret_val = qcloud_iot_shadow_update(&client, QCLOUD_IOT_MY_DEVICE_NAME, updateRequestJson,
                                      onRequestCallback, NULL, 4, false);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    resetTlsBuffer();
    snprintf(jsonFullDocument, 200, "%s", "");
    params.payload_len = strlen(TEST_JSON_RESPONSE_UPDATE_DOCUMENT);
    params.payload = TEST_JSON_RESPONSE_UPDATE_DOCUMENT;
    params.qos = QOS0;
    setTLSRxBufferWithMsgOnSubscribedTopic(UPDATE_ACCEPTED_TOPIC, strlen(UPDATE_ACCEPTED_TOPIC), QOS0, params.payload);
    ret_val = qcloud_iot_shadow_yield(&client, 200);

    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    CHECK_EQUAL_C_STRING(TEST_JSON_RESPONSE_UPDATE_DOCUMENT, jsonFullDocument);
    CHECK_EQUAL_C_INT(UPDATE, methodRx);
    CHECK_EQUAL_C_INT(ACK_ACCEPTED, ackStatusRx);

}

TEST_C(CloudDeviceTests, GetTheFullJSONDocumentTimeout) {
    int ret_val = QCLOUD_ERR_SUCCESS;
    PublishParams params;

    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback, NULL, 4, false);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    sleep(4 + 1);

    params.payload_len = strlen(TEST_JSON_RESPONSE_FULL_DOCUMENT);
    params.payload = TEST_JSON_RESPONSE_FULL_DOCUMENT;
    params.qos = QOS0;
    setTLSRxBufferWithMsgOnSubscribedTopic(GET_ACCEPTED_TOPIC, strlen(GET_ACCEPTED_TOPIC), QOS0, params.payload);
    ret_val = qcloud_iot_shadow_yield(&client, 200);

    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    CHECK_EQUAL_C_STRING(TEST_JSON_RESPONSE_FULL_DOCUMENT, jsonFullDocument);
    CHECK_EQUAL_C_INT(GET, methodRx);
    CHECK_EQUAL_C_INT(ACK_TIMEOUT, ackStatusRx);

}

// Subscribe and UnSubscribe on reception of thing names. Will perform the test with  Get operation
TEST_C(CloudDeviceTests, SubscribeToAcceptedRejectedOnGet) {
    int ret_val = QCLOUD_ERR_SUCCESS;

    uint8_t firstByte, secondByte;
    uint16_t topicNameLen;
    char topicName[128] = "test";

    lastSubscribeMsgLen = 11;
    snprintf(LastSubscribeMessage, lastSubscribeMsgLen, "No Message");
    secondLastSubscribeMsgLen = 11;
    snprintf(SecondLastSubscribeMessage, secondLastSubscribeMsgLen, "No Message");

    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback, NULL, 4, false);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    CHECK_EQUAL_C_STRING(GET_REJECTED_TOPIC, LastSubscribeMessage);
    CHECK_EQUAL_C_STRING(GET_ACCEPTED_TOPIC, SecondLastSubscribeMessage);

    firstByte = (uint8_t) (TxBuffer.pBuffer[2]);
    secondByte = (uint8_t) (TxBuffer.pBuffer[3]);
    topicNameLen = (uint16_t) (secondByte + (256 * firstByte));

    snprintf(topicName, topicNameLen + 1u, "%s", &(TxBuffer.pBuffer[4])); // Added one for null character

    // Verify publish happens
    CHECK_EQUAL_C_STRING(GET_PUB_TOPIC, topicName);
}

TEST_C(CloudDeviceTests, UnSubscribeToAcceptedRejectedOnGetResponse) {
    int ret_val = QCLOUD_ERR_SUCCESS;
    PublishParams params;

    lastSubscribeMsgLen = 11;
    snprintf(LastSubscribeMessage, lastSubscribeMsgLen, "No Message");
    secondLastSubscribeMsgLen = 11;
    snprintf(SecondLastSubscribeMessage, secondLastSubscribeMsgLen, "No Message");

    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback, NULL, 4, false);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    params.payload_len = strlen(TEST_JSON_RESPONSE_FULL_DOCUMENT);
    params.payload = TEST_JSON_RESPONSE_FULL_DOCUMENT;
    params.qos = QOS0;
    setTLSRxBufferWithMsgOnSubscribedTopic(GET_ACCEPTED_TOPIC, strlen(GET_ACCEPTED_TOPIC), QOS0, params.payload);
    ret_val = qcloud_iot_shadow_yield(&client, 200);

    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    CHECK_EQUAL_C_STRING(GET_REJECTED_TOPIC, LastSubscribeMessage);
    CHECK_EQUAL_C_STRING(GET_ACCEPTED_TOPIC, SecondLastSubscribeMessage);

}

TEST_C(CloudDeviceTests, UnSubscribeToAcceptedRejectedOnGetTimeout) {
    int ret_val = QCLOUD_ERR_SUCCESS;
    PublishParams params;

    snprintf(jsonFullDocument, 200, "aa");
    lastSubscribeMsgLen = 11;
    snprintf(LastSubscribeMessage, lastSubscribeMsgLen, "No Message");
    secondLastSubscribeMsgLen = 11;
    snprintf(SecondLastSubscribeMessage, secondLastSubscribeMsgLen, "No Message");

    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback, NULL, 4, false);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    sleep(4 + 1);

    params.payload_len = strlen(TEST_JSON_RESPONSE_FULL_DOCUMENT);
    params.payload = TEST_JSON_RESPONSE_FULL_DOCUMENT;
    params.qos = QOS0;
    setTLSRxBufferWithMsgOnSubscribedTopic(GET_ACCEPTED_TOPIC, strlen(GET_ACCEPTED_TOPIC), QOS0, params.payload);
    ret_val = qcloud_iot_shadow_yield(&client, 200);

    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    CHECK_EQUAL_C_STRING("aa", jsonFullDocument);

    CHECK_EQUAL_C_STRING(GET_REJECTED_TOPIC, LastSubscribeMessage);
    CHECK_EQUAL_C_STRING(GET_ACCEPTED_TOPIC, SecondLastSubscribeMessage);

}

TEST_C(CloudDeviceTests, UnSubscribeToAcceptedRejectedOnGetTimeoutWithSticky) {
    int ret_val = QCLOUD_ERR_SUCCESS;
    PublishParams params;

    snprintf(jsonFullDocument, 200, "aa");
    lastSubscribeMsgLen = 11;
    snprintf(LastSubscribeMessage, lastSubscribeMsgLen, "No Message");
    secondLastSubscribeMsgLen = 11;
    snprintf(SecondLastSubscribeMessage, secondLastSubscribeMsgLen, "No Message");

    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback, NULL, 4, true);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    sleep(4 + 1);

    params.payload_len = strlen(TEST_JSON_RESPONSE_FULL_DOCUMENT);
    params.payload = TEST_JSON_RESPONSE_FULL_DOCUMENT;
    params.qos = QOS0;
    setTLSRxBufferWithMsgOnSubscribedTopic(GET_ACCEPTED_TOPIC, strlen(GET_ACCEPTED_TOPIC), QOS0, params.payload);
    ret_val = qcloud_iot_shadow_yield(&client, 200);

    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    CHECK_EQUAL_C_STRING("aa", jsonFullDocument);

    CHECK_EQUAL_C_STRING(GET_REJECTED_TOPIC, LastSubscribeMessage);
    CHECK_EQUAL_C_STRING(GET_ACCEPTED_TOPIC, SecondLastSubscribeMessage);

}

#define TEST_JSON_RESPONSE_FULL_DOCUMENT_WITH_VERSION(num) "{\"state\":{\"reported\":{\"sensor1\":98}}, \"clientToken\":\"" QCLOUD_IOT_MQTT_CLIENT_ID "-0\",\"version\":" #num "}"

TEST_C(CloudDeviceTests, GetVersionFromAckStatus) {
    int ret_val = QCLOUD_ERR_SUCCESS;
    PublishParams params;
    PublishParams params2;

    snprintf(jsonFullDocument, 200, "timeout");

    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback, NULL, 4, true);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    resetTlsBuffer();
    params.payload = TEST_JSON_RESPONSE_FULL_DOCUMENT_WITH_VERSION(1);
    params.payload_len = strlen(params.payload);
    params.qos = QOS0;
    setTLSRxBufferWithMsgOnSubscribedTopic(GET_ACCEPTED_TOPIC, strlen(GET_ACCEPTED_TOPIC), QOS0, params.payload);
    ret_val = qcloud_iot_shadow_yield(&client, 200);

    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    CHECK_C(1u == qcloud_iot_shadow_get_document_version());

    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback, NULL, 4, false);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    resetTlsBuffer();
    params2.payload = TEST_JSON_RESPONSE_FULL_DOCUMENT_WITH_VERSION(132387);
    params2.payload_len = strlen(params2.payload);
    params2.qos = QOS0;
    setTLSRxBufferWithMsgOnSubscribedTopic(GET_ACCEPTED_TOPIC, strlen(GET_ACCEPTED_TOPIC), QOS0, params2.payload);
    ret_val = qcloud_iot_shadow_yield(&client, 200);

    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    CHECK_C(132387u == qcloud_iot_shadow_get_document_version());

}

#define TEST_JSON_RESPONSE_FULL_DOCUMENT_ALWAYS_WRONG_TOKEN "{\"state\":{\"reported\":{\"sensor1\":98}}, \"clientToken\":\"TroubleToken1234\"}"

TEST_C(CloudDeviceTests, WrongTokenInGetResponse) {
    int ret_val = QCLOUD_ERR_SUCCESS;
    PublishParams params;

    snprintf(jsonFullDocument, 200, "timeout");

    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback, NULL, 4, false);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    sleep(4 + 1);

    params.payload_len = strlen(TEST_JSON_RESPONSE_FULL_DOCUMENT_ALWAYS_WRONG_TOKEN);
    params.payload = TEST_JSON_RESPONSE_FULL_DOCUMENT_ALWAYS_WRONG_TOKEN;
    params.qos = QOS0;
    setTLSRxBufferWithMsgOnSubscribedTopic(GET_ACCEPTED_TOPIC, strlen(GET_ACCEPTED_TOPIC), QOS0, params.payload);
    ret_val = qcloud_iot_shadow_yield(&client, 200);

    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    CHECK_EQUAL_C_STRING("timeout", jsonFullDocument);
    CHECK_EQUAL_C_INT(GET, methodRx);
    CHECK_EQUAL_C_INT(ACK_TIMEOUT, ackStatusRx);

}

#define TEST_JSON_RESPONSE_FULL_DOCUMENT_NO_TOKEN "{\"state\":{\"reported\":{\"sensor1\":98}}}"

TEST_C(CloudDeviceTests, NoTokenInGetResponse) {
    int ret_val = QCLOUD_ERR_SUCCESS;
    PublishParams params;

    snprintf(jsonFullDocument, 200, "timeout");

    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback, NULL, 4, false);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    sleep(4 + 1);

    params.payload_len = strlen(TEST_JSON_RESPONSE_FULL_DOCUMENT_NO_TOKEN);
    params.payload = TEST_JSON_RESPONSE_FULL_DOCUMENT_NO_TOKEN;
    params.qos = QOS0;
    setTLSRxBufferWithMsgOnSubscribedTopic(GET_ACCEPTED_TOPIC, strlen(GET_ACCEPTED_TOPIC), QOS0, params.payload);
    ret_val = qcloud_iot_shadow_yield(&client, 200);

    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    CHECK_EQUAL_C_STRING("timeout", jsonFullDocument);
    CHECK_EQUAL_C_INT(GET, methodRx);
    CHECK_EQUAL_C_INT(ACK_TIMEOUT, ackStatusRx);

}

TEST_C(CloudDeviceTests, InvalidInboundJSONInGetResponse) {
    int ret_val = QCLOUD_ERR_SUCCESS;
    PublishParams params;

    snprintf(jsonFullDocument, 200, "NOT_VISITED");

    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback, NULL, 4, false);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    params.payload_len = strlen("{\"state\":{{");
    params.payload = "{\"state\":{{";
    params.qos = QOS0;
    setTLSRxBufferWithMsgOnSubscribedTopic(GET_ACCEPTED_TOPIC, strlen(GET_ACCEPTED_TOPIC), QOS0, params.payload);
    ret_val = qcloud_iot_shadow_yield(&client, 200);

    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    CHECK_EQUAL_C_STRING("NOT_VISITED", jsonFullDocument);

}

TEST_C(CloudDeviceTests, AcceptedSubFailsGetRequest) {
    int ret_val = QCLOUD_ERR_SUCCESS;
    PublishParams params;

    snprintf(jsonFullDocument, 200, "NOT_SENT");

    resetTlsBuffer();
    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback, NULL, 4, false);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_REQUEST_TIMEOUT, ret_val); // Should never subscribe and publish

    resetTlsBuffer();
    params.payload_len = strlen(TEST_JSON_RESPONSE_FULL_DOCUMENT);
    params.payload = TEST_JSON_RESPONSE_FULL_DOCUMENT;
    params.qos = QOS0;
    setTLSRxBufferWithMsgOnSubscribedTopic(GET_ACCEPTED_TOPIC, strlen(GET_ACCEPTED_TOPIC), QOS0, params.payload);
    ret_val = qcloud_iot_shadow_yield(&client, 200);

    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    CHECK_EQUAL_C_STRING("NOT_SENT", jsonFullDocument); // Never called callback

}

TEST_C(CloudDeviceTests, RejectedSubFailsGetRequest) {
    int ret_val = QCLOUD_ERR_SUCCESS;
    PublishParams params;

    snprintf(jsonFullDocument, 200, "NOT_SENT");

    params.payload_len = strlen(TEST_JSON_RESPONSE_FULL_DOCUMENT);
    params.payload = TEST_JSON_RESPONSE_FULL_DOCUMENT;
    params.qos = QOS0;

    resetTlsBuffer();
    setTLSRxBufferForSuback(QOS0);
    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback, NULL, 4, false);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_REQUEST_TIMEOUT, ret_val); // Should never subscribe and publish

    resetTlsBuffer();
    setTLSRxBufferWithMsgOnSubscribedTopic(GET_ACCEPTED_TOPIC, strlen(GET_ACCEPTED_TOPIC), QOS0, params.payload);
    ret_val = qcloud_iot_shadow_yield(&client, 200);

    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    CHECK_EQUAL_C_STRING("NOT_SENT", jsonFullDocument); // Never called callback
}

TEST_C(CloudDeviceTests, PublishFailsGetRequest) {
    int ret_val = QCLOUD_ERR_SUCCESS;
    PublishParams params;

    snprintf(jsonFullDocument, 200, "NOT_SENT");

    resetTlsBuffer();

    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback, NULL, 4, false);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_REQUEST_TIMEOUT, ret_val); // Should never subscribe and publish

    resetTlsBuffer();
    params.payload_len = strlen(TEST_JSON_RESPONSE_FULL_DOCUMENT);
    params.payload = TEST_JSON_RESPONSE_FULL_DOCUMENT;
    params.qos = QOS0;
    setTLSRxBufferWithMsgOnSubscribedTopic(GET_ACCEPTED_TOPIC, strlen(GET_ACCEPTED_TOPIC), QOS0, params.payload);
    ret_val = qcloud_iot_shadow_yield(&client, 200);

    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    CHECK_EQUAL_C_STRING("NOT_SENT", jsonFullDocument); // Never called callback

}

TEST_C(CloudDeviceTests, StickyNonStickyNeverConflict) {
    int ret_val = QCLOUD_ERR_SUCCESS;
    PublishParams params;

    lastSubscribeMsgLen = 11;
    snprintf(LastSubscribeMessage, lastSubscribeMsgLen, "No Message");
    secondLastSubscribeMsgLen = 11;
    snprintf(SecondLastSubscribeMessage, secondLastSubscribeMsgLen, "No Message");

    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback, NULL, 4, true);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    params.payload_len = strlen(TEST_JSON_RESPONSE_FULL_DOCUMENT);
    params.payload = TEST_JSON_RESPONSE_FULL_DOCUMENT;
    params.qos = QOS0;
    setTLSRxBufferWithMsgOnSubscribedTopic(GET_ACCEPTED_TOPIC, strlen(GET_ACCEPTED_TOPIC), QOS0, params.payload);
    ret_val = qcloud_iot_shadow_yield(&client, 200);

    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    CHECK_EQUAL_C_STRING(TEST_JSON_RESPONSE_FULL_DOCUMENT, jsonFullDocument);
    CHECK_EQUAL_C_INT(GET, methodRx);
    CHECK_EQUAL_C_INT(ACK_ACCEPTED, ackStatusRx);

    lastSubscribeMsgLen = 11;
    snprintf(LastSubscribeMessage, lastSubscribeMsgLen, "No Message");
    secondLastSubscribeMsgLen = 11;
    snprintf(SecondLastSubscribeMessage, secondLastSubscribeMsgLen, "No Message");

    // Non-sticky shadow get, same thing name. Should never unsub since they are sticky
    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback, NULL, 4, false);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    resetTlsBuffer();

    params.payload_len = strlen(TEST_JSON_RESPONSE_FULL_DOCUMENT);
    params.payload = TEST_JSON_RESPONSE_FULL_DOCUMENT;
    params.qos = QOS0;
    setTLSRxBufferWithMsgOnSubscribedTopic(GET_ACCEPTED_TOPIC, strlen(GET_ACCEPTED_TOPIC), QOS0, params.payload);
    ret_val = qcloud_iot_shadow_yield(&client, 200);

    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    CHECK_EQUAL_C_STRING(TEST_JSON_RESPONSE_FULL_DOCUMENT, jsonFullDocument);
    CHECK_EQUAL_C_INT(GET, methodRx);
    CHECK_EQUAL_C_INT(ACK_ACCEPTED, ackStatusRx);

    CHECK_EQUAL_C_STRING("No Message", LastSubscribeMessage);
    CHECK_EQUAL_C_STRING("No Message", SecondLastSubscribeMessage);

}

TEST_C(CloudDeviceTests, ACKWaitingMoreThanAllowed) {
    int ret_val = QCLOUD_ERR_SUCCESS;

    // 1st
    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback, NULL, 100, false); // 100 sec to timeout
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    // 2nd
    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback, NULL, 100, false); // 100 sec to timeout
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    // 3rd
    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback, NULL, 100, false); // 100 sec to timeout
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    // 4th
    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback, NULL, 100, false); // 100 sec to timeout
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    // 5th
    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback, NULL,
                                             100, false); // 100 sec to timeout
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    // 6th
    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback,
                                             NULL,
                                             100, false); // 100 sec to timeout
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    // 7th
    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback,
                                             NULL,
                                             100, false); // 100 sec to timeout
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    // 8th
    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback,
                                             NULL,
                                             100, false); // 100 sec to timeout
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    // 9th
    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback,
                                             NULL,
                                             100, false); // 100 sec to timeout
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    // 10th
    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback,
                                             NULL,
                                             100, false); // 100 sec to timeout
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    // 11th
    // Should return some error code, since we are running out of ACK space
    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback,
                                             NULL,
                                             100, false); // 100 sec to timeout
    CHECK_EQUAL_C_INT(QCLOUD_ERR_MAX_APPENDING_REQUEST, ret_val);

}

#define JSON_SIZE_OVERFLOW "{\"state\":{\"reported\":{\"file\":\"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\"}}, \"clientToken\":\"" QCLOUD_IOT_MQTT_CLIENT_ID "-0\"}"

TEST_C(CloudDeviceTests, InboundDataTooBigForBuffer) {
    int ret_val = QCLOUD_ERR_SUCCESS;
    PublishParams params;

    snprintf(jsonFullDocument, 200, "NOT_VISITED");

    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback,
                                             NULL, 4,
                                             false);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    params.payload_len = strlen(JSON_SIZE_OVERFLOW);
    params.payload = JSON_SIZE_OVERFLOW;
    params.qos = QOS0;
    setTLSRxBufferWithMsgOnSubscribedTopic(GET_ACCEPTED_TOPIC, strlen(GET_ACCEPTED_TOPIC), QOS0, params.payload);
    ret_val = qcloud_iot_shadow_yield(&client, 200);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_BUF_TOO_SHORT, ret_val);
    CHECK_EQUAL_C_STRING("NOT_VISITED", jsonFullDocument);

}

#define TEST_JSON_RESPONSE_NO_TOKEN "{\"state\":{\"reported\":{\"sensor1\":98}}}"

TEST_C(CloudDeviceTests, NoClientTokenForShadowAction) {
    int ret_val = QCLOUD_ERR_SUCCESS;
    PublishParams params;

    uint8_t firstByte, secondByte;
    uint16_t topicNameLen;
    char topicName[128] = "test";

    snprintf(jsonFullDocument, 200, "NOT_VISITED");

    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, onRequestCallback, NULL, 4, false);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    params.payload_len = strlen(TEST_JSON_RESPONSE_NO_TOKEN);
    params.payload = TEST_JSON_RESPONSE_NO_TOKEN;
    params.qos = QOS0;
    setTLSRxBufferWithMsgOnSubscribedTopic(GET_ACCEPTED_TOPIC, strlen(GET_ACCEPTED_TOPIC), QOS0, params.payload);
    ret_val = qcloud_iot_shadow_yield(&client, 200);

    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    // Should never subscribe to accepted/rejected topics since we have no token to track the response
    CHECK_EQUAL_C_STRING("NOT_VISITED", jsonFullDocument);

    firstByte = (uint8_t) (TxBuffer.pBuffer[2]);
    secondByte = (uint8_t) (TxBuffer.pBuffer[3]);
    topicNameLen = (uint16_t) (secondByte + (256 * firstByte));

    snprintf(topicName, topicNameLen + 1u, "%s", &(TxBuffer.pBuffer[4])); // Added one for null character

    // Verify publish happens
    CHECK_EQUAL_C_STRING(GET_PUB_TOPIC, topicName);
}

TEST_C(CloudDeviceTests, NoCallbackForShadowAction) {
    int ret_val = QCLOUD_ERR_SUCCESS;
    PublishParams params;

    uint8_t firstByte, secondByte;
    uint16_t topicNameLen;
    char topicName[128] = "test";

    snprintf(jsonFullDocument, 200, "NOT_VISITED");

    ret_val = qcloud_iot_shadow_get(&client, QCLOUD_IOT_MY_DEVICE_NAME, NULL, NULL, 4, false);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    params.qos = QOS0;
    params.payload_len = strlen(TEST_JSON_RESPONSE_FULL_DOCUMENT);
    params.payload = TEST_JSON_RESPONSE_FULL_DOCUMENT;
    setTLSRxBufferWithMsgOnSubscribedTopic(GET_ACCEPTED_TOPIC, strlen(GET_ACCEPTED_TOPIC), QOS0, params.payload);
    ret_val = qcloud_iot_shadow_yield(&client, 200);

    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    // Should never subscribe to accepted/rejected topics since we have no callback to track the response
    CHECK_EQUAL_C_STRING("NOT_VISITED", jsonFullDocument);

    firstByte = (uint8_t) (TxBuffer.pBuffer[2]);
    secondByte = (uint8_t) (TxBuffer.pBuffer[3]);
    topicNameLen = (uint16_t) (secondByte + (256 * firstByte));

    snprintf(topicName, topicNameLen + 1u, "%s", &(TxBuffer.pBuffer[4])); // Added one for null character

    // Verify publish happens
    CHECK_EQUAL_C_STRING(GET_PUB_TOPIC, topicName);
}
