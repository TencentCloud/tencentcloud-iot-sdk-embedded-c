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
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"
#include "lite-utils.h"
#include "data_config.c"



#ifdef AUTH_MODE_CERT
    static char sg_cert_file[PATH_MAX + 1];      //客户端证书全路径
    static char sg_key_file[PATH_MAX + 1];       //客户端密钥全路径
#endif

static DeviceInfo sg_devInfo;


static MQTTEventType sg_subscribe_event_result = MQTT_EVENT_UNDEF;
static bool sg_delta_arrived = false;
//static bool sg_dev_report_new_data = false;


static char sg_data_report_buffer[2048];
size_t sg_data_report_buffersize = sizeof(sg_data_report_buffer) / sizeof(sg_data_report_buffer[0]);

#ifdef EVENT_POST_ENABLED

#include "events_config.c"
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

static void event_post_cb(void *pClient, MQTTMessage *msg)
{
	Log_d("Reply:%.*s", msg->payload_len, msg->payload);
	clearEventFlag(FLAG_EVENT0|FLAG_EVENT1|FLAG_EVENT2);
}

static void eventPostCheck(void *client)
{
	int i;
	int rc;
	uint32_t eflag;
	sEvent *pEventList[EVENT_COUNTS];
	uint8_t eventCont;
		
	//事件上报
	setEventFlag(FLAG_EVENT0|FLAG_EVENT1|FLAG_EVENT2);
	eflag = getEventFlag();
	if((EVENT_COUNTS > 0 )&& (eflag > 0)){	
		eventCont = 0;
		for(i = 0; i < EVENT_COUNTS; i++){
			if((eflag&(1<<i))&ALL_EVENTS_MASK){
				 pEventList[eventCont++] = &(g_events[i]);
				 update_events_timestamp(&g_events[i], 1);
				clearEventFlag(1<<i);
			}
		}	
	
		rc = qcloud_iot_post_event(client, sg_data_report_buffer, sg_data_report_buffersize, \
											eventCont, pEventList, event_post_cb);
		if(rc < 0){
			Log_e("events post failed: %d", rc);
		}
	}

}

#endif


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
	initParams->shadow_type = eTEMPLATE;	
    initParams->event_handle.h_fp = event_handler;

    return QCLOUD_ERR_SUCCESS;
}

/*如果有自定义的字符串或者json，需要在这里解析*/
static int update_self_define_value(const char *pJsonDoc, DeviceProperty *pProperty) 
{
    int rc = QCLOUD_ERR_SUCCESS;
		
	if((NULL == pJsonDoc)||(NULL == pProperty)){
		return QCLOUD_ERR_INVAL;
	}

	/*convert const char* to char * */
	char *pTemJsonDoc =HAL_Malloc(strlen(pJsonDoc) + 1);
	strcpy(pTemJsonDoc, pJsonDoc);

	char* property_data = LITE_json_string_value_strip_transfer(pProperty->key, pTemJsonDoc);	
	
    if(property_data != NULL){
		if(pProperty->type == TYPE_TEMPLATE_STRING){
			/*如果多个字符串属性,根据pProperty->key值匹配，处理字符串*/			
			Log_d("string type wait to be deal,%s", property_data);
		}else if(pProperty->type == TYPE_TEMPLATE_JOBJECT){
			Log_d("Json type wait to be deal,%s",property_data);	
		}
		
		HAL_Free(property_data);
    }else{
		
		rc = QCLOUD_ERR_FAILURE;
		Log_d("Property:%s no matched",pProperty->key);	
	}
	
	HAL_Free(pTemJsonDoc);
		
    return rc;
}

void OnDeltaTemplateCallback(void *pClient, const char *pJsonValueBuffer, uint32_t valueLength, DeviceProperty *pProperty) 
{
    int i = 0;

    for (i = 0; i < TOTAL_PROPERTY_COUNT; i++) {
		/*其他数据类型已经在_handle_delta流程统一处理了，字符串和json串需要在这里处理，因为只有产品自己才知道string/json的自定义解析*/
        if (strcmp(sg_DataTemplate[i].data_property.key, pProperty->key) == 0) {
            sg_DataTemplate[i].state = eCHANGED;
			if((sg_DataTemplate[i].data_property.type == TYPE_TEMPLATE_STRING)
				||(sg_DataTemplate[i].data_property.type == TYPE_TEMPLATE_JOBJECT)){

				update_self_define_value(pJsonValueBuffer, &(sg_DataTemplate[i].data_property));
			}
		
            Log_i("Property=%s changed", pProperty->key);
            sg_delta_arrived = true;
            return;
        }
    }

    Log_e("Property=%s changed no match", pProperty->key);
}


void OnShadowUpdateCallback(void *pClient, Method method, RequestAck requestAck, const char *pJsonDocument, void *pUserdata) {
	Log_i("recv shadow update response, response ack: %d", requestAck);
}


/**
 * 注册数据模板属性
 */
static int _register_data_template_property(void *pshadow_client)
{
	int i,rc;
	
    for (i = 0; i < TOTAL_PROPERTY_COUNT; i++) {
	    rc = IOT_Shadow_Register_Property(pshadow_client, &sg_DataTemplate[i].data_property, OnDeltaTemplateCallback);
	    if (rc != QCLOUD_ERR_SUCCESS) {
	        rc = IOT_Shadow_Destroy(pshadow_client);
	        Log_e("register device data template property failed, err: %d", rc);
	        return rc;
	    } else {
	        Log_i("data template property=%s registered.", sg_DataTemplate[i].data_property.key);
	    }
    }

	return QCLOUD_ERR_SUCCESS;
}


