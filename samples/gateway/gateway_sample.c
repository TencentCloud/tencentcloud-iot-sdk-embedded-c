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


#ifdef AUTH_MODE_CERT
/* 产品名称, 与云端同步设备状态时需要  */
#define QCLOUD_IOT_MY_PRODUCT_ID            "YOUR_GATE_WAY_PRODUCT_ID"
/* 设备名称, 与云端同步设备状态时需要 */
#define QCLOUD_IOT_MY_DEVICE_NAME           "YOUR_GATE_WAY_DEVICE_NAME"

/* 客户端证书文件名  非对称加密使用*/
#define QCLOUD_IOT_CERT_FILENAME		    "YOUR_GATE_WAY_cert.crt"
/* 客户端私钥文件名 非对称加密使用*/
#define QCLOUD_IOT_KEY_FILENAME 		    "YOUR_GATE_WAY_private.key"

static char sg_cert_file[PATH_MAX + 1]; 	 //客户端证书全路径
static char sg_key_file[PATH_MAX + 1];		 //客户端密钥全路径


#else
/* 产品名称, 与云端同步设备状态时需要  */
#define QCLOUD_IOT_MY_PRODUCT_ID            "YOUR_GATE_WAY_PRODUCT_ID"
/* 设备名称, 与云端同步设备状态时需要 */
#define QCLOUD_IOT_MY_DEVICE_NAME           "YOUR_GATE_WAY_DEVICE_NAME"

/*非证书方式下的设备密钥*/
#define QCLOUD_IOT_DEVICE_SECRET				"YOUR_GATE_WAY_DEV_PSK"

#endif

#ifdef AUTH_MODE_CERT
/* 子产品名称, 与云端同步设备状态时需要  */
#define QCLOUD_IOT_SUBDEV_PRODUCT_ID            "YOUR_SUB_DEV_PRODUCT_ID"
/* 子设备名称, 与云端同步设备状态时需要 */
#define QCLOUD_IOT_SUBDEV_DEVICE_NAME           "YOUR_SUB_DEV_NAME"
#else
/* 子产品名称, 与云端同步设备状态时需要  */
#define QCLOUD_IOT_SUBDEV_PRODUCT_ID            "YOUR_SUB_DEV_PRODUCT_ID"
/* 子设备名称, 与云端同步设备状态时需要 */
#define QCLOUD_IOT_SUBDEV_DEVICE_NAME           "YOUR_SUB_DEV_NAME"
#endif


#define MAX_SIZE_OF_TOPIC (128)
#define MAX_SIZE_OF_DATA (128)



//static int sg_count = 0;
static int sg_sub_packet_id = -1;


