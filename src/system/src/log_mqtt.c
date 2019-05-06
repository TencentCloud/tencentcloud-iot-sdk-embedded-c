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

#ifdef LOG_UPLOAD

#include <string.h>

#include "mqtt_client.h"
#include "lite-utils.h"
#include "device.h"
#include "log_upload.h"


static bool sg_log_recv_ok   = false;
static bool sg_log_sub_ok   = false;
static MQTTEventHandleFun sg_log_EventHandle = NULL;
static int sg_log_level = ERROR;
static unsigned int sg_client_token = 1;

static bool _get_json_log_level(char *json, int32_t* res) {
	char *v = LITE_json_value_of("log_level", json);
	if (v == NULL) {
        Log_e("Invalid log level from JSON: %s", json);
		return false;
	}
	if (LITE_get_int32(res, v) != QCLOUD_ERR_SUCCESS) {
        Log_e("Invalid log level from JSON: %s", json);
        HAL_Free(v);
		return false;
	}
    HAL_Free(v);
	return true;
}

static void _log_level_sub_cb(void *pClient, MQTTMessage *message, void *userData)
{
#define LOG_JSON_LENGTH 128        
    char json_buf[LOG_JSON_LENGTH] = {0};
    int32_t json_buf_len = 0; 
	if (message == NULL) {
		return;
	}

    json_buf_len = Min(LOG_JSON_LENGTH - 1, message->payload_len);
	memcpy(json_buf, message->payload, json_buf_len);
	json_buf[json_buf_len] = '\0';    // json_parse relies on a string

	Log_d("Recv Msg Topic:%s, payload:%s", message->ptopic, json_buf);
	
    if (!_get_json_log_level(json_buf, &sg_log_level)) {        
        return ;
    }

    switch(sg_log_level) {
        case DISABLE:
            Log_w("Upload log level change to: %d", sg_log_level);
            clear_upload_buffer();
            set_log_upload_in_comm_err(true);
            IOT_Log_Set_Upload_Level(ERROR);
            break;
        case ERROR:            
        case WARN:
        case INFO:
        case DEBUG:            
            if (sg_log_level < IOT_Log_Get_Upload_Level())
                clear_upload_buffer();
            IOT_Log_Set_Upload_Level((LOG_LEVEL)sg_log_level);
            Log_w("Upload log level change to: %d", sg_log_level);
            set_log_upload_in_comm_err(false);                            
            break;
        default:
            Log_e("Invalid log level: %d", sg_log_level);
            break;
    }

    sg_log_recv_ok   = true;
    
}

static void _log_mqtt_event_handler(void *pclient, void *handle_context, MQTTEventMsg *msg) {
	uintptr_t packet_id = (uintptr_t)msg->msg;

	switch(msg->event_type) {
		case MQTT_EVENT_SUBCRIBE_SUCCESS:
			Log_d("subscribe success, packet-id=%u", (unsigned int)packet_id);
            sg_log_sub_ok = true;			
			break;

		case MQTT_EVENT_SUBCRIBE_TIMEOUT:
			Log_i("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
            sg_log_sub_ok = false;			
			break;

		case MQTT_EVENT_SUBCRIBE_NACK:
			Log_i("subscribe nack, packet-id=%u", (unsigned int)packet_id);
            sg_log_sub_ok = false;			
			break;
		case MQTT_EVENT_UNDEF:
		case MQTT_EVENT_DISCONNECT:
		case MQTT_EVENT_RECONNECT:
		case MQTT_EVENT_PUBLISH_RECVEIVED:
		case MQTT_EVENT_UNSUBCRIBE_SUCCESS:
		case MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
		case MQTT_EVENT_UNSUBCRIBE_NACK:
		case MQTT_EVENT_PUBLISH_SUCCESS:
		case MQTT_EVENT_PUBLISH_TIMEOUT:
		case MQTT_EVENT_PUBLISH_NACK:
		default:
            return;
	}
}

static int _iot_log_level_get_publish(void *pClient)
{
    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);

    Qcloud_IoT_Client   *mqtt_client = (Qcloud_IoT_Client *)pClient;
    DeviceInfo          *dev_info = iot_device_info_get();
    POINTER_SANITY_CHECK(dev_info, QCLOUD_ERR_INVAL);

    char topic_name[128] = {0};
    char payload_content[128] = {0};

    HAL_Snprintf(topic_name, sizeof(topic_name), "$log/operation/%s/%s", dev_info->product_id, dev_info->device_name);
    HAL_Snprintf(payload_content, sizeof(payload_content), "{\"type\": \"get_log_level\", "
                        "\"clientToken\": \"%s-%u\"}", dev_info->product_id, sg_client_token++);

    PublishParams pub_params = DEFAULT_PUB_PARAMS;
    pub_params.qos = QOS0;
    pub_params.payload = payload_content;
    pub_params.payload_len = strlen(payload_content);

	return IOT_MQTT_Publish(mqtt_client, topic_name, &pub_params);
}

