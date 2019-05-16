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
#include <signal.h>
#include <time.h>
#include "qcloud_iot_export.h"
#include "events_config.c"


#ifdef AUTH_MODE_CERT
    static char sg_cert_file[PATH_MAX + 1];      //客户端证书全路径
    static char sg_key_file[PATH_MAX + 1];       //客户端密钥全路径
#endif

static DeviceInfo sg_devInfo;


static char sg_shadow_update_buffer[2048];
size_t sg_shadow_update_buffersize = sizeof(sg_shadow_update_buffer) / sizeof(sg_shadow_update_buffer[0]);

//static DeviceProperty sg_shadow_property;
//static int sg_current_update_count = 0;
static bool sg_delta_arrived = false;

static MQTTEventType sg_subscribe_event_result = MQTT_EVENT_UNDEF;


void OnDeltaCallback(void *pClient, const char *pJsonValueBuffer, uint32_t valueLength, DeviceProperty *pProperty) {
	int rc = IOT_Shadow_JSON_ConstructDesireAllNull(pClient, sg_shadow_update_buffer, sg_shadow_update_buffersize);

	if (rc == QCLOUD_ERR_SUCCESS) {
		sg_delta_arrived = true;
	}
	else {
		Log_e("construct desire failed, err: %d", rc);
	}
}

void OnShadowUpdateCallback(void *pClient, Method method, RequestAck requestAck, const char *pJsonDocument, void *pUserdata) {
	Log_i("recv shadow update response, response ack: %d", requestAck);
}

