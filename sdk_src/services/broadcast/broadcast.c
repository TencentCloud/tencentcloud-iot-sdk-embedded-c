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

#include "mqtt_client.h"
#include "qcloud_iot_export_broadcast.h"
#include "utils_param_check.h"

#ifdef BROADCAST_ENABLED

static void _broadcast_message_cb(void *pClient, MQTTMessage *message, void *pContext)
{
    OnBroadcastMessageCallback callback = (OnBroadcastMessageCallback)pContext;
    Log_d("topic=%.*s", message->topic_len, message->ptopic);
    Log_i("len=%u, topic_msg=%.*s", message->payload_len, message->payload_len, (char *)message->payload);
    if (callback) {
        callback(pClient, message->payload, message->payload_len);
    }
}

static void _broadcast_event_callback(void *pClient, MQTTEventType event_type, void *user_data)
{
    Qcloud_IoT_Client *mqtt_client = (Qcloud_IoT_Client *)pClient;
    switch (event_type) {
        case MQTT_EVENT_SUBCRIBE_SUCCESS:
            Log_d("broadcast topic subscribe success");
            mqtt_client->broadcast_state = true;
            break;
        case MQTT_EVENT_SUBCRIBE_TIMEOUT:
            Log_i("broadcast topic subscribe timeout");
            mqtt_client->broadcast_state = false;
            break;
        case MQTT_EVENT_SUBCRIBE_NACK:
            Log_i("broadcast topic subscribe NACK");
            mqtt_client->broadcast_state = false;
            break;
        case MQTT_EVENT_UNSUBSCRIBE:
            Log_i("broadcast topic has been unsubscribed");
            mqtt_client->broadcast_state = false;
            break;
        case MQTT_EVENT_CLIENT_DESTROY:
            Log_i("mqtt client has been destroyed");
            mqtt_client->broadcast_state = false;
            break;
        default:
            return;
    }
}

int IOT_Broadcast_Subscribe(void *pClient, OnBroadcastMessageCallback callback)
{
    int  ret;
    char broadcast_topic[MAX_SIZE_OF_CLOUD_TOPIC + 1];

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
    Qcloud_IoT_Client *mqtt_client = (Qcloud_IoT_Client *)pClient;

    SubscribeParams sub_params      = DEFAULT_SUB_PARAMS;
    sub_params.on_message_handler   = _broadcast_message_cb;
    sub_params.on_sub_event_handler = _broadcast_event_callback;
    sub_params.qos                  = QOS1;
    sub_params.user_data            = callback;

    HAL_Snprintf(broadcast_topic, MAX_SIZE_OF_CLOUD_TOPIC, "$broadcast/rxd/%s/%s", mqtt_client->device_info.product_id,
                 mqtt_client->device_info.device_name);

    if (!mqtt_client->broadcast_state) {
        for (int cntSub = 0; cntSub < 3; cntSub++) {
            ret = IOT_MQTT_Subscribe(mqtt_client, broadcast_topic, &sub_params);
            if (ret < 0) {
                Log_e("broadcast topic subscribe failed: %d, cnt: %d", ret, cntSub);
                continue;
            }

            /* wait for sub ack */
            ret = qcloud_iot_mqtt_yield_mt(mqtt_client, 500);
            if (ret || mqtt_client->broadcast_state) {
                break;
            }
        }
    }

    if (!mqtt_client->broadcast_state) {
        Log_e("Subscribe broadcast topic failed!");
        return QCLOUD_ERR_FAILURE;
    }
    return QCLOUD_RET_SUCCESS;
}

#endif