/*用户需要实现的下行数据的业务逻辑,待用户实现*/
void deal_down_stream_user_logic(ProductDataDefine   * pData)
{
	Log_d("someting about your own product logic wait to be done");
}

/*用户需要实现的上行数据的业务逻辑,此处仅供示例*/
int deal_up_stream_user_logic(DeviceProperty *pReportDataList[], int *pCount)
{
	int i, j;
	
     for (i = 0, j = 0; i < TOTAL_PROPERTY_COUNT; i++) {       
        if(eCHANGED == sg_DataTemplate[i].state) {
            pReportDataList[j++] = &(sg_DataTemplate[i].data_property);
			sg_DataTemplate[i].state = eNOCHANGE;
        }
    }
	*pCount = j;

	return (*pCount > 0)?QCLOUD_ERR_SUCCESS:QCLOUD_ERR_FAILURE;
}

int main(int argc, char **argv) {
    int rc;
	DeviceProperty *pReportDataList[TOTAL_PROPERTY_COUNT];
	int ReportCont;


    //init log level
    IOT_Log_Set_Level(DEBUG);

    //init connection
    ShadowInitParams init_params = DEFAULT_SHAWDOW_INIT_PARAMS;
    rc = _setup_connect_init_params(&init_params);
    if (rc != QCLOUD_ERR_SUCCESS) {
		Log_e("init params err,rc=%d", rc);
		return rc;
	}

    void *client = IOT_Shadow_Construct(&init_params);
    if (client != NULL) {
        Log_i("Cloud Device Construct Success");
    } else {
        Log_e("Cloud Device Construct Failed");
        return QCLOUD_ERR_FAILURE;
    }

    //init data template
    _init_data_template();
	
#ifdef EVENT_POST_ENABLED
	rc = event_init(client);
	if (rc < 0) 
	{
		Log_e("event init failed: %d", rc);
		return rc;
	}
#endif

    //register data template propertys here
    rc = _register_data_template_property(client);
    if (rc == QCLOUD_ERR_SUCCESS) {
        Log_i("Register data template propertys Success");
    } else {
        Log_e("Register data template propertys Failed: %d", rc);
        return rc;
    }

	//同步离线数据
	rc = IOT_Shadow_Get_Sync(client, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);
	if (rc != QCLOUD_ERR_SUCCESS) {
		Log_e("get device shadow failed, err: %d", rc);
		return rc;
	}


    while (IOT_Shadow_IsConnected(client) || rc == QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT 
		|| rc == QCLOUD_ERR_MQTT_RECONNECTED || QCLOUD_ERR_SUCCESS == rc) {

        rc = IOT_Shadow_Yield(client, 200);

        if (rc == QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT) {
            HAL_SleepMs(1000);
            continue;
        }
		else if (rc != QCLOUD_ERR_SUCCESS) {
			Log_e("Exit loop caused of errCode: %d", rc);
		}


		/*服务端下行消息，业务处理逻辑1入口*/
		if (sg_delta_arrived) {	
			
			deal_down_stream_user_logic(&sg_ProductData);
			
			/*业务逻辑处理完后需要同步通知服务端:设备数据已更新，删除dseire数据*/	
            int rc = IOT_Shadow_JSON_ConstructDesireAllNull(client, sg_data_report_buffer, sg_data_report_buffersize);
            if (rc == QCLOUD_ERR_SUCCESS) {
                rc = IOT_Shadow_Update_Sync(client, sg_data_report_buffer, sg_data_report_buffersize, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);
                sg_delta_arrived = false;
                if (rc == QCLOUD_ERR_SUCCESS) {
                    Log_i("shadow update(desired) success");
                } else {
                    Log_e("shadow update(desired) failed, err: %d", rc);
                }

            } else {
                Log_e("construct desire failed, err: %d", rc);
            }
		}	else{
			Log_d("No delta msg received...");
		}

		/*设备上行消息,业务逻辑2入口*/
		/*delta消息是属性的desire和属性的report的差异集，收到deseire消息处理后，要report属性的状态*/
		if(QCLOUD_ERR_SUCCESS == deal_up_stream_user_logic(pReportDataList, &ReportCont)){
			
			rc = IOT_Shadow_JSON_ConstructReportArray(client, sg_data_report_buffer, sg_data_report_buffersize, ReportCont, pReportDataList);
	        if (rc == QCLOUD_ERR_SUCCESS) {
	            rc = IOT_Shadow_Update(client, sg_data_report_buffer, sg_data_report_buffersize, 
	                    OnShadowUpdateCallback, NULL, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);
	            if (rc == QCLOUD_ERR_SUCCESS) {
	                Log_i("shadow update(reported) success");
	            } else {
	                Log_e("shadow update(reported) failed, err: %d", rc);
	            }
	        } else {
	            Log_e("construct reported failed, err: %d", rc);
	        }

		}else{
			 Log_d("no data need to be reported");
		}

#ifdef EVENT_POST_ENABLED	
		eventPostCheck(client);
#endif

        HAL_SleepMs(3000);
    }

    rc = IOT_Shadow_Destroy(client);

    return rc;
}
