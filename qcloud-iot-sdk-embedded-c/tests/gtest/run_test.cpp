#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>

#include "dev_iot_config.h"
#include "util.h"
#include "gtest/gtest.h"

extern "C"{
#include "qcloud_iot_export.h"
};
using namespace std;

#define MAX_LENGTH_OF_UPDATE_JSON_BUFFER 200
#define MAX_SIZE_OF_TOPIC_CONTENT 100
#define DEBUG "[Log]"
#define ROOM_TEMPERATURE 32.0f

static char g_rootCA[PATH_MAX + 1];
static char g_clientCert[PATH_MAX + 1];
static char g_clientKey[PATH_MAX + 1];
static char ca_file[PATH_MAX + 1];
static char cert_file[PATH_MAX + 1];
static char key_file[PATH_MAX + 1];
static bool s_isAirConditionerOpen = false;
static float s_energy_consumption = 0.0f;
static float s_desireTemperature = 20.0f;
static DeviceProperty s_energyConsumptionProp;
static DeviceProperty s_temperatureDesireProp;
static float s_reportTemperature = 88.0f;

char s_cJsonDocumentBuffer[MAX_LENGTH_OF_UPDATE_JSON_BUFFER];
size_t s_sizeOfJsonDocumentBuffer = sizeof(s_cJsonDocumentBuffer) / sizeof(s_cJsonDocumentBuffer[0]);

const char *g_pStatusStr;
const char *g_pMethodStr;

/**
 * 设置MQTT connet初始化参数
 *
 * @param pInitParams MQTT connet初始化参数
 */
static void _setup_connect_init_params(MQTTInitParams* pInitParams, char* tx_client_id, char* tx_device_password, char* tx_product_name,
                                        char* tx_device_name, char* tx_ca, char* tx_cert, char* tx_private, char* tx_psk, int tx_sym)
{
	pInitParams->client_id = tx_client_id;
	pInitParams->device_name = tx_device_name;
	pInitParams->product_name = tx_product_name;
	pInitParams->password = tx_device_password;

	pInitParams->is_asymc_encryption = tx_sym;
	char cert_dir[PATH_MAX + 1] = "certs";
    char cur_dir[PATH_MAX + 1];

    static char ca_file[PATH_MAX + 1];
	getcwd(cur_dir, sizeof(cur_dir));
	sprintf(ca_file, "%s/%s/%s", cur_dir, cert_dir, tx_ca);
    pInitParams->ca_file = ca_file;
	if (tx_sym == 1) {
	    printf("%s 非对称加密登录\n", DEBUG);
		static char cert_file[PATH_MAX + 1];
		static char key_file[PATH_MAX + 1];
		sprintf(cert_file, "%s/%s/%s", cur_dir, cert_dir, tx_cert);
		sprintf(key_file, "%s/%s/%s", cur_dir, cert_dir, tx_private);


		pInitParams->cert_file = cert_file;
		pInitParams->key_file = key_file;
	}
	else {
	    printf("%s 对称加密登录\n", DEBUG);
		pInitParams->psk = tx_psk;
	}

	pInitParams->command_timeout = QCLOUD_IOT_MQTT_COMMAND_TIMEOUT;
	pInitParams->connect_timeout = QCLOUD_IOT_TLS_HANDSHAKE_TIMEOUT;
	pInitParams->keep_alive_interval = QCLOUD_IOT_MQTT_KEEP_ALIVE_INTERNAL;

	pInitParams->is_asymc_encryption = tx_sym;
    pInitParams->on_disconnect_handler = NULL;
}

/**
 * 1. 设置客户端初始化参数
 * 2. 设置MQTT connet初始化参数
 * 3. 连接腾讯云
 */
static int tx_init_conn_iot(string connType, int testPort){
    MQTTInitParams initParams = DEFAULT_MQTTINIT_PARAMS;

    if(connType == "door"){
        _setup_connect_init_params(&initParams, DOOR_QCLOUD_IOT_MQTT_CLIENT_ID, DOOR_QCLOUD_IOT_MQTT_PASSWORD, DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, DOOR_QCLOUD_IOT_CA_FILENAME, DOOR_QCLOUD_IOT_CERT_FILENAME, DOOR_QCLOUD_IOT_KEY_FILENAME, DOOR_QCLOUD_IOT_PSK, testPort);
    }else{
        _setup_connect_init_params(&initParams, AIR_CONDITION_QCLOUD_IOT_MQTT_CLIENT_ID, AIR_CONDITION_QCLOUD_IOT_MQTT_PASSWORD, AIR_CONDITION_QCLOUD_IOT_MY_PRODUCT_NAME, AIR_CONDITION_QCLOUD_IOT_MY_DEVICE_NAME,AIR_CONDITION_QCLOUD_IOT_CA_FILENAME, AIR_CONDITION_QCLOUD_IOT_CERT_FILENAME, AIR_CONDITION_QCLOUD_IOT_KEY_FILENAME, AIR_CONDITION_QCLOUD_IOT_PSK, testPort);
    }

    void *client = IOT_MQTT_Construct(&initParams);
    if (client != NULL) {
        printf("%s Cloud Device Construct[Conn] Success\n", DEBUG);
        return QCLOUD_ERR_SUCCESS;
    } else {
        printf("%s Cloud Device Construct[Conn] Failed\n", DEBUG);
        return QCLOUD_ERR_FAILURE;
    }
}

