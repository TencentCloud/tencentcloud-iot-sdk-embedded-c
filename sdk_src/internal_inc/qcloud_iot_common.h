/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2018-2020 THL A29 Limited, a Tencent company. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef QCLOUD_IOT_COMMON_H_
#define QCLOUD_IOT_COMMON_H_

/* IoT C-SDK APPID */
#define QCLOUD_IOT_DEVICE_SDK_APPID     "21010406"
#define QCLOUD_IOT_DEVICE_SDK_APPID_LEN (sizeof(QCLOUD_IOT_DEVICE_SDK_APPID) - 1)

#define QCLOUD_IOT_MQTT_DOMAIN_WITH_PREFIX 1

/* MQTT server domain */
#define QCLOUD_IOT_MQTT_COAP_DIRECT_DOMAIN     "iotcloud.tencentdevices.com"
#define QCLOUD_IOT_MQTT_COAP_US_EAST_DOMAIN    "us-east.iotcloud.tencentdevices.com"
#define QCLOUD_IOT_MQTT_COAP_EUROPE_DOMAIN     "europe.iothub.tencentdevices.com"
#define QCLOUD_IOT_MQTT_COAP_AP_BANGKOK_DOMAIN "ap-bangkok.iothub.tencentdevices.com"

#define MQTT_SERVER_PORT_TLS   8883
#define MQTT_SERVER_PORT_NOTLS 1883

#define COAP_SERVER_PORT 5684

/* WEBSOCKET MQTT server domain */
#define QCLOUD_IOT_WEBSOCKET_MQTT_DIRECT_DOMAIN     "ap-guangzhou.iothub.tencentdevices.com"
#define QCLOUD_IOT_WEBSOCKET_MQTT_US_EAST_DOMAIN    "us-east.iothub.tencentdevices.com"
#define QCLOUD_IOT_WEBSOCKET_MQTT_EUROPE_DOMAIN     "europe.iothub.tencentdevices.com"
#define QCLOUD_IOT_WEBSOCKET_MQTT_AP_BANGKOK_DOMAIN "ap-bangkok.iothub.tencentdevices.com"

#define QCLOUD_IOT_WEBSOCKET_MQTT_SERVER_PORT_TLS   443
#define QCLOUD_IOT_WEBSOCKET_MQTT_SERVER_PORT_NOTLS 80

/* server domain for dynamic registering device */
#define DYNREG_LOG_SERVER_URL            "ap-guangzhou.gateway.tencentdevices.com"
#define DYNREG_LOG_SERVER_US_EAST_URL    "us-east.gateway.tencentdevices.com"
#define DYNREG_LOG_SERVER_EUROPE_URL     "europe.gateway.tencentdevices.com"
#define DYNREG_LOG_SERVER_AP_BANGKOK_URL "ap-bangkok.gateway.tencentdevices.com"

#define DYN_REG_SERVER_URL_PATH "/device/register"
#define DYN_REG_SERVER_PORT     80
#define DYN_REG_SERVER_PORT_TLS 443

#define LOG_UPLOAD_SERVER_POSTFIX "gateway.tencentdevices.com"
#define UPLOAD_LOG_URI_PATH       "/device/reportlog"
#define LOG_UPLOAD_SERVER_PORT    80

#define REMOTE_LOGIN_WEBSOCKET_SSH_URL            "ap-guangzhou.gateway.tencentdevices.com/ssh/device"
#define REMOTE_LOGIN_WEBSOCKET_SSH_US_EAST_URL    "us-east.gateway.tencentdevices.com/ssh/device"
#define REMOTE_LOGIN_WEBSOCKET_SSH_EUROPE_URL     "europe.gateway.tencentdevices.com/ssh/device"
#define REMOTE_LOGIN_WEBSOCKET_SSH_AP_BANGKOK_URL "ap-bangkok.gateway.tencentdevices.com/ssh/device"

#define LOCAL_SSH_PORT 22
#define LOCAL_SSH_IP   "127.0.0.1"

/* Max size of a host name */
#define HOST_STR_LENGTH 64

/* Max size of base64 encoded PSK = 64, after decode: 64/4*3 = 48*/
#define DECODE_PSK_LENGTH 48

#endif /* QCLOUD_IOT_COMMON_H_ */
