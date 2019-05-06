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

#ifdef SYSTEM_COMM 

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "mqtt_client.h"
#include "lite-utils.h"
#include "device.h"


static bool sg_sys_recv_ok   = false;
static bool sg_sys_sub_ok   = false;
static char sg_time[11]      = {0};
static MQTTEventHandleFun sg_sys_EventHandle = NULL;

static void on_system_mqtt_message_callback(void *pClient, MQTTMessage *message, void *userData)
{
#define MAX_RECV_LEN (512)

    POINTER_SANITY_CHECK_RTN(message);	

    static char rcv_buf[MAX_RECV_LEN + 1];
	size_t len = (message->payload_len > MAX_RECV_LEN)?MAX_RECV_LEN:(message->payload_len);

	if(message->payload_len > MAX_RECV_LEN){
		Log_e("paload len exceed buffer size");
	}
    memcpy(rcv_buf, message->payload, len);
    rcv_buf[len] = '\0';    // jsmn_parse relies on a string    

    Log_d("Recv Msg Topic:%s, payload:%s", message->ptopic, rcv_buf);

    char* value = LITE_json_value_of("time", rcv_buf);
    if (value != NULL) {
        memcpy(sg_time, value, sizeof(sg_time)/sizeof(char));        
    }
    sg_sys_recv_ok = true;
    HAL_Free(value);
    return;
}

static void system_mqtt_event_handler(void *pclient, void *handle_context, MQTTEventMsg *msg) {
	uintptr_t packet_id = (uintptr_t)msg->msg;

	switch(msg->event_type) {
		case MQTT_EVENT_SUBCRIBE_SUCCESS:
			Log_d("subscribe success, packet-id=%u", (unsigned int)packet_id);
            sg_sys_sub_ok = true;			
			break;

		case MQTT_EVENT_SUBCRIBE_TIMEOUT:
			Log_i("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
            sg_sys_sub_ok = false;			
			break;

		case MQTT_EVENT_SUBCRIBE_NACK:
			Log_i("subscribe nack, packet-id=%u", (unsigned int)packet_id);
            sg_sys_sub_ok = false;			
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

static int _iot_system_info_get_publish(void *pClient)
{
    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);

    Qcloud_IoT_Client   *mqtt_client = (Qcloud_IoT_Client *)pClient;
    DeviceInfo          *dev_info = iot_device_info_get();
    POINTER_SANITY_CHECK(dev_info, QCLOUD_ERR_INVAL);

    char topic_name[128] = {0};
    char payload_content[128] = {0};

    HAL_Snprintf(topic_name, sizeof(topic_name), "$sys/operation/%s/%s", dev_info->product_id, dev_info->device_name);
    HAL_Snprintf(payload_content, sizeof(payload_content), "{\"type\": \"get\", \"resource\": [\"time\"]}");

    PublishParams pub_params = DEFAULT_PUB_PARAMS;
    pub_params.qos = QOS0;
    pub_params.payload = payload_content;
    pub_params.payload_len = strlen(payload_content);

	return IOT_MQTT_Publish(mqtt_client, topic_name, &pub_params);
}

static int _iot_system_info_result_subscribe(void *pClient, OnMessageHandler pCallback)
{
    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);

    DeviceInfo          *dev_info = iot_device_info_get();
    POINTER_SANITY_CHECK(dev_info, QCLOUD_ERR_INVAL);

    char topic_name[128] = {0};
    int size = HAL_Snprintf(topic_name, sizeof(topic_name), "$sys/operation/result/%s/%s", dev_info->product_id, dev_info->device_name);
    if (size < 0 || size > sizeof(topic_name) - 1)
    {
        Log_e("topic content length not enough! content size:%d  buf size:%d", size, (int)sizeof(topic_name));
        return QCLOUD_ERR_FAILURE;
    }
    SubscribeParams sub_params = DEFAULT_SUB_PARAMS;
    sub_params.on_message_handler = pCallback;

    return IOT_MQTT_Subscribe(pClient, topic_name, &sub_params);
}

int IOT_SYSTEM_GET_TIME(void* pClient, long *time)
{
    int ret = 0;
    int cntSub = 0;
    int cntRev = 0;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
    Qcloud_IoT_Client   *mqtt_client = (Qcloud_IoT_Client *)pClient;

    //如果第一次订阅$sys/operation/get/${productid}/${devicename}, 则执行订阅操作
    //否则，防止多次获取时间的情况下，多次重复订阅
    if(!sg_sys_sub_ok){
        sg_sys_EventHandle = mqtt_client->event_handle.h_fp;
        mqtt_client->event_handle.h_fp = system_mqtt_event_handler;

        for(cntSub = 0; cntSub < 3; cntSub++){
            ret = _iot_system_info_result_subscribe(mqtt_client, on_system_mqtt_message_callback);
            if (ret < 0) {
                Log_w("_iot_system_info_result_subscribe failed: %d cnt: %d", ret, cntSub);
                continue;
            }

            ret = qcloud_iot_mqtt_yield((Qcloud_IoT_Client *)pClient, 100);

            if(sg_sys_sub_ok) {
                break;
            }
        }
        mqtt_client->event_handle.h_fp = sg_sys_EventHandle;
    }

    // 如果订阅3次均失败，则直接返回失败
    if(!sg_sys_sub_ok){
        Log_e("Subscribe sys topic failed!");
        return QCLOUD_ERR_FAILURE;
    }

    sg_sys_recv_ok = false;
    // 发布获取时间
	ret = _iot_system_info_get_publish(mqtt_client);
	if (ret < 0) {
		Log_e("client publish sys topic failed :%d.", ret);
        return ret;
	}

    do{
        ret = qcloud_iot_mqtt_yield((Qcloud_IoT_Client *)pClient, 100);
        cntRev++;
    }while(!sg_sys_recv_ok && cntRev < 20);
   
    if (sg_sys_recv_ok) {
        *time = atol(sg_time);
        Log_d("Get system time success(yield cnt %d). Time is %ld", cntRev, *time);        
        ret = QCLOUD_ERR_SUCCESS;
    } else {
        *time = 0;
        ret = QCLOUD_ERR_FAILURE;
    }  

    return QCLOUD_ERR_SUCCESS;
}

#ifdef __cplusplus
}
#endif

#endif
