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
#include "gateway_common.h"

#include "lite-utils.h"
#include "mqtt_client.h"
#include "utils_base64.h"
#include "utils_md5.h"
#include "utils_hmac.h"
#include "json_parser.h"

enum { SUBDEV_OP_DESCRIBE, SUBDEV_OP_BIND, SUBDEV_OP_UNBIND };
static bool get_json_type(char *json, char **v)
{
    *v = LITE_json_value_of("type", json);
    return *v == NULL ? false : true;
}

static bool get_json_devices(char *json, char **v)
{
    *v = LITE_json_value_of("payload.devices", json);
    return *v == NULL ? false : true;
}

static bool get_json_result(char *json, int32_t *res)
{
    char *v = LITE_json_value_of("result", json);
    if (v == NULL) {
        return false;
    }
    if (LITE_get_int32(res, v) != QCLOUD_RET_SUCCESS) {
        HAL_Free(v);
        return false;
    }
    HAL_Free(v);
    return true;
}

static bool get_json_payload_status(char *json, int32_t *res)
{
    char *v = LITE_json_value_of("payload.status", json);
    if (!v)
        return false;
    bool ret = true;
    if (LITE_get_int32(res, v) != QCLOUD_RET_SUCCESS) {
        ret = false;
    }
    HAL_Free(v);
    return ret;
}

static bool get_json_product_id(char *json, char **v)
{
    *v = LITE_json_value_of("product_id", json);
    return *v == NULL ? false : true;
}

static bool get_json_device_name(char *json, char **v)
{
    *v = LITE_json_value_of("device_name", json);
    return *v == NULL ? false : true;
}

#define MIN(a, b) ((a > b) ? b : a)
static SubdevBindInfo *_subdev_add_bindinfo(Gateway *gateway, char *subdev_product_id, char *subdev_device_name)
{
    SubdevBindInfo *bindinfo = NULL;

    POINTER_SANITY_CHECK(gateway, NULL);
    STRING_PTR_SANITY_CHECK(subdev_product_id, NULL);
    STRING_PTR_SANITY_CHECK(subdev_device_name, NULL);

    bindinfo = HAL_Malloc(sizeof(SubdevBindInfo));
    if (bindinfo == NULL) {
        Log_e("add bindinfo not enough memory");
        IOT_FUNC_EXIT_RC(NULL);
    }

    memset(bindinfo, 0, sizeof(SubdevBindInfo));
    /* add subdev bind info to list */
    bindinfo->next                   = gateway->bind_list.bindlist_head;
    gateway->bind_list.bindlist_head = bindinfo;

    strncpy(bindinfo->product_id, subdev_product_id, MAX_SIZE_OF_PRODUCT_ID);
    bindinfo->product_id[MAX_SIZE_OF_PRODUCT_ID] = '\0';
    int size                                     = strlen(subdev_device_name);
    strncpy(bindinfo->device_name, subdev_device_name, MIN(size, MAX_SIZE_OF_DEVICE_NAME));
    bindinfo->device_name[MIN(size, MAX_SIZE_OF_DEVICE_NAME)] = '\0';

    gateway->bind_list.bind_num += 1;

    IOT_FUNC_EXIT_RC(bindinfo);
}
#undef MIN

static int _subdev_unbind_device(Gateway *gateway, const char *product_id, const char *device_name)
{
    SubdevBindInfo *pre_bindinfo = NULL, *cur_bindinfo = gateway->bind_list.bindlist_head;

    while (cur_bindinfo) {
        if (!strncmp(product_id, cur_bindinfo->product_id, MAX_SIZE_OF_PRODUCT_ID) &&
            !strncmp(device_name, cur_bindinfo->device_name, MAX_SIZE_OF_DEVICE_NAME)) {
            if (pre_bindinfo) {
                pre_bindinfo->next = cur_bindinfo->next;
            } else {
                gateway->bind_list.bindlist_head = cur_bindinfo->next;
            }
            gateway->bind_list.bind_num--;
            HAL_Free(cur_bindinfo);

            IOT_FUNC_EXIT_RC(QCLOUD_RET_SUCCESS);
        }
        pre_bindinfo = cur_bindinfo;
        cur_bindinfo = cur_bindinfo->next;
    }
    Log_e("subdev %s:%s not found", product_id, device_name);
    IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
}