/**
 * 发送topic消息
 *
 * @param action 行为
 */
 static int tx_publish_msg(void *client, char* action, char* targetDeviceName, char* productName, char* deviceName, char* topic, QoS tx_qos, string tx_pay_load)
{
    char topicName[1028] = {0};
    sprintf(topicName,"%s/%s/%s", productName, deviceName, topic);

    PublishParams pubParams = DefaultPubParams;
    pubParams.qos = tx_qos;

    char topicContent[1028] = {0};
    if(tx_pay_load == "big"){
        // payload 超过512
        sprintf(topicContent,"%s", "abc_abc__abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc_abc");
    }
    else if(tx_pay_load == "normal"){
        // 正常场景化测试
        sprintf(topicContent,"{\"action\": \"%s\", \"targetDevice\": \"%s\"}", action, targetDeviceName);
    }
    else
    {
        // payload 为空
        sprintf(topicContent,"%s", "");
    }

    pubParams.payload = topicContent;
    pubParams.payload_len = strlen(topicContent);

    printf("%s 发送Topic: %s\n", DEBUG, topicName);
    printf("%s 发送内容 : %s\n", DEBUG, topicContent);
    int rc = IOT_MQTT_Publish(client, topicName, &pubParams);
    return rc;
}

/**
 * MQTT 消息接收处理函数 提供给订阅函数使用，本文档使用函数：_register_subscribe_topics
 *
 * @param topicName         topic主题
 * @param topicNameLen      topic长度
 * @param message           已订阅消息的结构
 * @param pUserdata         消息负载
 */
static void on_message_callback(char *topicName, uint16_t topicNameLen, MQTTMessage *message, void *pUserData) {
    int32_t tokenCount = 0;
    char fieldValue[100];

    if (topicName == NULL || topicNameLen == 0 || message == NULL || message->payload_len > CLOUD_RX_BUF_LEN) {
        return;
    }

    Log_i("Receive Message With topicName:%.*s, payload:%.*s",
          (int) topicNameLen, topicName, (int) message->payload_len,  message->payload);

    static char cloud_rcv_buf[CLOUD_RX_BUF_LEN];
    memcpy(cloud_rcv_buf, message->payload, message->payload_len);
    cloud_rcv_buf[message->payload_len] = '\0';    // jsmn_parse relies on a string

    bool bGetAction = IOT_MQTT_JSON_GetAction(cloud_rcv_buf, tokenCount, fieldValue);
    if(bGetAction)
    {
        if(strcmp(fieldValue, "come_home") == 0)
        {
            s_isAirConditionerOpen = true;
        }
        else if(strcmp(fieldValue, "leave_home") == 0)
        {
            s_isAirConditionerOpen = false;
        }
    }
    else {
    	Log_w("get action from json failed.");
    }
}

/**
 * 订阅topic消息
 *
 */
static int _register_subscribe_topics(void *client, char* productName, char* deviceName, char* topic)
{
    static char topicName[MAX_SIZE_OF_CLOUD_TOPIC] = {0};
    sprintf(topicName,"%s/%s/%s", productName, deviceName, topic);
    printf("%s 订阅Topic: %s\n", DEBUG, topicName);
    SubscribeParams subParams = DEFAULT_SUB_PARAMS;
    subParams.on_message_handler = on_message_callback;
    return IOT_MQTT_Subscribe(client, topicName, &subParams);
}

/**
 * 比较浮点数是否相等
 *
 * @param left 左值
 * @param right 左值
 */
bool _is_value_equal(float left, float right)
{
    if((left<right+0.01) && (left>right-0.01))
        return true;
    else
        return false;
}

/**
 * 模拟室内温度
 *
 * @param pRoomTemperature
 */
static void _simulate_room_temperature(float *pRoomTemperature) {
    float deltaChange = 0;

    if (!s_isAirConditionerOpen) {
        if(!_is_value_equal(*pRoomTemperature, ROOM_TEMPERATURE)) {
            deltaChange = (*pRoomTemperature)>ROOM_TEMPERATURE ? -0.5: 0.5;
        }
    } else {
        s_energy_consumption += 1;
        if (!_is_value_equal(*pRoomTemperature, s_desireTemperature)) {
            deltaChange = (*pRoomTemperature)>s_desireTemperature ? -1.0: 1.0;
        }
    }

    *pRoomTemperature += deltaChange;
}

/**
 * 文档操作请求返回回调函数
 *
 * @param pProductName   产品名
 * @param pDeviceName   设备名
 * @param method        文档操作方式
 * @param status        请求响应返回的类型
 * @param pJsonDoc      json字符串
 * @param pUserdata     数据负载
 */
