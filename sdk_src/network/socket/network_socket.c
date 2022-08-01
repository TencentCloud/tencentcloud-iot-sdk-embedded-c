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

#include "network_interface.h"
#include "qcloud_iot_export_error.h"
#include "qcloud_iot_import.h"
#include "utils_param_check.h"
#include "utils_websocket_client.h"
#include "utils_ringbuff.h"
#include <string.h>
#include "utils_timer.h"

/*
 * TCP/UDP socket API
 */

int network_tcp_init(Network *pNetwork)
{
    return QCLOUD_RET_SUCCESS;
}

int network_tcp_connect(Network *pNetwork)
{
    POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

    pNetwork->handle = HAL_TCP_Connect(pNetwork->host, pNetwork->port);
    if (0 == pNetwork->handle) {
        return -1;
    }

    return 0;
}

int network_tcp_read(Network *pNetwork, unsigned char *data, size_t datalen, uint32_t timeout_ms, size_t *read_len)
{
    POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

    int rc = 0;

    rc = HAL_TCP_Read(pNetwork->handle, data, (uint32_t)datalen, timeout_ms, read_len);

    return rc;
}

int network_tcp_write(Network *pNetwork, unsigned char *data, size_t datalen, uint32_t timeout_ms, size_t *written_len)
{
    POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

    int rc = 0;

    rc = HAL_TCP_Write(pNetwork->handle, data, datalen, timeout_ms, written_len);

    return rc;
}

void network_tcp_disconnect(Network *pNetwork)
{
    POINTER_SANITY_CHECK_RTN(pNetwork);

    if (0 == pNetwork->handle) {
        return;
    }

    HAL_TCP_Disconnect(pNetwork->handle);
    pNetwork->handle = 0;
    return;
}

#if (defined COAP_COMM_ENABLED) && (defined AUTH_WITH_NOTLS)

int network_udp_init(Network *pNetwork)
{
    return QCLOUD_RET_SUCCESS;
}

int network_udp_read(Network *pNetwork, unsigned char *data, size_t datalen, uint32_t timeout_ms, size_t *read_len)
{
    POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

    int ret = HAL_UDP_ReadTimeout(pNetwork->handle, data, datalen, timeout_ms);
    if (ret > 0) {
        *read_len = ret;
        ret       = 0;
    }

    return ret;
}

int network_udp_write(Network *pNetwork, unsigned char *data, size_t datalen, uint32_t timeout_ms, size_t *written_len)
{
    POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

    int ret = HAL_UDP_Write(pNetwork->handle, data, datalen);
    if (ret > 0) {
        *written_len = ret;
        ret          = 0;
    }

    return ret;
}

void network_udp_disconnect(Network *pNetwork)
{
    POINTER_SANITY_CHECK_RTN(pNetwork);

    HAL_UDP_Disconnect(pNetwork->handle);
    pNetwork->handle = 0;

    return;
}

int network_udp_connect(Network *pNetwork)
{
    POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

    pNetwork->handle = HAL_UDP_Connect(pNetwork->host, pNetwork->port);
    if (0 == pNetwork->handle) {
        return -1;
    }
    return 0;
}

#endif

#ifdef WEBSOCKET_MQTT
/*
 * websocket API
 */
typedef struct {
    UtilsIotWSClientCtx websocket_ctx;
    sRingbuff           recv_ring_buff;
    char                recv_buf[QCLOUD_IOT_MQTT_RX_BUF_LEN];
    char                disconnected;
} QcloudWebsocketMqtt;

static void _websocket_mqtt_recv_data_error(void *iotwsctx)
{
    QcloudWebsocketMqtt *ws_mqtt = (QcloudWebsocketMqtt *)iotwsctx;
    ws_mqtt->disconnected        = 1;

    Log_e("recv error");
}

static void _websocket_mqtt_send_data_error(void *iotwsctx)
{
    QcloudWebsocketMqtt *ws_mqtt = (QcloudWebsocketMqtt *)iotwsctx;
    ws_mqtt->disconnected        = 1;

    Log_e("send error");
}

static void _websocket_mqtt_recv_msg_callback(char *data, int datalen, void *iotwsctx)
{
    Log_d("actual recv len:%d", datalen);
    QcloudWebsocketMqtt *ws_mqtt = (QcloudWebsocketMqtt *)iotwsctx;
    int                  ret     = ring_buff_push_data(&(ws_mqtt->recv_ring_buff), (uint8_t *)data, datalen);
    if (ret != RINGBUFF_OK) {
        Log_e("ring buff push data error %d", ret);
    }
}

int network_websocket_mqtt_init(Network *pNetwork)
{
    return QCLOUD_RET_SUCCESS;
}

