#include <string.h>
#include <stdio.h>
#include <CppUTest/TestHarness_c.h>
#include <qcloud_iot_mqtt_client.h>
#include <qcloud_iot_shadow_device.h>
#include <iot_unit_config.h>
#include <unit_helper_functions.h>

static Qcloud_IoT_Client client;
static ShadowConnectParams cloudConnectParams;

static char receivedNestedObject[100] = "";
static char sentNestedObjectData[100] = "{\"sensor1\":23}";
static char shadowDeltaTopic[MAX_SIZE_OF_CLOUD_TOPIC];

#define SHADOW_DELTA_UPDATE "$shadow/update/delta/%s/%s"

#undef QCLOUD_IOT_MY_DEVICE_NAME
#define QCLOUD_IOT_MY_DEVICE_NAME "QCLOUD-IoT-C-SDK"

void genericCallback(const char *pJsonStringData, uint32_t JsonStringDataLen, DeviceProperty *pContext) {
    printf("\nkey[%s]==Data[%.*s]\n", pContext->key, JsonStringDataLen, pJsonStringData);
}

void nestedObjectCallback(const char *pJsonStringData, uint32_t JsonStringDataLen, DeviceProperty *pContext) {
    printf("\nkey[%s]==Data[%.*s]\n", pContext->key, JsonStringDataLen, pJsonStringData);
    snprintf(receivedNestedObject, 100, "%.*s", JsonStringDataLen, pJsonStringData);
}

