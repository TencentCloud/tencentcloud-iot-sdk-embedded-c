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

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "qcloud_iot_sdk_impl_internal.h"
#include "qcloud_iot_export_event.h"
#include "shadow_client.h"
#include "lite-utils.h"


static sReply  g_eventWaitReplylist[MAX_EVENT_WAIT_REPLY];
static uint32_t sg_flags = 0;

void setEventFlag(uint32_t flag)
{
	sg_flags |= flag&0xffffffff;
}

void clearEventFlag(uint32_t flag)
{
	sg_flags &= (~flag)&0xffffffff;
}

uint32_t getEventFlag(void)
{
	return sg_flags;
}


sReply * _find_empty_reply_request(sReply *p_replylist) 
{

	int i;

	if(!p_replylist){
		Log_e("null input");
		return NULL;
	}

    for (i = 0; i < MAX_EVENT_WAIT_REPLY; i++) {
        if (strlen(p_replylist[i].client_token) == 0) {
            return &p_replylist[i];
        }
    }

	Log_e("event request list is full");
    for (i = 0; i < MAX_EVENT_WAIT_REPLY; i++) {
        Log_d("eventToken[%d] wait for reply", p_replylist[i].client_token);
    }
	
    return NULL;
}

void _release_reply_request(sReply *  p_reply) 
{
	POINTER_SANITY_CHECK_RTN(p_reply);

    Log_d("eventToken[%s] released", p_reply->client_token);
    memset(p_reply->client_token, 0, EVENT_TOKEN_MAX_LEN);
	p_reply->callback = NULL;
	
    return ;
}


static void _on_event_reply_callback(void *pClient, MQTTMessage *message, void *userData) 
{
	POINTER_SANITY_CHECK_RTN(message);

	int32_t code;
	sReply *pReply = g_eventWaitReplylist;;
    char* client_token = NULL;
	char* status = NULL;
	int i;

	Log_i("Receive Message With topicName:%.*s, payload:%.*s",(int) message->topic_len, message->ptopic, (int) message->payload_len, (char *) message->payload);

	// 解析事件回复中的clientToken
    if (!parse_client_token((char *) message->payload, &client_token)) {
        Log_e("fail to parse client token!");
        return;
    }

	// 解析事件回复中的处理结果
	if(!parse_code_return((char *) message->payload, &code)){
        Log_e("fail to parse code");
        return;
    }

	if(!parse_status_return((char *) message->payload, &status)){
       // Log_d("no status return");
    }


	Log_d("eventToken:%s code:%d status:%s", client_token, code, status);
	
	for (i = 0; i < MAX_EVENT_WAIT_REPLY; i++) {			
		if(pReply[i].client_token[0] == '\0'){
		 	continue;
		}
		
		if(!strcmp(client_token, pReply[i].client_token)){
			if(NULL != pReply[i].callback){				
				pReply[i].callback(pClient, message);
			}
			_release_reply_request(&pReply[i]);
			break;
		}else if(expired(&pReply[i].timer)){
			Log_e("eventToken[%s] timeout",pReply[i].client_token);
		}
	}
	
	return;
}



