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

#ifndef _QCLOUD_IOT_EVENT_H_
#define _QCLOUD_IOT_EVENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>

#include "qcloud_iot_export_shadow.h"





#define NAME_MAX_LEN			(32)
#define TYPE_MAX_LEN			(32)
#define EVENT_TOKEN_MAX_LEN		(32)
#define SIGLE_EVENT			    (1) 
#define MUTLTI_EVENTS		    (2)


#define MAX_EVENT_WAIT_REPLY    (10) 
#define EVENT_MAX_DATA_NUM		(255)

#define EVENT_SDK_VERSION		"1.0"
#define POST_EVENT				"event_post"
#define POST_EVENTS				"events_post"
#define REPLY_EVENT				"event_reply"

#define TYPE_STR_INFO			"info"
#define TYPE_STR_ALERT			"alert"
#define TYPE_STR_FAULT			"fault"

#define  FLAG_EVENT0 			(1U<<0)
#define  FLAG_EVENT1			(1U<<1)
#define  FLAG_EVENT2			(1U<<2)
#define  FLAG_EVENT3			(1U<<3)
#define  FLAG_EVENT4 			(1U<<4)
#define  FLAG_EVENT5			(1U<<5)
#define  FLAG_EVENT6			(1U<<6)
#define  FLAG_EVENT7			(1U<<7)
#define  FLAG_EVENT8 			(1U<<8)
#define  FLAG_EVENT9			(1U<<9)

#define  ALL_EVENTS_MASK		(0xFFFFFFFF)

/*如果使用事件时间戳，必须保证时间戳是准确的UTC时间ms，否则会判断为错误*/
#define  EVENT_TIMESTAMP_USED				


/**
 * @brief EVENT操作方法，上报和回复
 */
typedef enum {
    eEVENT_POST,    
    eEVENT_REPLY, 
} eEventMethod;

typedef enum {
	eEVENT_INFO,
	eEVENT_ALERT,    
    eEVENT_FAULT, 
}eEventType;
	
typedef struct  _sEvent_{
	char 	 *event_name;		 //事件名称	
	char 	 *type;			 //事件类型	
    uint32_t timestamp;			 //事件时戳	
	uint8_t eventDataNum;		 //事件属性点个数
    DeviceProperty *pEventData;  //事件属性点
} sEvent;

/**
 * @brief 事件上报回复回调。
 *
 * @param pJsonDocument    事件响应返回的文档
 *
 */

typedef void (*OnEventReplyCallback)(void *pClient,  MQTTMessage *messag);

typedef struct _sReply_{
    char       client_token[EVENT_TOKEN_MAX_LEN];               // 标识该请求的clientToken字段
    void       *user_context;                                   // 用户数据
    Timer      timer;                                           // 请求超时定时器

    OnEventReplyCallback      callback;                         // 事件上报回复回调
} sReply;


/**
 * @brief 设置事件标记
 *
 * @param  flag  设置发生的事件集
 */
void setEventFlag(uint32_t flag);

/**
 * @brief 清除事件标记
 *
 * @param  flag  待清除的事件集
 */
void clearEventFlag(uint32_t flag);

/**
 * @brief 获取已置位的事件集
 *
 * @return 已置位的事件集
 */
uint32_t getEventFlag(void);

/**
 * @brief 事件client初始化，使用事件功能前需先调用
 *
 * @param c    shadow 实例指针
 */
int event_init(void *c);

/**
 * @brief 事件上报，传入事件数组，SDK完成事件的json格式封装
 * @param pClient shadow 实例指针
 * @param pJsonDoc    用于构建json格式上报信息的buffer
 * @param sizeOfBuffer    用于构建json格式上报信息的buffer大小
 * @param event_count     待上报的事件个数
 * @param pEventArry	  待上报的事件数组指
 * @param replyCb	  事件回复消息的回调 
 * @return @see IoT_Error_Code	  
 */
int qcloud_iot_post_event(void *pClient, char *pJsonDoc, size_t sizeOfBuffer, uint8_t event_count, sEvent *pEventArry[], OnEventReplyCallback replyCb);                                            

/**
 * @brief 事件上报，用户传入已构建好的事件的json格式，SDK增加事件头部即上报
 * @param pClient shadow 实例指针
 * @param pJsonDoc    用于构建json格式上报信息的buffer
 * @param sizeOfBuffer    用于构建json格式上报信息的buffer大小
 * @param pEventMsg     待上报的事件json信息 
 *  json事件格式：
 *  单个事件：
 *	 {"method": "event_post",
 *		"clientToken": "123",
 *		"version": "1.0",
 *		"eventId": "PowerAlarm",
 *		"type": "fatal",
 *		"timestamp": 1212121221,
 *		"params": {
 *			"Voltage": 2.8,
 *			"Percent": 20
 *		}
 *	}
 *
 *  多个事件：
 *	 {
 *		 "eventId": "PowerAlarm",
 *		 "type": "fatal",
 *		 "timestamp": 1212121221,
 *		 "params": {
 *			 "Voltage": 2.8,
 *			 "Percent": 20
 *		 }
 *	 },
 *	 {
 *		 "name": "PowerAlarm",
 *		 "type": "fatal",
 *		 "timestamp": 1212121223,
 *		 "params": {
 *			 "Voltage": 2.1,
 *			 "Percent": 10
 *		 }
 *	 },
 *   ....
 *
 * @param replyCb	  事件回复消息的回调 
 * @return @see IoT_Error_Code	  
 */
int qcloud_iot_post_event_raw(void *pClient, char *pJsonDoc, size_t sizeOfBuffer, char *pEventMsg, OnEventReplyCallback replyCb);                                            


#ifdef __cplusplus
}
#endif

#endif //IOT_SHADOW_CLIENT_JSON_H_