int qcloud_log_topic_subscribe(void *client)
{
    /* subscribe the log topic: "$log/operation/result/${productId}/${deviceName}" */
    char topic_name[128] = {0};
    int size = HAL_Snprintf(topic_name, sizeof(topic_name), "$log/operation/result/%s/%s", 
            iot_device_info_get()->product_id, iot_device_info_get()->device_name);
    if (size < 0 || size > sizeof(topic_name) - 1)
    {
        Log_e("topic content buf not enough! content size:%d buf size:%d", size, (int)sizeof(topic_name));
        return QCLOUD_ERR_FAILURE;
    }

    SubscribeParams sub_params = DEFAULT_SUB_PARAMS;
    sub_params.on_message_handler = _log_level_sub_cb;
    sub_params.qos = QOS0;    

    return IOT_MQTT_Subscribe(client, topic_name, &sub_params);

}


int qcloud_get_log_level(void* pClient, int *log_level)
{
    int ret = 0;
    int cntSub = 0;
    int cntRev = 0;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
    Qcloud_IoT_Client   *mqtt_client = (Qcloud_IoT_Client *)pClient;

    //如果第一次订阅$log/operation/get/${productid}/${devicename}, 则执行订阅操作
    //否则，防止多次获取日志等级的情况下，多次重复订阅
    if(!sg_log_sub_ok){
        sg_log_EventHandle = mqtt_client->event_handle.h_fp;
        mqtt_client->event_handle.h_fp = _log_mqtt_event_handler;

        for(cntSub = 0; cntSub < 3; cntSub++){
            ret = qcloud_log_topic_subscribe(mqtt_client);
            if (ret < 0) {
                Log_w("qcloud_log_topic_subscribe failed: %d, cnt: %d", ret, cntSub);
                continue;
            }

            /* wait for sub ack */
            ret = qcloud_iot_mqtt_yield((Qcloud_IoT_Client *)pClient, 100);
            if(sg_log_sub_ok) {                
                break;
            }
        }
        mqtt_client->event_handle.h_fp = sg_log_EventHandle;
    }

    // 如果订阅3次均失败，则直接返回失败
    if(!sg_log_sub_ok){
        Log_e("Subscribe log topic failed!");
        return QCLOUD_ERR_FAILURE;
    }

    sg_log_recv_ok = false;
    // 发布获取时间
	ret = _iot_log_level_get_publish(mqtt_client);
	if (ret < 0) {
		Log_e("client publish log topic failed :%d", ret);
        return ret;
	}

    do{
        ret = qcloud_iot_mqtt_yield((Qcloud_IoT_Client *)pClient, 100);
        cntRev++;
    }while(!sg_log_recv_ok && cntRev < 20);

    *log_level = sg_log_level;
    if (sg_log_recv_ok)
        ret = QCLOUD_ERR_SUCCESS;
    else
        ret = QCLOUD_ERR_FAILURE;

    return ret;
}

#endif
