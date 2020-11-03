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

#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"

#ifdef REMOTE_CONFIG_MQTT
#include <string.h>
#include "lite-utils.h"
#include "mqtt_client.h"
#include "qcloud_iot_device.h"
#include "qcloud_iot_export_remote_config.h"

#define JSON_TYPE_STRING_PUSH  "push"
#define JSON_TYPE_STRING_REPLY "reply"
#define JSON_TYPE_STRING_GET   "get"

#define CONFIG_PUBLISH_TOPIC_FORMAT   "$config/operation/%s/%s"
#define CONFIG_SUBSCRIBE_TOPIC_FORMAT "$config/operation/result/%s/%s"

static int _check_snprintf_return(int32_t return_code, size_t max_size_of_write)
{
    if (return_code >= max_size_of_write) {
        return QCLOUD_ERR_JSON_BUFFER_TRUNCATED;
    } else if (return_code < 0) {  // err
        return QCLOUD_ERR_JSON;
    }

    return QCLOUD_RET_SUCCESS;
}

/****************************
{
   "type":"push",
   "payload": {}
}

{
   "type":"reply",
   "result":0/1001,
   "payload": {}
}

****************************/
static void _config_mqtt_message_callback(void *client, MQTTMessage *message, void *user_data)
{
    POINTER_SANITY_CHECK_RTN(message);
    POINTER_SANITY_CHECK_RTN(client);

    ConfigSubscirbeUserData *config_sub_userdata = (ConfigSubscirbeUserData *)user_data;
    POINTER_SANITY_CHECK_RTN(config_sub_userdata);
    POINTER_SANITY_CHECK_RTN(config_sub_userdata->on_config_proc);
    POINTER_SANITY_CHECK_RTN(config_sub_userdata->json_buffer);

    Qcloud_IoT_Client *mqtt_client    = (Qcloud_IoT_Client *)client;
    ConfigMQTTState *  config_state   = &mqtt_client->config_state;
    char *             payload        = config_sub_userdata->json_buffer;
    char *             type           = NULL;
    char *             result         = NULL;
    char *             config_payload = NULL;

    // proc recv buff, copy recv data to config_sub_userdata json buffer, need 1B to save '\0'
    if (message->payload_len > (config_sub_userdata->json_buffer_len - 1)) {
        Log_e("topic message arrived, config user data json buffer len :%d < recv buff len :%d",
              config_sub_userdata->json_buffer_len - 1, message->payload_len);
        return;
    } else {
        memcpy(payload, message->payload, message->payload_len);
        payload[message->payload_len] = '\0';
    }

    Log_d("Recv Msg Topic:%s, buff data:%s", STRING_PTR_PRINT_SANITY_CHECK(message->ptopic), payload);

    type = LITE_json_value_of("type", payload);
    if (NULL == type) {
        Log_e("topic message arrived, data error,no type key");
        goto exit;
    }
    // reply data ?
    if (0 == strcmp(type, JSON_TYPE_STRING_REPLY)) {
        config_state->get_reply_ok = true;

        result = LITE_json_value_of("result", payload);
        if (NULL == result) {
            Log_e("topic message arrived, data error,no result key");
            goto exit;
        }
    } else if (0 != strcmp(type, JSON_TYPE_STRING_PUSH)) {
        Log_e("topic message arrived, data error, type: %s is unknow", type);
        goto exit;
    }

    if ((result != NULL) && (REMOTE_CONFIG_ERRCODE_DISABLE == atoi(result))) {
        Log_i("topic message arrived, get config failed cloud platform config disable");
    }

    config_payload = LITE_json_value_of("payload", payload);
    // copy config data to user_data json buffer
    if (NULL == config_payload) {
        config_sub_userdata->json_buffer[0] = '\0';
    } else {
        memcpy(config_sub_userdata->json_buffer, config_payload, strlen(config_payload));
        config_sub_userdata->json_buffer[strlen(config_payload)] = '\0';
    }

    if (NULL == result) {
        config_sub_userdata->on_config_proc(client, REMOTE_CONFIG_ERRCODE_SUCCESS, config_sub_userdata->json_buffer,
                                            strlen(config_sub_userdata->json_buffer));
    } else {
        config_sub_userdata->on_config_proc(client, atoi(result), config_sub_userdata->json_buffer,
                                            strlen(config_sub_userdata->json_buffer));
    }

exit:
    HAL_Free(type);
    HAL_Free(result);
    HAL_Free(config_payload);

    return;
}