static void _subdev_proc_devlist(Gateway *gateway, char *devices, int type)
{
    char *subdev_product_id  = NULL;
    char *subdev_device_name = NULL;
    char *pos                = NULL;
    char *entry              = NULL;
    int   entry_len          = 0;
    int   entry_type         = 0;
    char  old_ch             = 0;

    int cont = 1;

    // parser json array
    json_array_for_each_entry(devices, pos, entry, entry_len, entry_type)
    {
        if (!entry)
            continue;
        backup_json_str_last_char(entry, entry_len, old_ch);
        cont = 1;
        do {
            if (!get_json_product_id(entry, &subdev_product_id) || !get_json_device_name(entry, &subdev_device_name)) {
                restore_json_str_last_char(entry, entry_len, old_ch);

                break;
            }
            Log_d("entry is %s, %s %s", entry, subdev_product_id, subdev_device_name);
            if (SUBDEV_OP_DESCRIBE == type || SUBDEV_OP_BIND == type) {
                if (!_subdev_add_bindinfo(gateway, subdev_product_id, subdev_device_name)) {
                    Log_e("Failed to add bind info");
                    cont = 0;
                }
            } else if (SUBDEV_OP_UNBIND == type) {
                if (_subdev_unbind_device(gateway, subdev_product_id, subdev_device_name)) {
                    Log_e("failed to unbind device %s:%d", subdev_product_id, subdev_device_name);
                    cont = 0;
                }
            }
        } while (0);

        HAL_Free(subdev_product_id);
        HAL_Free(subdev_device_name);
        restore_json_str_last_char(entry, entry_len, old_ch);
        if (!cont)
            break;
    }

    return;
}

static void _gateway_ack_change(char *devices, Qcloud_IoT_Client *mqtt, int32_t status)
{
    char        reply_buf[1024];
    char        topic_name[128];
    const char *ack_fmt_prefix = "{\"type\":\"change\", \"payload\":{\"status\":%d, \"devices\":[";
    int         ret            = 0;
    char *      p              = reply_buf;
    int         left_sz        = sizeof(reply_buf) - 1;

    if ((ret = HAL_Snprintf(reply_buf, left_sz, ack_fmt_prefix, status)) < 0)
        return;
    p += ret;
    left_sz -= ret;

    char *pos        = NULL;
    char *entry      = NULL;
    int   entry_len  = 0;
    int   entry_type = 0;
    char  old_ch     = 0;

    char *subdev_product_id  = NULL;
    char *subdev_device_name = NULL;
    int   is_first = 1, is_cont = 1;

#define MAX_SUBDEV_INFO_SZ 128

    json_array_for_each_entry(devices, pos, entry, entry_len, entry_type)
    {
        if (!entry)
            continue;
        backup_json_str_last_char(entry, entry_len, old_ch);
        subdev_product_id  = NULL;
        subdev_device_name = NULL;
        is_cont            = 1;

        if (left_sz < MAX_SUBDEV_INFO_SZ)
            break;
        do {
            if (!is_first) {
                if ((ret = HAL_Snprintf(p, left_sz, ",")) < 0) {
                    is_cont = 0;
                    break;
                }
                p += ret;
                left_sz -= ret;
            } else {
                is_first = 0;
            }
            if (!get_json_product_id(entry, &subdev_product_id) || !get_json_device_name(entry, &subdev_device_name)) {
                break;
            }
            ret = HAL_Snprintf(p, left_sz, "{\"product_id\":\"%s\",\"device_name\":\"%s\",\"result\":%d}",
                               subdev_product_id, subdev_device_name, 0);
            if (ret <= 0) {
                is_cont = 0;
                break;
            }
            p += ret;
            left_sz -= ret;
        } while (0);
        restore_json_str_last_char(entry, entry_len, old_ch);
        HAL_Free(subdev_product_id);
        HAL_Free(subdev_device_name);
        if (!is_cont)
            break;
    }
    HAL_Snprintf(p, left_sz, "]}}");

    HAL_Snprintf(topic_name, 128, GATEWAY_TOPIC_OPERATION_FMT, mqtt->device_info.product_id,
                 mqtt->device_info.device_name);
    Log_d("reply %s", reply_buf);

    PublishParams params = DEFAULT_PUB_PARAMS;
    params.qos           = QOS0;
    params.payload_len   = strlen(reply_buf);
    params.payload       = (char *)reply_buf;

    IOT_MQTT_Publish(mqtt, topic_name, &params);
#undef MAX_SUBDEV_INFO_SZ
}

