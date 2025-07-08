/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2018-2020 Tencent. All rights reserved.

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

#ifdef SYSTEM_COMM
#include <string.h>

#include "lite-utils.h"
#include "mqtt_client.h"
#include "qcloud_iot_device.h"
#include "qcloud_iot_export_system.h"
#include "system_mqtt_ssh_proxy.h"

static void _system_mqtt_ssh_message_callback(void *pClient, char *payload, void *pUserData)
{
    char *ssh_switch_str = LITE_json_value_of("switch", payload);

    int ssh_switch = atoi(ssh_switch_str);

    if (true == ssh_switch) {
        IOT_QCLOUD_SSH_Start(pClient);
    } else {
        IOT_QCLOUD_SSH_Stop();
    }

    HAL_Free(ssh_switch_str);

    return;
}

static void _system_mqtt_message_callback(void *pClient, MQTTMessage *message, void *pUserData)
{
#define MAX_RECV_LEN (512)

    POINTER_SANITY_CHECK_RTN(message);

    static char rcv_buf[MAX_RECV_LEN + 1];
    size_t      len = (message->payload_len > MAX_RECV_LEN) ? MAX_RECV_LEN : (message->payload_len);

    if (message->payload_len > MAX_RECV_LEN) {
        Log_e("payload len oversize");
    }
    memcpy(rcv_buf, message->payload, len);
    rcv_buf[len]        = '\0';  // jsmn_parse relies on a string
    SysMQTTState *state = (SysMQTTState *)pUserData;

    Log_d("Recv Msg Topic:%s, payload:%s", STRING_PTR_PRINT_SANITY_CHECK(message->ptopic), rcv_buf);

    char *type = LITE_json_value_of("type", rcv_buf);
    if (type != NULL) {
        if (0 == strcmp(type, "ssh")) {
            _system_mqtt_ssh_message_callback(pClient, rcv_buf, pUserData);
            goto end;
        }
    }

    char *value = LITE_json_value_of("time", rcv_buf);
    if (value != NULL)
        state->time = atol(value);

    // ntp time 64bit platform parse ntp time
    if (4 != sizeof(size_t)) {
        char *ntptime1 = LITE_json_value_of("ntptime1", rcv_buf);
        if (ntptime1 != NULL) {
#ifdef _WIN64
            state->ntptime1 = _atoi64(ntptime1);
#else
            state->ntptime1 = atol(ntptime1);
#endif
        } else {
            state->ntptime1 = ((size_t)(state->time) * 1000);
        }
        char *ntptime2 = LITE_json_value_of("ntptime2", rcv_buf);
        if (ntptime2 != NULL) {
#ifdef _WIN64
            state->ntptime2 = _atoi64(ntptime2);
#else
            state->ntptime2 = atol(ntptime2);
#endif
        } else {
            state->ntptime2 = ((size_t)(state->time) * 1000);
        }
        HAL_Free(ntptime1);
        HAL_Free(ntptime2);
    }
    state->result_recv_ok = true;
    HAL_Free(value);

end:
    HAL_Free(type);
    return;
}

static void _system_mqtt_sub_event_handler(void *pclient, MQTTEventType event_type, void *pUserData)
{
    SysMQTTState *state = (SysMQTTState *)pUserData;

    switch (event_type) {
        case MQTT_EVENT_SUBCRIBE_SUCCESS:
            Log_d("mqtt sys topic subscribe success");
            state->topic_sub_ok = true;
            break;

        case MQTT_EVENT_SUBCRIBE_TIMEOUT:
            Log_i("mqtt sys topic subscribe timeout");
            state->topic_sub_ok = false;
            break;

        case MQTT_EVENT_SUBCRIBE_NACK:
            Log_i("mqtt sys topic subscribe NACK");
            state->topic_sub_ok = false;
            break;
        case MQTT_EVENT_UNSUBSCRIBE:
            Log_i("mqtt sys topic has been unsubscribed");
            state->topic_sub_ok = false;
            ;
            break;
        case MQTT_EVENT_CLIENT_DESTROY:
            Log_i("mqtt client has been destroyed");
            state->topic_sub_ok = false;
            ;
            break;
        default:
            return;
    }
}

