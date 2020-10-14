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

#include <string.h>

#include "gateway_common.h"
#include "mqtt_client.h"
#include "utils_param_check.h"

void _gateway_event_handler(void *client, void *context, MQTTEventMsg *msg)
{
    uintptr_t packet_id = (uintptr_t)msg->msg;
    Gateway * gateway   = (Gateway *)context;

    POINTER_SANITY_CHECK_RTN(context);
    POINTER_SANITY_CHECK_RTN(msg);
    MQTTMessage *topic_info = (MQTTMessage *)msg->msg;

    switch (msg->event_type) {
        case MQTT_EVENT_SUBCRIBE_SUCCESS:
        case MQTT_EVENT_UNSUBCRIBE_SUCCESS:
            Log_d("gateway sub|unsub(%d) success, packet-id=%u", msg->event_type, (unsigned int)packet_id);
            if (gateway->gateway_data.sync_status == packet_id) {
                gateway->gateway_data.sync_status = 0;
                return;
            }
            break;

        case MQTT_EVENT_SUBCRIBE_TIMEOUT:
        case MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
        case MQTT_EVENT_SUBCRIBE_NACK:
        case MQTT_EVENT_UNSUBCRIBE_NACK:
            Log_d("gateway timeout|nack(%d) event, packet-id=%u", msg->event_type, (unsigned int)packet_id);
            if (gateway->gateway_data.sync_status == packet_id) {
                gateway->gateway_data.sync_status = -1;
                return;
            }
            break;

        case MQTT_EVENT_PUBLISH_RECVEIVED:
            Log_d("gateway topic message arrived but without any related handle: topic=%.*s, topic_msg=%.*s",
                  topic_info->topic_len, topic_info->ptopic, topic_info->payload_len, topic_info->payload);
            break;

        default:
            break;
    }

    if (gateway->event_handle.h_fp != NULL) {
        gateway->event_handle.h_fp(client, gateway->event_handle.context, msg);
    }

    return;
}

void *IOT_Gateway_Construct(GatewayInitParam *init_param)
{
    int          rc    = 0;
    GatewayParam param = DEFAULT_GATEWAY_PARAMS;
    POINTER_SANITY_CHECK(init_param, NULL);

    Gateway *gateway = (Gateway *)HAL_Malloc(sizeof(Gateway));
    if (gateway == NULL) {
        Log_e("gateway malloc failed");
        IOT_FUNC_EXIT_RC(NULL);
    }

    memset(gateway, 0, sizeof(Gateway));

    /* replace user event handle */
    gateway->event_handle.h_fp    = init_param->init_param.event_handle.h_fp;
    gateway->event_handle.context = init_param->init_param.event_handle.context;

    /* set _gateway_event_handler as mqtt event handle */
    init_param->init_param.event_handle.h_fp    = _gateway_event_handler;
    init_param->init_param.event_handle.context = gateway;

    /* construct MQTT client */
    gateway->mqtt = IOT_MQTT_Construct(&init_param->init_param);
    if (NULL == gateway->mqtt) {
        Log_e("construct MQTT failed");
        HAL_Free(gateway);
        IOT_FUNC_EXIT_RC(NULL);
    }

    /* subscribe default topic */
    param.product_id  = init_param->init_param.product_id;
    param.device_name = init_param->init_param.device_name;
    rc                = gateway_subscribe_unsubscribe_default(gateway, &param);
    if (QCLOUD_RET_SUCCESS != rc) {
        Log_e("subscribe default topic failed");
        IOT_Gateway_Destroy((void *)gateway);
        IOT_FUNC_EXIT_RC(NULL);
    }

    return (void *)gateway;
}

