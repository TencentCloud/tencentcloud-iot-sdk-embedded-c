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
#include <time.h>

#include "lite-utils.h"
#include "mqtt_client.h"
#include "utils_base64.h"
#include "utils_sha1.h"
#include "utils_hmac.h"
#include "qcloud_iot_device.h"
#include "qcloud_iot_export_system.h"
#include "utils_websocket_client.h"
#include "qcloud_iot_ca.h"
#include "wslay/wslay.h"

#ifdef WEBSOCKET_CLIENT

#define WEBSOCKET_HANDSHAKE_MAX_TIMEOUT_SEC 10

#define TCP_READDATA_MAX_TIMEOUT_MS 10
#define TCP_SENDDATA_MAX_TIMEOUT_MS 500

/* tcp connect */
static int _network_transport_init(Network *pNetwork)
{
    Log_d("init %s:%d, %s", pNetwork->host, pNetwork->port, pNetwork->type == NETWORK_TLS ? "TLS" : "TCP");
    return network_init(pNetwork);
}

static int _network_transport_connect(Network *pNetwork)
{
    Log_d("start connect %s:%d", pNetwork->host, pNetwork->port);

    if (QCLOUD_RET_SUCCESS != pNetwork->connect(pNetwork)) {
        return QCLOUD_ERR_FAILURE;
    }

    Log_d("connected %s:%d, handle:%p", pNetwork->host, pNetwork->port, pNetwork->handle);
    return QCLOUD_RET_SUCCESS;
}

/* tcp send */
static int _network_transport_send(Network *network, char *buf, int len, int timeout_ms)
{
    size_t written_len = 0;
    int    rc          = network->write(network, (unsigned char *)buf, len, timeout_ms, &written_len);
    if (written_len > 0) {
        // Log_d("Written %lu bytes", written_len);
    } else if (written_len == 0) {
        Log_e("written_len == 0,Connection was closed by server");
        return QCLOUD_ERR_FAILURE; /* Connection was closed by server */
    } else {
        Log_e("Connection error (send returned %d)", rc);
        return QCLOUD_ERR_FAILURE;
    }

    return written_len;
}

/* tcp recv */
static int _network_transport_recv(Network *network, char *buf, int buf_len, uint32_t timeout_ms)
{
    int    rc        = 0;
    size_t recv_size = 0;

    rc = network->read(network, (unsigned char *)buf, buf_len, timeout_ms, &recv_size);
    if (rc == QCLOUD_ERR_SSL_NOTHING_TO_READ || rc == QCLOUD_ERR_TCP_NOTHING_TO_READ) {
        rc = QCLOUD_RET_SUCCESS;
    } else if (rc == QCLOUD_ERR_SSL_READ_TIMEOUT || rc == QCLOUD_ERR_TCP_READ_TIMEOUT) {
        rc = QCLOUD_RET_SUCCESS;
    } else if (rc == QCLOUD_ERR_TCP_PEER_SHUTDOWN && recv_size > 0) {
        rc = QCLOUD_RET_SUCCESS;
    } else if (rc != QCLOUD_RET_SUCCESS) {
        Log_e("recv error %d, rc = %d (recv returned %d)", network->handle, rc, recv_size);
        Log_e("recv error %s:%d", network->host, network->port);
        return rc;
    }

    rc = recv_size;

    return rc;
}

/* tcp disconnect */
static int _network_transport_disconnect(Network *network)
{
    network->disconnect(network);
    return QCLOUD_RET_SUCCESS;
}

static ssize_t _websocket_recv_callback(wslay_event_context_ptr ctx, uint8_t *data, size_t len, int flags,
                                        void *user_data)
{
    UtilsIotWSClientCtx *iot_ctx = (UtilsIotWSClientCtx *)user_data;

    int recv_len = _network_transport_recv(&(iot_ctx->network_stack), (char *)data, len, TCP_READDATA_MAX_TIMEOUT_MS);
    if (recv_len < QCLOUD_RET_SUCCESS) {
        Log_e("recv error %d, %d", recv_len, wslay_event_get_close_received(ctx));
        if (iot_ctx->recv_data_err) {
            iot_ctx->recv_data_err(user_data);
        }
        return QCLOUD_ERR_FAILURE;
    }
    return recv_len;
}