static void on_request_handler(Method method, RequestAck status, const char *pJsonDoc, void *pUserdata) {

    if (status == ACK_TIMEOUT) {
        g_pStatusStr = "ACK_TIMEOUT";
    } else if (status == ACK_ACCEPTED) {
        g_pStatusStr = "ACK_ACCEPTED";
    } else if (status == ACK_REJECTED) {
        g_pStatusStr = "ACK_REJECTED";
    }

    if (method == GET) {
        g_pMethodStr = "GET";
    } else if (method == UPDATE) {
        g_pMethodStr = "UPDATE";
    } else if (method == DELETE) {
        g_pMethodStr = "DELETE";
    }

    Log_i("Method=%s|Ack=%s", g_pMethodStr, g_pStatusStr);
    Log_i("received jsonString=%s", pJsonDoc);
}

/**
 * report energy consumption
 */
static int _do_report_energy_consumption(void *client, char *pJsonBuffer, size_t sizeOfBuffer) {
    int rc = IOT_Shadow_JSON_ConstructReport(pJsonBuffer, sizeOfBuffer, 1, &s_energyConsumptionProp);

    if (rc != QCLOUD_ERR_SUCCESS) {
    	Log_e("shadow construct report failed: %d", rc);
        return rc;
    }

    Log_i("Update Shadow: %s", pJsonBuffer);
    rc = IOT_Shadow_Update(client, pJsonBuffer, on_request_handler, NULL, 10);

    if (rc != QCLOUD_ERR_SUCCESS) {
        Log_i("Update Shadow Failed: %d", rc);
        return rc;
    } else {
        Log_i("Update Shadow Success");
    }

    return rc;
}

/**
 * delta消息回调处理函数
 *
 * @param pJsonString           返回的JSON数据字符串
 * @param JsonStringDataLen     JSON数据长度
 * @param pContext              注册的属性
 */
static void on_temperature_actuate_callback(const char *pJsonString, uint32_t JsonStringDataLen, DeviceProperty *pContext) {
    Log_i("jsonString=%s|dataLen=%u", pJsonString, JsonStringDataLen);

//    if (pContext != NULL) {
//        Log_i("modify desire temperature to: %f", *(float *) pContext->data);
//
//        int rc = IOT_Shadow_JSON_ConstructReport(s_cJsonDocumentBuffer, s_sizeOfJsonDocumentBuffer, 1, &s_temperatureDesireProp);
//
//		if (rc != QCLOUD_ERR_SUCCESS) {
//			return;
//		}
//
//		Log_i("update desire temperature: %s", s_cJsonDocumentBuffer);
//		rc = IOT_Shadow_Update(client, s_cJsonDocumentBuffer, on_request_handler, NULL, 10, false);
//    }
}

/**
 * 设置MQTT connet初始化参数
 *
 * @param pInitParams MQTT connet初始化参数
 */
static void _setup_shadow_connect_init_params(ShadowInitParams* pInitParams)
{
	pInitParams->mqtt.client_id = AIR_CONDITION_QCLOUD_IOT_MQTT_CLIENT_ID;
	pInitParams->mqtt.device_name = AIR_CONDITION_QCLOUD_IOT_MY_DEVICE_NAME;
	pInitParams->mqtt.product_name = AIR_CONDITION_QCLOUD_IOT_MY_PRODUCT_NAME;
	pInitParams->mqtt.password = AIR_CONDITION_QCLOUD_IOT_MQTT_PASSWORD;

	char certDirectory[PATH_MAX + 1] = "certs";
	char currentWD[PATH_MAX + 1];

	getcwd(currentWD, sizeof(currentWD));
	sprintf(g_rootCA, "%s/%s/%s", currentWD, certDirectory, AIR_CONDITION_QCLOUD_IOT_CA_FILENAME);
	pInitParams->mqtt.ca_file = g_rootCA;

    // 确定加密方式
	pInitParams->mqtt.is_asymc_encryption = AIR_CONDITION_QCLOUD_IOT_IS_ASYMC_ENCRYPTION;
	if (pInitParams->mqtt.is_asymc_encryption) {
		// 获取CA证书、客户端证书以及私钥文件的路径
		sprintf(g_clientCert, "%s/%s/%s", currentWD, certDirectory, AIR_CONDITION_QCLOUD_IOT_CERT_FILENAME);
		sprintf(g_clientKey, "%s/%s/%s", currentWD, certDirectory, AIR_CONDITION_QCLOUD_IOT_KEY_FILENAME);

		pInitParams->mqtt.cert_file = g_clientCert;
		pInitParams->mqtt.key_file = g_clientKey;
	} else {
		// 设置对称密钥
		pInitParams->mqtt.psk = AIR_CONDITION_QCLOUD_IOT_PSK;
	}

    pInitParams->mqtt.on_disconnect_handler = NULL;
}

/**
 * 设置shadow相关属性参数
 *
 */