int IOT_Gateway_Subdev_Online(void *client, GatewayParam *param)
{
    int            rc                                      = 0;
    char           topic[MAX_SIZE_OF_CLOUD_TOPIC + 1]      = {0};
    char           payload[GATEWAY_PAYLOAD_BUFFER_LEN + 1] = {0};
    int            size                                    = 0;
    SubdevSession *session                                 = NULL;
    PublishParams  params                                  = DEFAULT_PUB_PARAMS;
    Gateway *      gateway                                 = (Gateway *)client;

    POINTER_SANITY_CHECK(gateway, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(param, QCLOUD_ERR_INVAL);

    STRING_PTR_SANITY_CHECK(param->product_id, QCLOUD_ERR_INVAL);
    STRING_PTR_SANITY_CHECK(param->device_name, QCLOUD_ERR_INVAL);
    STRING_PTR_SANITY_CHECK(param->subdev_product_id, QCLOUD_ERR_INVAL);
    STRING_PTR_SANITY_CHECK(param->subdev_device_name, QCLOUD_ERR_INVAL);

    session = subdev_find_session(gateway, param->subdev_product_id, param->subdev_device_name);
    if (NULL == session) {
        Log_d("there is no session, create a new session");

        /* create subdev session */
        session = subdev_add_session(gateway, param->subdev_product_id, param->subdev_device_name);
        if (NULL == session) {
            Log_e("create session error!");
            IOT_FUNC_EXIT_RC(QCLOUD_ERR_GATEWAY_CREATE_SESSION_FAIL);
        }
    } else {
        if (SUBDEV_SEESION_STATUS_ONLINE == session->session_status) {
            Log_i("device have online");
            IOT_FUNC_EXIT_RC(QCLOUD_ERR_GATEWAY_SUBDEV_ONLINE);
        }
    }

    size = HAL_Snprintf(topic, MAX_SIZE_OF_CLOUD_TOPIC + 1, GATEWAY_TOPIC_OPERATION_FMT, param->product_id,
                        param->device_name);
    if (size < 0 || size > MAX_SIZE_OF_CLOUD_TOPIC) {
        Log_e("buf size < topic length!");
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    size = HAL_Snprintf(payload, GATEWAY_PAYLOAD_BUFFER_LEN + 1, GATEWAY_PAYLOAD_STATUS_FMT, "online",
                        param->subdev_product_id, param->subdev_device_name);
    if (size < 0 || size > GATEWAY_PAYLOAD_BUFFER_LEN) {
        Log_e("buf size < payload length!");
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    size = HAL_Snprintf(gateway->gateway_data.online.client_id, MAX_SIZE_OF_CLIENT_ID, GATEWAY_CLIENT_ID_FMT,
                        param->subdev_product_id, param->subdev_device_name);
    if (size < 0 || size > MAX_SIZE_OF_CLIENT_ID) {
        Log_e("buf size < client_id length!");
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }
    gateway->gateway_data.online.result = -1001;

    params.qos         = QOS0;
    params.payload_len = strlen(payload);
    params.payload     = (char *)payload;

    /* publish packet */
    rc = gateway_publish_sync(gateway, topic, &params, &gateway->gateway_data.online.result);
    if (QCLOUD_RET_SUCCESS != rc) {
        subdev_remove_session(gateway, param->subdev_product_id, param->subdev_device_name);
        IOT_FUNC_EXIT_RC(gateway->gateway_data.online.result);
    }

    session->session_status = SUBDEV_SEESION_STATUS_ONLINE;
    IOT_FUNC_EXIT_RC(QCLOUD_RET_SUCCESS);
}

int IOT_Gateway_Subdev_Offline(void *client, GatewayParam *param)
{
    int            rc                                      = 0;
    char           topic[MAX_SIZE_OF_CLOUD_TOPIC + 1]      = {0};
    char           payload[GATEWAY_PAYLOAD_BUFFER_LEN + 1] = {0};
    int            size                                    = 0;
    SubdevSession *session                                 = NULL;
    Gateway *      gateway                                 = (Gateway *)client;

    POINTER_SANITY_CHECK(gateway, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(param, QCLOUD_ERR_INVAL);

    STRING_PTR_SANITY_CHECK(param->product_id, QCLOUD_ERR_INVAL);
    STRING_PTR_SANITY_CHECK(param->device_name, QCLOUD_ERR_INVAL);
    STRING_PTR_SANITY_CHECK(param->subdev_product_id, QCLOUD_ERR_INVAL);
    STRING_PTR_SANITY_CHECK(param->subdev_device_name, QCLOUD_ERR_INVAL);

    session = subdev_find_session(gateway, param->subdev_product_id, param->subdev_device_name);
    if (NULL == session) {
        Log_d("no session, can not offline");
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_GATEWAY_SESSION_NO_EXIST);
    }
    if (SUBDEV_SEESION_STATUS_OFFLINE == session->session_status) {
        Log_i("device have offline");
        /* free session */
        subdev_remove_session(gateway, param->subdev_product_id, param->subdev_device_name);
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_GATEWAY_SUBDEV_OFFLINE);
    }

    size = HAL_Snprintf(topic, MAX_SIZE_OF_CLOUD_TOPIC + 1, GATEWAY_TOPIC_OPERATION_FMT, param->product_id,
                        param->device_name);
    if (size < 0 || size > MAX_SIZE_OF_CLOUD_TOPIC) {
        Log_e("buf size < topic length!");
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    size = HAL_Snprintf(payload, GATEWAY_PAYLOAD_BUFFER_LEN + 1, GATEWAY_PAYLOAD_STATUS_FMT, "offline",
                        param->subdev_product_id, param->subdev_device_name);
    if (size < 0 || size > GATEWAY_PAYLOAD_BUFFER_LEN) {
        Log_e("buf size < payload length!");
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    size = HAL_Snprintf(gateway->gateway_data.offline.client_id, MAX_SIZE_OF_CLIENT_ID, GATEWAY_CLIENT_ID_FMT,
                        param->subdev_product_id, param->subdev_device_name);
    if (size < 0 || size > MAX_SIZE_OF_CLIENT_ID) {
        Log_e("buf size < client_id length!");
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }
    gateway->gateway_data.offline.result = -1001;

    PublishParams params = DEFAULT_PUB_PARAMS;
    params.qos           = QOS0;
    params.payload_len   = strlen(payload);
    params.payload       = (char *)payload;

    /* publish packet */
    rc = gateway_publish_sync(gateway, topic, &params, &gateway->gateway_data.offline.result);
    if (QCLOUD_RET_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(gateway->gateway_data.offline.result);
    }

    session->session_status = SUBDEV_SEESION_STATUS_OFFLINE;

    /* free session */
    subdev_remove_session(gateway, param->subdev_product_id, param->subdev_device_name);

    IOT_FUNC_EXIT_RC(QCLOUD_RET_SUCCESS);
}

int IOT_Gateway_Subdev_GetBindList(void *client, GatewayParam *param, SubdevBindList *subdev_bindlist)
{
    POINTER_SANITY_CHECK(client, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(param, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(subdev_bindlist, QCLOUD_ERR_INVAL);

    char     topic[MAX_SIZE_OF_CLOUD_TOPIC + 1];
    char     payload[GATEWAY_PAYLOAD_BUFFER_LEN + 1];
    int      size    = 0;
    Gateway *gateway = (Gateway *)client;

    gateway->bind_list.bindlist_head = NULL;
    gateway->bind_list.bind_num = 0;

    memset(topic, 0, MAX_SIZE_OF_CLOUD_TOPIC);
    size = HAL_Snprintf(topic, MAX_SIZE_OF_CLOUD_TOPIC + 1, GATEWAY_TOPIC_OPERATION_FMT, param->product_id,
                        param->device_name);
    if (size < 0 || size > MAX_SIZE_OF_CLOUD_TOPIC) {
        Log_e("buf size < topic length!");
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    size = HAL_Snprintf(payload, GATEWAY_PAYLOAD_BUFFER_LEN, "{\"type\":\"%s\"}", GATEWAY_DESCRIBE_SUBDEVIES_OP_STR);
    if (size < 0 || size > GATEWAY_PAYLOAD_BUFFER_LEN) {
        Log_e("buf size < payload length!");
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    PublishParams params = DEFAULT_PUB_PARAMS;
    params.qos           = QOS0;
    params.payload_len   = strlen(payload);
    params.payload       = (char *)payload;

    /* publish packet */
    gateway->gateway_data.get_bindlist.result = -1001;
    int rc = gateway_publish_sync(gateway, topic, &params, &gateway->gateway_data.get_bindlist.result);
    if (QCLOUD_RET_SUCCESS != rc) {
        Log_e("get bind list failed :%d!", rc);
        IOT_FUNC_EXIT_RC(gateway->gateway_data.get_bindlist.result);
    }

    subdev_bindlist->bindlist_head = gateway->bind_list.bindlist_head;
    subdev_bindlist->bind_num = gateway->bind_list.bind_num;

    gateway->bind_list.bindlist_head = NULL;
    gateway->bind_list.bind_num = 0;

    IOT_FUNC_EXIT_RC(QCLOUD_RET_SUCCESS);
}

void IOT_Gateway_Subdev_DestoryBindList(SubdevBindList *subdev_bindlist)
{
    POINTER_SANITY_CHECK_RTN(subdev_bindlist);
    
    SubdevBindInfo *cur_bindinfo = subdev_bindlist->bindlist_head;
    while (cur_bindinfo) {
        SubdevBindInfo *bindinfo = cur_bindinfo;
        cur_bindinfo             = cur_bindinfo->next;
        HAL_Free(bindinfo);
    }

    subdev_bindlist->bindlist_head = NULL;
    subdev_bindlist->bind_num = 0;
}

int IOT_Gateway_Subdev_Bind(void *client, GatewayParam *param, DeviceInfo *pBindSubDevInfo)
{
    char     topic[MAX_SIZE_OF_CLOUD_TOPIC + 1];
    char     payload[GATEWAY_PAYLOAD_BUFFER_LEN + 1];
    int      size    = 0;
    Gateway *gateway = (Gateway *)client;

    memset(topic, 0, MAX_SIZE_OF_CLOUD_TOPIC);
    size = HAL_Snprintf(topic, MAX_SIZE_OF_CLOUD_TOPIC + 1, GATEWAY_TOPIC_OPERATION_FMT, param->product_id,
                        param->device_name);
    if (size < 0 || size > MAX_SIZE_OF_CLOUD_TOPIC) {
        Log_e("buf size < topic length!");
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    srand((unsigned)HAL_GetTimeMs());
    int  nonce     = rand();
    long timestamp = HAL_Timer_current_sec();

    /*cal sign*/
    char sign[SUBDEV_BIND_SIGN_LEN];
    memset(sign, 0, SUBDEV_BIND_SIGN_LEN);
    if (QCLOUD_RET_SUCCESS !=
        subdev_bind_hmac_sha1_cal(pBindSubDevInfo, sign, SUBDEV_BIND_SIGN_LEN, nonce, timestamp)) {
        Log_e("cal sign fail");
        return QCLOUD_ERR_FAILURE;
    }
    memset(payload, 0, GATEWAY_PAYLOAD_BUFFER_LEN);
#ifdef AUTH_MODE_CERT
    size = HAL_Snprintf(payload, GATEWAY_PAYLOAD_BUFFER_LEN + 1, GATEWAY_PAYLOAD_OP_FMT, GATEWAY_BIND_OP_STR,
                        pBindSubDevInfo->product_id, pBindSubDevInfo->device_name, sign, nonce, timestamp, "hmacsha1",
                        "certificate");
#else
    size = HAL_Snprintf(payload, GATEWAY_PAYLOAD_BUFFER_LEN + 1, GATEWAY_PAYLOAD_OP_FMT, GATEWAY_BIND_OP_STR,
                        pBindSubDevInfo->product_id, pBindSubDevInfo->device_name, sign, nonce, timestamp, "hmacsha1",
                        "psk");
#endif

    if (size < 0 || size > GATEWAY_PAYLOAD_BUFFER_LEN) {
        Log_e("buf size < payload length!");
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    size = HAL_Snprintf(gateway->gateway_data.bind.client_id, MAX_SIZE_OF_CLIENT_ID, GATEWAY_CLIENT_ID_FMT,
                        pBindSubDevInfo->product_id, pBindSubDevInfo->device_name);
    if (size < 0 || size > MAX_SIZE_OF_CLIENT_ID) {
        Log_e("buf size < client_id length!");
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    PublishParams params = DEFAULT_PUB_PARAMS;
    params.qos           = QOS0;
    params.payload_len   = strlen(payload);
    params.payload       = (char *)payload;

    /* publish packet */
    gateway->gateway_data.bind.result = -1001;
    int rc = gateway_publish_sync(gateway, topic, &params, &gateway->gateway_data.bind.result);
    if (QCLOUD_RET_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(gateway->gateway_data.bind.result);
    }

    IOT_FUNC_EXIT_RC(QCLOUD_RET_SUCCESS);
}

int IOT_Gateway_Subdev_Unbind(void *client, GatewayParam *param, DeviceInfo *pSubDevInfo)
{
    char     topic[MAX_SIZE_OF_CLOUD_TOPIC + 1];
    char     payload[GATEWAY_PAYLOAD_BUFFER_LEN + 1];
    int      size    = 0;
    Gateway *gateway = (Gateway *)client;

    memset(topic, 0, MAX_SIZE_OF_CLOUD_TOPIC);
    size = HAL_Snprintf(topic, MAX_SIZE_OF_CLOUD_TOPIC + 1, GATEWAY_TOPIC_OPERATION_FMT, param->product_id,
                        param->device_name);
    if (size < 0 || size > MAX_SIZE_OF_CLOUD_TOPIC) {
        Log_e("buf size < topic length!");
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }
    memset(payload, 0, GATEWAY_PAYLOAD_BUFFER_LEN);
    size = HAL_Snprintf(payload, GATEWAY_PAYLOAD_BUFFER_LEN + 1, GATEWAY_PAYLOAD_STATUS_FMT, GATEWAY_UNBIND_OP_STR,
                        pSubDevInfo->product_id, pSubDevInfo->device_name);
    if (size < 0 || size > GATEWAY_PAYLOAD_BUFFER_LEN) {
        Log_e("buf size < payload length!");
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    size = HAL_Snprintf(gateway->gateway_data.unbind.client_id, MAX_SIZE_OF_CLIENT_ID, GATEWAY_CLIENT_ID_FMT,
                        pSubDevInfo->product_id, pSubDevInfo->device_name);
    if (size < 0 || size > MAX_SIZE_OF_CLIENT_ID) {
        Log_e("buf size < client_id length!");
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    PublishParams params = DEFAULT_PUB_PARAMS;
    params.qos           = QOS0;
    params.payload_len   = strlen(payload);
    params.payload       = (char *)payload;

    /* publish packet */
    gateway->gateway_data.unbind.result = -1001;
    int rc = gateway_publish_sync(gateway, topic, &params, &gateway->gateway_data.unbind.result);
    if (QCLOUD_RET_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(gateway->gateway_data.unbind.result);
    }

    IOT_FUNC_EXIT_RC(QCLOUD_RET_SUCCESS);
}

void *IOT_Gateway_Get_Mqtt_Client(void *client)
{
    POINTER_SANITY_CHECK(client, NULL);
    Gateway *gateway = (Gateway *)client;

    return gateway->mqtt;
}

int IOT_Gateway_Destroy(void *client)
{
    Gateway *gateway = (Gateway *)client;
    POINTER_SANITY_CHECK(gateway, QCLOUD_ERR_INVAL);

    SubdevSession *cur_session = gateway->session_list;
    while (cur_session) {
        SubdevSession *session = cur_session;
        cur_session            = cur_session->next;
        HAL_Free(session);
    }

    IOT_MQTT_Destroy(&gateway->mqtt);
    HAL_Free(client);

    IOT_FUNC_EXIT_RC(QCLOUD_RET_SUCCESS)
}

int IOT_Gateway_Yield(void *client, uint32_t timeout_ms)
{
    Gateway *gateway = (Gateway *)client;
    POINTER_SANITY_CHECK(gateway, QCLOUD_ERR_INVAL);

    return IOT_MQTT_Yield(gateway->mqtt, timeout_ms);
}

int IOT_Gateway_Subscribe(void *client, char *topic_filter, SubscribeParams *params)
{
    Gateway *gateway = (Gateway *)client;
    POINTER_SANITY_CHECK(gateway, QCLOUD_ERR_INVAL);

    return IOT_MQTT_Subscribe(gateway->mqtt, topic_filter, params);
}

int IOT_Gateway_Unsubscribe(void *client, char *topic_filter)
{
    Gateway *gateway = (Gateway *)client;
    POINTER_SANITY_CHECK(gateway, QCLOUD_ERR_INVAL);

    return IOT_MQTT_Unsubscribe(gateway->mqtt, topic_filter);
}

int IOT_Gateway_IsSubReady(void *client, char *topic_filter)
{
    Gateway *gateway = (Gateway *)client;
    POINTER_SANITY_CHECK(gateway, QCLOUD_ERR_INVAL);

    return IOT_MQTT_IsSubReady(gateway->mqtt, topic_filter);
}

int IOT_Gateway_Publish(void *client, char *topic_name, PublishParams *params)
{
    Gateway *gateway = (Gateway *)client;
    POINTER_SANITY_CHECK(gateway, QCLOUD_ERR_INVAL);

    return IOT_MQTT_Publish(gateway->mqtt, topic_name, params);
}
