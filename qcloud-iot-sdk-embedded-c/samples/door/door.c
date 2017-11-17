
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>

#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"
#include "iot_config.h"

#define MAX_SIZE_OF_TOPIC_CONTENT 100

static char ca_file[PATH_MAX + 1];
static char cert_file[PATH_MAX + 1];
static char key_file[PATH_MAX + 1];


void printUsage()
{
    printf("1. ./door come_home targetDeviceName\n");
    printf("2. ./door leave_home targetDeviceName\n");
}

bool log_handler(const char* message) {
	//实现日志回调的写方法
	//实现内容后请返回true
	return false;
}

/**
 * 设置MQTT connet初始化参数
 *
 * @param pInitParams MQTT connet初始化参数
 */
static void _setup_connect_init_params(MQTTInitParams* pInitParams)
{
	pInitParams->client_id = QCLOUD_IOT_MQTT_CLIENT_ID;
	pInitParams->device_name = QCLOUD_IOT_MY_DEVICE_NAME;
	pInitParams->product_name = QCLOUD_IOT_MY_PRODUCT_NAME;
	pInitParams->password = QCLOUD_IOT_MQTT_PASSWORD;

    char cert_dir[PATH_MAX + 1] = "../../certs";
    char cur_dir[PATH_MAX + 1];
    getcwd(cur_dir, sizeof(cur_dir));
    sprintf(ca_file, "%s/%s/%s", cur_dir, cert_dir, QCLOUD_IOT_CA_FILENAME);
    pInitParams->ca_file = ca_file;

	pInitParams->is_asymc_encryption = QCLOUD_IOT_IS_ASYMC_ENCRYPTION;

	if (pInitParams->is_asymc_encryption) {
		sprintf(cert_file, "%s/%s/%s", cur_dir, cert_dir, QCLOUD_IOT_CERT_FILENAME);
		sprintf(key_file, "%s/%s/%s", cur_dir, cert_dir, QCLOUD_IOT_KEY_FILENAME);

		pInitParams->cert_file = cert_file;
		pInitParams->key_file = key_file;

	}
	else {
		pInitParams->psk = QCLOUD_IOT_PSK;
	}
    
	pInitParams->command_timeout = QCLOUD_IOT_MQTT_COMMAND_TIMEOUT;
	pInitParams->connect_timeout = QCLOUD_IOT_TLS_HANDSHAKE_TIMEOUT;
	pInitParams->keep_alive_interval = QCLOUD_IOT_MQTT_KEEP_ALIVE_INTERNAL;

	pInitParams->auto_connect_enable = 1;

	pInitParams->is_asymc_encryption = QCLOUD_IOT_IS_ASYMC_ENCRYPTION;
    pInitParams->on_disconnect_handler = NULL;
}

/**
 * 发送topic消息
 *
 * @param action 行为
 */
static int _publish_msg(void *client, char* action, char* targetDeviceName)
{
    if(NULL == action || NULL == targetDeviceName) 
        return -1;

    char topicName[MAX_SIZE_OF_CLOUD_TOPIC] = {0};
    sprintf(topicName,"%s/%s/%s", QCLOUD_IOT_MY_PRODUCT_NAME, QCLOUD_IOT_MY_DEVICE_NAME, "update");

    PublishParams pubParams = DefaultPubParams;
    pubParams.qos = QOS1;

    char topicContent[MAX_SIZE_OF_TOPIC_CONTENT + 1] = {0};
    if(strcmp(action, "come_home") == 0 || strcmp(action, "leave_home") == 0)
    {
        int size = HAL_Snprintf(topicContent, sizeof(topicContent), "{\"action\": \"%s\", \"targetDevice\": \"%s\"}", action, targetDeviceName);
        if (size < 0 || size > sizeof(topicContent) - 1)
        {
            Log_e("payload content length not enough! content size:%d  buf size:%d", size, (int)sizeof(topicContent));
            return -3;
        }

        pubParams.payload = topicContent;
        pubParams.payload_len = strlen(topicContent);
    }
    else
    {
        printUsage();
        return -2;
    }

    int rc = IOT_MQTT_Publish(client, topicName, &pubParams);
    if (rc != QCLOUD_ERR_SUCCESS) {
        Log_e("Client publish Topic:%s Failed :%d with content: %s", topicName, rc, (char*)pubParams.payload);
        return rc;
    } else {
        Log_i("Client publish Topic:%s Success with content: %s", topicName, (char*)pubParams.payload);
    }
    return 0;
}

int main(int argc, char **argv) {

    if(argc != 3)
    {
        printUsage();
        return -1;
    }

    //init log level
    IOT_Log_Set_Level(DEBUG);
    IOT_Log_Set_MessageHandler(log_handler);

    //init connection
    MQTTInitParams initParams = DEFAULT_MQTTINIT_PARAMS;
    _setup_connect_init_params(&initParams);

    void *client = IOT_MQTT_Construct(&initParams);
    if (client != NULL) {
        Log_i("Cloud Device Construct Success");
    } else {
        Log_e("Cloud Device Construct Failed");
        return QCLOUD_ERR_FAILURE;
    }

    //publish msg
    char* action = argv[1];
    char* targetDeviceName = argv[2];
    int rc = _publish_msg(client, action, targetDeviceName);
    if (rc < 0)
    {
        Log_e("Demo publish fail rc=%d", rc);
    }

    rc = IOT_MQTT_Destroy(&client);

    return rc;
}
