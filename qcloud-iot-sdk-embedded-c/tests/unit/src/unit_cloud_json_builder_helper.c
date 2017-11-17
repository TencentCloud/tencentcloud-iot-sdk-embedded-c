#include <string.h>
#include <CppUTest/TestHarness_c.h>
#include <qcloud_iot_json_doc_helper.h>
#include <qcloud_iot_shadow_device.h>
#include <iot_unit_config.h>
#include <unit_helper_functions.h>
#include <jsmn.h>

static DeviceProperty dataFloatHandler;
static DeviceProperty dataDoubleHandler;
static DeviceProperty dataIntegerHandler;
static int integerData = 1;
static double doubleData = 4.0908f;
static float floatData = 3.445f;
static ShadowConnectParams connectParams;
static Qcloud_IoT_Client client;

TEST_GROUP_C_SETUP(CloudJsonBuilderTests) {

    int ret_val;

    connectParams.mqtt.cert_file = QCLOUD_IOT_CERT_FILENAME;
    connectParams.mqtt.key_file = QCLOUD_IOT_KEY_FILENAME;
    connectParams.mqtt.ca_file = QCLOUD_IOT_CA_FILENAME;
    connectParams.mqtt.on_disconnect_handler = NULL;
    connectParams.mqtt.auto_connect_enable = 0;
    ret_val = qcloud_iot_shadow_init(&client, &connectParams.mqtt);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    connectParams.mqtt.client_id = QCLOUD_IOT_MQTT_CLIENT_ID;
    connectParams.mqtt.device_name = QCLOUD_IOT_MY_DEVICE_NAME;
    connectParams.mqtt.product_name = QCLOUD_IOT_MY_PRODUCT_NAME;

    setTLSRxBufferForConnack(&connectParams.mqtt, 0, 0);
    ret_val = qcloud_iot_shadow_connect(&client, &connectParams);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    
    dataFloatHandler.data = &floatData;
    dataFloatHandler.key = "floatData";
    dataFloatHandler.type = FLOAT;
    
    dataDoubleHandler.data = &doubleData;
    dataDoubleHandler.key = "doubleData";
    dataDoubleHandler.type = DOUBLE;

    dataIntegerHandler.data = &integerData;
    dataIntegerHandler.key = "integerData";
    dataIntegerHandler.type = INT32;
}

TEST_GROUP_C_TEARDOWN(CloudJsonBuilderTests) {
    /* Clean up. Not checking return code here because this is common to all tests.
     * A test might have already caused a disconnect by this point.
     */
    qcloud_iot_shadow_disconnect(&client);
}

#define TEST_JSON_UPDATE_DOCUMENT "{\"state\":{\"reported\":{\"doubleData\":4.090800,\"floatData\":3.445000}}, \"clientToken\":\"" QCLOUD_IOT_MQTT_CLIENT_ID "-0\"}"
#define TEST_JSON_EMPTY_GET_DELETE_DOCUMENT "{\"clientToken\":\"" QCLOUD_IOT_MQTT_CLIENT_ID "-0\"}"

#define TEST_JSON_RESPONSE_UPDATE_DOCUMENT "{\"state\":{\"reported\":{\"integerData\":24}}, \"clientToken\":\"" QCLOUD_IOT_MQTT_CLIENT_ID "-0\", \"version\":1, \"code\": 200, \"message\": \"OK\"}"

#define SIZE_OF_UPFATE_BUF 200

TEST_C(CloudJsonBuilderTests, BuildEmptyJson) {
    char getOrDeleteRequestJson[SIZE_OF_UPFATE_BUF];
    build_empty_json(getOrDeleteRequestJson);
    CHECK_EQUAL_C_STRING(TEST_JSON_EMPTY_GET_DELETE_DOCUMENT, getOrDeleteRequestJson);
}

TEST_C(CloudJsonBuilderTests, UpdateTheJSONDocumentBuilder) {
    int ret_val;
    char updateRequestJson[SIZE_OF_UPFATE_BUF];
    size_t jsonBufSize = sizeof(updateRequestJson) / sizeof(updateRequestJson[0]);

    Log_d("\n-->Running Cloud Json Builder Tests - Update the Json document builder \n");

    ret_val = json_document_init(updateRequestJson, jsonBufSize);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    ret_val = json_document_add_reported(updateRequestJson, jsonBufSize, 2, &dataDoubleHandler, &dataFloatHandler);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    ret_val = json_document_finalize(updateRequestJson, jsonBufSize);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);

    CHECK_EQUAL_C_STRING(TEST_JSON_UPDATE_DOCUMENT, updateRequestJson);
}

TEST_C(CloudJsonBuilderTests, PassingNullValue) {
    int ret_val;

    Log_d("\n-->Running Cloud Json Builder Tests - Passing Null value to Shadow json builder \n");

    ret_val = json_document_init(NULL, SIZE_OF_UPFATE_BUF);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, ret_val);
    ret_val = json_document_add_reported(NULL, SIZE_OF_UPFATE_BUF, 2, &dataDoubleHandler, &dataFloatHandler);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, ret_val);
    ret_val = json_document_add_desired(NULL, SIZE_OF_UPFATE_BUF, 2, &dataDoubleHandler, &dataFloatHandler);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, ret_val);
    ret_val = json_document_finalize(NULL, SIZE_OF_UPFATE_BUF);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, ret_val);
}