static int _iot_system_info_get_publish(void *pClient)
{
    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);

    Qcloud_IoT_Client *mqtt_client = (Qcloud_IoT_Client *)pClient;
    DeviceInfo *       dev_info    = &mqtt_client->device_info;
    POINTER_SANITY_CHECK(dev_info, QCLOUD_ERR_INVAL);

    char topic_name[128]      = {0};
    char payload_content[128] = {0};

    HAL_Snprintf(topic_name, sizeof(topic_name), "$sys/operation/%s/%s",
                 STRING_PTR_PRINT_SANITY_CHECK(dev_info->product_id),
                 STRING_PTR_PRINT_SANITY_CHECK(dev_info->device_name));
    HAL_Snprintf(payload_content, sizeof(payload_content), "{\"type\": \"get\", \"resource\": [\"time\"]}");

    PublishParams pub_params = DEFAULT_PUB_PARAMS;
    pub_params.qos           = QOS0;
    pub_params.payload       = payload_content;
    pub_params.payload_len   = strlen(payload_content);

    return IOT_MQTT_Publish(mqtt_client, topic_name, &pub_params);
}

static int _iot_system_info_result_subscribe(void *pClient)
{
    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);

    Qcloud_IoT_Client *mqtt_client = (Qcloud_IoT_Client *)pClient;
    DeviceInfo *       dev_info    = &mqtt_client->device_info;
    SysMQTTState *     sys_state   = &mqtt_client->sys_state;

    char topic_name[128] = {0};
    int  size            = HAL_Snprintf(topic_name, sizeof(topic_name), "$sys/operation/result/%s/%s",
                            STRING_PTR_PRINT_SANITY_CHECK(dev_info->product_id),
                            STRING_PTR_PRINT_SANITY_CHECK(dev_info->device_name));
    if (size < 0 || size > sizeof(topic_name) - 1) {
        Log_e("topic content length not enough! content size:%d  buf size:%d", size, (int)sizeof(topic_name));
        return QCLOUD_ERR_FAILURE;
    }
    SubscribeParams sub_params      = DEFAULT_SUB_PARAMS;
    sub_params.on_message_handler   = _system_mqtt_message_callback;
    sub_params.on_sub_event_handler = _system_mqtt_sub_event_handler;
    sub_params.user_data            = (void *)sys_state;
    sub_params.qos                  = QOS0;

    return IOT_MQTT_Subscribe(pClient, topic_name, &sub_params);
}

int IOT_Get_SysTime(void *pClient, long *time)
{
    int ret    = 0;
    int cntSub = 0;
    int cntRev = 0;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
    Qcloud_IoT_Client *mqtt_client = (Qcloud_IoT_Client *)pClient;

    SysMQTTState *sys_state = &mqtt_client->sys_state;
    // subscribe sys topic: $sys/operation/get/${productid}/${devicename}
    // skip this if the subscription is done and valid
    if (!sys_state->topic_sub_ok) {
        for (cntSub = 0; cntSub < 3; cntSub++) {
            ret = _iot_system_info_result_subscribe(mqtt_client);
            if (ret < 0) {
                Log_w("_iot_system_info_result_subscribe failed: %d, cnt: %d", ret, cntSub);
                continue;
            }

            /* wait for sub ack */
            ret = qcloud_iot_mqtt_yield_mt((Qcloud_IoT_Client *)pClient, 100);
            if (ret || sys_state->topic_sub_ok) {
                break;
            }
        }
    }

    // return failure if subscribe failed
    if (!sys_state->topic_sub_ok) {
        Log_e("Subscribe sys topic failed!");
        return QCLOUD_ERR_FAILURE;
    }

    sys_state->result_recv_ok = false;
    // publish msg to get system timestamp
    ret = _iot_system_info_get_publish(mqtt_client);
    if (ret < 0) {
        Log_e("client publish sys topic failed :%d.", ret);
        return ret;
    }

    do {
        ret = qcloud_iot_mqtt_yield_mt((Qcloud_IoT_Client *)pClient, 100);
        cntRev++;
    } while (!ret && !sys_state->result_recv_ok && cntRev < 20);

    if (sys_state->result_recv_ok) {
        *time = sys_state->time;
        ret   = QCLOUD_RET_SUCCESS;
    } else {
        *time = 0;
        ret   = QCLOUD_ERR_FAILURE;
    }

    return ret;
}

