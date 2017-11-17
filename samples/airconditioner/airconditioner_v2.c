
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>

#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"
#include "iot_config.h"

#define MAX_LENGTH_OF_UPDATE_JSON_BUFFER 200

#define ROOM_TEMPERATURE 32.0f
static float s_desireTemperature = 20.0f;
static float s_reportTemperature = ROOM_TEMPERATURE;

static float s_energy_consumption = 0.0f;

static bool s_isAirConditionerOpen = false;
static DeviceProperty s_energyConsumptionProp;
static DeviceProperty s_temperatureDesireProp;

static bool sg_messageArrivedOnDelta = false;

static char g_rootCA[PATH_MAX + 1];
static char g_clientCert[PATH_MAX + 1];
static char g_clientKey[PATH_MAX + 1];

char *g_pStatusStr;
char *g_pMethodStr;

char s_cJsonDocumentBuffer[MAX_LENGTH_OF_UPDATE_JSON_BUFFER];
size_t s_sizeOfJsonDocumentBuffer = sizeof(s_cJsonDocumentBuffer) / sizeof(s_cJsonDocumentBuffer[0]);

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
 * MQTT消息接收处理函数
 *
 * @param topicName         topic主题
 * @param topicNameLen      topic长度
 * @param message           已订阅消息的结构
 * @param pUserData         消息负载
 */
static void on_message_callback(char *topicName, uint16_t topicNameLen, MQTTMessage *message, void *pUserData) {
    int32_t tokenCount = 0;
    char fieldValue[100];

    if (topicName == NULL || topicNameLen == 0 || message == NULL || message->payload_len > CLOUD_RX_BUF_LEN) {
        return;
    }

    Log_i("Receive Message With topicName:%.*s, payload:%.*s",
          (int) topicNameLen, topicName, (int) message->payload_len, (char *) message->payload);

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

static int _do_report_temperature_desire(void *client, char *pJsonBuffer, size_t sizeOfBuffer) {
	int rc = IOT_Shadow_JSON_ConstructReport(s_cJsonDocumentBuffer, s_sizeOfJsonDocumentBuffer, 1, &s_temperatureDesireProp);

	if (rc != QCLOUD_ERR_SUCCESS) {
		Log_e("shadow construct report failed: %d", rc);
		return rc;
	}

	Log_i("update desire temperature: %s", s_cJsonDocumentBuffer);
	rc = IOT_Shadow_Update(client, s_cJsonDocumentBuffer, on_request_handler, NULL, 10);

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

    if (pContext != NULL) {
        Log_i("modify desire temperature to: %f", *(float *) pContext->data);
        sg_messageArrivedOnDelta = true;
    }
}

/**
 * 设置MQTT connet初始化参数
 *
 * @param pInitParams MQTT connet初始化参数
 */
static void _setup_connect_init_params(ShadowInitParams* pInitParams)
{
	pInitParams->mqtt.client_id = QCLOUD_IOT_MQTT_CLIENT_ID;
	pInitParams->mqtt.device_name = QCLOUD_IOT_MY_DEVICE_NAME;
	pInitParams->mqtt.product_name = QCLOUD_IOT_MY_PRODUCT_NAME;
	pInitParams->mqtt.password = QCLOUD_IOT_MQTT_PASSWORD;

    // 获取CA证书、客户端证书以及私钥文件的路径
    char certDirectory[PATH_MAX + 1] = "../../certs";
    char currentWD[PATH_MAX + 1];
    getcwd(currentWD, sizeof(currentWD));
    sprintf(g_rootCA, "%s/%s/%s", currentWD, certDirectory, QCLOUD_IOT_CA_FILENAME);
    pInitParams->mqtt.ca_file = g_rootCA;


    // 确定加密方式
	pInitParams->mqtt.is_asymc_encryption = QCLOUD_IOT_IS_ASYMC_ENCRYPTION;
	if (pInitParams->mqtt.is_asymc_encryption) {		
		sprintf(g_clientCert, "%s/%s/%s", currentWD, certDirectory, QCLOUD_IOT_CERT_FILENAME);
		sprintf(g_clientKey, "%s/%s/%s", currentWD, certDirectory, QCLOUD_IOT_KEY_FILENAME);
		pInitParams->mqtt.cert_file = g_clientCert;
		pInitParams->mqtt.key_file = g_clientKey;
	} else {
		// 设置对称密钥
		pInitParams->mqtt.psk = QCLOUD_IOT_PSK;
	}

	pInitParams->mqtt.auto_connect_enable = 1;
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

    int size = HAL_Snprintf(topicName, sizeof(topicName), "%s/%s/%s", QCLOUD_IOT_MY_PRODUCT_NAME, QCLOUD_IOT_MY_DEVICE_NAME, "get");
    if (size < 0 || size > sizeof(topicName) - 1)
    {
        Log_e("topic content length not enough! content size:%d  buf size:%d", size, (int)sizeof(topicName));
        return QCLOUD_ERR_FAILURE;
    }
    Log_d("topicName:%s", topicName);

    SubscribeParams subParams = DEFAULT_SUB_PARAMS;
    subParams.on_message_handler = on_message_callback;
    return IOT_MQTT_Subscribe(client, topicName, &subParams);
}

int main(int argc, char **argv) {
    int rc;

    //init log level
    IOT_Log_Set_Level(DEBUG);

    //init connection
    ShadowInitParams initParams = DEFAULT_SHAWDOW_INIT_PARAMS;
    _setup_connect_init_params(&initParams);

    void *client = IOT_Shadow_Construct(&initParams);
    if (client != NULL) {
        Log_i("Cloud Device Construct Success");
    } else {
        Log_e("Cloud Device Construct Failed");
        return QCLOUD_ERR_FAILURE;
    }

    //init shadow data
    _setup_shadow_data();

    //register config shadow propertys here
    rc = _register_config_shadow_property(client);
    if (rc == QCLOUD_ERR_SUCCESS) {
        Log_i("Cloud Device Register Delta Success");
    } else {
        Log_e("Cloud Device Register Delta Failed: %d", rc);
        return rc;
    }

    //register subscribe topics here
    rc = _register_subscribe_topics(client);
    if (rc != QCLOUD_ERR_SUCCESS) {
        Log_e("Client Subscribe Topic Failed: %d", rc);
        return rc;
    } else {
        Log_i("Client Subscribe Topic Success");
    }

    while (IOT_Shadow_IsConnected(client) || rc == QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT || rc == QCLOUD_ERR_MQTT_RECONNECTED) {

        rc = IOT_Shadow_Yield(client, 200);

        if (rc == QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT) {
            sleep(1);
            continue;
        }

        _simulate_room_temperature(&s_reportTemperature);
        printf("\n");
        Log_i("airConditioner state: %s", s_isAirConditionerOpen ? "open" : "close");
        Log_i("currentTemperature: %f, energyConsumption: %f", s_reportTemperature, s_energy_consumption);

        _do_report_energy_consumption(client, s_cJsonDocumentBuffer, s_sizeOfJsonDocumentBuffer);

		if (sg_messageArrivedOnDelta) {
			_do_report_temperature_desire(client, s_cJsonDocumentBuffer, s_sizeOfJsonDocumentBuffer);
			sg_messageArrivedOnDelta = false;
		}

        sleep(1);
    }

    rc = IOT_Shadow_Destroy(client);

    return rc;
}
