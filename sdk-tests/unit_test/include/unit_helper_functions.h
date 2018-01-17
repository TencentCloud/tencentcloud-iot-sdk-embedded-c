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

#ifndef MQTT_CLIENT_C_UNIT_HELPER_FUNCTIONS_H
#define MQTT_CLIENT_C_UNIT_HELPER_FUNCTIONS_H

#include "mqtt_client.h"
#include "iot_unit_config.h"


#define QCLOUD_TOPIC_PATH "/"//"$qcloud/devices/"
#define SHADOW_TOPIC "$shadow/"
#define ACCEPTED_TOPIC "accepted/"
#define REJECTED_TOPIC "rejected/"
#define UPDATE_TOPIC "update/"
#define GET_TOPIC "get/"
#define DELETE_TOPIC "delete/"


#define GET_ACCEPTED_TOPIC  SHADOW_TOPIC GET_TOPIC ACCEPTED_TOPIC QCLOUD_IOT_MY_PRODUCT_ID QCLOUD_TOPIC_PATH QCLOUD_IOT_MY_DEVICE_NAME 
#define GET_REJECTED_TOPIC  SHADOW_TOPIC GET_TOPIC REJECTED_TOPIC QCLOUD_IOT_MY_PRODUCT_ID QCLOUD_TOPIC_PATH QCLOUD_IOT_MY_DEVICE_NAME 
#define GET_PUB_TOPIC       SHADOW_TOPIC GET_TOPIC QCLOUD_IOT_MY_PRODUCT_ID QCLOUD_TOPIC_PATH QCLOUD_IOT_MY_DEVICE_NAME 

#define DELETE_ACCEPTED_TOPIC SHADOW_TOPIC DELETE_TOPIC ACCEPTED_TOPIC QCLOUD_IOT_MY_PRODUCT_ID QCLOUD_TOPIC_PATH QCLOUD_IOT_MY_DEVICE_NAME
#define DELETE_REJECTED_TOPIC SHADOW_TOPIC DELETE_TOPIC REJECTED_TOPIC QCLOUD_IOT_MY_PRODUCT_ID QCLOUD_TOPIC_PATH QCLOUD_IOT_MY_DEVICE_NAME  

#define UPDATE_ACCEPTED_TOPIC SHADOW_TOPIC UPDATE_TOPIC ACCEPTED_TOPIC QCLOUD_IOT_MY_PRODUCT_ID QCLOUD_TOPIC_PATH QCLOUD_IOT_MY_DEVICE_NAME 
#define UPDATE_REJECTED_TOPIC SHADOW_TOPIC UPDATE_TOPIC REJECTED_TOPIC QCLOUD_IOT_MY_PRODUCT_ID QCLOUD_TOPIC_PATH QCLOUD_IOT_MY_DEVICE_NAME

#define SIZE_OF_JSON_BUFFER 256


void MQTTInitParamsSetup(MQTTInitParams *pParams, bool enableAutoReconnect);

void ConnectMQTTParamsSetup_Detailed(MQTTConnectParams *connectParams, MQTTInitParams *initParams,
                                     QoS qos, bool isCleanSession, bool isWillMsgPresent, 
                                     char *pWillTopicName, char *pWillMessage, char *pUsername);

void SetupMQTTConnectInitParams(MQTTInitParams* initParams);

void SetupShadowConnectInitParams(ShadowInitParams* initParams);


void ConnectParamsSetup(MQTTConnectParams *connectParams, MQTTInitParams *initParams);

#endif //MQTT_CLIENT_C_UNIT_HELPER_FUNCTIONS_H