static ssize_t _websocket_send_callback(wslay_event_context_ptr ctx, const uint8_t *data, size_t len, int flags,
                                        void *user_data)
{
    UtilsIotWSClientCtx *iot_ctx = (UtilsIotWSClientCtx *)user_data;
    int send_len = _network_transport_send(&(iot_ctx->network_stack), (char *)data, len, TCP_SENDDATA_MAX_TIMEOUT_MS);

    if (send_len < QCLOUD_RET_SUCCESS || wslay_event_get_close_sent(ctx)) {
        Log_e("send error %d, %d", send_len, wslay_event_get_close_sent(ctx));
        if (iot_ctx->send_data_err) {
            iot_ctx->send_data_err(user_data);
        }
        return QCLOUD_ERR_FAILURE;
    }

    return send_len;
}

static int _websocket_genmask_callback(wslay_event_context_ptr ctx, uint8_t *buf, size_t len, void *user_data)
{
    srand((unsigned)time(NULL));
    int nonce = rand() % 99999999999 + 10000;
    HAL_Snprintf((char *)buf, len, "%0*d", (int)(len - 1), nonce);
    return 0;
}

static void _websocket_on_msg_recv_callback(wslay_event_context_ptr ctx, const struct wslay_event_on_msg_recv_arg *arg,
                                            void *user_data)
{
    UtilsIotWSClientCtx *iot_ctx = (UtilsIotWSClientCtx *)user_data;
    // proc msg
    if (arg->msg_length <= 0) {
        return;
    }

    // Log_d("%d", arg->msg_length);

    iot_ctx->proc_recv_msg_callback((char *)arg->msg, arg->msg_length, iot_ctx);
}