static void _setup_shadow_data()
{
    //s_temperatureReportProp: device ---> shadow <---> app
    s_energyConsumptionProp.key = "energyConsumption";
    s_energyConsumptionProp.data = &s_energy_consumption;
    s_energyConsumptionProp.type = JFLOAT;
    //s_temperatureDesireProp: app ---> shadow ---> device
    s_temperatureDesireProp.key = "temperatureDesire";
    s_temperatureDesireProp.data = &s_desireTemperature;
    s_temperatureDesireProp.type = JFLOAT;
}

/**
 * 注册shadow配置型属性
 *
 */
static int _register_config_shadow_property(void *client)
{
    return IOT_Shadow_Register_Property(client, &s_temperatureDesireProp, on_temperature_actuate_callback);
}

/**
 * 订阅关注topic和注册相应回调处理
 *
 */
static int _register_subscribe_topics(void *client)
{
    static char topicName[MAX_SIZE_OF_CLOUD_TOPIC] = {0};
    sprintf(topicName,"%s/%s/%s", AIR_CONDITION_QCLOUD_IOT_MY_PRODUCT_NAME, AIR_CONDITION_QCLOUD_IOT_MY_DEVICE_NAME, "get");
    SubscribeParams subParams = DEFAULT_SUB_PARAMS;
    subParams.on_message_handler = on_message_callback;
    return IOT_MQTT_Subscribe(client, topicName, &subParams);
}

/**
 *  *********************************************************
 *  所有测试用例集合
 *  *********************************************************
 */
 // 对接加密以及非对称加密进行conn
 TEST(SDKTest, Test_Asy_Pass_Connect)
{
    cout << log_time() << " [Log] [非对称加密-登陆]连接腾讯云\n" << endl;
    int rc = tx_init_conn_iot("door", 1);
    ASSERT_EQ(rc,QCLOUD_ERR_SUCCESS);
}

TEST(SDKTest, Test_Sym_Pass_Connect)
{
    cout << log_time() << " [Log] [对称加密-登录] [正常测试] 连接腾讯云\n" << endl;
    int rc = tx_init_conn_iot("door", 0);
    ASSERT_EQ(rc,QCLOUD_ERR_SUCCESS);
}

// 对称加密登录 -> 错误参数：1.Client ID 2.PSK
TEST(SDKTest, Test_Sym_Fail_ErrorClientID)
{
    cout << log_time() << " [Log] [对称加密] [异常测试] [错误ClientID]连接腾讯云\n" << endl;
    MQTTInitParams initParams = DEFAULT_MQTTINIT_PARAMS;

    _setup_connect_init_params(&initParams, "123456", DOOR_QCLOUD_IOT_MQTT_PASSWORD, DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, DOOR_QCLOUD_IOT_CA_FILENAME, DOOR_QCLOUD_IOT_CERT_FILENAME, DOOR_QCLOUD_IOT_KEY_FILENAME, DOOR_QCLOUD_IOT_PSK, 0);

    void *client = IOT_MQTT_Construct(&initParams);
    int result = 0;
    if (client != NULL) {
        printf("%s Cloud Device Construct[Conn] Success\n", DEBUG);
    } else {
        printf("%s Cloud Device Construct[Conn] Failed\n", DEBUG);
        result += 1;
    }
    ASSERT_NE(result,0);
}

TEST(SDKTest, Test_Sym_Fail_ErrorPSK)
{
    cout << log_time() << " [Log] [对称加密] [异常测试] [错误PSK]连接腾讯云\n" << endl;
    MQTTInitParams initParams = DEFAULT_MQTTINIT_PARAMS;

    _setup_connect_init_params(&initParams, DOOR_QCLOUD_IOT_MQTT_CLIENT_ID, DOOR_QCLOUD_IOT_MQTT_PASSWORD, DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, DOOR_QCLOUD_IOT_CA_FILENAME, DOOR_QCLOUD_IOT_CERT_FILENAME, DOOR_QCLOUD_IOT_KEY_FILENAME, "123456", 0);

    void *client = IOT_MQTT_Construct(&initParams);
    int result = 0;
    if (client != NULL) {
        printf("%s Cloud Device Construct[Conn] Success\n", DEBUG);
    } else {
        printf("%s Cloud Device Construct[Conn] Failed\n", DEBUG);
        result += 1;
    }
    ASSERT_NE(result,0);
}

// 非对称加密登录 -> 错误参数：1.根证书 2.设备证书 3.设备私钥
TEST(SDKTest, Test_Asy_Fail_ErrorCA)
{
    cout << log_time() << " [Log] [对称加密] [异常测试] [错误CA]连接腾讯云\n" << endl;
    MQTTInitParams initParams = DEFAULT_MQTTINIT_PARAMS;

    _setup_connect_init_params(&initParams, DOOR_QCLOUD_IOT_MQTT_CLIENT_ID, DOOR_QCLOUD_IOT_MQTT_PASSWORD, DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, "test", DOOR_QCLOUD_IOT_CERT_FILENAME, DOOR_QCLOUD_IOT_KEY_FILENAME, DOOR_QCLOUD_IOT_PSK, 1);

    void *client = IOT_MQTT_Construct(&initParams);
    int result = 0;
    if (client != NULL) {
        printf("%s Cloud Device Construct[Conn] Success\n", DEBUG);
    } else {
        printf("%s Cloud Device Construct[Conn] Failed\n", DEBUG);
        result += 1;
    }
    ASSERT_NE(result,0);
}

