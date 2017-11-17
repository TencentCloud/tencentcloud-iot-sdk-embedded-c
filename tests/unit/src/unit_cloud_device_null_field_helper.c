#include <stdio.h>
#include <string.h>
#include <CppUTest/TestHarness_c.h>
#include <qcloud_iot_mqtt_client.h>
#include <qcloud_iot_shadow_device.h>
#include <unit_helper_functions.h>

static Qcloud_IoT_Client client;
static ShadowConnectParams cloudConnectParams;

static RequestAck ackStatusRx;
static Method actionRx;
static char jsonFullDocument[200];

void actionCallbackNullTest(const char *pThingName, Method method, RequestAck status,
                            const char *pReceivedJsonDocument, void *pContextData) {
    actionRx = method;
    ackStatusRx = status;
    if(status != ACK_TIMEOUT) {
        strcpy(jsonFullDocument, pReceivedJsonDocument);
    }
}

TEST_GROUP_C_SETUP(CloudNullFields) {
    resetTlsBuffer();
}

TEST_GROUP_C_TEARDOWN(CloudNullFields) { }

TEST_C(CloudNullFields, NullHost) {
    int rc = qcloud_iot_shadow_init(&client, &cloudConnectParams.mqtt);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

TEST_C(CloudNullFields, NullPort) {
    int rc = qcloud_iot_shadow_init(&client, &cloudConnectParams.mqtt);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

TEST_C(CloudNullFields, NullClientID) {
    cloudConnectParams.mqtt.cert_file = QCLOUD_IOT_CERT_FILENAME;
    cloudConnectParams.mqtt.ca_file = QCLOUD_IOT_CA_FILENAME;
    cloudConnectParams.mqtt.key_file = QCLOUD_IOT_KEY_FILENAME;
    cloudConnectParams.mqtt.on_disconnect_handler = NULL;
    cloudConnectParams.mqtt.auto_connect_enable = false;
    int rc = qcloud_iot_shadow_init(&client, &cloudConnectParams.mqtt);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    cloudConnectParams.mqtt.product_name = QCLOUD_IOT_MY_PRODUCT_NAME;
    cloudConnectParams.mqtt.device_name = QCLOUD_IOT_MY_DEVICE_NAME;
    cloudConnectParams.mqtt.client_id = NULL;
    rc = qcloud_iot_shadow_connect(&client, &cloudConnectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

TEST_C(CloudNullFields, NullClientInit) {
    int rc = qcloud_iot_shadow_init(NULL, &cloudConnectParams.mqtt);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

TEST_C(CloudNullFields, NullClientConnect) {
    cloudConnectParams.mqtt.cert_file = QCLOUD_IOT_CERT_FILENAME;
    cloudConnectParams.mqtt.ca_file = QCLOUD_IOT_CA_FILENAME;
    cloudConnectParams.mqtt.key_file = QCLOUD_IOT_KEY_FILENAME;
    cloudConnectParams.mqtt.on_disconnect_handler = NULL;
    cloudConnectParams.mqtt.auto_connect_enable = false;
    int rc = qcloud_iot_shadow_init(&client, &cloudConnectParams.mqtt);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

    cloudConnectParams.mqtt.product_name = QCLOUD_IOT_MY_PRODUCT_NAME;
    cloudConnectParams.mqtt.device_name = QCLOUD_IOT_MY_DEVICE_NAME;
    cloudConnectParams.mqtt.client_id = QCLOUD_IOT_MQTT_CLIENT_ID;
    rc = qcloud_iot_shadow_connect(NULL, &cloudConnectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

TEST_C(CloudNullFields, NullUpdateDocument) {
    int rc = qcloud_iot_shadow_update(&client, QCLOUD_IOT_MY_DEVICE_NAME, NULL, actionCallbackNullTest, NULL, 4, false);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

TEST_C(CloudNullFields, NullClientYield) {
    int rc = qcloud_iot_shadow_yield(NULL, 1000);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

TEST_C(CloudNullFields, NullClientDisconnect) {
    int rc = qcloud_iot_shadow_disconnect(NULL);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

TEST_C(CloudNullFields, NullClientShadowGet) {
    int rc = qcloud_iot_shadow_get( NULL, QCLOUD_IOT_MY_DEVICE_NAME, actionCallbackNullTest, NULL, 100, true);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

TEST_C(CloudNullFields, NullClientShadowUpdate) {
    int rc = qcloud_iot_shadow_update(NULL, QCLOUD_IOT_MY_DEVICE_NAME, jsonFullDocument, actionCallbackNullTest, NULL, 100, true);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

TEST_C(CloudNullFields, NullClientShadowDelete) {

    int rc = qcloud_iot_shadow_delete(NULL, QCLOUD_IOT_MY_DEVICE_NAME, actionCallbackNullTest, NULL, 100, true);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}