int IOT_Sync_NTPTime(void *pClient)
{
    int ret    = 0;
    int cntSub = 0;
    int cntRev = 0;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
    Qcloud_IoT_Client *mqtt_client = (Qcloud_IoT_Client *)pClient;

    SysMQTTState *sys_state = &mqtt_client->sys_state;
    // subscribe sys topic: $sys/operation/get/${productid}/${devicename}
    // skip this if the subscription is done and valid
    if (!sys_state->topic_sub_ok) {
        for (cntSub = 0; cntSub < 3; cntSub++) {
            ret = _iot_system_info_result_subscribe(mqtt_client);
            if (ret < 0) {
                Log_w("_iot_system_info_result_subscribe failed: %d, cnt: %d", ret, cntSub);
                continue;
            }

            /* wait for sub ack */
            ret = qcloud_iot_mqtt_yield_mt((Qcloud_IoT_Client *)pClient, 100);
            if (ret || sys_state->topic_sub_ok) {
                break;
            }
        }
    }

    // return failure if subscribe failed
    if (!sys_state->topic_sub_ok) {
        Log_e("Subscribe sys topic failed!");
        return QCLOUD_ERR_FAILURE;
    }

    size_t localtime_before_ms = 0;
    // 64bit platform
    if (4 != sizeof(size_t)) {
        localtime_before_ms = HAL_GetTimeMs();
    }
    sys_state->result_recv_ok = false;
    // publish msg to get system timestamp
    ret = _iot_system_info_get_publish(mqtt_client);
    if (ret < 0) {
        Log_e("client publish sys topic failed :%d.", ret);
        return ret;
    }

    do {
        ret = qcloud_iot_mqtt_yield_mt((Qcloud_IoT_Client *)pClient, 100);
        cntRev++;
    } while (!ret && !sys_state->result_recv_ok && cntRev < 20);

    if (sys_state->result_recv_ok) {
        // 32bit platform
        if (4 == sizeof(size_t)) {
            ret = HAL_Timer_set_systime_sec(sys_state->time);
            if (0 != ret) {
                Log_e("set systime sec failed, timestamp %ld sec,  please check permission or other ret:%d",
                      sys_state->time, ret);
            } else {
                Log_i("set systime sec success, timestamp %ld sec", sys_state->time);
            }
        } else {
            // 64bit platform
            size_t localtime_after_ms = HAL_GetTimeMs();
            size_t local_ntptime =
                ((sys_state->ntptime2 + sys_state->ntptime1 + localtime_after_ms - localtime_before_ms) / 2);
            ret = HAL_Timer_set_systime_ms(local_ntptime);
            if (0 != ret) {
                Log_e("set systime ms failed, timestamp %lld, please check permission or other ret :%d", local_ntptime,
                      ret);
            } else {
                Log_i("set systime ms success, timestamp %lld ms", local_ntptime);
            }
        }
        ret = QCLOUD_RET_SUCCESS;
    } else {
        ret = QCLOUD_ERR_FAILURE;
    }

    return ret;
}

int IOT_Ssh_state_report(void *pClient, int state)
{
    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);

    Qcloud_IoT_Client *mqtt_client = (Qcloud_IoT_Client *)pClient;
    DeviceInfo *       dev_info    = &mqtt_client->device_info;
    POINTER_SANITY_CHECK(dev_info, QCLOUD_ERR_INVAL);

    char topic_name[128]      = {0};
    char payload_content[128] = {0};

    HAL_Snprintf(topic_name, sizeof(topic_name), "$sys/operation/%s/%s",
                 STRING_PTR_PRINT_SANITY_CHECK(dev_info->product_id),
                 STRING_PTR_PRINT_SANITY_CHECK(dev_info->device_name));
    HAL_Snprintf(payload_content, sizeof(payload_content), "{\"type\": \"ssh\", \"switch\": %d}", state);

    PublishParams pub_params = DEFAULT_PUB_PARAMS;
    pub_params.qos           = QOS0;
    pub_params.payload       = payload_content;
    pub_params.payload_len   = strlen(payload_content);

    return IOT_MQTT_Publish(mqtt_client, topic_name, &pub_params);
}

int IOT_Ssh_enable(void *pClient)
{
    int ret    = 0;
    int cntSub = 0;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
    Qcloud_IoT_Client *mqtt_client = (Qcloud_IoT_Client *)pClient;

    SysMQTTState *sys_state = &mqtt_client->sys_state;
    // subscribe sys topic: $sys/operation/get/${productid}/${devicename}
    // skip this if the subscription is done and valid
    if (!sys_state->topic_sub_ok) {
        for (cntSub = 0; cntSub < 3; cntSub++) {
            ret = _iot_system_info_result_subscribe(mqtt_client);
            if (ret < 0) {
                Log_w("_iot_system_info_result_subscribe failed: %d, cnt: %d", ret, cntSub);
                continue;
            }

            /* wait for sub ack */
            ret = qcloud_iot_mqtt_yield_mt((Qcloud_IoT_Client *)pClient, 100);
            if (ret || sys_state->topic_sub_ok) {
                break;
            }
        }
    }

    // return failure if subscribe failed
    if (!sys_state->topic_sub_ok) {
        Log_e("Subscribe sys topic failed!");
        return QCLOUD_ERR_FAILURE;
    }

    ret = IOT_Ssh_state_report(mqtt_client, 0);
    if (ret < 0) {
        Log_e("client publish ssh info failed :%d.", ret);
    }

    return ret;
}

#ifdef __cplusplus
}
#endif

#endif
