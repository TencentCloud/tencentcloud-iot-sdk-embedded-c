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

#include <string.h>

#include "resource_client.h"
#include "resource_lib.h"

#define QCLOUD_RESOURCE_MAX_TOPIC_LEN (128)

typedef struct {
    void *mqtt_client;  // MQTT cient

    const char *product_id;
    const char *device_name;

    char                      topic[QCLOUD_RESOURCE_MAX_TOPIC_LEN];  // resource download MQTT Topic
    OnResourceMessageCallback msg_callback;
    void *                    context;
    int                       upload_result_code;
    bool                      topic_sub_ready;
} QCLOUD_RESOURCE_MQTT_T;

static const char *sg_report_str[] = {"uploading", "downloading", "done", "fail"};

static char *sg_result_code_str[] = {"", "time_out", "file_not_exist", "sign invalid", "md5sum not matched"};

static void _qcloud_resource_mqtt_cb(void *pClient, MQTTMessage *message, void *pcontext)
{
    QCLOUD_RESOURCE_MQTT_T *handle = (QCLOUD_RESOURCE_MQTT_T *)pcontext;

    Log_d("topic=%.*s", message->topic_len, STRING_PTR_PRINT_SANITY_CHECK(message->ptopic));
    Log_i("len=%u, topic_msg=%.*s", message->payload_len, message->payload_len,
          STRING_PTR_PRINT_SANITY_CHECK((char *)message->payload));

    if (NULL != handle->msg_callback) {
        handle->msg_callback(handle->context, message->payload, message->payload_len);
    }
}

static void _qcloud_resource_mqtt_event_callback(void *pclient, MQTTEventType event_type, void *user_data)
{
    QCLOUD_RESOURCE_MQTT_T *handle = (QCLOUD_RESOURCE_MQTT_T *)user_data;

    switch (event_type) {
        case MQTT_EVENT_SUBCRIBE_SUCCESS:
            Log_d("resource topic subscribe success");
            handle->topic_sub_ready = true;
            break;

        case MQTT_EVENT_SUBCRIBE_TIMEOUT:
            Log_i("resource topic subscribe timeout");
            handle->topic_sub_ready = false;
            break;

        case MQTT_EVENT_SUBCRIBE_NACK:
            Log_i("resource topic subscribe NACK");
            handle->topic_sub_ready = false;
            break;
        case MQTT_EVENT_UNSUBSCRIBE:
            Log_i("resource topic has been unsubscribed");
            handle->topic_sub_ready = false;
            break;
        case MQTT_EVENT_CLIENT_DESTROY:
            Log_i("mqtt client has been destroyed");
            handle->topic_sub_ready = false;
            break;
        default:
            return;
    }
}

void *qcloud_resource_mqtt_init(const char *productId, const char *deviceName, void *mqtt_client,
                                OnResourceMessageCallback user_callback, void *user_context)
{
    int                     ret;
    QCLOUD_RESOURCE_MQTT_T *handle = NULL;
    if (NULL == (handle = HAL_Malloc(sizeof(QCLOUD_RESOURCE_MQTT_T)))) {
        Log_e("allocate for h_osc failed");
        goto do_exit;
    }

    memset(handle, 0, sizeof(QCLOUD_RESOURCE_MQTT_T));

    /* subscribe the resources download topic: "$resources/download/${productID}/${deviceName}" */
    ret = HAL_Snprintf(handle->topic, QCLOUD_RESOURCE_MAX_TOPIC_LEN, "$resource/down/service/%s/%s", productId,
                       deviceName);

    SubscribeParams sub_params      = DEFAULT_SUB_PARAMS;
    sub_params.on_message_handler   = _qcloud_resource_mqtt_cb;
    sub_params.on_sub_event_handler = _qcloud_resource_mqtt_event_callback;
    sub_params.qos                  = QOS1;
    sub_params.user_data            = handle;

    ret = IOT_MQTT_Subscribe(mqtt_client, handle->topic, &sub_params);
    if (ret < 0) {
        Log_e("resource mqtt subscribe failed!");
        goto do_exit;
    }

    int wait_cnt = 10;
    while (!handle->topic_sub_ready && (wait_cnt > 0)) {
        // wait for subscription result
        IOT_MQTT_Yield(mqtt_client, 200);
        wait_cnt--;
    }

    if (wait_cnt == 0) {
        Log_e("resource mqtt subscribe timeout!");
        goto do_exit;
    }

    handle->mqtt_client  = mqtt_client;
    handle->product_id   = productId;
    handle->device_name  = deviceName;
    handle->msg_callback = user_callback;
    handle->context      = user_context;

    return handle;

do_exit:
    if (NULL != handle) {
        HAL_Free(handle);
    }

    return NULL;
}