/* websocket connect */
int Utils_WSClient_connect(const char *protocol, UtilsIotWSClientCtx *ctx, const char *ws_custom_header)
{
    char          header_buf[512];
    unsigned char client_out_key[64];
    size_t        out_len = 0;

#define WEBSOCKET_HANDSHAKE_HEADER   \
    "GET /%s HTTP/1.1\r\n"            \
    "Host: %s:%d\r\n"                \
    "Upgrade: websocket\r\n"         \
    "Connection: Upgrade\r\n"        \
    "Sec-Websocket-Protocol: %s\r\n" \
    "Sec-WebSocket-Key: %s\r\n"      \
    "Sec-WebSocket-Version: 13\r\n\r\n"

    /* calc websocket client key */
    char host[256] = {0};
    strncpy(host, ctx->network_stack.host, sizeof(host) - 1);
    char *uri  = strstr(host, "/");
    int   port = ctx->network_stack.port;

    if (uri != NULL) {
        *uri = '\0';
        uri += 1;
    }

    qcloud_iot_utils_base64encode(client_out_key, sizeof(client_out_key), &out_len, (unsigned char *)"1234567890abcdef",
                                  sizeof("1234567890abcdef") - 1);
    int len = HAL_Snprintf(header_buf, sizeof(header_buf), WEBSOCKET_HANDSHAKE_HEADER, uri == NULL ? "" : uri, host,
                           port, protocol == NULL ? "test" : protocol, client_out_key);

    const char *temp_host = ctx->network_stack.host;
    ctx->network_stack.host = host;
    if (QCLOUD_RET_SUCCESS != _network_transport_init(&ctx->network_stack)) {
        return QCLOUD_ERR_FAILURE;
    }

    if (QCLOUD_RET_SUCCESS != _network_transport_connect(&ctx->network_stack)) {
        return QCLOUD_ERR_FAILURE;
    }
    ctx->network_stack.host = temp_host;

    if (uri) {
        *uri = '/';
    }

    if (ws_custom_header != NULL) {
        len -= 2;
    }

    Log_d("header: %s", header_buf);
    if (len != _network_transport_send(&ctx->network_stack, header_buf, len, 5000)) {
        goto end;
    }

    if (ws_custom_header != NULL) {
        int ws_custom_header_len = strlen(ws_custom_header);
        if (ws_custom_header_len !=
            _network_transport_send(&ctx->network_stack, (char *)ws_custom_header, ws_custom_header_len, 5000)) {
            goto end;
        }
        if (2 != _network_transport_send(&ctx->network_stack, "\r\n", 2, 5000)) {
            goto end;
        }
    }

    int read_len = 0;
    int count    = 0;
    do {
        read_len = _network_transport_recv(&ctx->network_stack, header_buf, sizeof(header_buf), 500);
        count++;
    } while (count < (WEBSOCKET_HANDSHAKE_MAX_TIMEOUT_SEC * 1000 / 500) && read_len == 0);

    if (count >= (WEBSOCKET_HANDSHAKE_MAX_TIMEOUT_SEC * 1000 / 500) || read_len <= 0) {
        goto end;
    }

    int code = atoi(header_buf + 9);
    Log_e("websocket resp code :%d", code);

    char *resp = strstr(header_buf, "Sec-WebSocket-Accept: ");
    if (!resp) {
        goto end;
    }
    resp += sizeof("Sec-WebSocket-Accept: ") - 1;
    char *resp_end = strstr(resp, "\r\n");
    len            = resp_end - resp;

    unsigned char sha1[20];
    strcpy((char *)client_out_key + out_len, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    utils_sha1(client_out_key, out_len + sizeof("258EAFA5-E914-47DA-95CA-C5AB0DC85B11") - 1, sha1);

    qcloud_iot_utils_base64encode(client_out_key, sizeof(client_out_key), &out_len, sha1, sizeof(sha1));

    client_out_key[out_len] = '\0';
    if (0 != strncmp((char *)client_out_key, resp, len)) {
        goto end;
    }

    struct wslay_event_callbacks callbacks = {_websocket_recv_callback,
                                              _websocket_send_callback,
                                              _websocket_genmask_callback,
                                              NULL, /* on_frame_recv_start_callback */
                                              NULL, /* on_frame_recv_callback */
                                              NULL, /* on_frame_recv_end_callback */
                                              _websocket_on_msg_recv_callback};

    wslay_event_context_client_init((wslay_event_context_ptr *)&(ctx->wslay_ctx), &callbacks, ctx);

    return QCLOUD_RET_SUCCESS;

end:
    _network_transport_disconnect(&ctx->network_stack);
    return QCLOUD_ERR_FAILURE;
}

/* websocket send */
int Utils_WSClient_send(UtilsIotWSClientCtx *ctx, char *data, int data_len)
{
    // Log_d("websocket send :%d", data_len);
    struct wslay_event_msg ssh_msg;
    ssh_msg.opcode     = WSLAY_BINARY_FRAME;
    ssh_msg.msg        = (uint8_t *)data;
    ssh_msg.msg_length = data_len;
    if (WSLAY_ERR_NO_MORE_MSG == wslay_event_queue_msg((wslay_event_context_ptr)ctx->wslay_ctx, &ssh_msg)) {
        if (ctx->send_data_err) {
            ctx->send_data_err(ctx);
        }
        // wslay_status_code
        Log_e(
            "websocket send err, send code status:%d, recv code status:%d, close sent %d, write_enabled:%d, close "
            "received: %d ",
            wslay_event_get_status_code_sent((wslay_event_context_ptr)ctx->wslay_ctx),
            wslay_event_get_status_code_received((wslay_event_context_ptr)ctx->wslay_ctx),
            wslay_event_get_close_sent((wslay_event_context_ptr)ctx->wslay_ctx),
            wslay_event_get_write_enabled((wslay_event_context_ptr)ctx->wslay_ctx),
            wslay_event_get_close_received((wslay_event_context_ptr)ctx->wslay_ctx));

        wslay_event_send((wslay_event_context_ptr)ctx->wslay_ctx);
        return QCLOUD_ERR_FAILURE;
    }

    return wslay_event_send((wslay_event_context_ptr)ctx->wslay_ctx);
}

/* websocket recv */
int Utils_WSClient_recv(UtilsIotWSClientCtx *ctx)
{
    if (wslay_event_get_close_received((wslay_event_context_ptr)ctx->wslay_ctx)) {
        if (ctx->recv_data_err) {
            ctx->recv_data_err(ctx);
        }
        // wslay_status_code
        Log_e("websocket recv closed; send code status:%d, recv code status:%d",
              wslay_event_get_status_code_sent((wslay_event_context_ptr)ctx->wslay_ctx),
              wslay_event_get_status_code_received((wslay_event_context_ptr)ctx->wslay_ctx));
        wslay_event_send((wslay_event_context_ptr)ctx->wslay_ctx);
        return QCLOUD_ERR_FAILURE;
    }
    return wslay_event_recv((wslay_event_context_ptr)ctx->wslay_ctx);
}

/* websocket disconnect */
void Utils_WSClient_disconn(UtilsIotWSClientCtx *ctx)
{
    if (ctx->network_stack.handle <= 0) {
        return;
    }

    _network_transport_disconnect(&(ctx->network_stack));
    wslay_event_context_free((wslay_event_context_ptr)ctx->wslay_ctx);
    memset(ctx, 0, sizeof(UtilsIotWSClientCtx));
}
#endif

#ifdef __cplusplus
}
#endif