/**
* @brief 检查函数snprintf的返回值
*
* @param returnCode		函数snprintf的返回值
* @param maxSizeOfWrite	可写最大字节数
* @return 				返回QCLOUD_ERR_JSON, 表示出错; 返回QCLOUD_ERR_JSON_BUFFER_TRUNCATED, 表示截断
*/
static inline int _check_snprintf_return(int32_t returnCode, size_t maxSizeOfWrite) {

	if (returnCode >= maxSizeOfWrite) {
		return QCLOUD_ERR_JSON_BUFFER_TRUNCATED;
	} else if (returnCode < 0) { // 写入出错
		return QCLOUD_ERR_JSON;
	}

	return QCLOUD_ERR_SUCCESS;
}

 
 static int _IOT_Event_JSON_Init(void *handle, char *jsonBuffer, size_t sizeOfBuffer, uint8_t event_count, OnEventReplyCallback replyCb, uint32_t reply_timeout_ms)
{
	POINTER_SANITY_CHECK(jsonBuffer, QCLOUD_ERR_INVAL);
	
	int32_t rc_of_snprintf = 0;
	Qcloud_IoT_Shadow* pshadow = (Qcloud_IoT_Shadow*)handle;
	sReply *pReply;
	
	memset(jsonBuffer, 0, sizeOfBuffer);	

	pReply = _find_empty_reply_request(g_eventWaitReplylist);
	if(!pReply){
		Log_e("no reply resource");
		return QCLOUD_ERR_FAILURE;
	}
	pReply->callback = replyCb;
	InitTimer(&pReply->timer);
	countdown_ms(&pReply->timer, reply_timeout_ms);
	
  
	memset(pReply->client_token, 0, EVENT_TOKEN_MAX_LEN);	
	HAL_Snprintf(pReply->client_token, MAX_SIZE_OF_JSON_WITH_CLIENT_TOKEN, "%s-%u", iot_device_info_get()->product_id, pshadow->inner_data.token_num++);
	if(event_count > SIGLE_EVENT){	
		rc_of_snprintf = HAL_Snprintf(jsonBuffer, sizeOfBuffer, "{\"method\":\"%s\", \"clientToken\":\"%s\", ", \
																		POST_EVENTS, pReply->client_token);
	}else{	
		rc_of_snprintf = HAL_Snprintf(jsonBuffer, sizeOfBuffer, "{\"method\":\"%s\", \"clientToken\":\"%s\", ", \
																		POST_EVENT, pReply->client_token);
	}
		
    return _check_snprintf_return(rc_of_snprintf, sizeOfBuffer);
 }

