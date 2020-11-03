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

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "mqtt_client.h"
#include "qcloud_iot_export_error.h"
#include "qcloud_iot_export_rrpc.h"
#include "utils_param_check.h"

#ifdef RRPC_ENABLED

static char sg_process_id_buffer[MAX_RRPC_PROCESS_ID_LEN + 1] = {0};  // process id buffer

static int _publish_rrpc_to_cloud(void *client, const char *processId, char *pJsonDoc)
{
    IOT_FUNC_ENTRY;
    int                rc                             = QCLOUD_RET_SUCCESS;
    char               topic[MAX_SIZE_OF_CLOUD_TOPIC] = {0};
    Qcloud_IoT_Client *mqtt_client                    = (Qcloud_IoT_Client *)client;

    int size = HAL_Snprintf(topic, MAX_SIZE_OF_CLOUD_TOPIC, "$rrpc/txd/%s/%s/%s",
                            STRING_PTR_PRINT_SANITY_CHECK(mqtt_client->device_info.product_id),
                            STRING_PTR_PRINT_SANITY_CHECK(mqtt_client->device_info.device_name),
                            STRING_PTR_PRINT_SANITY_CHECK(processId));
    if (size < 0 || size > sizeof(topic) - 1) {
        Log_e("topic content length not enough! content size:%d  buf size:%d", size, (int)sizeof(topic));
        return QCLOUD_ERR_FAILURE;
    }

    PublishParams pubParams = DEFAULT_PUB_PARAMS;
    pubParams.qos           = QOS0;
    pubParams.payload_len   = strlen(pJsonDoc);
    pubParams.payload       = (char *)pJsonDoc;

    rc = IOT_MQTT_Publish(mqtt_client, topic, &pubParams);

    IOT_FUNC_EXIT_RC(rc);
}

static int _rrpc_get_process_id(char *processIdBuffer, size_t sizeOfBuffer, const char *topic, size_t topic_len)
{
    char *p  = NULL;
    char  ch = '/';

    p = strrchr(topic, ch);
    if (p == NULL) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_INVAL);
    }

    p++;
    if ((topic_len - (p - topic)) > sizeOfBuffer) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    strncpy(processIdBuffer, p, topic_len - (p - topic));
    Log_i("len=%u, process id=%.*s", topic_len - (p - topic), topic_len - (p - topic), processIdBuffer);

    IOT_FUNC_EXIT_RC(QCLOUD_RET_SUCCESS);
}

static void _rrpc_message_cb(void *pClient, MQTTMessage *message, void *pContext)
{
    OnRRPCMessageCallback callback = (OnRRPCMessageCallback)pContext;

    Log_d("topic=%.*s", message->topic_len, message->ptopic);
    Log_i("len=%u, topic_msg=%.*s", message->payload_len, message->payload_len,
          STRING_PTR_PRINT_SANITY_CHECK((char *)message->payload));

    int rc = _rrpc_get_process_id(sg_process_id_buffer, MAX_RRPC_PROCESS_ID_LEN, message->ptopic, message->topic_len);
    if (rc != QCLOUD_RET_SUCCESS) {
        Log_e("rrpc get process id failed: %d", rc);
        return;
    }

    if (callback) {
        callback(pClient, message->payload, message->payload_len);
    }
}

static void _rrpc_event_callback(void *pClient, MQTTEventType event_type, void *user_data)
{
    Qcloud_IoT_Client *mqtt_client = (Qcloud_IoT_Client *)pClient;
    switch (event_type) {
        case MQTT_EVENT_SUBCRIBE_SUCCESS:
            Log_d("rrpc topic subscribe success");
            mqtt_client->rrpc_state = true;
            break;
        case MQTT_EVENT_SUBCRIBE_TIMEOUT:
            Log_i("rrpc topic subscribe timeout");
            mqtt_client->rrpc_state = false;
            break;
        case MQTT_EVENT_SUBCRIBE_NACK:
            Log_i("rrpc topic subscribe NACK");
            mqtt_client->rrpc_state = false;
            break;
        case MQTT_EVENT_UNSUBSCRIBE:
            Log_i("rrpc topic has been unsubscribed");
            mqtt_client->rrpc_state = false;
            break;
        case MQTT_EVENT_CLIENT_DESTROY:
            Log_i("mqtt client has been destroyed");
            mqtt_client->rrpc_state = false;
            break;
        default:
            return;
    }
}

int IOT_RRPC_Init(void *pClient, OnRRPCMessageCallback callback)
{
    int  rc                                      = QCLOUD_RET_SUCCESS;
    char rrpc_topic[MAX_SIZE_OF_CLOUD_TOPIC + 1] = {0};

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
    Qcloud_IoT_Client *mqtt_client = (Qcloud_IoT_Client *)pClient;

    SubscribeParams sub_params      = DEFAULT_SUB_PARAMS;
    sub_params.on_message_handler   = _rrpc_message_cb;
    sub_params.on_sub_event_handler = _rrpc_event_callback;
    sub_params.qos                  = QOS0;
    sub_params.user_data            = callback;

    HAL_Snprintf(rrpc_topic, MAX_SIZE_OF_CLOUD_TOPIC, "$rrpc/rxd/%s/%s/+",
                 STRING_PTR_PRINT_SANITY_CHECK(mqtt_client->device_info.product_id),
                 STRING_PTR_PRINT_SANITY_CHECK(mqtt_client->device_info.device_name));

    if (!mqtt_client->rrpc_state) {
        for (int cntSub = 0; cntSub < 3; cntSub++) {
            rc = IOT_MQTT_Subscribe(mqtt_client, rrpc_topic, &sub_params);
            if (rc < 0) {
                Log_e("rrpc topic subscribe failed: %d, cnt: %d", rc, cntSub);
                continue;
            }

            /* wait for sub ack */
            rc = qcloud_iot_mqtt_yield_mt(mqtt_client, 500);
            if ((rc != QCLOUD_RET_SUCCESS) || mqtt_client->rrpc_state) {
                break;
            }
        }
    }

    if (!mqtt_client->rrpc_state) {
        Log_e("Subscribe rrpc topic failed!");
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }
    IOT_FUNC_EXIT_RC(QCLOUD_RET_SUCCESS);
}

int IOT_RRPC_Reply(void *pClient, char *pJsonDoc, size_t sizeOfBuffer, sRRPCReplyPara *replyPara)
{
    int rc = QCLOUD_RET_SUCCESS;
    rc     = _publish_rrpc_to_cloud(pClient, sg_process_id_buffer, pJsonDoc);
    if (rc < 0) {
        Log_e("publish rrpc to cloud fail, %d", rc);
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_RRPC_REPLY_ERR);
    }

    IOT_FUNC_EXIT_RC(QCLOUD_RET_SUCCESS);
}

#endif

#ifdef __cplusplus
}
#endif