static void _config_mqtt_sub_event_handler(void *client, MQTTEventType event_type, void *user_data)
{
    POINTER_SANITY_CHECK_RTN(client);

    Qcloud_IoT_Client *mqtt_client  = (Qcloud_IoT_Client *)client;
    ConfigMQTTState *  config_state = &(mqtt_client->config_state);

    switch (event_type) {
        case MQTT_EVENT_SUBCRIBE_SUCCESS:
            Log_d("mqtt config topic subscribe success");

            config_state->topic_sub_ok = true;
            break;

        case MQTT_EVENT_SUBCRIBE_TIMEOUT:
            Log_i("mqtt config topic subscribe timeout");

            config_state->topic_sub_ok = false;
            break;

        case MQTT_EVENT_SUBCRIBE_NACK:
            Log_i("mqtt config topic subscribe NACK");

            config_state->topic_sub_ok = false;
            break;
        case MQTT_EVENT_UNSUBSCRIBE:
            Log_i("mqtt config topic has been unsubscribed");

            config_state->topic_sub_ok = false;
            break;
        case MQTT_EVENT_CLIENT_DESTROY:
            Log_i("mqtt config has been destroyed");

            config_state->topic_sub_ok = false;
            break;
        default:
            return;
    }

    config_state->get_reply_ok = true;
}

