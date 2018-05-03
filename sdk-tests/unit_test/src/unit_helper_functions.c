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

#include "unit_helper_functions.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>

#include "qcloud_iot_export.h"
#include "iot_unit_config.h"
#include "device.h"

static char g_clientCert[PATH_MAX + 1];
static char g_clientKey[PATH_MAX + 1];

static void _make_client_id(MQTTConnectParams *connectParams) {
	DeviceInfo *info = iot_device_info_get();

	int clientid_len = strlen(info->product_id) + strlen(info->device_name) + 1;
	connectParams->client_id = (char*)HAL_Malloc(clientid_len);
	sprintf(connectParams->client_id, "%s%s", info->product_id, info->device_name);
}

/**
 * 设置初始化参数
 *
 * @param pParams
 * @param pHost
 * @param port
 * @param enableAutoReconnect
 * @param callback
 */
void MQTTInitParamsSetup(MQTTInitParams *pParams, bool enableAutoReconnect) 
{
    pParams->auto_connect_enable = enableAutoReconnect;

    char cert_dir[PATH_MAX + 1] = "certs";

    sprintf(g_clientCert, "%s/%s", cert_dir, QCLOUD_IOT_CERT_FILENAME);
    sprintf(g_clientKey, "%s/%s", cert_dir, QCLOUD_IOT_KEY_FILENAME);
#ifndef AUTH_WITH_NOTLS
#ifdef AUTH_MODE_CERT
    pParams->cert_file = g_clientCert;
    pParams->key_file = g_clientKey;
#else

#endif
#endif

    pParams->device_name = (char *)QCLOUD_IOT_MY_DEVICE_NAME;
    pParams->product_id = (char *)QCLOUD_IOT_MY_PRODUCT_ID;
    pParams->command_timeout = QCLOUD_IOT_MQTT_COMMAND_TIMEOUT;
}

void ConnectMQTTParamsSetup_Detailed(MQTTConnectParams *connectParams, MQTTInitParams *initParams,
                                     QoS qos, bool isCleanSession, bool isWillMsgPresent, 
                                     char *pWillTopicName, char *pWillMessage, char *pUsername) 
{
    connectParams->keep_alive_interval = 10;
    connectParams->clean_session = isCleanSession;
    connectParams->mqtt_version = MQTT_3_1_1;
    connectParams->username = pUsername;

    iot_device_info_init();
    iot_device_info_set(initParams->product_id, initParams->device_name);
    _make_client_id(connectParams);
}

/**
 * 设置连接参数
 *
 * @param pParams
 * @param pClientId
 */
void ConnectParamsSetup(MQTTConnectParams *connectParams, MQTTInitParams *initParams) 
{
    connectParams->mqtt_version = MQTT_3_1_1;
    connectParams->keep_alive_interval = (uint16_t)QCLOUD_IOT_MQTT_KEEP_ALIVE_INTERNAL;
	connectParams->clean_session = initParams->clean_session;
	connectParams->auto_connect_enable = initParams->auto_connect_enable;

    iot_device_info_init();
    iot_device_info_set(initParams->product_id, initParams->device_name);
    _make_client_id(connectParams);
}

void SetupMQTTConnectInitParams(MQTTInitParams* pInitParams)
{
    pInitParams->device_name = (char *)QCLOUD_IOT_MY_DEVICE_NAME;
    pInitParams->product_id = (char *)QCLOUD_IOT_MY_PRODUCT_ID;

    char cert_dir[PATH_MAX + 1] = "certs";
    sprintf(g_clientCert, "%s/%s", cert_dir, QCLOUD_IOT_CERT_FILENAME);
    sprintf(g_clientKey, "%s/%s", cert_dir, QCLOUD_IOT_KEY_FILENAME);
#ifndef AUTH_WITH_NOTLS
#ifdef AUTH_MODE_CERT
    pInitParams->cert_file = g_clientCert;
    pInitParams->key_file = g_clientKey;
#else

#endif
#endif
    
    pInitParams->command_timeout = QCLOUD_IOT_MQTT_COMMAND_TIMEOUT;
    pInitParams->keep_alive_interval_ms = QCLOUD_IOT_MQTT_KEEP_ALIVE_INTERNAL;

}

void SetupShadowConnectInitParams(ShadowInitParams* initParams)
{
    initParams->device_name = (char *)QCLOUD_IOT_MY_DEVICE_NAME;
    initParams->product_id = (char *)QCLOUD_IOT_MY_PRODUCT_ID;

    char cert_dir[PATH_MAX + 1] = "certs";
    sprintf(g_clientCert, "%s/%s", cert_dir, QCLOUD_IOT_CERT_FILENAME);
    sprintf(g_clientKey, "%s/%s", cert_dir, QCLOUD_IOT_KEY_FILENAME);
#ifndef AUTH_WITH_NOTLS
#ifdef AUTH_MODE_CERT
    initParams->cert_file = g_clientCert;
    initParams->key_file = g_clientKey;
#else

#endif
#endif
}