/**
 * Topic $resources/report/${productID}/${deviceName}
 */
int qcloud_resource_mqtt_report_progress(void *resource_mqtt, QCLOUD_RESOURCE_REPORT_E state,
                                         QCLOUD_RESOURCE_RESULTCODE_E resultcode, int percent, char *resource_name,
                                         bool download)
{
    IOT_FUNC_ENTRY;

    char                    topic[QCLOUD_RESOURCE_MAX_TOPIC_LEN] = {0};
    int                     ret;
    PublishParams           pub_params   = DEFAULT_PUB_PARAMS;
    QCLOUD_RESOURCE_MQTT_T *handle       = (QCLOUD_RESOURCE_MQTT_T *)resource_mqtt;
    char                    payload[256] = {0};
    int                     payload_len;
    char *                  report_type = NULL;
    if (resultcode <= QCLOUD_RESOURCE_RESULTCODE_END || resultcode > QCLOUD_RESOURCE_RESULTCODE_SUCCESS_E) {
        Log_e("resultcode is error. %d", resultcode);
        return QCLOUD_ERR_FAILURE;
    }
    report_type = download ? "report_download_progress" : "report_upload_progress";
    payload_len = sizeof("{\"type\" :\"%s\",\"name\":\"%s\",\"progress\":{") + sizeof(",\"percent\":%d") +
                  sizeof("\"state\":\"%s\",\"result_code\":%d,\"result_msg\":\"%s\"}}") + strlen(resource_name) +
                  (sizeof("report_download_progress") - 1);

    HAL_Snprintf(payload, payload_len, "{\"type\":\"%s\",\"name\":\"%s\",\"progress\":{", report_type, resource_name);

    if (state == QCLOUD_RESOURCE_REPORT_DOWNLOADING_E || state == QCLOUD_RESOURCE_REPORT_UPLOADING_E) {
        HAL_Snprintf(payload + strlen(payload), payload_len - strlen(payload), "\"percent\":%d,", percent);
    }

    HAL_Snprintf(payload + strlen(payload), payload_len - strlen(payload),
                 "\"state\":\"%s\",\"result_code\":%d,\"result_msg\":\"%s\"}}", sg_report_str[state], resultcode,
                 sg_result_code_str[(0 - resultcode)]);

    if (state == QCLOUD_RESOURCE_REPORT_DOWNLOADING_E || state == QCLOUD_RESOURCE_REPORT_UPLOADING_E) {
        pub_params.qos = QOS0;
    } else {
        pub_params.qos = QOS1;
    }

    pub_params.payload     = (void *)payload;
    pub_params.payload_len = strlen(payload);

    ret = HAL_Snprintf(topic, payload_len, "$resource/up/service/%s/%s", handle->product_id, handle->device_name);

    ret = IOT_MQTT_Publish(handle->mqtt_client, topic, &pub_params);
    if (ret < 0) {
        Log_e("publish to topic: %s failed", topic);
        ret = QCLOUD_RESOURCE_ERRCODE_OSC_FAILED_E;
    }
    IOT_FUNC_EXIT_RC(ret);
}