static int _IOT_Construct_Event_JSON(void *handle, char *jsonBuffer, size_t sizeOfBuffer,
														uint8_t event_count, 
														sEvent *pEventArry[],
														OnEventReplyCallback replyCb, 
														uint32_t reply_timeout_ms) 
{	 
	 size_t remain_size = 0;
	 int32_t rc_of_snprintf = 0;
	 uint8_t i,j;
	 Qcloud_IoT_Shadow* pshadow = (Qcloud_IoT_Shadow*)handle;
	 
	 POINTER_SANITY_CHECK(pshadow, QCLOUD_ERR_INVAL);
	 POINTER_SANITY_CHECK(jsonBuffer, QCLOUD_ERR_INVAL);
	 POINTER_SANITY_CHECK(pEventArry, QCLOUD_ERR_INVAL);

	 int rc = _IOT_Event_JSON_Init(pshadow, jsonBuffer, sizeOfBuffer, event_count, replyCb, reply_timeout_ms);
 
	 if (rc != QCLOUD_ERR_SUCCESS) {
		 Log_e("event json init failed: %d", rc);
		 return rc;
	 }
	 //Log_d("event_count:%d, Doc_init:%s",event_count, jsonBuffer);
 
	 if ((remain_size = sizeOfBuffer - strlen(jsonBuffer)) <= 1) {
		 return QCLOUD_ERR_JSON_BUFFER_TOO_SMALL;
	 }

	if(event_count > SIGLE_EVENT){//多个事件
		rc_of_snprintf = HAL_Snprintf(jsonBuffer + strlen(jsonBuffer), remain_size, "\"events\":[");
		rc = _check_snprintf_return(rc_of_snprintf, remain_size);			
		if (rc != QCLOUD_ERR_SUCCESS) {
				return rc;
		}

		if ((remain_size = sizeOfBuffer - strlen(jsonBuffer)) <= 1) {
			return QCLOUD_ERR_JSON_BUFFER_TOO_SMALL;
		}

		for(i = 0; i < event_count; i++){
			sEvent *pEvent = pEventArry[i];
			if(NULL == pEvent){
				Log_e("%dth/%d null event", i, event_count);
				return QCLOUD_ERR_INVAL;
			}
			rc_of_snprintf = HAL_Snprintf(jsonBuffer + strlen(jsonBuffer), remain_size, "{\"eventId\":\"%s\", \"type\":\"%s\", \"timestamp\":%u000, \"params\":{",\
											pEvent->event_name, pEvent->type, pEvent->timestamp);
			rc = _check_snprintf_return(rc_of_snprintf, remain_size);			
			if (rc != QCLOUD_ERR_SUCCESS) {
					return rc;
			}

			if ((remain_size = sizeOfBuffer - strlen(jsonBuffer)) <= 1) {
				return QCLOUD_ERR_JSON_BUFFER_TOO_SMALL;
			}

			DeviceProperty *pJsonNode = pEvent->pEventData;
		    for (j = 0; j < pEvent->eventDataNum; j++) {
		        if (pJsonNode != NULL && pJsonNode->key != NULL) {
		            rc = event_put_json_node(jsonBuffer, remain_size, pJsonNode->key, pJsonNode->data, pJsonNode->type);

		            if (rc != QCLOUD_ERR_SUCCESS) {
		                return rc;
		            }
		        } else {
		        	Log_e("%dth/%d null event property data", i, pEvent->eventDataNum);
		            return QCLOUD_ERR_INVAL;
		        }
				pJsonNode++;
	    	}	
			if ((remain_size = sizeOfBuffer - strlen(jsonBuffer)) <= 1) {
				return QCLOUD_ERR_JSON_BUFFER_TOO_SMALL;
			}
			
			rc_of_snprintf = HAL_Snprintf(jsonBuffer + strlen(jsonBuffer)-1, remain_size, "}}," );

			rc = _check_snprintf_return(rc_of_snprintf, remain_size);
			if (rc != QCLOUD_ERR_SUCCESS) {
					return rc;
			}

			pEvent++;	
		}

		if ((remain_size = sizeOfBuffer - strlen(jsonBuffer)) <= 1) {
			return QCLOUD_ERR_JSON_BUFFER_TOO_SMALL;
		}
		rc_of_snprintf = HAL_Snprintf(jsonBuffer + strlen(jsonBuffer) - 1, remain_size, "]");
		rc = _check_snprintf_return(rc_of_snprintf, remain_size);
		if (rc != QCLOUD_ERR_SUCCESS) {
				return rc;
		}

	}else{ //单个事件		
		sEvent *pEvent = pEventArry[0];
		rc_of_snprintf = HAL_Snprintf(jsonBuffer + strlen(jsonBuffer), remain_size, "\"eventId\":\"%s\", \"type\":\"%s\", \"timestamp\":%u000, \"params\":{",\
										pEvent->event_name, pEvent->type, pEvent->timestamp);

		rc = _check_snprintf_return(rc_of_snprintf, remain_size);
		if (rc != QCLOUD_ERR_SUCCESS) {
				return rc;
		}

		if ((remain_size = sizeOfBuffer - strlen(jsonBuffer)) <= 1) {
			return QCLOUD_ERR_JSON_BUFFER_TOO_SMALL;
		}

		DeviceProperty *pJsonNode = pEvent->pEventData;
	    for (i = 0; i < pEvent->eventDataNum; i++) {			
	        if (pJsonNode != NULL && pJsonNode->key != NULL) {
	            rc = event_put_json_node(jsonBuffer, remain_size, pJsonNode->key, pJsonNode->data, pJsonNode->type);

	            if (rc != QCLOUD_ERR_SUCCESS) {
	                return rc;
	            }
	        } else {
				Log_e("%dth/%d null event property data", i, pEvent->eventDataNum);
	            return QCLOUD_ERR_INVAL;
	        }
			pJsonNode++;
    	}	
		if ((remain_size = sizeOfBuffer - strlen(jsonBuffer)) <= 1) {
			return QCLOUD_ERR_JSON_BUFFER_TOO_SMALL;
		}
		
		rc_of_snprintf = HAL_Snprintf(jsonBuffer + strlen(jsonBuffer) - 1, remain_size, "}" );

		rc = _check_snprintf_return(rc_of_snprintf, remain_size);
		if (rc != QCLOUD_ERR_SUCCESS) {
				return rc;
		}
	}

	//finish json
	 if ((remain_size = sizeOfBuffer - strlen(jsonBuffer)) < 1) {
		 return QCLOUD_ERR_JSON_BUFFER_TOO_SMALL;
	 }
	 rc_of_snprintf = HAL_Snprintf(jsonBuffer + strlen(jsonBuffer), remain_size, "}");
	 
	 return _check_snprintf_return(rc_of_snprintf, remain_size);
}												
												