TEST_C(CloudJsonBuilderTests, SmallBuffer) {
    int ret_val;
    char updateRequestJson[14];
    size_t jsonBufSize = sizeof(updateRequestJson) / sizeof(updateRequestJson[0]);

    Log_d("\n-->Running Shadow Json Builder Tests - Json Buffer is too small \n");

    ret_val = json_document_init(updateRequestJson, jsonBufSize);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, ret_val);
    ret_val = json_document_add_reported(updateRequestJson, jsonBufSize, 2, &dataDoubleHandler, &dataFloatHandler);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_JSON_BUFFER_TRUNCATED, ret_val);
    ret_val = json_document_add_desired(updateRequestJson, jsonBufSize, 2, &dataDoubleHandler, &dataFloatHandler);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_JSON_BUFFER_TOO_SMALL, ret_val);
    ret_val = json_document_finalize(updateRequestJson, jsonBufSize);
    CHECK_EQUAL_C_INT(QCLOUD_ERR_JSON_BUFFER_TOO_SMALL, ret_val);
}

TEST_C(CloudJsonBuilderTests, CheckParseJson) {
    bool ret_val;
    int32_t tokenCount = 0;
    jsmntok_t tokens[20];
    ret_val = check_and_parse_json(TEST_JSON_UPDATE_DOCUMENT, &tokenCount, tokens);
    CHECK_EQUAL_C_INT(11, tokenCount);
    CHECK_EQUAL_C_INT(1, (int) ret_val);
}

TEST_C(CloudJsonBuilderTests, InvalidCheckParseJson) {
    bool ret_val;
    int32_t tokenCount = 0;
    jsmntok_t tokens[20];
    ret_val = check_and_parse_json("*", &tokenCount, tokens);
    CHECK_EQUAL_C_INT(0, tokenCount);
    CHECK_EQUAL_C_INT(0, (int) ret_val);
}

TEST_C(CloudJsonBuilderTests, UpdateValueIfKeyMatch) {

    bool ret_val;
    int32_t  tokenCount = 0;
    uint32_t dataLength = 0;
    int32_t  dataPosition = 0;
    ret_val = check_and_parse_json(TEST_JSON_RESPONSE_UPDATE_DOCUMENT, &tokenCount, NULL);
    CHECK_EQUAL_C_INT(15, tokenCount);
    CHECK_EQUAL_C_INT(1, (int) ret_val);

    ret_val = update_value_if_key_match(TEST_JSON_RESPONSE_UPDATE_DOCUMENT, tokenCount, &dataIntegerHandler, &dataLength, &dataPosition);
    integerData = *(int *) dataIntegerHandler.data;
    CHECK_EQUAL_C_INT(1, (int) ret_val);
    CHECK_EQUAL_C_INT(24, integerData);
    CHECK_EQUAL_C_INT(2, dataLength);
    CHECK_EQUAL_C_INT(36, dataPosition);

    ret_val = update_value_if_key_match(TEST_JSON_RESPONSE_UPDATE_DOCUMENT, tokenCount, &dataDoubleHandler, &dataLength, &dataPosition);
    CHECK_EQUAL_C_INT(0, (int) ret_val);
}

TEST_C(CloudJsonBuilderTests, ParseClientToken) {
    bool ret_val;
    char clientToken[100];
    char expected[100];
    strcpy(expected, QCLOUD_IOT_MQTT_CLIENT_ID);
    strcat(expected, "-0");

    ret_val = parse_client_token(TEST_JSON_RESPONSE_UPDATE_DOCUMENT, 0, clientToken);
    CHECK_EQUAL_C_INT(1, (int) ret_val);
    CHECK_EQUAL_C_STRING(expected, clientToken);
}

TEST_C(CloudJsonBuilderTests, ParseVersionNum) {
    bool ret_val;
    uint32_t version;
    uint32_t expected = 1;

    ret_val = parse_version_num(TEST_JSON_RESPONSE_UPDATE_DOCUMENT, 0, &version);
    CHECK_EQUAL_C_INT(1, (int) ret_val);
    CHECK_EQUAL_C_INT(expected, version);
}

TEST_C(CloudJsonBuilderTests, ParseErrorCode) {
    bool ret_val;
    uint16_t errorCode;
    uint16_t expected = 200;

    ret_val = parse_error_code(TEST_JSON_RESPONSE_UPDATE_DOCUMENT, 0, &errorCode);
    CHECK_EQUAL_C_INT(1, (int) ret_val);
    CHECK_EQUAL_C_INT(expected, errorCode);
}

TEST_C(CloudJsonBuilderTests, ParseMessage) {
    bool ret_val;
    char message[100];
    char expected[100];
    strcpy(expected, "OK");

    ret_val = parse_error_message(TEST_JSON_RESPONSE_UPDATE_DOCUMENT, 0, message);
    CHECK_EQUAL_C_INT(1, (int) ret_val);
    CHECK_EQUAL_C_STRING(expected, message);
}
