
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

static char ca_file[PATH_MAX + 1];
static char cert_file[PATH_MAX + 1];
static char key_file[PATH_MAX + 1];

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
    else {
    	Log_w("get action from json failed.");
    }
}

/**
 * 设置MQTT connet初始化参数
 *
 * @param initParams MQTT connet初始化参数
 */
static void _setup_connect_init_params(ShadowInitParams* initParams)
{
	initParams->mqtt.client_id = QCLOUD_IOT_MQTT_CLIENT_ID;
	initParams->mqtt.device_name = QCLOUD_IOT_MY_DEVICE_NAME;
	initParams->mqtt.product_name = QCLOUD_IOT_MY_PRODUCT_NAME;
	initParams->mqtt.password = QCLOUD_IOT_MQTT_PASSWORD;

    // 获取CA证书、客户端证书以及私钥文件的路径
    char certDirectory[PATH_MAX + 1] = "../../certs";
    char currentWD[PATH_MAX + 1];
    getcwd(currentWD, sizeof(currentWD));
    sprintf(ca_file, "%s/%s/%s", currentWD, certDirectory, QCLOUD_IOT_CA_FILENAME);
    initParams->mqtt.ca_file = ca_file;

    // 确定加密方式
    initParams->mqtt.is_asymc_encryption = QCLOUD_IOT_IS_ASYMC_ENCRYPTION;
    if (initParams->mqtt.is_asymc_encryption) {
		sprintf(cert_file, "%s/%s/%s", currentWD, certDirectory, QCLOUD_IOT_CERT_FILENAME);
		sprintf(key_file, "%s/%s/%s", currentWD, certDirectory, QCLOUD_IOT_KEY_FILENAME);
		initParams->mqtt.cert_file = cert_file;
		initParams->mqtt.key_file = key_file;

	} else {
		// 设置对称密钥
		initParams->mqtt.psk = QCLOUD_IOT_PSK;
	}

    initParams->mqtt.auto_connect_enable = 1;
    initParams->mqtt.on_disconnect_handler = NULL;
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
        Log_i("airConditioner state: %s", s_isAirConditionerOpen ? "open" : "close");
        Log_i("currentTemperature: %f, energyConsumption: %f", s_reportTemperature, s_energy_consumption);

        sleep(1);
    }

    rc = IOT_Shadow_Destroy(client);

    return rc;
}