static void _gateway_ack_search(Qcloud_IoT_Client *mqtt, int32_t status)
{
    char        reply_buf[1024];
    char        topic_name[128];
    const char *search_ack_fmt = "{\"type\":\"search_devices\", \"payload\":{\"status\":%d, \"result\":%d}}";

    HAL_Snprintf(reply_buf, sizeof(reply_buf), search_ack_fmt, status, 0);
    HAL_Snprintf(topic_name, 128, GATEWAY_TOPIC_OPERATION_FMT, mqtt->device_info.product_id,
                 mqtt->device_info.device_name);

    PublishParams params = DEFAULT_PUB_PARAMS;
    params.qos           = QOS0;
    params.payload_len   = strlen(reply_buf);
    params.payload       = (char *)reply_buf;

    Log_d("reply %s", reply_buf);

    IOT_MQTT_Publish(mqtt, topic_name, &params);
}

static int _gateway_subdev_unbind_all(Gateway *gateway)
{
    SubdevSession *cur_session = NULL;
    SubdevSession *pre_session = NULL;

    POINTER_SANITY_CHECK(gateway, QCLOUD_ERR_FAILURE);

    pre_session = cur_session = gateway->session_list;

    if (NULL == cur_session) {
        Log_e("session list is empty");
        IOT_FUNC_EXIT_RC(QCLOUD_RET_SUCCESS);
    }

    /* session is exist */
    while (cur_session) {
        pre_session = cur_session;
        cur_session = cur_session->next;
        Log_d("remove all session product id: %s device_name: %s", pre_session->product_id, pre_session->device_name);
        HAL_Free(pre_session);
        pre_session = NULL;
    }

    gateway->session_list = NULL;

    return QCLOUD_RET_SUCCESS;
}

static void _gateway_subdev_unbind_all_reply(Qcloud_IoT_Client *mqtt)
{
    char        reply_buf[1024];
    char        topic_name[128];
    const char *search_ack_fmt = "{\"type\":\"%s\", \"payload\":{\"result\":%d}}";

    HAL_Snprintf(reply_buf, sizeof(reply_buf), search_ack_fmt, GATEWAY_UNBIND_ALL_OP_STR, 0);
    HAL_Snprintf(topic_name, 128, GATEWAY_TOPIC_OPERATION_FMT, mqtt->device_info.product_id,
                 mqtt->device_info.device_name);

    PublishParams params = DEFAULT_PUB_PARAMS;
    params.qos           = QOS0;
    params.payload_len   = strlen(reply_buf);
    params.payload       = (char *)reply_buf;

    Log_d("reply %s", reply_buf);

    IOT_MQTT_Publish(mqtt, topic_name, &params);
}