TEST(SDKTest, Test_Asy_Fail_ErrorCERT)
{
    cout << log_time() << " [Log] [对称加密] [异常测试] [错误CERT]连接腾讯云\n" << endl;
    MQTTInitParams initParams = DEFAULT_MQTTINIT_PARAMS;

    _setup_connect_init_params(&initParams, DOOR_QCLOUD_IOT_MQTT_CLIENT_ID, DOOR_QCLOUD_IOT_MQTT_PASSWORD, DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, DOOR_QCLOUD_IOT_CA_FILENAME, "test", DOOR_QCLOUD_IOT_KEY_FILENAME, DOOR_QCLOUD_IOT_PSK, 1);

    void *client = IOT_MQTT_Construct(&initParams);
    int result = 0;
    if (client != NULL) {
        printf("%s Cloud Device Construct[Conn] Success\n", DEBUG);
    } else {
        printf("%s Cloud Device Construct[Conn] Failed\n", DEBUG);
        result += 1;
    }
    ASSERT_NE(result,0);
}

TEST(SDKTest, Test_Asy_Fail_ErrorKEY)
{
    cout << log_time() << " [Log] [对称加密] [异常测试] [错误KEY]连接腾讯云\n" << endl;
    MQTTInitParams initParams = DEFAULT_MQTTINIT_PARAMS;

    _setup_connect_init_params(&initParams, DOOR_QCLOUD_IOT_MQTT_CLIENT_ID, DOOR_QCLOUD_IOT_MQTT_PASSWORD, DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, DOOR_QCLOUD_IOT_CA_FILENAME, DOOR_QCLOUD_IOT_CERT_FILENAME, "test", DOOR_QCLOUD_IOT_PSK, 1);

    void *client = IOT_MQTT_Construct(&initParams);
    int result = 0;
    if (client != NULL) {
        printf("%s Cloud Device Construct[Conn] Success\n", DEBUG);
    } else {
        printf("%s Cloud Device Construct[Conn] Failed\n", DEBUG);
        result += 1;
    }
    ASSERT_NE(result,0);
}

TEST(SDKTest, Test_Sym_Pass_SendTopicMessage)
{
    cout << log_time() << " [Log] [对称加密] [正常测试] [具有发布权限] 进行发布Topic消息\n" << endl;
    MQTTInitParams initParams = DEFAULT_MQTTINIT_PARAMS;

    _setup_connect_init_params(&initParams, DOOR_QCLOUD_IOT_MQTT_CLIENT_ID, DOOR_QCLOUD_IOT_MQTT_PASSWORD, DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, DOOR_QCLOUD_IOT_CA_FILENAME, DOOR_QCLOUD_IOT_CERT_FILENAME, DOOR_QCLOUD_IOT_KEY_FILENAME, DOOR_QCLOUD_IOT_PSK, 1);

    void *client = IOT_MQTT_Construct(&initParams);
    if (client != NULL) {
        printf("%s Cloud Device Construct[Conn] Success\n", DEBUG);
    } else {
        printf("%s Cloud Device Construct[Conn] Failed\n", DEBUG);
    }

    int rc;
    rc = tx_publish_msg(client, "come_home", "AirCondition", DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, "update", QOS1, "normal");
    if (rc == QCLOUD_ERR_SUCCESS) {
        printf("%s Publish Success. And Test Success\n", DEBUG);
    } else {
        printf("%s Publish Failed. But Test Failed :%d\n", DEBUG, rc);
    }
    ASSERT_EQ(rc,QCLOUD_ERR_SUCCESS);
}

TEST(SDKTest, Test_Sym_Pass_SendBigTopicMessage)
{
    cout << log_time() << " [Log] [对称加密] [正常测试] [具有发布权限] 进行发布Topic消息,Payload=长度超过512\n" << endl;
    MQTTInitParams initParams = DEFAULT_MQTTINIT_PARAMS;

    _setup_connect_init_params(&initParams, DOOR_QCLOUD_IOT_MQTT_CLIENT_ID, DOOR_QCLOUD_IOT_MQTT_PASSWORD, DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, DOOR_QCLOUD_IOT_CA_FILENAME, DOOR_QCLOUD_IOT_CERT_FILENAME, DOOR_QCLOUD_IOT_KEY_FILENAME, DOOR_QCLOUD_IOT_PSK, 1);

    void *client = IOT_MQTT_Construct(&initParams);
    if (client != NULL) {
        printf("%s Cloud Device Construct[Conn] Success\n", DEBUG);
    } else {
        printf("%s Cloud Device Construct[Conn] Failed\n", DEBUG);
    }

    int rc;
    rc = tx_publish_msg(client, "come_home", "AirCondition", DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, "update", QOS1, "big");
    if (rc == QCLOUD_ERR_SUCCESS) {
        printf("%s Publish Success. And Test Failed\n", DEBUG);
    } else {
        printf("%s Publish Failed. But Test Success :%d\n", DEBUG, rc);
    }
    ASSERT_EQ(rc,QCLOUD_ERR_BUF_TOO_SHORT);
}

