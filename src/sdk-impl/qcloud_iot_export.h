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

#ifndef QCLOUD_IOT_EXPORT_H_
#define QCLOUD_IOT_EXPORT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* IoT C-SDK APPID */
#define QCLOUD_IOT_DEVICE_SDK_APPID                                	"21010406"

/* MQTT心跳消息发送周期, 单位:ms */
#define QCLOUD_IOT_MQTT_KEEP_ALIVE_INTERNAL                         (240 * 1000)

/* MQTT 阻塞调用(包括连接, 订阅, 发布等)的超时时间, 单位:ms 建议5000ms */
#define QCLOUD_IOT_MQTT_COMMAND_TIMEOUT                             (5 * 1000)

/* TLS连接握手超时时间, 单位:ms */
#define QCLOUD_IOT_TLS_HANDSHAKE_TIMEOUT                            (5 * 1000)

/* 设备ID的最大长度, 必须保持唯一 */
#define MAX_SIZE_OF_CLIENT_ID                                       (80)

/* 产品名称的最大长度 */
#define MAX_SIZE_OF_PRODUCT_ID                                    	(10)

/* 设备ID的最大长度 */
#define MAX_SIZE_OF_DEVICE_NAME                                     (64)

/* MQTT消息发送buffer大小, 支持最大256*1024 */
#define QCLOUD_IOT_MQTT_TX_BUF_LEN                                  (512)

/* MQTT消息接收buffer大小, 支持最大256*1024 */
#define QCLOUD_IOT_MQTT_RX_BUF_LEN                                  (512)

/* COAP 发送消息buffer大小，最大支持64*1024字节 */
#define COAP_SENDMSG_MAX_BUFLEN                   					(512)

/* COAP 接收消息buffer大小，最大支持64*1024字节 */
#define COAP_RECVMSG_MAX_BUFLEN                   					(512)

/* 重连最大等待时间 */
#define MAX_RECONNECT_WAIT_INTERVAL 								(60 * 1000)

/* 最大接入有效时间偏移，时间毫秒，0：表示永不过期；>0：表示在当前时间+timeout与后台对时*/
#define MAX_ACCESS_EXPIRE_TIMEOUT									(0)

/* MQTT连接域名 */
#define QCLOUD_IOT_MQTT_DIRECT_DOMAIN        						"iotcloud.tencentdevices.com"

/* CoAP连接域名 */
#define QCLOUD_IOT_COAP_DEIRECT_DOMAIN								"iotcloud.tencentdevices.com"

typedef struct {
	char	product_id[MAX_SIZE_OF_PRODUCT_ID + 1];
	char 	device_name[MAX_SIZE_OF_DEVICE_NAME + 1];
	char	client_id[MAX_SIZE_OF_CLIENT_ID + 1];
} DeviceInfo;

#include "qcloud_iot_export_coap.h"
#include "qcloud_iot_export_log.h"
#include "qcloud_iot_export_error.h"
#include "qcloud_iot_export_mqtt.h"
#include "qcloud_iot_export_shadow.h"
#include "qcloud_iot_export_ota.h"
#include "qcloud_iot_export_system.h"
#include "qcloud_iot_export_nbiot.h"

#ifdef __cplusplus
}
#endif

#endif /* QCLOUD_IOT_EXPORT_H_ */