int qcloud_resource_mqtt_deinit(void *resource_mqtt)
{
    QCLOUD_RESOURCE_MQTT_T *handle = (QCLOUD_RESOURCE_MQTT_T *)resource_mqtt;
    IOT_MQTT_Unsubscribe(handle->mqtt_client, handle->topic);
    HAL_Free(handle);
    return 0;
}

int qcloud_resource_mqtt_yield(void *resource_mqtt)
{
    QCLOUD_RESOURCE_MQTT_T *handle = (QCLOUD_RESOURCE_MQTT_T *)resource_mqtt;
    int                     rc     = IOT_MQTT_Yield(handle->mqtt_client, 200);
    if (rc != QCLOUD_RET_SUCCESS && rc != QCLOUD_RET_MQTT_RECONNECTED) {
        Log_e("MQTT error: %d", rc);
        return QCLOUD_ERR_FAILURE;
    }
    return QCLOUD_RET_SUCCESS;
}

int qcloud_resource_mqtt_upload_request(void *resource_mqtt, void *request_data)
{
    IOT_FUNC_ENTRY;

    char topic[QCLOUD_RESOURCE_MAX_TOPIC_LEN] = {0};
    char payload[256]                         = {0};

    QCLOUD_RESOURCE_MQTT_T *handle = (QCLOUD_RESOURCE_MQTT_T *)resource_mqtt;

    QCLOUD_RESOURCE_UPLOAD_REQUEST_S *resource_data = (QCLOUD_RESOURCE_UPLOAD_REQUEST_S *)request_data;

    int           ret;
    PublishParams pub_params = DEFAULT_PUB_PARAMS;

    pub_params.payload_len = HAL_Snprintf(
        payload, sizeof(payload), "{\"type\": \"create_upload_task\",\"size\": %d,\"name\": \"%s\",\"md5sum\": \"%s\"}",
        resource_data->resource_size, resource_data->resource_name, resource_data->resource_md5sum);

    pub_params.qos     = QOS1;
    pub_params.payload = (void *)payload;

    ret = HAL_Snprintf(topic, QCLOUD_RESOURCE_MAX_TOPIC_LEN, "$resource/up/service/%s/%s", handle->product_id,
                       handle->device_name);

    ret = IOT_MQTT_Publish(handle->mqtt_client, topic, &pub_params);
    if (ret < 0) {
        Log_e("publish to topic: %s failed", topic);
        IOT_FUNC_EXIT_RC(QCLOUD_RESOURCE_ERRCODE_OSC_FAILED_E);
    }

    IOT_FUNC_EXIT_RC(ret);
}

int qcloud_resource_mqtt_download_get(void *resource_mqtt)
{
    char topic[QCLOUD_RESOURCE_MAX_TOPIC_LEN] = {0};
    char payload[64]                          = {0};

    QCLOUD_RESOURCE_MQTT_T *handle = (QCLOUD_RESOURCE_MQTT_T *)resource_mqtt;

    int           ret;
    PublishParams pub_params = DEFAULT_PUB_PARAMS;

    HAL_Snprintf(payload, sizeof(payload), "{\"type\": \"get_download_task\"}");

    pub_params.qos         = QOS1;
    pub_params.payload     = (void *)payload;
    pub_params.payload_len = strlen(payload);

    ret = HAL_Snprintf(topic, QCLOUD_RESOURCE_MAX_TOPIC_LEN, "$resource/up/service/%s/%s", handle->product_id,
                       handle->device_name);

    ret = IOT_MQTT_Publish(handle->mqtt_client, topic, &pub_params);
    if (ret < 0) {
        Log_e("publish to topic: %s failed", topic);
        IOT_FUNC_EXIT_RC(QCLOUD_RESOURCE_ERRCODE_OSC_FAILED_E);
    }

    return ret;
}

#ifdef __cplusplus
}
#endif