int network_websocket_mqtt_connect(Network *pNetwork)
{
    POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);
    QcloudWebsocketMqtt *ws_mqtt = HAL_Malloc(sizeof(QcloudWebsocketMqtt));
    memset(ws_mqtt, 0, sizeof(QcloudWebsocketMqtt));

    int ret = QCLOUD_RET_SUCCESS;

    ws_mqtt->websocket_ctx.network_stack.host = pNetwork->host;
    ws_mqtt->websocket_ctx.network_stack.port = pNetwork->port;

    ws_mqtt->websocket_ctx.network_stack.type = NETWORK_TCP;
#ifndef AUTH_WITH_NOTLS
    ws_mqtt->websocket_ctx.network_stack.ssl_connect_params = pNetwork->ssl_connect_params;
    ws_mqtt->websocket_ctx.network_stack.type               = NETWORK_TLS;
#endif

    ws_mqtt->websocket_ctx.send_data_err          = _websocket_mqtt_send_data_error;
    ws_mqtt->websocket_ctx.recv_data_err          = _websocket_mqtt_recv_data_error;
    ws_mqtt->websocket_ctx.proc_recv_msg_callback = _websocket_mqtt_recv_msg_callback;
    ret                                           = Utils_WSClient_connect("mqtt", &(ws_mqtt->websocket_ctx), NULL);
    if (ret != QCLOUD_RET_SUCCESS) {
        Log_d("websocket free");
        HAL_Free(ws_mqtt);
        return -1;
    }

    ring_buff_init(&(ws_mqtt->recv_ring_buff), ws_mqtt->recv_buf, sizeof(ws_mqtt->recv_buf));
    ring_buff_flush(&(ws_mqtt->recv_ring_buff));

    pNetwork->handle = (uintptr_t)ws_mqtt;

    Log_d("websocket conn create");

    return 0;
}

int network_websocket_mqtt_read(Network *pNetwork, unsigned char *data, size_t datalen, uint32_t timeout_ms,
                           size_t *read_len)
{
    POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

    QcloudWebsocketMqtt *ws_mqtt = (QcloudWebsocketMqtt *)pNetwork->handle;

    Timer recv_timer;
    InitTimer(&recv_timer);
    countdown_ms(&recv_timer, timeout_ms);
    int read_total_len = 0;

    // Log_d("websocket expect read %dB",datalen);
    do {
        // first from ringbuff
        int ring_read_len =
            ring_buff_pop_data(&(ws_mqtt->recv_ring_buff), data + read_total_len, datalen - read_total_len);

        read_total_len += ring_read_len;
        *read_len = read_total_len;

        // if read len short read from websocket buff
        if (datalen == read_total_len) {
            return QCLOUD_RET_SUCCESS;
        }

        // recv from websocket
        int ret = Utils_WSClient_recv(&ws_mqtt->websocket_ctx);
        if (ret != QCLOUD_RET_SUCCESS) {
            Log_e("%s-%d read error :%d, %d", ws_mqtt->websocket_ctx.network_stack.host,
                  ws_mqtt->websocket_ctx.network_stack.port, ret, ws_mqtt->disconnected);

            return QCLOUD_ERR_TCP_READ_FAIL;
        }
    } while (!expired(&recv_timer));

    if (ws_mqtt->disconnected) {
        return QCLOUD_ERR_TCP_READ_FAIL;
    }
    return QCLOUD_ERR_TCP_NOTHING_TO_READ;
}

int network_websocket_mqtt_write(Network *pNetwork, unsigned char *data, size_t datalen, uint32_t timeout_ms,
                            size_t *written_len)
{
    POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

    QcloudWebsocketMqtt *ws_mqtt = (QcloudWebsocketMqtt *)pNetwork->handle;
    if (QCLOUD_RET_SUCCESS != Utils_WSClient_send(&(ws_mqtt->websocket_ctx), (char *)data, datalen) ||
        ws_mqtt->disconnected) {
        return QCLOUD_ERR_FAILURE;
    }

    *written_len = datalen;
    return QCLOUD_RET_SUCCESS;
}

void network_websocket_mqtt_disconnect(Network *pNetwork)
{
    POINTER_SANITY_CHECK_RTN(pNetwork);

    QcloudWebsocketMqtt *ws_mqtt = (QcloudWebsocketMqtt *)pNetwork->handle;

    if (0 == pNetwork->handle) {
        return;
    }

    Log_d("websocket disconn");
    Utils_WSClient_disconn(&(ws_mqtt->websocket_ctx));
    HAL_Free((void *)(pNetwork->handle));
    pNetwork->handle = 0;
    return;
}
#endif
