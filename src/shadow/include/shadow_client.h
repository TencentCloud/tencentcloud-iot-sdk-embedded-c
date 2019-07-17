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

/**
 * @brief 设备影子文档操作相关的一些接口
 *
 * 这里提供一些接口用于管理设备影子文档或与设备影子文档进行交互; 通过DeviceName,
 * 可以与设备影子进行交互, 包括当前设备的设备影子和其他设备的设备影子; 一
 * 个设备一共有三种不同操作与设备影子交互:
 *     1. Get
 *     2. Update
 *     3. Delete
 *
 * 以上三种操作, 底层还是基于MQTT协议, 工作原理也是基于发布/订阅模型, 当执行
 * 上述操作是, 会收到相应的响应: 1. accepted; 2. rejected。例如, 我们执行
 * Get与设备影子进行交互, 设备端将发送和接收到一下信息:
 *     1. 发布MQTT主题: $shadow/get/{productName}/{deviceName};
 *     2. 订阅MQTT主题: $shadow/get/accepted/{productName}/{deviceName} 和 $shadow/get/rejected/{productName}/{deviceName}
 *     3. 如果整个请求成功的话, 设备端会收到accepted主题, 以及相应设备的json文档。
 */
#ifndef IOT_SHADOW_CLIENT_H_
#define IOT_SHADOW_CLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"
#include "qcloud_iot_sdk_impl_internal.h"
#include "device.h"
#include "mqtt_client.h"
#include "shadow_client_json.h"

/* 在任意给定时间内, 处于appending状态的请求最大个数 */
#define MAX_APPENDING_REQUEST_AT_ANY_GIVEN_TIME                     (10)

/* 一个clientToken的最大长度 */
#define MAX_SIZE_OF_CLIENT_TOKEN                                    (MAX_SIZE_OF_CLIENT_ID + 10)

/* 一个仅包含clientToken字段的JSON文档的最大长度 */
#define MAX_SIZE_OF_JSON_WITH_CLIENT_TOKEN                          (MAX_SIZE_OF_CLIENT_TOKEN + 20)

/* 在任意给定时间内, 可以同时操作设备的最大个数 */
#define MAX_DEVICE_HANDLED_AT_ANY_GIVEN_TIME                        (10)

/* 除设备名称之外, 云端保留主题的最大长度 */
#define MAX_SIZE_OF_CLOUD_TOPIC_WITHOUT_DEVICE_NAME                 (60)

/* 接收云端返回的JSON文档的buffer大小 */
#define CLOUD_IOT_JSON_RX_BUF_LEN                                   (QCLOUD_IOT_MQTT_RX_BUF_LEN + 1)

/**
 * @brief 文档操作请求的参数结构体定义
 */
typedef struct _RequestParam {

    Method               	method;              	// 文档请求方式: GET, UPDATE, DELETE

    uint32_t             	timeout_sec;         	// 请求超时时间, 单位:s

    OnRequestCallback    	request_callback;    	// 请求回调方法

    void                 	*user_context;          // 用户数据, 会通过回调方法OnRequestCallback返回

} RequestParams;

#define DEFAULT_REQUEST_PARAMS {GET, 4, NULL, NULL};

/**
 * @brief 该结构体用于保存已登记的设备属性及设备属性处理的回调方法
 */
typedef struct {

    void *property;							// 设备属性

    OnPropRegCallback callback;      // 回调处理函数

} PropertyHandler;

typedef struct _ShadowInnerData {
    uint32_t token_num;
    int32_t sync_status;
    List *request_list;
    List *property_handle_list;
    char *result_topic;
} ShadowInnerData;

typedef struct _Shadow {
    void *mqtt;
    void *mutex;
	eShadowType shadow_type;
    MQTTEventHandler event_handle;
    ShadowInnerData inner_data;
} Qcloud_IoT_Shadow;

int qcloud_iot_shadow_init(Qcloud_IoT_Shadow *pShadow);

void qcloud_iot_shadow_reset(void *pClient);

/**
 * @brief 处理请求队列中已经超时的请求
 * 
 * @param pShadow   shadow client
 */
void handle_expired_request(Qcloud_IoT_Shadow *pShadow);

/**
 * @brief 所有的云端设备文档操作请求, 通过该方法进行中转分发
 *
 * @param pShadow       shadow client
 * @param pParams  		请求参数
 * @param pJsonDoc 		请求文档
 * @param sizeOfBuffer 	文档缓冲区大小
 * @return         		返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int do_shadow_request(Qcloud_IoT_Shadow *pShadow, RequestParams *pParams, char *pJsonDoc, size_t sizeOfBuffer);

/**
 * @brief 订阅设备影子操作结果topic
 *
 * @param pShadow       shadow client
 * @param pParams  		请求参数
 * @param pJsonDoc 		请求文档
 * @param sizeOfBuffer 	文档缓冲区大小
 * @return         		返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int subscribe_operation_result_to_cloud(Qcloud_IoT_Shadow *pShadow);

#ifdef __cplusplus
}
#endif

#endif /* IOT_SHADOW_CLIENT_H_ */