static void _gateway_message_handler(void *client, MQTTMessage *message, void *user_data)
{
    Qcloud_IoT_Client *mqtt          = NULL;
    Gateway *          gateway       = NULL;
    char *             topic         = NULL;
    size_t             topic_len     = 0;
    int                cloud_rcv_len = 0;
    char *             type          = NULL;
    char *             devices = NULL, *devices_strip = NULL;
    char *             product_id                           = NULL;
    char *             device_name                          = NULL;
    int32_t            result                               = 0;
    char               client_id[MAX_SIZE_OF_CLIENT_ID + 1] = {0};
    int                size                                 = 0;

    POINTER_SANITY_CHECK_RTN(client);
    POINTER_SANITY_CHECK_RTN(message);

    mqtt    = (Qcloud_IoT_Client *)client;
    gateway = (Gateway *)mqtt->event_handle.context;
    POINTER_SANITY_CHECK_RTN(gateway);

    topic     = (char *)message->ptopic;
    topic_len = message->topic_len;
    if (NULL == topic || topic_len == 0) {
        Log_e("topic == NULL or topic_len == 0.");
        return;
    }

    if (message->payload_len > GATEWAY_RECEIVE_BUFFER_LEN) {
        Log_e("message->payload_len > GATEWAY_RECEIVE_BUFFER_LEN.");
        return;
    }

    cloud_rcv_len  = Min(GATEWAY_RECEIVE_BUFFER_LEN - 1, message->payload_len);
    char *json_buf = gateway->recv_buf;
    memcpy(gateway->recv_buf, message->payload, cloud_rcv_len);
    json_buf[cloud_rcv_len] = '\0';  // jsmn_parse relies on a string

    Log_d("msg payload: %s", json_buf);

    if (!get_json_type(json_buf, &type)) {
        Log_e("Fail to parse type from msg: %s", json_buf);
        return;
    }

    if (!strncmp(type, GATEWAY_UNBIND_ALL_OP_STR, sizeof(GATEWAY_UNBIND_ALL_OP_STR) - 1)) {
        Log_d("recv request for unbind_all");

        _gateway_subdev_unbind_all(gateway);
        _gateway_subdev_unbind_all_reply(mqtt);

        MQTTEventMsg msg;
        msg.event_type = MQTT_EVENT_GATEWAY_UNBIND_ALL;
        msg.msg        = NULL;
        mqtt->event_handle.h_fp(mqtt, mqtt->event_handle.context, &msg);

        goto exit;
    }

    if (!strncmp(type, GATEWAY_SEARCH_OP_STR, sizeof(GATEWAY_SEARCH_OP_STR) - 1)) {
        Log_d("recv request for searching");
        int32_t search_status;
        if (get_json_payload_status(json_buf, &search_status)) {
            _gateway_ack_search(mqtt, search_status);

            MQTTEventMsg msg;
            msg.event_type = MQTT_EVENT_GATEWAY_SEARCH;
            msg.msg        = (void *)&search_status;
            mqtt->event_handle.h_fp(mqtt, mqtt->event_handle.context, &msg);
        }
        goto exit;
    }

    if (!get_json_devices(json_buf, &devices)) {
        Log_e("Fail to parse devices from msg: %s", json_buf);
        goto exit;
    }

    if (devices[0] == '[') {
        devices_strip = devices + 1;
    } else {
        devices_strip = devices;
    }

    if (!strncmp(type, GATEWAY_DESCRIBE_SUBDEVIES_OP_STR, sizeof(GATEWAY_DESCRIBE_SUBDEVIES_OP_STR) - 1)) {
        _subdev_proc_devlist(gateway, devices, SUBDEV_OP_DESCRIBE);
        gateway->gateway_data.get_bindlist.result = 0;
        goto exit;
    }

    if (!strncmp(type, GATEWAY_CHANGE_OP_STR, sizeof(GATEWAY_CHANGE_OP_STR) - 1)) {
        Log_d("Get change request from server");
        int32_t change_status = 0, change_type;
        if (get_json_payload_status(json_buf, &change_status)) {
            Log_d("Request status is %d", change_status);
            change_type = change_status ? SUBDEV_OP_BIND : SUBDEV_OP_UNBIND;
            _subdev_proc_devlist(gateway, devices, change_type);
        }
        _gateway_ack_change(devices, mqtt, change_status);
        goto exit;
    }

    if (!get_json_result(devices_strip, &result)) {
        Log_e("Fail to parse result from msg: %s", json_buf);
        goto exit;
    }
    if (!get_json_product_id(devices_strip, &product_id)) {
        Log_e("Fail to parse product_id from msg: %s", json_buf);
        goto exit;
    }
    if (!get_json_device_name(devices_strip, &device_name)) {
        Log_e("Fail to parse device_name from msg: %s", json_buf);
        goto exit;
    }
    size = HAL_Snprintf(client_id, MAX_SIZE_OF_CLIENT_ID + 1, GATEWAY_CLIENT_ID_FMT, product_id, device_name);
    if (size < 0 || size > MAX_SIZE_OF_CLIENT_ID) {
        Log_e("generate client_id fail.");
        goto exit;
    }
    if (!strncmp(type, GATEWAY_ONLINE_OP_STR, sizeof(GATEWAY_ONLINE_OP_STR) - 1)) {
        if (strncmp(client_id, gateway->gateway_data.online.client_id, size) == 0) {
            Log_i("client_id(%s), online result %d", client_id, result);
            gateway->gateway_data.online.result = result;
        }
        goto exit;
    }
    if (!strncmp(type, GATEWAY_OFFLIN_OP_STR, sizeof(GATEWAY_OFFLIN_OP_STR) - 1)) {
        if (strncmp(client_id, gateway->gateway_data.offline.client_id, size) == 0) {
            Log_i("client_id(%s), offline result %d", client_id, result);
            gateway->gateway_data.offline.result = result;
        }
        goto exit;
    }
    if (!strncmp(type, GATEWAY_BIND_OP_STR, sizeof(GATEWAY_BIND_OP_STR) - 1)) {
        if (strncmp(client_id, gateway->gateway_data.bind.client_id, size) == 0) {
            gateway->gateway_data.bind.result = result;
            Log_i("client_id(%s), bind result %d", client_id, gateway->gateway_data.bind.result);
        }
        goto exit;
    }
    if (!strncmp(type, GATEWAY_UNBIND_OP_STR, sizeof(GATEWAY_UNBIND_OP_STR) - 1)) {
        if (strncmp(client_id, gateway->gateway_data.unbind.client_id, size) == 0) {
            gateway->gateway_data.unbind.result = result;
            Log_i("client_id(%s), unbind result %d", client_id, gateway->gateway_data.unbind.result);
        }
    } else {
        Log_e("shouldnt reach here: unknown type %s", type);
    }
exit:
    HAL_Free(type);
    HAL_Free(devices);
    HAL_Free(product_id);
    HAL_Free(device_name);
}