static void event_handler(void *pclient, void *handle_context, MQTTEventMsg *msg) 
{	
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

		case MQTT_EVENT_SUBCRIBE_SUCCESS:
            sg_subscribe_event_result = msg->event_type;
			Log_i("subscribe success, packet-id=%u", (unsigned int)packet_id);
			break;

		case MQTT_EVENT_SUBCRIBE_TIMEOUT:
            sg_subscribe_event_result = msg->event_type;
			Log_i("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
			break;

		case MQTT_EVENT_SUBCRIBE_NACK:
            sg_subscribe_event_result = msg->event_type;
			Log_i("subscribe nack, packet-id=%u", (unsigned int)packet_id);
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


/**
 * 设置MQTT connet初始化参数
 */
static int _setup_connect_init_params(ShadowInitParams* initParams)
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


	initParams->auto_connect_enable = 1;
	initParams->shadow_type = eSHADOW;
    initParams->event_handle.h_fp = event_handler;

    return QCLOUD_ERR_SUCCESS;
}

void dumpEventInfo(int count,sEvent *pEventArry)
{
	int i, j;

	Log_d("dumpEventInfo entry:%p",pEventArry);
	for(i=0; i < count; i++)
	{
		HAL_Printf("name:%s ", pEventArry[i].event_name);
		HAL_Printf("type:%d ", pEventArry[i].type);
		HAL_Printf("timestamp: %u ", pEventArry[i].timestamp);
		HAL_Printf("num:%d ", pEventArry[i].eventDataNum);
	
		DeviceProperty *pTemp = pEventArry[i].pEventData;	
		for(j = 0; j < pEventArry[i].eventDataNum; j++)
		{
			if(!pTemp)
			{
				Log_e("%dth/%d event %dth/%d property data null", i, count, j, pEventArry[i].eventDataNum);
			}
			HAL_Printf("%dth property: ", j);
			HAL_Printf("key:%s   ", pTemp->key);
			HAL_Printf("type:%d  ", pTemp->type);		
			pTemp++;
		}		
		HAL_Printf("\n\r");
	}
}

static void update_events_timestamp(sEvent *pEvents, int count)
{
	int i;
	
	for(i = 0; i < count; i++){
        if (NULL == (&pEvents[i])) { 
	        Log_e("null event pointer"); 
	        return; 
        }
#ifdef EVENT_TIMESTAMP_USED		
		pEvents[i].timestamp = time(NULL); //should be UTC and accurate
#else
		pEvents[i].timestamp = 0;
#endif

	}
}


static void raw_event_post_cb(void *pClient, MQTTMessage *msg)
{
	Log_d("Reply:%.*s", msg->payload_len, msg->payload);
}

static void event_post_cb(void *pClient, MQTTMessage *msg)
{
	Log_d("Reply:%.*s", msg->payload_len, msg->payload);
	clearEventFlag(FLAG_EVENT0|FLAG_EVENT1|FLAG_EVENT2);
}

static void single_event_post_cb(void *pClient, MQTTMessage *msg)
{
	Log_d("Reply:%.*s", msg->payload_len, msg->payload);
	clearEventFlag(FLAG_EVENT1);
}


int main(int argc, char **argv) {
    int rc;
	int i;
	uint32_t event_count;
	uint32_t eflag;
	sEvent *pEventList[EVENT_COUNTS];

    //init log level
    IOT_Log_Set_Level(DEBUG);

    //init connection
    ShadowInitParams init_params = DEFAULT_SHAWDOW_INIT_PARAMS;
    rc = _setup_connect_init_params(&init_params);
    if (rc != QCLOUD_ERR_SUCCESS) {
		Log_e("init params err,rc=%d", rc);
		return rc;
	}

    void *shadow_client = IOT_Shadow_Construct(&init_params);
    if (shadow_client != NULL) {
        Log_i("Cloud Device Construct Success");
    } else {
        Log_e("Cloud Device Construct Failed");
        return QCLOUD_ERR_FAILURE;
    }

	
//====================event func beging ===================//

#ifdef EVENT_POST_ENABLED
	rc = event_init(shadow_client);
	if (rc < 0) 
	{
        Log_e("event init failed: %d", rc);
        return rc;
    }

	dumpEventInfo(EVENT_COUNTS, g_events);
#endif
	
	


	while (IOT_Shadow_IsConnected(shadow_client) || QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT == rc ||
			QCLOUD_ERR_MQTT_RECONNECTED == rc || QCLOUD_ERR_SUCCESS == rc) {

		rc = IOT_Shadow_Yield(shadow_client, 200);

		if (QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT == rc) {
			sleep(1);
			continue;
		}
		else if (rc != QCLOUD_ERR_SUCCESS && rc != QCLOUD_ERR_MQTT_RECONNECTED) {
			Log_e("exit with error: %d", rc);
			return rc;
		}

		//====================event func use case ===================//
#ifdef EVENT_POST_ENABLED			
		//单个事件上报	
		setEventFlag(FLAG_EVENT1); //示例，用户需根据业务场景置位事件
		eflag = getEventFlag();
		if((eflag&FLAG_EVENT1)&ALL_EVENTS_MASK){
			pEventList[0] = &g_events[1];
			update_events_timestamp(pEventList[0], 1);	
			rc = qcloud_iot_post_event(shadow_client, sg_shadow_update_buffer, sg_shadow_update_buffersize,\
										1, pEventList, single_event_post_cb);											
			if (rc < 0) 
			{
				Log_e("sigle event post failed: %d", rc);
				return rc;
			}
		}
	
		//多个事件上报
		setEventFlag(FLAG_EVENT0|FLAG_EVENT1|FLAG_EVENT2);
		eflag = getEventFlag();
		if((EVENT_COUNTS > 0 )&& (eflag > 0)){	
			event_count = 0;
			for(i = 0; i < EVENT_COUNTS; i++){
				if((eflag&(1<<i))&ALL_EVENTS_MASK){
					 pEventList[event_count++] = &(g_events[i]);
					 update_events_timestamp(&g_events[i], 1);
					clearEventFlag(1<<i);
				}
			}	

			rc = qcloud_iot_post_event(shadow_client, sg_shadow_update_buffer, sg_shadow_update_buffersize, \
					 							event_count, pEventList, event_post_cb);
			if(rc < 0){
				Log_e("events post failed: %d", rc);
			}
		}	


		//事件Json,用户按照事件json格式传入事件部分json
		rc = qcloud_iot_post_event_raw(shadow_client, sg_shadow_update_buffer, sg_shadow_update_buffersize, \
										"{\"eventId\":\"status_report\", \"type\":\"info\", \"timestamp\":0, \"params\":{\"status\":1,\"message\":\"test\"}},{\"eventId\":\"low_voltage\", \"type\":\"alert\", \"timestamp\":0, \"params\":{\"voltage\":4.500000}}",
										raw_event_post_cb);		
		if (rc < 0) 
		{
	        Log_e("raw event post failed: %d", rc);
	        return rc;
		}

#endif

	//====================event func end ===================//	
		// sleep for some time in seconds
		sleep(1);
	}

	Log_e("loop exit with error: %d", rc);

	rc = IOT_Shadow_Destroy(shadow_client);

	return rc;
}