TEST(SDKTest, Test_Sym_Pass_SendSmallTopicMessage)
{
    cout << log_time() << " [Log] [对称加密] [正常测试] [具有发布权限] 进行发布Topic消息, Payload=长度为0\n" << endl;
    MQTTInitParams initParams = DEFAULT_MQTTINIT_PARAMS;

    _setup_connect_init_params(&initParams, DOOR_QCLOUD_IOT_MQTT_CLIENT_ID, DOOR_QCLOUD_IOT_MQTT_PASSWORD, DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, DOOR_QCLOUD_IOT_CA_FILENAME, DOOR_QCLOUD_IOT_CERT_FILENAME, DOOR_QCLOUD_IOT_KEY_FILENAME, DOOR_QCLOUD_IOT_PSK, 1);

    void *client = IOT_MQTT_Construct(&initParams);
    if (client != NULL) {
        printf("%s Cloud Device Construct[Conn] Success\n", DEBUG);
    } else {
        printf("%s Cloud Device Construct[Conn] Failed\n", DEBUG);
    }

    int rc;
    rc = tx_publish_msg(client, "come_home", "AirCondition", DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, "update", QOS1, "small");
    if (rc == QCLOUD_ERR_SUCCESS) {
        printf("%s Publish Success. And Test Success\n", DEBUG);
    } else {
        printf("%s Publish Failed. But Test Failed :%d\n", DEBUG, rc);
    }
    ASSERT_EQ(rc,QCLOUD_ERR_SUCCESS);
}

TEST(SDKTest, Test_Sym_Fail_SendNoPolicyTopicMessage)
{
    cout << log_time() << " [Log] [对称加密] [异常测试] [不具有发布权限] 进行发布Topic消息\n" << endl;
    MQTTInitParams initParams = DEFAULT_MQTTINIT_PARAMS;

    _setup_connect_init_params(&initParams, DOOR_QCLOUD_IOT_MQTT_CLIENT_ID, DOOR_QCLOUD_IOT_MQTT_PASSWORD, DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, DOOR_QCLOUD_IOT_CA_FILENAME, DOOR_QCLOUD_IOT_CERT_FILENAME, DOOR_QCLOUD_IOT_KEY_FILENAME, DOOR_QCLOUD_IOT_PSK, 1);

    void *client = IOT_MQTT_Construct(&initParams);
    if (client != NULL) {
        printf("%s Cloud Device Construct[Conn] Success\n", DEBUG);
    } else {
        printf("%s Cloud Device Construct[Conn] Failed\n", DEBUG);
    }

    int rc;
    rc = tx_publish_msg(client, "come_home", "AirCondition", DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, "get", QOS1, "normal");
    if (rc != QCLOUD_ERR_SUCCESS) {
        printf("%s Publish Failed. And Test Success. Return Code:%d\n", DEBUG, rc);
    } else {
        printf("%s Publish Success. But Test Failed:%d\n", DEBUG, rc);
    }
    ASSERT_NE(rc,QCLOUD_ERR_SUCCESS);
}

TEST(SDKTest, Test_Sym_Pass_SendQOSZeroTopicMessage)
{
    cout << log_time() << " [Log] [对称加密] [正常测试] [具有发布权限] [QOS0] 进行发布Topic消息\n" << endl;
    MQTTInitParams initParams = DEFAULT_MQTTINIT_PARAMS;

    _setup_connect_init_params(&initParams, DOOR_QCLOUD_IOT_MQTT_CLIENT_ID, DOOR_QCLOUD_IOT_MQTT_PASSWORD, DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, DOOR_QCLOUD_IOT_CA_FILENAME, DOOR_QCLOUD_IOT_CERT_FILENAME, DOOR_QCLOUD_IOT_KEY_FILENAME, DOOR_QCLOUD_IOT_PSK, 1);

    void *client = IOT_MQTT_Construct(&initParams);
    if (client != NULL) {
        printf("%s Cloud Device Construct[Conn] Success\n", DEBUG);
    } else {
        printf("%s Cloud Device Construct[Conn] Failed\n", DEBUG);
    }

    int rc;
    rc = tx_publish_msg(client, "come_home", "AirCondition", DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, "update", QOS0, "normal");
    if (rc == QCLOUD_ERR_SUCCESS) {
        printf("%s Publish Success. And Test Success\n", DEBUG);
    } else {
        printf("%s Publish Failed. But Test Failed :%d\n", DEBUG, rc);
    }
    ASSERT_EQ(rc,QCLOUD_ERR_SUCCESS);
}

