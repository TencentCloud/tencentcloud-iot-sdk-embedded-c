/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"

#define MAX_SIZE_OF_TOPIC_CONTENT 100

#ifdef AUTH_MODE_CERT
    static char sg_cert_file[PATH_MAX + 1];      //客户端证书全路径
    static char sg_key_file[PATH_MAX + 1];       //客户端密钥全路径
#endif

static DeviceInfo sg_devInfo;


void printUsage()
{
    printf("1. ./door_coap come_home [targetDeviceName]\n");
    printf("2. ./door_coap leave_home [targetDeviceName]\n");
}


void event_handler(void *pcontext, CoAPEventMessage *message)
{
	switch (message->event_type) {
		case COAP_EVENT_RECEIVE_ACK:
			Log_i("message received ACK, msgid: %d", (unsigned)(uintptr_t)message->message);
			break;
		case COAP_EVENT_RECEIVE_RESPCONTENT:	/* 暂不支持消息交互，仅支持数据上报 */
			break;
		case COAP_EVENT_UNAUTHORIZED:
			Log_i("coap client auth token expired or invalid, msgid: %d", (unsigned)(uintptr_t)message->message);
			break;
		case COAP_EVENT_FORBIDDEN:
			Log_i("coap URI is invalid for this device, msgid: %d", (unsigned)(uintptr_t)message->message);
			break;
		case COAP_EVENT_INTERNAL_SERVER_ERROR:
			Log_i("coap server internal error, msgid: %d", (unsigned)(uintptr_t)message->message);
			break;
		case COAP_EVENT_ACK_TIMEOUT:
			Log_i("message receive ACK timeout, msgid: %d", (unsigned)(uintptr_t)message->message);
			break;
		case COAP_EVENT_SEPRESP_TIMEOUT:		/* 暂不支持消息交互，仅支持数据上报 */
			break;
		default:
			Log_e("unrecogonized event type: %d", message->event_type);
			break;
	}
}

static int _setup_connect_init_params(CoAPInitParams* initParams)
{
	int ret;
	
	ret = HAL_GetDevInfo((void *)&sg_devInfo);	
	if(QCLOUD_ERR_SUCCESS != ret){
		return ret;
	}
	
	initParams->device_name = sg_devInfo.device_name;
	initParams->product_id = sg_devInfo.product_id;

#ifdef AUTH_MODE_CERT
	/* 使用非对称加密*/
	char certs_dir[PATH_MAX + 1] = "certs";
	char current_path[PATH_MAX + 1];
	char *cwd = getcwd(current_path, sizeof(current_path));
	if (cwd == NULL)
	{
		Log_e("getcwd return NULL");
		return QCLOUD_ERR_FAILURE;
	}
	sprintf(sg_cert_file, "%s/%s/%s", current_path, certs_dir, sg_devInfo.devCertFileName);
	sprintf(sg_key_file, "%s/%s/%s", current_path, certs_dir, sg_devInfo.devPrivateKeyFileName);

	initParams->cert_file = sg_cert_file;
	initParams->key_file = sg_key_file;
#else
	initParams->device_secret = sg_devInfo.devSerc;
#endif

	initParams->command_timeout = QCLOUD_IOT_MQTT_COMMAND_TIMEOUT;
	initParams->event_handle.h_fp = event_handler;
	initParams->max_retry_count = 3;


    return QCLOUD_ERR_SUCCESS;
}


int main(int argc, char **argv)
{
	int rc = QCLOUD_ERR_SUCCESS;
	
    if(argc != 3)
    {
        printUsage();
        return -1;
    }

	IOT_Log_Set_Level(DEBUG);

	CoAPInitParams init_params = DEFAULT_COAPINIT_PARAMS;
	rc = _setup_connect_init_params(&init_params);
	if (rc != QCLOUD_ERR_SUCCESS) {
		Log_e("init params err,rc=%d", rc);
		return rc;
	}

	void *coap_client = IOT_COAP_Construct(&init_params);
	if (coap_client == NULL) {
		Log_e("COAP Client construct failed.");
		return QCLOUD_ERR_FAILURE;
	}
	else {
		Log_e("%p", coap_client);
	}

    char* action = argv[1];
    char* target_device_name = argv[2];

    if(strcmp(action, "come_home") == 0 || strcmp(action, "leave_home") == 0)
    {
        char topic_content[MAX_SIZE_OF_TOPIC_CONTENT + 1] = {0};
    	SendMsgParams send_params = DEFAULT_SENDMSG_PARAMS;

        int size = HAL_Snprintf(topic_content, sizeof(topic_content), "{\"action\": \"%s\", \"targetDevice\": \"%s\"}", action, target_device_name);
        if (size < 0 || size > sizeof(topic_content) - 1)
        {
            Log_e("payload content length not enough! content size:%d  buf size:%d", size, (int)sizeof(topic_content));
            return -3;
        }

        send_params.pay_load = topic_content;
    	send_params.pay_load_len = strlen(topic_content);

        char topicName[128] = "";
        sprintf(topicName, "%s/%s/event", sg_devInfo.product_id, sg_devInfo.device_name);
        Log_i("topic name is %s", topicName);

        rc = IOT_COAP_SendMessage(coap_client, topicName, &send_params);
	    if (rc < 0) {
	        Log_e("client publish topic failed :%d.", rc);
	    }
	    else {
	    	Log_d("client topic has been sent, msg_id: %d", rc);
	    }

    	rc = IOT_COAP_Yield(coap_client, 200);

		if (rc != QCLOUD_ERR_SUCCESS){
			Log_e("exit with error: %d", rc);
		}
    }
    else
    {
        printUsage();
        return -2;
    }

    IOT_COAP_Destroy(&coap_client);

    return QCLOUD_ERR_SUCCESS;
}