static int _iot_config_mqtt_subscribe(void *client, ConfigSubscirbeUserData *config_sub_userdata)
{
    POINTER_SANITY_CHECK(client, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(config_sub_userdata, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(config_sub_userdata->on_config_proc, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(config_sub_userdata->json_buffer, QCLOUD_ERR_INVAL);

    Qcloud_IoT_Client *mqtt_client = (Qcloud_IoT_Client *)client;
    DeviceInfo *       dev_info    = &mqtt_client->device_info;

    char topic_name[128] = {0};

    int size = HAL_Snprintf(topic_name, sizeof(topic_name), CONFIG_SUBSCRIBE_TOPIC_FORMAT,
                            STRING_PTR_PRINT_SANITY_CHECK(dev_info->product_id),
                            STRING_PTR_PRINT_SANITY_CHECK(dev_info->device_name));
    if (size < 0 || size > sizeof(topic_name) - 1) {
        Log_e("topic content length not enough! content size:%d  buf size:%d", size, (int)sizeof(topic_name));
        return QCLOUD_ERR_FAILURE;
    }
    SubscribeParams sub_params      = DEFAULT_SUB_PARAMS;
    sub_params.on_message_handler   = _config_mqtt_message_callback;
    sub_params.on_sub_event_handler = _config_mqtt_sub_event_handler;
    sub_params.user_data            = (void *)config_sub_userdata;
    sub_params.qos                  = QOS0;

    return IOT_MQTT_Subscribe(client, topic_name, &sub_params);
}

int IOT_Subscribe_Config(void *client, ConfigSubscirbeUserData *config_sub_userdata, int subscribe_timeout)
{
    int ret = QCLOUD_RET_SUCCESS;

    POINTER_SANITY_CHECK(client, QCLOUD_ERR_INVAL);
    Qcloud_IoT_Client *mqtt_client  = (Qcloud_IoT_Client *)client;
    ConfigMQTTState *  config_state = &mqtt_client->config_state;
    Timer              timer;
    int                packet_id = 0;

    config_state->topic_sub_ok = false;
    config_state->get_reply_ok = false;

    ret = _iot_config_mqtt_subscribe(client, config_sub_userdata);

    if (0 > ret) {
        Log_e("config topic subscribe failed, ret:%d", ret);

        return ret;
    }
    packet_id = ret;
    // wait for sub ack
    InitTimer(&timer);
    countdown_ms(&timer, subscribe_timeout);
    while (false == config_state->get_reply_ok) {
        if (expired(&timer)) {
            ret = QCLOUD_ERR_MQTT_REQUEST_TIMEOUT;
            break;
        }
        ret = qcloud_iot_mqtt_yield_mt(mqtt_client, 100);
        if (QCLOUD_RET_SUCCESS != ret) {
            break;
        }
    }
    if (true == config_state->topic_sub_ok) {
        ret = packet_id;
    } else if (QCLOUD_RET_SUCCESS == ret) {  // multi thread nack
        ret = QCLOUD_ERR_MQTT_SUB;
    }

    return ret;
}

static int _iot_config_report_mqtt_publish(void *client, void *json_buffer)
{
    POINTER_SANITY_CHECK(client, QCLOUD_ERR_INVAL);

    Qcloud_IoT_Client *mqtt_client = (Qcloud_IoT_Client *)client;
    DeviceInfo *       dev_info    = &mqtt_client->device_info;
    POINTER_SANITY_CHECK(dev_info, QCLOUD_ERR_INVAL);

    char topic_name[128] = {0};

    HAL_Snprintf(topic_name, sizeof(topic_name), CONFIG_PUBLISH_TOPIC_FORMAT,
                 STRING_PTR_PRINT_SANITY_CHECK(dev_info->product_id),
                 STRING_PTR_PRINT_SANITY_CHECK(dev_info->device_name));

    PublishParams pub_params = DEFAULT_PUB_PARAMS;
    pub_params.qos           = QOS0;
    pub_params.payload       = json_buffer;
    pub_params.payload_len   = strlen(json_buffer);

    return IOT_MQTT_Publish(mqtt_client, topic_name, &pub_params);
}

int IOT_Get_Config(void *client, char *json_buffer, int buffer_size, int reply_timeout)
{
    POINTER_SANITY_CHECK(client, QCLOUD_ERR_INVAL);
    int                ret         = 0;
    Qcloud_IoT_Client *mqtt_client = (Qcloud_IoT_Client *)client;
    Timer              timer;
    ConfigMQTTState *  config_state   = &mqtt_client->config_state;
    int32_t            rc_of_snprintf = 0;

    // return failure if subscribe failed
    if (false == config_state->topic_sub_ok) {
        Log_e("Subscribe config topic failed!");
        return QCLOUD_ERR_FAILURE;
    }

    // create get json string
    rc_of_snprintf = HAL_Snprintf(json_buffer, buffer_size, "{\"type\":\"%s\"}", JSON_TYPE_STRING_GET);
    ret            = _check_snprintf_return(rc_of_snprintf, buffer_size);
    if (QCLOUD_RET_SUCCESS != ret) {
        Log_e("Construct Report Info failed, ret:%d!", ret);
        return ret;
    }

    config_state->get_reply_ok = false;

    // publish msg to get config
    ret = _iot_config_report_mqtt_publish(mqtt_client, json_buffer);
    if (ret < 0) {
        Log_e("client publish config topic failed :%d.", ret);
        return ret;
    }

    // wait for reply
    InitTimer(&timer);
    countdown_ms(&timer, reply_timeout);
    while (false == config_state->get_reply_ok) {
        if (expired(&timer)) {
            ret = QCLOUD_ERR_MQTT_REQUEST_TIMEOUT;
            Log_e("get config wait reply timeout");
            break;
        }
        ret = qcloud_iot_mqtt_yield_mt(mqtt_client, 100);

        if (QCLOUD_RET_SUCCESS != ret) {
            break;
        }
    }
    return ret;
}

#ifdef __cplusplus
}
#endif

#endif