TEST(SDKTest, Test_Sym_Pass_SendQOSTwoTopicMessage)
{
    cout << log_time() << " [Log] [对称加密] [正常测试] [具有发布权限] [QOS2] 进行发布Topic消息\n" << endl;
    MQTTInitParams initParams = DEFAULT_MQTTINIT_PARAMS;

    _setup_connect_init_params(&initParams, DOOR_QCLOUD_IOT_MQTT_CLIENT_ID, DOOR_QCLOUD_IOT_MQTT_PASSWORD, DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, DOOR_QCLOUD_IOT_CA_FILENAME, DOOR_QCLOUD_IOT_CERT_FILENAME, DOOR_QCLOUD_IOT_KEY_FILENAME, DOOR_QCLOUD_IOT_PSK, 1);

    void *client = IOT_MQTT_Construct(&initParams);
    if (client != NULL) {
        printf("%s Cloud Device Construct[Conn] Success\n", DEBUG);
    } else {
        printf("%s Cloud Device Construct[Conn] Failed\n", DEBUG);
    }

    int rc;
    rc = tx_publish_msg(client, "come_home", "AirCondition", DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, "update", QOS2, "normal");
    if (rc == QCLOUD_ERR_SUCCESS) {
        printf("%s Publish Success. And Test Success\n", DEBUG);
    } else {
        printf("%s Publish Failed. But Test Failed :%d\n", DEBUG, rc);
    }
    ASSERT_EQ(rc,QCLOUD_ERR_MQTT_REQUEST_TIMEOUT);
}

TEST(SDKTest, Test_Sym_Pass_PolicyAllSendTopicMessage)
{
    cout << log_time() << " [Log] [对称加密] [正常测试] [具有发布和订阅权限] 进行发布Topic消息\n" << endl;
    MQTTInitParams initParams = DEFAULT_MQTTINIT_PARAMS;

    _setup_connect_init_params(&initParams, DOOR_QCLOUD_IOT_MQTT_CLIENT_ID, DOOR_QCLOUD_IOT_MQTT_PASSWORD, DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, DOOR_QCLOUD_IOT_CA_FILENAME, DOOR_QCLOUD_IOT_CERT_FILENAME, DOOR_QCLOUD_IOT_KEY_FILENAME, DOOR_QCLOUD_IOT_PSK, 1);

    void *client = IOT_MQTT_Construct(&initParams);
    if (client != NULL) {
        printf("%s Cloud Device Construct[Conn] Success\n", DEBUG);
    } else {
        printf("%s Cloud Device Construct[Conn] Failed\n", DEBUG);
    }

    int rc;
    rc = tx_publish_msg(client, "come_home", "AirCondition", DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, "update/error", QOS1, "normal");
    if (rc == QCLOUD_ERR_SUCCESS) {
        printf("%s Publish Success. And Test Success\n", DEBUG);
    } else {
        printf("%s Publish Failed. But Test Failed :%d\n", DEBUG, rc);
    }
    ASSERT_EQ(rc,QCLOUD_ERR_SUCCESS);
}

TEST(SDKTest, Test_Sym_Pass_Subscribe)
{
    cout << log_time() << " [Log] [对称加密] [正常测试] [具有订阅权限] 订阅Topic消息\n" << endl;
    MQTTInitParams initParams = DEFAULT_MQTTINIT_PARAMS;

    _setup_connect_init_params(&initParams, DOOR_QCLOUD_IOT_MQTT_CLIENT_ID, DOOR_QCLOUD_IOT_MQTT_PASSWORD, DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, DOOR_QCLOUD_IOT_CA_FILENAME, DOOR_QCLOUD_IOT_CERT_FILENAME, DOOR_QCLOUD_IOT_KEY_FILENAME, DOOR_QCLOUD_IOT_PSK, 1);

    void *client = IOT_MQTT_Construct(&initParams);
    if (client != NULL) {
        printf("%s Cloud Device Construct[Conn] Success\n", DEBUG);
    } else {
        printf("%s Cloud Device Construct[Conn] Failed\n", DEBUG);
    }

    int rc;
    rc = _register_subscribe_topics(client, DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, "get");
    if (rc == QCLOUD_ERR_SUCCESS) {
        printf("%s Subscribe Success. And Test Success\n", DEBUG);
    } else {
        printf("%s Subscribe Failed. And Test Failed:%d\n", DEBUG, rc);
    }
    ASSERT_EQ(rc,QCLOUD_ERR_SUCCESS);
}