TEST_GROUP_C_SETUP(CloudDeltaTest) {
    int ret_val = QCLOUD_ERR_SUCCESS;

    cloudConnectParams.mqtt.cert_file = QCLOUD_IOT_CERT_FILENAME;
    cloudConnectParams.mqtt.ca_file = QCLOUD_IOT_CA_FILENAME;
    cloudConnectParams.mqtt.key_file = QCLOUD_IOT_KEY_FILENAME;
    cloudConnectParams.mqtt.on_disconnect_handler = NULL;
    cloudConnectParams.mqtt.auto_connect_enable = false;
    ret_val = qcloud_iot_shadow_init(&client, &cloudConnectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    cloudConnectParams.mqtt.product_name = QCLOUD_IOT_MY_PRODUCT_NAME;
    cloudConnectParams.mqtt.device_name = QCLOUD_IOT_MY_DEVICE_NAME;
    cloudConnectParams.mqtt.client_id = QCLOUD_IOT_MQTT_CLIENT_ID;
    ConnectParamsSetup(&cloudConnectParams.mqtt, QCLOUD_IOT_MQTT_CLIENT_ID);
    setTLSRxBufferForConnack(&cloudConnectParams.mqtt, 0, 0);
    ret_val = qcloud_iot_shadow_connect(&client, &cloudConnectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    snprintf(shadowDeltaTopic, MAX_SIZE_OF_CLOUD_TOPIC, SHADOW_DELTA_UPDATE, QCLOUD_IOT_MY_DEVICE_NAME);
}

TEST_GROUP_C_TEARDOWN(CloudDeltaTest) {

}

TEST_C(CloudDeltaTest, registerDeltaSuccess) {
    DeviceProperty windowHandler;
    char deltaJSONString[] = "{\"state\":{\"delta\":{\"window\":true}},\"version\":1}";
    bool windowOpenData = false;
    PublishParams params;
    int ret_val = QCLOUD_ERR_SUCCESS;

    windowHandler.key = "window";
    windowHandler.type = BOOL;
    windowHandler.data = &windowOpenData;

    params.payload_len = strlen(deltaJSONString);
    params.payload = deltaJSONString;
    params.qos = QOS0;

    resetTlsBuffer();
    setTLSRxBufferForSuback(QOS0);

    ret_val = qcloud_iot_shadow_register_property(&client, &windowHandler, genericCallback);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    resetTlsBuffer();
    setTLSRxBufferWithMsgOnSubscribedTopic(shadowDeltaTopic, strlen(shadowDeltaTopic), QOS0, params.payload);

    ret_val = qcloud_iot_shadow_yield(&client, 3000);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    CHECK_EQUAL_C_INT(true, windowOpenData);
}


TEST_C(CloudDeltaTest, registerDeltaInt) {
    int ret_val = QCLOUD_ERR_SUCCESS;
    DeviceProperty intHandler;
    int intData = 0;
    char deltaJSONString[] = "{\"state\":{\"delta\":{\"length\":23}},\"version\":1}";
    PublishParams params;

    intHandler.key = "length";
    intHandler.type = INT32;
    intHandler.data = &intData;

    params.payload_len = strlen(deltaJSONString);
    params.payload = deltaJSONString;
    params.qos = QOS0;

    resetTlsBuffer();
    setTLSRxBufferForSuback(QOS0);

    ret_val = qcloud_iot_shadow_register_property(&client, &intHandler, genericCallback);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    resetTlsBuffer();
    setTLSRxBufferWithMsgOnSubscribedTopic(shadowDeltaTopic, strlen(shadowDeltaTopic), QOS0, params.payload);

    qcloud_iot_shadow_yield(&client, 3000);
    CHECK_EQUAL_C_INT(23, intData);
}

TEST_C(CloudDeltaTest, registerDeltaIntNoCallback) {
    int ret_val = QCLOUD_ERR_SUCCESS;
    DeviceProperty intHandler;
    int intData = 0;
    intHandler.key = "length_nocb";
    intHandler.type = INT32;
    intHandler.data = &intData;

    ret_val = qcloud_iot_shadow_register_property(&client, &intHandler, NULL);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, ret_val);
}

TEST_C(CloudDeltaTest, DeltaNestedObject) {
    int ret_val = QCLOUD_ERR_SUCCESS;
    PublishParams params;
    DeviceProperty nestedObjectHandler;
    char nestedObject[100];
    char deltaJSONString[100];

    nestedObjectHandler.key = "sensors";
    nestedObjectHandler.type = OBJECT;
    nestedObjectHandler.data = &nestedObject;

    snprintf(deltaJSONString, 100, "{\"state\":{\"delta\":{\"%s\":%s}},\"version\":1}", nestedObjectHandler.key,
             sentNestedObjectData);

    params.payload_len = strlen(deltaJSONString);
    params.payload = deltaJSONString;
    params.qos = QOS0;

    resetTlsBuffer();
    setTLSRxBufferForSuback(QOS0);

    ret_val = qcloud_iot_shadow_register_property(&client, &nestedObjectHandler, nestedObjectCallback);

    resetTlsBuffer();
    setTLSRxBufferWithMsgOnSubscribedTopic(shadowDeltaTopic, strlen(shadowDeltaTopic), QOS0, params.payload);

    qcloud_iot_shadow_yield(&client, 3000);
    CHECK_EQUAL_C_STRING(sentNestedObjectData, receivedNestedObject);
}


// Send back to back version and ensure a wrong version is ignored with old message enabled
TEST_C(CloudDeltaTest, DeltaVersionIgnoreOldVersion) {
    int ret_val = QCLOUD_ERR_SUCCESS;
    char deltaJSONString[100];
    DeviceProperty nestedObjectHandler;
    char nestedObject[100];
    PublishParams params;

    printf("\n-->Running Shadow Delta Tests - delta received, old version ignored \n");

    nestedObjectHandler.key = "sensors";
    nestedObjectHandler.type = OBJECT;
    nestedObjectHandler.data = &nestedObject;

    snprintf(deltaJSONString, 100, "{\"state\":{\"delta\":{\"%s\":%s}},\"version\":1}", nestedObjectHandler.key,
             sentNestedObjectData);

    params.payload_len = strlen(deltaJSONString);
    params.payload = deltaJSONString;
    params.qos = QOS0;

    resetTlsBuffer();
    setTLSRxBufferForSuback(QOS0);

    ret_val = qcloud_iot_shadow_register_property(&client, &nestedObjectHandler, nestedObjectCallback);

    resetTlsBuffer();
    setTLSRxBufferWithMsgOnSubscribedTopic(shadowDeltaTopic, strlen(shadowDeltaTopic), QOS0, params.payload);

    qcloud_iot_shadow_yield(&client, 100);
    CHECK_EQUAL_C_STRING(sentNestedObjectData, receivedNestedObject);

    snprintf(receivedNestedObject, 100, " ");
    snprintf(deltaJSONString, 100, "{\"state\":{\"delta\":{\"%s\":%s}},\"version\":2}", nestedObjectHandler.key,
             sentNestedObjectData);
    params.payload_len = strlen(deltaJSONString);
    params.payload = deltaJSONString;
    params.qos = QOS0;

    resetTlsBuffer();
    setTLSRxBufferWithMsgOnSubscribedTopic(shadowDeltaTopic, strlen(shadowDeltaTopic), QOS0,  params.payload);

    qcloud_iot_shadow_yield(&client, 100);
    CHECK_EQUAL_C_STRING(sentNestedObjectData, receivedNestedObject);

    snprintf(receivedNestedObject, 100, " ");
    snprintf(deltaJSONString, 100, "{\"state\":{\"delta\":{\"%s\":%s}},\"version\":2}", nestedObjectHandler.key,
             sentNestedObjectData);
    params.payload_len = strlen(deltaJSONString);
    params.payload = deltaJSONString;
    params.qos = QOS0;

    resetTlsBuffer();
    setTLSRxBufferWithMsgOnSubscribedTopic(shadowDeltaTopic, strlen(shadowDeltaTopic), QOS0, params.payload);

    qcloud_iot_shadow_yield(&client, 100);
    CHECK_EQUAL_C_STRING(" ", receivedNestedObject);

    snprintf(receivedNestedObject, 100, " ");
    snprintf(deltaJSONString, 100, "{\"state\":{\"delta\":{\"%s\":%s}},\"version\":3}", nestedObjectHandler.key,
             sentNestedObjectData);
    params.payload_len = strlen(deltaJSONString);
    params.payload = deltaJSONString;
    params.qos = QOS0;

    resetTlsBuffer();
    setTLSRxBufferWithMsgOnSubscribedTopic(shadowDeltaTopic, strlen(shadowDeltaTopic), QOS0, params.payload);

    qcloud_iot_shadow_yield(&client, 100);
    CHECK_EQUAL_C_STRING(sentNestedObjectData, receivedNestedObject);

    iot_shadow_reset_document_version();

    snprintf(receivedNestedObject, 100, " ");
    snprintf(deltaJSONString, 100, "{\"state\":{\"delta\":{\"%s\":%s}},\"version\":3}", nestedObjectHandler.key,
             sentNestedObjectData);
    params.payload_len = strlen(deltaJSONString);
    params.payload = deltaJSONString;
    params.qos = QOS0;

    resetTlsBuffer();
    setTLSRxBufferWithMsgOnSubscribedTopic(shadowDeltaTopic, strlen(shadowDeltaTopic), QOS0, params.payload);

    qcloud_iot_shadow_yield(&client, 100);
    CHECK_EQUAL_C_STRING(sentNestedObjectData, receivedNestedObject);

    snprintf(receivedNestedObject, 100, " ");
    snprintf(deltaJSONString, 100, "{\"state\":{\"delta\":{\"%s\":%s}},\"version\":3}", nestedObjectHandler.key,
             sentNestedObjectData);
    params.payload_len = strlen(deltaJSONString);
    params.payload = deltaJSONString;
    params.qos = QOS0;

    resetTlsBuffer();
    setTLSRxBufferWithMsgOnSubscribedTopic(shadowDeltaTopic, strlen(shadowDeltaTopic), QOS0, params.payload);

    qcloud_iot_shadow_yield(&client, 100);
    CHECK_EQUAL_C_STRING(" ", receivedNestedObject);

    qcloud_iot_set_discard_old_delta_enable(false);

    snprintf(receivedNestedObject, 100, " ");
    snprintf(deltaJSONString, 100, "{\"state\":{\"delta\":{\"%s\":%s}},\"version\":3}", nestedObjectHandler.key,
             sentNestedObjectData);
    params.payload_len = strlen(deltaJSONString);
    params.payload = deltaJSONString;
    params.qos = QOS0;

    resetTlsBuffer();
    setTLSRxBufferWithMsgOnSubscribedTopic(shadowDeltaTopic, strlen(shadowDeltaTopic), QOS0, params.payload);

    qcloud_iot_shadow_yield(&client, 100);
    CHECK_EQUAL_C_STRING(sentNestedObjectData, receivedNestedObject);
}