int gateway_subscribe_unsubscribe_topic(Gateway *gateway, char *topic_filter, SubscribeParams *params, int is_subscribe)
{
    int      rc         = 0;
    int      loop_count = 0;
    uint32_t status     = -1;

    POINTER_SANITY_CHECK(gateway, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(params, QCLOUD_ERR_INVAL);

    STRING_PTR_SANITY_CHECK(topic_filter, QCLOUD_ERR_INVAL);

    params->qos                       = QOS1;
    gateway->gateway_data.sync_status = status;

    if (is_subscribe) {
        /* subscribe */
        rc = IOT_MQTT_Subscribe(gateway->mqtt, topic_filter, params);
    } else {
        /* unsubscribe */
        rc = IOT_MQTT_Unsubscribe(gateway->mqtt, topic_filter);
    }

    if (rc < 0) {
        Log_e("subscribe or un(%d), result(%d)", is_subscribe, rc);
        IOT_FUNC_EXIT_RC(rc);
    }

    gateway->gateway_data.sync_status = status = rc;
    while (status == gateway->gateway_data.sync_status) {
        if (loop_count > GATEWAY_LOOP_MAX_COUNT) {
            Log_i("loop max count, time out");
            IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
        }
        IOT_Gateway_Yield(gateway, 200);
        loop_count++;
    }

    if (gateway->gateway_data.sync_status != 0) {
        Log_e("gateway->gateway_data.sync_status(%u) != 0", gateway->gateway_data.sync_status);
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    IOT_FUNC_EXIT_RC(QCLOUD_RET_SUCCESS);
}

int gateway_subscribe_unsubscribe_default(Gateway *gateway, GatewayParam *param)
{
    int             rc                                        = 0;
    int             size                                      = 0;
    char            topic_filter[MAX_SIZE_OF_CLOUD_TOPIC + 1] = {0};
    SubscribeParams subscribe_params                          = DEFAULT_SUB_PARAMS;

    POINTER_SANITY_CHECK(param, QCLOUD_ERR_INVAL);

    STRING_PTR_SANITY_CHECK(param->product_id, QCLOUD_ERR_INVAL);
    STRING_PTR_SANITY_CHECK(param->device_name, QCLOUD_ERR_INVAL);

    // subscribe  online/offline operation reslut
    size = HAL_Snprintf(topic_filter, MAX_SIZE_OF_CLOUD_TOPIC + 1, GATEWAY_TOPIC_OPERATION_RESULT_FMT,
                        param->product_id, param->device_name);
    if (size < 0 || size > MAX_SIZE_OF_CLOUD_TOPIC) {
        Log_e("buf size < topic length!");
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }
    subscribe_params.on_message_handler = _gateway_message_handler;
    rc = gateway_subscribe_unsubscribe_topic(gateway, topic_filter, &subscribe_params, IOT_TRUE);
    if (QCLOUD_RET_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    IOT_FUNC_EXIT_RC(QCLOUD_RET_SUCCESS);
}

SubdevSession *subdev_find_session(Gateway *gateway, char *product_id, char *device_name)
{
    SubdevSession *session = NULL;

    POINTER_SANITY_CHECK(gateway, NULL);
    STRING_PTR_SANITY_CHECK(product_id, NULL);
    STRING_PTR_SANITY_CHECK(device_name, NULL);

    session = gateway->session_list;

    /* session is exist */
    while (session) {
        if (0 == strcmp(session->product_id, product_id) && 0 == strcmp(session->device_name, device_name)) {
            IOT_FUNC_EXIT_RC(session);
        }
        session = session->next;
    }

    IOT_FUNC_EXIT_RC(NULL);
}

SubdevSession *subdev_add_session(Gateway *gateway, char *product_id, char *device_name)
{
    SubdevSession *session = NULL;

    POINTER_SANITY_CHECK(gateway, NULL);
    STRING_PTR_SANITY_CHECK(product_id, NULL);
    STRING_PTR_SANITY_CHECK(device_name, NULL);

    session = HAL_Malloc(sizeof(SubdevSession));
    if (session == NULL) {
        Log_e("Not enough memory");
        IOT_FUNC_EXIT_RC(NULL);
    }

    memset(session, 0, sizeof(SubdevSession));
    /* add session to list */
    session->next         = gateway->session_list;
    gateway->session_list = session;

    int size = strlen(product_id);
    strncpy(session->product_id, product_id, size);
    session->product_id[size] = '\0';
    size                      = strlen(device_name);
    strncpy(session->device_name, device_name, size);
    session->device_name[size] = '\0';
    session->session_status    = SUBDEV_SEESION_STATUS_INIT;

    IOT_FUNC_EXIT_RC(session);
}

int subdev_remove_session(Gateway *gateway, char *product_id, char *device_name)
{
    SubdevSession *cur_session = NULL;
    SubdevSession *pre_session = NULL;

    POINTER_SANITY_CHECK(gateway, QCLOUD_ERR_FAILURE);
    STRING_PTR_SANITY_CHECK(product_id, QCLOUD_ERR_FAILURE);
    STRING_PTR_SANITY_CHECK(device_name, QCLOUD_ERR_FAILURE);

    pre_session = cur_session = gateway->session_list;

    if (NULL == cur_session) {
        Log_e("session list is empty");
        IOT_FUNC_EXIT_RC(QCLOUD_RET_SUCCESS);
    }

    /* session is exist */
    while (cur_session) {
        if (0 == strcmp(cur_session->product_id, product_id) && 0 == strcmp(cur_session->device_name, device_name)) {
            if (cur_session == gateway->session_list) {
                gateway->session_list = cur_session->next;
            } else {
                pre_session->next = cur_session->next;
            }
            HAL_Free(cur_session);
            IOT_FUNC_EXIT_RC(QCLOUD_RET_SUCCESS);
        }
        pre_session = cur_session;
        cur_session = cur_session->next;
    }

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
}

int gateway_publish_sync(Gateway *gateway, char *topic, PublishParams *params, int32_t *result)
{
    int     rc         = 0;
    int     loop_count = 0;
    int32_t res        = *result;

    POINTER_SANITY_CHECK(gateway, QCLOUD_ERR_INVAL);

    rc = IOT_Gateway_Publish(gateway, topic, params);
    if (rc < 0) {
        Log_e("publish fail.");
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    /* wait for response */
    while (res == *result) {
        if (loop_count > GATEWAY_LOOP_MAX_COUNT) {
            Log_i("loop max count, time out.");
            IOT_FUNC_EXIT_RC(QCLOUD_ERR_GATEWAY_SESSION_TIMEOUT);
        }

        IOT_Gateway_Yield(gateway, 200);
        loop_count++;
    }

    if (*result != 0) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }
    IOT_FUNC_EXIT_RC(QCLOUD_RET_SUCCESS);
}

#ifdef AUTH_MODE_CERT
static int gen_key_from_cert_file(const char *file_path, char *keybuff, int buff_len)
{
    FILE *   fp;
    uint32_t length;
    int      ret = QCLOUD_RET_SUCCESS;

    if ((fp = fopen(file_path, "r")) == NULL) {
        Log_e("fail to open cert file %s", file_path);
        return QCLOUD_ERR_FAILURE;
    }

    fseek(fp, 0L, SEEK_END);
    length        = ftell(fp);
    uint8_t *data = HAL_Malloc(length + 1);
    if (!data) {
        Log_e("malloc mem err");
        return QCLOUD_ERR_MALLOC;
    }

    fseek(fp, 0, SEEK_SET);
    if (length != fread(data, 1, length, fp)) {
        Log_e("read data len fail");
        ret = QCLOUD_ERR_FAILURE;
        goto exit;
    }

    utils_md5_str(data, length, (uint8_t *)keybuff);
    Log_d("sign key: %s", STRING_PTR_PRINT_SANITY_CHECK(keybuff));

exit:

    HAL_Free(data);
    fclose(fp);

    return ret;
}

#endif

int subdev_bind_hmac_sha1_cal(DeviceInfo *pDevInfo, char *signout, int max_signlen, int nonce, long timestamp)
{
    int         text_len, ret;
    size_t      olen      = 0;
    char *      pSignText = NULL;
    const char *sign_fmt  = "%s%s;%d;%d";  //${product_id}${device_name};${random};${expiration_time}

    /*format sign data*/
    text_len = strlen(sign_fmt) + strlen(pDevInfo->device_name) + strlen(pDevInfo->product_id) + sizeof(int) +
               sizeof(long) + 10;
    pSignText = HAL_Malloc(text_len);
    if (pSignText == NULL) {
        Log_e("malloc sign source buff fail");
        return QCLOUD_ERR_FAILURE;
    }
    memset(pSignText, 0, text_len);
    HAL_Snprintf((char *)pSignText, text_len, sign_fmt, pDevInfo->product_id, pDevInfo->device_name, nonce, timestamp);

    // gen digest key
    char key[BIND_SIGN_KEY_SIZE + 1] = {0};
#ifdef AUTH_MODE_CERT
    ret = gen_key_from_cert_file(pDevInfo->dev_cert_file_name, key, BIND_SIGN_KEY_SIZE);
    if (QCLOUD_RET_SUCCESS != ret) {
        Log_e("gen key from cert file fail, ret:%d", ret);
        HAL_Free(pSignText);
        return ret;
    }
#else
    strncpy(key, pDevInfo->device_secret, strlen(pDevInfo->device_secret));
#endif

    /*cal hmac sha1*/
    char sign[SUBDEV_BIND_SIGN_LEN] = {0};
    int  sign_len                   = utils_hmac_sha1_hex(pSignText, strlen(pSignText), sign, key, strlen(key));

    /*base64 encode*/
    ret = qcloud_iot_utils_base64encode((uint8_t *)signout, max_signlen, &olen, (const uint8_t *)sign, sign_len);
    HAL_Free(pSignText);

    return (olen > max_signlen) ? QCLOUD_ERR_FAILURE : ret;
}