static int _publish_event_to_cloud(void *c, char *pJsonDoc)
{
	IOT_FUNC_ENTRY;
	int rc = QCLOUD_ERR_SUCCESS;
	char topic[MAX_SIZE_OF_CLOUD_TOPIC] = {0};
	Qcloud_IoT_Shadow *pShadow = (Qcloud_IoT_Shadow *)c;


	int size = HAL_Snprintf(topic, MAX_SIZE_OF_CLOUD_TOPIC, "$thing/up/event/%s/%s", iot_device_info_get()->product_id, iot_device_info_get()->device_name);
	if (size < 0 || size > sizeof(topic) - 1)
	{
		Log_e("topic content length not enough! content size:%d  buf size:%d", size, (int)sizeof(topic));
		return QCLOUD_ERR_FAILURE;
	}

	PublishParams pubParams = DEFAULT_PUB_PARAMS;
	pubParams.qos = QOS0;
	pubParams.payload_len = strlen(pJsonDoc);
	pubParams.payload = (char *) pJsonDoc;

	rc = IOT_MQTT_Publish(pShadow->mqtt, topic, &pubParams);

	IOT_FUNC_EXIT_RC(rc);
}
												

int event_init(void *c)
{
	static char topic_name[MAX_SIZE_OF_CLOUD_TOPIC] = {0};
	Qcloud_IoT_Shadow *shadow_client = (Qcloud_IoT_Shadow *)c;

	//init reply list
	memset((char *)g_eventWaitReplylist, 0, MAX_EVENT_WAIT_REPLY*sizeof(sReply));
	

	int size = HAL_Snprintf(topic_name, MAX_SIZE_OF_CLOUD_TOPIC, "$thing/down/event/%s/%s", iot_device_info_get()->product_id, iot_device_info_get()->device_name);

	
	if (size < 0 || size > sizeof(topic_name) - 1)
	{
		Log_e("topic content length not enough! content size:%d  buf size:%d", size, (int)sizeof(topic_name));
		return QCLOUD_ERR_FAILURE;
	}
	SubscribeParams sub_params = DEFAULT_SUB_PARAMS;
	sub_params.on_message_handler = _on_event_reply_callback;
	
	return IOT_MQTT_Subscribe(shadow_client->mqtt, topic_name, &sub_params);
}


int qcloud_iot_post_event(void *pClient, char *pJsonDoc, size_t sizeOfBuffer, uint8_t event_count, sEvent *pEventArry[], OnEventReplyCallback replyCb) 									                                           
{
	int rc;
	
	rc = _IOT_Construct_Event_JSON(pClient, pJsonDoc, sizeOfBuffer, event_count, pEventArry, replyCb, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);
	if (rc != QCLOUD_ERR_SUCCESS) {
		Log_e("construct event json fail, %d",rc);
		return rc;
	}

	//Log_d("JsonDoc:%s", pJsonDoc);
		
	rc = _publish_event_to_cloud(pClient, pJsonDoc);	
	if (rc < 0) {
		Log_e("publish event to cloud fail, %d",rc);
	}

	return rc;
}


int qcloud_iot_post_event_raw(void *pClient, char *pJsonDoc, size_t sizeOfBuffer, char *pEventMsg, OnEventReplyCallback replyCb)                                            
{
	int rc;
	size_t remain_size = 0;
	int32_t rc_of_snprintf;

	Qcloud_IoT_Shadow* pshadow = (Qcloud_IoT_Shadow*)pClient;

	POINTER_SANITY_CHECK(pshadow, QCLOUD_ERR_INVAL);
	POINTER_SANITY_CHECK(pJsonDoc, QCLOUD_ERR_INVAL);
	POINTER_SANITY_CHECK(pEventMsg, QCLOUD_ERR_INVAL);

	rc = _IOT_Event_JSON_Init(pshadow, pJsonDoc, sizeOfBuffer, MUTLTI_EVENTS, replyCb, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);
	if (rc != QCLOUD_ERR_SUCCESS) {
		Log_e("event json init failed: %d", rc);
		return rc;
	}

	if ((remain_size = sizeOfBuffer - strlen(pJsonDoc)) <= 1) {
		return QCLOUD_ERR_JSON_BUFFER_TOO_SMALL;
	}

	rc_of_snprintf = HAL_Snprintf(pJsonDoc + strlen(pJsonDoc), remain_size, "\"events\":[%s]}", pEventMsg);
	rc = _check_snprintf_return(rc_of_snprintf, remain_size);			
	if (rc != QCLOUD_ERR_SUCCESS) {
			return rc;
	}

	Log_d("JsonDoc:%s", pJsonDoc);
	
	rc = _publish_event_to_cloud(pClient, pJsonDoc);	
	if (rc < 0) {
		Log_e("publish event raw to cloud fail, %d",rc);
	}

	return rc;
}


#ifdef __cplusplus
}
#endif