void _event_handler(void *client, void *context, MQTTEventMsg *msg) 
{
	MQTTMessage* mqtt_messge = (MQTTMessage*)msg->msg;
	uintptr_t packet_id = (uintptr_t)msg->msg;

	switch(msg->event_type) {
		case MQTT_EVENT_UNDEF:
			Log_i("undefined event occur.");
			break;

		case MQTT_EVENT_DISCONNECT:
			Log_i("MQTT disconnect.");
			break;

		case MQTT_EVENT_RECONNECT:
			Log_i("MQTT reconnect.");
			break;

		case MQTT_EVENT_PUBLISH_RECVEIVED:
			Log_i("topic message arrived but without any related handle: topic=%.*s, topic_msg=%.*s",
					mqtt_messge->topic_len,
					mqtt_messge->ptopic,
					mqtt_messge->payload_len,
					mqtt_messge->payload);
			break;
		case MQTT_EVENT_SUBCRIBE_SUCCESS:
			Log_i("subscribe success, packet-id=%u", (unsigned int)packet_id);
			sg_sub_packet_id = packet_id;
			break;

		case MQTT_EVENT_SUBCRIBE_TIMEOUT:
			Log_i("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
			sg_sub_packet_id = packet_id;
			break;

		case MQTT_EVENT_SUBCRIBE_NACK:
			Log_i("subscribe nack, packet-id=%u", (unsigned int)packet_id);
			sg_sub_packet_id = packet_id;
			break;

		case MQTT_EVENT_UNSUBCRIBE_SUCCESS:
			Log_i("unsubscribe success, packet-id=%u", (unsigned int)packet_id);
			break;

		case MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
			Log_i("unsubscribe timeout, packet-id=%u", (unsigned int)packet_id);
			break;

		case MQTT_EVENT_UNSUBCRIBE_NACK:
			Log_i("unsubscribe nack, packet-id=%u", (unsigned int)packet_id);
			break;

		case MQTT_EVENT_PUBLISH_SUCCESS:
			Log_i("publish success, packet-id=%u", (unsigned int)packet_id);
			break;

		case MQTT_EVENT_PUBLISH_TIMEOUT:
			Log_i("publish timeout, packet-id=%u", (unsigned int)packet_id);
			break;

		case MQTT_EVENT_PUBLISH_NACK:
			Log_i("publish nack, packet-id=%u", (unsigned int)packet_id);
			break;
		default:
			Log_i("Should NOT arrive here.");
			break;
	}
}


static void _message_handler(void *client, MQTTMessage *message, void *user_data)
{
	char data[MAX_SIZE_OF_DATA+1] = {0};	
	int len;
	Log_d("_message_handler.");
	if(message == NULL) {
		return;	
	}

	len = message->topic_len > MAX_SIZE_OF_DATA?MAX_SIZE_OF_DATA:message->topic_len;
	if(message->ptopic != NULL) {
		memcpy(data, message->ptopic, len);
		Log_d("topic(%s),len(%d)", data, message->topic_len);	
	}


	len = message->payload_len> MAX_SIZE_OF_DATA?MAX_SIZE_OF_DATA:message->payload_len;
	if(message->payload != NULL) {
		memcpy(data, message->payload, len);
		Log_d("payload(%s),len(%d)", data, message->payload_len);
	}

	return;
}


int demo_gateway()
{
	int rc = QCLOUD_ERR_FAILURE;
	int count = 0;
	void* client = NULL;

	GatewayInitParam init_params = DEFAULT_GATEWAY_INIT_PARAMS;
	init_params.init_param.product_id = QCLOUD_IOT_MY_PRODUCT_ID;
	init_params.init_param.device_name = QCLOUD_IOT_MY_DEVICE_NAME;


#ifdef AUTH_MODE_CERT
	// 获取CA证书、客户端证书以及私钥文件的路径
	char certs_dir[PATH_MAX + 1] = "certs";
	char current_path[PATH_MAX + 1];
	char *cwd = getcwd(current_path, sizeof(current_path));
	if (cwd == NULL)
	{
		Log_e("getcwd return NULL");
		return QCLOUD_ERR_FAILURE;
	}
	sprintf(sg_cert_file, "%s/%s/%s", current_path, certs_dir, QCLOUD_IOT_CERT_FILENAME);
	sprintf(sg_key_file, "%s/%s/%s", current_path, certs_dir, QCLOUD_IOT_KEY_FILENAME);

	init_params.init_param.cert_file = sg_cert_file;
	init_params.init_param.key_file = sg_key_file;
#else
	init_params.init_param.device_secret = QCLOUD_IOT_DEVICE_SECRET;
#endif

	init_params.init_param.command_timeout = QCLOUD_IOT_MQTT_COMMAND_TIMEOUT;
	init_params.init_param.auto_connect_enable = 1;
    init_params.init_param.event_handle.h_fp = _event_handler;
	client = IOT_Gateway_Construct(&init_params);
	if (client == NULL) {
		Log_e("client constructed failed.");
		return QCLOUD_ERR_FAILURE;
	}

	//上线子设备
	GatewayParam param = DEFAULT_GATEWAY_PARAMS;;
	param.product_id = QCLOUD_IOT_MY_PRODUCT_ID;
	param.device_name = QCLOUD_IOT_MY_DEVICE_NAME;
	
	param.subdev_product_id = QCLOUD_IOT_SUBDEV_PRODUCT_ID;
	param.subdev_device_name = QCLOUD_IOT_SUBDEV_DEVICE_NAME;
	
	rc = IOT_Gateway_Subdev_Online(client, &param);
	if(rc != QCLOUD_ERR_SUCCESS) {
		Log_e("IOT_Gateway_Subdev_Online fail.");
		return rc;
	}
#if 1	
	//订阅消息
	char topic_filter[MAX_SIZE_OF_TOPIC+1] = {0};
	SubscribeParams sub_param = DEFAULT_SUB_PARAMS;
	int size = HAL_Snprintf(topic_filter, MAX_SIZE_OF_TOPIC+1, "%s/%s/control", QCLOUD_IOT_SUBDEV_PRODUCT_ID, QCLOUD_IOT_SUBDEV_DEVICE_NAME);
	if (size < 0 || size > MAX_SIZE_OF_TOPIC)
	{
		Log_e("buf size < topic length!");
		return QCLOUD_ERR_FAILURE;
	}
	sub_param.on_message_handler = _message_handler;
	rc = IOT_Gateway_Subscribe(client, topic_filter, &sub_param);
	if(rc < 0) {
		Log_e("IOT_Gateway_Subscribe fail.");
		return rc;
	}

	rc = IOT_Gateway_Yield(client, 200);

	//发布消息
	char topic_name[MAX_SIZE_OF_TOPIC+1] = {0};
	PublishParams pub_param = DEFAULT_PUB_PARAMS;
	size = HAL_Snprintf(topic_name, MAX_SIZE_OF_TOPIC+1, "%s/%s/control", QCLOUD_IOT_SUBDEV_PRODUCT_ID, QCLOUD_IOT_SUBDEV_DEVICE_NAME);
	if (size < 0 || size > MAX_SIZE_OF_TOPIC)
	{
		Log_e("buf size < topic length!");
		return QCLOUD_ERR_FAILURE;
	}

	pub_param.qos = QOS1;
	pub_param.payload = "{\"data\":\"test gateway\"}";
	pub_param.payload_len = sizeof("{\"data\":\"test gateway\"}");

	do {
		if(sg_sub_packet_id > 0) {
			rc = IOT_Gateway_Publish(client, topic_name, &pub_param);
			if(rc < 0) {
				Log_e("IOT_Gateway_Publish fail.");
			}
		}
		rc = IOT_Gateway_Yield(client, 200);

		if (rc == QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT) {
			sleep(1);
			continue;
		}
		else if (rc != QCLOUD_ERR_SUCCESS && rc != QCLOUD_ERR_MQTT_RECONNECTED){
			Log_e("exit with error: %d", rc);
			break;
		}

		sleep(1);

	} while (count++ < 5);
#endif	
	//下线子设备
	rc = IOT_Gateway_Subdev_Offline(client, &param);
	if(rc != QCLOUD_ERR_SUCCESS) {
		Log_e("IOT_Gateway_Subdev_Offline fail.");
		return rc;
	}
	
	rc = IOT_Gateway_Destroy(client);

	return rc;
}

int main()
{
	IOT_Log_Set_Level(DEBUG);

	demo_gateway();

	return 0;
}