TEST(SDKTest, Test_Sym_Fail_NoPolicySubscribe)
{
    cout << log_time() << " [Log] [对称加密] [异常测试] [不具有订阅权限] 订阅Topic消息\n" << endl;
    MQTTInitParams initParams = DEFAULT_MQTTINIT_PARAMS;

    _setup_connect_init_params(&initParams, DOOR_QCLOUD_IOT_MQTT_CLIENT_ID, DOOR_QCLOUD_IOT_MQTT_PASSWORD, DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, DOOR_QCLOUD_IOT_CA_FILENAME, DOOR_QCLOUD_IOT_CERT_FILENAME, DOOR_QCLOUD_IOT_KEY_FILENAME, DOOR_QCLOUD_IOT_PSK, 1);

    void *client = IOT_MQTT_Construct(&initParams);
    if (client != NULL) {
        printf("%s Cloud Device Construct[Conn] Success\n", DEBUG);
    } else {
        printf("%s Cloud Device Construct[Conn] Failed\n", DEBUG);
    }
    int rc;
    rc = _register_subscribe_topics(client, DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, "update");
    if (rc != QCLOUD_ERR_SUCCESS) {
        printf("%s Subscribe Success. And Test Success\n", DEBUG);
    } else {
        printf("%s Subscribe Failed. And Test Failed:%d\n", DEBUG, rc);
    }
    ASSERT_NE(rc,QCLOUD_ERR_SUCCESS);
}

TEST(SDKTest, Test_Sym_Pass_PolicyAllSubscribe)
{
    cout << log_time() << " [Log] [对称加密] [正常测试] [具有发布和订阅权限] 订阅Topic消息\n" << endl;
    MQTTInitParams initParams = DEFAULT_MQTTINIT_PARAMS;

    _setup_connect_init_params(&initParams, DOOR_QCLOUD_IOT_MQTT_CLIENT_ID, DOOR_QCLOUD_IOT_MQTT_PASSWORD, DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, DOOR_QCLOUD_IOT_CA_FILENAME, DOOR_QCLOUD_IOT_CERT_FILENAME, DOOR_QCLOUD_IOT_KEY_FILENAME, DOOR_QCLOUD_IOT_PSK, 1);

    void *client = IOT_MQTT_Construct(&initParams);
    if (client != NULL) {
        printf("%s Cloud Device Construct[Conn] Success\n", DEBUG);
    } else {
        printf("%s Cloud Device Construct[Conn] Failed\n", DEBUG);
    }
    int rc;
    rc = _register_subscribe_topics(client, DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, "update/error");
    if (rc == QCLOUD_ERR_SUCCESS) {
        printf("%s Subscribe Success. And Test Success\n", DEBUG);
    } else {
        printf("%s Subscribe Failed. And Test Failed:%d\n", DEBUG, rc);
    }
    ASSERT_EQ(rc,QCLOUD_ERR_SUCCESS);
}

TEST(SDKTest, Test_Sym_Pass_ShadowUpdate)
{
    cout << log_time() << " [Log] [对称加密] [正常测试] 更新设备影子\n" << endl;
    ShadowInitParams initParams = DEFAULT_SHAWDOW_INIT_PARAMS;
    _setup_shadow_connect_init_params(&initParams);

    void *client = IOT_Shadow_Construct(&initParams);
    if (client != NULL) {
        Log_i("Cloud Device Construct Success");
    } else {
        Log_e("Cloud Device Construct Failed");
    }

    //init shadow data
    _setup_shadow_data();

    //register config shadow propertys here
    int rc;
    rc = IOT_Shadow_Yield(client, 200);

    _simulate_room_temperature(&s_reportTemperature);
    printf("\n");
    Log_i("airConditioner state: %s", s_isAirConditionerOpen ? "open" : "close");
    Log_i("currentTemperature: %f, energyConsumption: %f", s_reportTemperature, s_energy_consumption);

    _do_report_energy_consumption(client, s_cJsonDocumentBuffer, s_sizeOfJsonDocumentBuffer);

    sleep(1);
    ASSERT_EQ(rc,QCLOUD_ERR_SUCCESS);
}

TEST(SDKTest, Test_Sym_Pass_ShadowGet)
{
    cout << log_time() << " [Log] [对称加密] [正常测试] 获取设备影子\n" << endl;
    ShadowInitParams initParams = DEFAULT_SHAWDOW_INIT_PARAMS;
    _setup_shadow_connect_init_params(&initParams);

    void *client = IOT_Shadow_Construct(&initParams);
    if (client != NULL) {
        Log_i("Cloud Device Construct Success");
    } else {
        Log_e("Cloud Device Construct Failed");
    }

    //init shadow data
    _setup_shadow_data();

    //register config shadow propertys here
    int rc;
    rc = _register_config_shadow_property(client);
    if (rc == QCLOUD_ERR_SUCCESS) {
        Log_i("Cloud Device Register Delta Success");
    } else {
        Log_e("Cloud Device Register Delta Failed: %d", rc);
    }

    //register subscribe topics here
    rc = _register_subscribe_topics(client);
    if (rc != QCLOUD_ERR_SUCCESS) {
        Log_e("Client Subscribe Topic Failed: %d", rc);
    } else {
        Log_i("Client Subscribe Topic Success");
    }
    ASSERT_EQ(rc,QCLOUD_ERR_SUCCESS);
}

int main(int argc, char* argv[])
{
    testing::GTEST_FLAG(output) = "xml:";
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
