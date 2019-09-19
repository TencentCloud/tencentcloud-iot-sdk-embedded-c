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
#include "qcloud_iot_import.h"



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

/*To enable event timesamp, accurate UTC timestamp in millisecond is required */
#define  EVENT_TIMESTAMP_USED				


/**
 * @brief EVENT method type
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
	char 	 *event_name;		 // event name
	char 	 *type;			     // event type
    uint32_t timestamp;			 // event timestamp
	uint8_t  eventDataNum;		 // number of event properties
    DeviceProperty *pEventData;  // event properties
} sEvent;

/**
 * @brief Callback for event reply
 */
typedef void (*OnEventReplyCallback)(void *pClient,  MQTTMessage *messag);

typedef struct _sReply_{
    char       client_token[EVENT_TOKEN_MAX_LEN];               // clientToken for this event reply
    void       *user_context;                                   // user context
    Timer      timer;                                           // timer for request timeout

    OnEventReplyCallback      callback;                         // callback for this event reply
} sReply;


/**
 * @brief Set event flag
 *
 * @param  flag event set
 */
void setEventFlag(uint32_t flag);

/**
 * @brief Clear event flag
 *
 * @param  flag event set
 */
void clearEventFlag(uint32_t flag);

/**
 * @brief Get event set with flag
 *
 * @return event set with flag
 */
uint32_t getEventFlag(void);

/**
 * @brief init event feature
 *
 * @param c    shadow pointer
 */
int event_init(void *c);

/**
 * @brief Report event to server, from event array
 *
 * @param pClient       reference to MQTT shadow client
 * @param pJsonDoc      buffer for constructing event JSON
 * @param sizeOfBuffer  Size of buffer for constructing event JSON
 * @param event_count   number of events to report
 * @param pEventArry	Array of events to report
 * @param replyCb	    callback for event reply
 *
 * @return QCLOUD_RET_SUCCESS for success, or err code for failure
 */
int qcloud_iot_post_event(void *pClient, char *pJsonDoc, size_t sizeOfBuffer, uint8_t event_count, sEvent *pEventArry[], OnEventReplyCallback replyCb);                                            

/**
 * @brief Report event to server, from event JSON msg
 *
 * @param pClient       reference to MQTT shadow client
 * @param pJsonDoc      buffer for constructing event JSON
 * @param sizeOfBuffer  Size of buffer for constructing event JSON
 * @param pEventMsg     event JSON msg to report 
 *  event JSON format:
 *  single event:
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
 *  multi events:
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
 *
 * @param replyCb	  callback for event reply 
 *
 * @return QCLOUD_RET_SUCCESS for success, or err code for failure	  
 */
int qcloud_iot_post_event_raw(void *pClient, char *pJsonDoc, size_t sizeOfBuffer, char *pEventMsg, OnEventReplyCallback replyCb);                                            


#ifdef __cplusplus
}
#endif

#endif //_QCLOUD_IOT_EVENT_H_