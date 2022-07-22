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

#if defined(SYSTEM_COMM) && defined(REMOTE_LOGIN_SSH)
#include <string.h>
#include <time.h>

#include "lite-utils.h"
#include "mqtt_client.h"
#include "utils_base64.h"
#include "utils_sha1.h"
#include "utils_hmac.h"
#include "qcloud_iot_device.h"
#include "qcloud_iot_export_system.h"
#include "system_mqtt_ssh_proxy.h"
#include "qcloud_iot_ca.h"
#include "wslay/wslay.h"

#define WEBSOCKET_HANDSHAKE_MAX_TIMEOUT_SEC 10

typedef struct {
    const char *host;
    int         port;
    const char *uri;
    const char *ca_crt_dir;
    const char *custom_header;
} IotWebsocketParams;

typedef struct {
    void *userdata;
    void (*recv_data_err)(void *iotwsctx);
    void (*send_data_err)(void *iotwsctx);
    void (*proc_recv_msg_callback)(char *data, int datalen, void *iotwsctx);
    wslay_event_context_ptr wslay_ctx;
    Network                 network_stack;
} IotWebsocketCtx;

typedef enum {
    NEW_SESSION = 0,
    NEW_SESSION_RESP,
    RELEASE_SESSION,
    RELEASE_SESSION_RESP,
    CMD_PING,
    CMD_PONG,
    SSH_RAWDATA,
    SSH_RAWDATA_RESP,
    VERIFY_DEVICE,
    VERIFY_DEVICE_RESP
} WebsocketMsgType;

typedef struct {
    char    token[32 + 1];
    bool    connected;
    Network local_ssh_network_stack;
} LocalSshNetworkStack;

typedef struct {
    IotWebsocketCtx websocket_ctx;
    Timer           ping_timer;
    Timer           verify_timer;
    int             ping_count;
    int             verify_count;
    bool            start;
    bool            disconnect;
    bool            wait_pong;
    bool            verify_success;
} QcloudWebsocketSsh;

#define LOCAL_SSH_COUNT_MAX          5
#define TCP_READDATA_MAX_TIMEOUT_MS  10
#define TCP_SENDDATA_MAX_TIMEOUT_MS  500
#define WEBSOCKET_SSH_HEADER_MAX_LEN 256

static LocalSshNetworkStack sg_local_network_stack[LOCAL_SSH_COUNT_MAX];
static int                  sg_local_ssh_count = 1;
static QcloudWebsocketSsh   sg_websocketssh;
static char                 sg_websocket_ssh_pakcet[WEBSOCKET_SSH_HEADER_MAX_LEN + 512];

/* tcp connect */
static int _network_tcp_init(Network *pNetwork, const char *host, int port, const char *ca_crt_dir)
{
    if (pNetwork == NULL) {
        return QCLOUD_ERR_INVAL;
    }
    pNetwork->type = NETWORK_TCP;

#ifndef AUTH_WITH_NOTLS
    if (ca_crt_dir != NULL) {
        pNetwork->ssl_connect_params.ca_crt     = ca_crt_dir;
        pNetwork->ssl_connect_params.ca_crt_len = strlen(pNetwork->ssl_connect_params.ca_crt);
        pNetwork->ssl_connect_params.timeout_ms = 10000;
        pNetwork->type                          = NETWORK_TLS;
    }
#endif
    pNetwork->host = host;
    pNetwork->port = port;

    Log_d("init %s:%d, %s", pNetwork->host, pNetwork->port, pNetwork->type == NETWORK_TLS ? "TLS" : "TCP");
    return network_init(pNetwork);
}

static int _network_tcp_connect(Network *pNetwork)
{
    Log_d("start connect %s:%d", pNetwork->host, pNetwork->port);

    if (QCLOUD_RET_SUCCESS != pNetwork->connect(pNetwork)) {
        return QCLOUD_ERR_FAILURE;
    }

    Log_d("connected %s:%d, handle:%p", pNetwork->host, pNetwork->port, pNetwork->handle);
    return QCLOUD_RET_SUCCESS;
}

/* tcp send */
static int _network_tcp_send(Network *network, char *buf, int len, int timeout_ms)
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
static int _network_tcp_recv(Network *network, char *buf, int buf_len, uint32_t timeout_ms)
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
        Log_e("Connection error %d, rc = %d (recv returned %d)", network->handle, rc, recv_size);
        Log_e("Connection error %s:%d", network->host, network->port);
        return rc;
    }

    rc = recv_size;

    return rc;
}

/* tcp disconnect */
static int _network_tcp_disconnect(Network *network)
{
    network->disconnect(network);
    return QCLOUD_RET_SUCCESS;
}

static ssize_t _websocket_recv_callback(wslay_event_context_ptr ctx, uint8_t *data, size_t len, int flags,
                                        void *user_data)
{
    IotWebsocketCtx *iot_ctx = (IotWebsocketCtx *)user_data;

    int recv_len = _network_tcp_recv(&(iot_ctx->network_stack), (char *)data, len, TCP_READDATA_MAX_TIMEOUT_MS);
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
    IotWebsocketCtx *iot_ctx = (IotWebsocketCtx *)user_data;
    int send_len = _network_tcp_send(&(iot_ctx->network_stack), (char *)data, len, TCP_SENDDATA_MAX_TIMEOUT_MS);

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
    HAL_Snprintf((char *)buf, len, "%0*d", len - 1, nonce);
    return 0;
}

static void _websocket_on_msg_recv_callback(wslay_event_context_ptr ctx, const struct wslay_event_on_msg_recv_arg *arg,
                                            void *user_data)
{
    IotWebsocketCtx *iot_ctx = (IotWebsocketCtx *)user_data;
    // proc msg
    if (arg->msg_length <= 0) {
        return;
    }

    Log_d("%d, %s", arg->msg_length, (char *)arg->msg);

    iot_ctx->proc_recv_msg_callback((char *)arg->msg, arg->msg_length, iot_ctx);
}

/* websocket connect */
int IOT_WebSocket_connect(IotWebsocketParams *params, IotWebsocketCtx *ctx)
{
    char          header_buf[512];
    unsigned char client_out_key[64];
    size_t        out_len = 0;

    if (QCLOUD_RET_SUCCESS != _network_tcp_init(&ctx->network_stack, params->host, params->port, params->ca_crt_dir)) {
        return QCLOUD_ERR_FAILURE;
    }

    if (QCLOUD_RET_SUCCESS != _network_tcp_connect(&ctx->network_stack)) {
        return QCLOUD_ERR_FAILURE;
    }

#define WEBSOCKET_HANDSHAKE_HEADER \
    "GET %s HTTP/1.1\r\n"          \
    "Host: %s:%d\r\n"              \
    "Upgrade: websocket\r\n"       \
    "Connection: Upgrade\r\n"      \
    "%s"                           \
    "Sec-WebSocket-Key: %s\r\n"    \
    "Sec-WebSocket-Version: 13\r\n\r\n"

    /* calc websocket client key */
    qcloud_iot_utils_base64encode(client_out_key, sizeof(client_out_key), &out_len, (unsigned char *)"1234567890abcdef",
                                  sizeof("1234567890abcdef") - 1);
    int len = HAL_Snprintf(header_buf, sizeof(header_buf), WEBSOCKET_HANDSHAKE_HEADER,
                           params->uri == NULL ? "/" : params->uri, params->host, params->port, params->custom_header,
                           client_out_key);

    if (len != _network_tcp_send(&ctx->network_stack, header_buf, len, 5000)) {
        goto end;
    }

    int read_len = 0;
    int count    = 0;
    do {
        read_len = _network_tcp_recv(&ctx->network_stack, header_buf, sizeof(header_buf), 500);
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

    wslay_event_context_client_init(&(ctx->wslay_ctx), &callbacks, ctx);

    return QCLOUD_RET_SUCCESS;

end:
    _network_tcp_disconnect(&ctx->network_stack);
    return QCLOUD_ERR_FAILURE;
}

/* websocket async send */
static int IOT_WebSocket_send(IotWebsocketCtx *ctx, char *data, int data_len)
{
    Log_d("websocket send :%d", data_len);
    struct wslay_event_msg ssh_msg;
    ssh_msg.opcode     = WSLAY_BINARY_FRAME;
    ssh_msg.msg        = (uint8_t *)data;
    ssh_msg.msg_length = data_len;
    if (WSLAY_ERR_NO_MORE_MSG == wslay_event_queue_msg(ctx->wslay_ctx, &ssh_msg)) {
        if (ctx->send_data_err) {
            ctx->send_data_err(ctx);
        }
        // wslay_status_code
        Log_e(
            "websocket send err, send code status:%d, recv code status:%d, close sent %d, write_enabled:%d, close "
            "received: %d ",
            wslay_event_get_status_code_sent(ctx->wslay_ctx), wslay_event_get_status_code_received(ctx->wslay_ctx),
            wslay_event_get_close_sent(ctx->wslay_ctx), wslay_event_get_write_enabled(ctx->wslay_ctx),
            wslay_event_get_close_received(ctx->wslay_ctx));

        wslay_event_send(ctx->wslay_ctx);
        return QCLOUD_ERR_FAILURE;
    }

    return wslay_event_send(ctx->wslay_ctx);
}

/* websocket async recv */
static int IOT_WebSocket_recv(IotWebsocketCtx *ctx)
{
    if (wslay_event_get_close_received(ctx->wslay_ctx)) {
        if (ctx->recv_data_err) {
            ctx->recv_data_err(ctx);
        }
        // wslay_status_code
        Log_e("websocket recv closed; send code status:%d, recv code status:%d",
              wslay_event_get_status_code_sent(ctx->wslay_ctx), wslay_event_get_status_code_received(ctx->wslay_ctx));
        wslay_event_send(ctx->wslay_ctx);
        return QCLOUD_ERR_FAILURE;
    }
    return wslay_event_recv(ctx->wslay_ctx);
}

/* websocket disconnect */
static void IOT_WebSocket_disconn(IotWebsocketCtx *ctx)
{
    if (ctx->network_stack.handle <= 0) {
        return;
    }

    _network_tcp_disconnect(&(ctx->network_stack));
    wslay_event_context_free(ctx->wslay_ctx);
    memset(ctx, 0, sizeof(IotWebsocketCtx));
}

static int _local_ssh_init(LocalSshNetworkStack *local_ssh)
{
    Network *network = &(local_ssh->local_ssh_network_stack);
    char *   host    = LOCAL_SSH_IP;
    int      port    = LOCAL_SSH_PORT;
    if (QCLOUD_RET_SUCCESS != _network_tcp_init(network, host, port, NULL)) {
        return QCLOUD_ERR_FAILURE;
    }

    return QCLOUD_RET_SUCCESS;
}

static int _local_ssh_connect(LocalSshNetworkStack *local_ssh)
{
    Network *network = &(local_ssh->local_ssh_network_stack);
    return _network_tcp_connect(network);
}

static int _local_ssh_create(char *token)
{
    int count = 0;

    if (token == NULL) {
        return QCLOUD_ERR_FAILURE;
    }

    for (count = 0; count < LOCAL_SSH_COUNT_MAX; count++) {
        if (sg_local_network_stack[count].token[0] == '\0') {
            break;
        }
    }

    if (count >= LOCAL_SSH_COUNT_MAX) {
        return QCLOUD_ERR_FAILURE;
    }

    memset(&sg_local_network_stack[count], 0, sizeof(sg_local_network_stack[count]));

    if (QCLOUD_RET_SUCCESS != _local_ssh_init(&sg_local_network_stack[count])) {
        return QCLOUD_ERR_FAILURE;
    }

    strncpy(sg_local_network_stack[count].token, token, sizeof(sg_local_network_stack[count].token));
    sg_local_ssh_count++;
    Log_d("new ssh session %s", token);

    return QCLOUD_RET_SUCCESS;
}

static void _local_ssh_deinit(LocalSshNetworkStack *local_ssh)
{
    _network_tcp_disconnect(&(local_ssh->local_ssh_network_stack));
}

static int _local_ssh_destory(char *token)
{
    int count = 0;

    if (token == NULL) {
        return QCLOUD_ERR_FAILURE;
    }

    for (count = 0; count < LOCAL_SSH_COUNT_MAX; count++) {
        if (!strcmp(token, sg_local_network_stack[count].token)) {
            break;
        }
    }

    if (count >= LOCAL_SSH_COUNT_MAX) {
        return QCLOUD_RET_SUCCESS;
    }

    _local_ssh_deinit(&sg_local_network_stack[count]);
    memset(&sg_local_network_stack[count], 0, sizeof(sg_local_network_stack[count]));
    sg_local_ssh_count--;
    Log_d("destory ssh session %s", token);

    return QCLOUD_RET_SUCCESS;
}

static void _local_ssh_deinit_all()
{
    int count = 0;
    for (count = 0; count < LOCAL_SSH_COUNT_MAX; count++) {
        if (sg_local_network_stack[count].token[0] != '\0') {
            Log_d("deinit local ssh %s", sg_local_network_stack[count].token);
            _local_ssh_deinit(&sg_local_network_stack[count]);
            break;
        }
    }

    memset(&sg_local_network_stack, 0, sizeof(sg_local_network_stack));
    sg_local_ssh_count = 1;

    return;
}

static void _websocket_ssh_proc_newsession_resp(char *token, int ret);

static int _local_ssh_send(char *token, char *data, int data_len)
{
    int count = 0;
    for (count = 0; count < LOCAL_SSH_COUNT_MAX; count++) {
        if (!strcmp(token, sg_local_network_stack[count].token)) {
            break;
        }
    }

    if (count >= LOCAL_SSH_COUNT_MAX) {
        return QCLOUD_ERR_FAILURE;
    }

    if (!sg_local_network_stack[count].connected) {
        if (QCLOUD_RET_SUCCESS != _local_ssh_connect(&sg_local_network_stack[count])) {
            _websocket_ssh_proc_newsession_resp(sg_local_network_stack[count].token, QCLOUD_ERR_FAILURE);
            _local_ssh_destory(sg_local_network_stack[count].token);
            return QCLOUD_ERR_FAILURE;
        }
        sg_local_network_stack[count].connected = true;
    }

    Network *network = &(sg_local_network_stack[count].local_ssh_network_stack);
    Log_d("send data to ssh %d", data_len);
    if (QCLOUD_RET_SUCCESS != _network_tcp_send(network, data, data_len, TCP_SENDDATA_MAX_TIMEOUT_MS)) {
        return QCLOUD_ERR_FAILURE;
    }
    return QCLOUD_RET_SUCCESS;
}

static int _websocket_ssh_construct_msgheader(char *buf, int buf_len, char *token, WebsocketMsgType msg_type,
                                              int payload_len);

static void _websocket_ssh_send_rawdata(char *token, char *payload, int payload_len)
{
    int header_len = _websocket_ssh_construct_msgheader(sg_websocket_ssh_pakcet, WEBSOCKET_SSH_HEADER_MAX_LEN, token,
                                                        SSH_RAWDATA, payload_len);

    Log_d("send packet:%s, %d, %d", sg_websocket_ssh_pakcet, header_len, payload_len);

    memmove(sg_websocket_ssh_pakcet + header_len, payload, payload_len);
    // send to cloud
    if (QCLOUD_RET_SUCCESS !=
        IOT_WebSocket_send(&sg_websocketssh.websocket_ctx, sg_websocket_ssh_pakcet, header_len + payload_len)) {
        sg_websocketssh.disconnect = true;
    }
}

static void _local_ssh_recv()
{
    int count = 0;
    for (count = 0; count < LOCAL_SSH_COUNT_MAX; count++) {
        if (sg_local_network_stack[count].token[0] == '\0') {
            continue;
        }

        if (!sg_local_network_stack[count].connected) {
            continue;
        }

        int      read_len = 0;
        Network *network  = &(sg_local_network_stack[count].local_ssh_network_stack);

        Timer recv_timeout_timer;
        InitTimer(&recv_timeout_timer);
        countdown_ms(&recv_timeout_timer, 200);
        char *buf = sg_websocket_ssh_pakcet + WEBSOCKET_SSH_HEADER_MAX_LEN;
        do {
            read_len = _network_tcp_recv(network, buf, sizeof(sg_websocket_ssh_pakcet) - WEBSOCKET_SSH_HEADER_MAX_LEN,
                                         TCP_READDATA_MAX_TIMEOUT_MS);
            if (read_len < QCLOUD_RET_SUCCESS) {
                Log_e("local ssh %s recv failed %d", sg_local_network_stack[count].token, read_len);
                /* deinit current local ssh */
                _websocket_ssh_proc_newsession_resp(sg_local_network_stack[count].token, read_len);
                _local_ssh_destory(sg_local_network_stack[count].token);
                break;
            } else if (read_len > 0) {
                Log_d("recv from local ssh %d", read_len);
                _websocket_ssh_send_rawdata(sg_local_network_stack[count].token, buf, read_len);
            } else {
                break;
            }
        } while (!expired(&recv_timeout_timer));
    }

    return;
}

static int _websocket_ssh_construct_msgheader(char *buf, int buf_len, char *token, WebsocketMsgType msg_type,
                                              int payload_len)
{
    int        timestamp = HAL_Timer_current_sec();
    static int nonce     = 0;

    int header_len = HAL_Snprintf(
        buf, buf_len,
        "{\"requestId\":\"ws-%08x\",\"msgType\":%d,\"payloadLen\":%d,\"serviceType\":0,\"timestmap\":%d,\"token\":"
        "\"%s\"}\r\n",
        nonce, msg_type, payload_len, timestamp, token);

    nonce++;

    return header_len;
}

static void _websocket_ssh_proc_newsession_resp(char *token, int ret)
{
    char payload[64];
    char packet[256];

    int payload_len = 0;
    if (ret == QCLOUD_RET_SUCCESS) {
        payload_len = HAL_Snprintf(payload, sizeof(payload), "{\"code\":0,\"message\":\"%s\"}", "success");
    } else {
        payload_len = HAL_Snprintf(payload, sizeof(payload), "{\"code\":%d,\"message\":\"%s\"}", ret, "fail");
    }

    int header_len = _websocket_ssh_construct_msgheader(packet, sizeof(packet), token, NEW_SESSION_RESP, payload_len);
    HAL_Snprintf(packet + header_len, sizeof(packet) - header_len, "%s", payload);

    Log_d("send packet:%s,%s", packet, token);
    if (QCLOUD_RET_SUCCESS != IOT_WebSocket_send(&sg_websocketssh.websocket_ctx, packet, header_len + payload_len)) {
        sg_websocketssh.disconnect = true;
    }
}

static void _websocket_ssh_proc_newsession(char *token)
{
    if (QCLOUD_RET_SUCCESS <= _local_ssh_create(token)) {
        _websocket_ssh_proc_newsession_resp(token, QCLOUD_RET_SUCCESS);
    } else {
        _websocket_ssh_proc_newsession_resp(token, QCLOUD_ERR_FAILURE);
    }
}

static void _websocket_ssh_proc_releasesession_resp(char *token, int ret)
{
    char payload[64];
    char packet[256];

    int payload_len = 0;
    if (ret == QCLOUD_RET_SUCCESS) {
        payload_len = HAL_Snprintf(payload, sizeof(payload), "{\"code\":0,\"message\":\"%s\"}", "success");
    } else {
        payload_len = HAL_Snprintf(payload, sizeof(payload), "{\"code\":%d,\"message\":\"%s\"}", ret, "fail");
    }

    int header_len =
        _websocket_ssh_construct_msgheader(packet, sizeof(packet), token, RELEASE_SESSION_RESP, payload_len);
    HAL_Snprintf(packet + header_len, sizeof(packet) - header_len, "%s", payload);

    Log_d("send packet:%s", packet);
    if (QCLOUD_RET_SUCCESS != IOT_WebSocket_send(&sg_websocketssh.websocket_ctx, packet, header_len + payload_len)) {
        sg_websocketssh.disconnect = true;
    }
}

static void _websocket_ssh_proc_releasesession(char *token)
{
    _websocket_ssh_proc_releasesession_resp(token, QCLOUD_RET_SUCCESS);
    _local_ssh_destory(token);
}

static void _websocket_ssh_proc_ping()
{
    if (!expired(&(sg_websocketssh.ping_timer))) {
        return;
    }

    if (sg_websocketssh.wait_pong) {
        if (sg_websocketssh.ping_count > 3) {
            // sg_websocketssh.disconnect = true;
            Log_e("no ping resp disconn websocket");
        }
    }

    countdown(&sg_websocketssh.ping_timer, 5);
    int  timestamp = HAL_GetTimeMs();
    char token[32];
    char packet[256];
    HAL_Snprintf(token, sizeof(token), "wsping-%x", timestamp);
    int header_len = _websocket_ssh_construct_msgheader(packet, sizeof(packet), token, CMD_PING, 0);

    sg_websocketssh.wait_pong = true;
    sg_websocketssh.ping_count++;

    Log_d("send packet:%s", packet);
    if (QCLOUD_RET_SUCCESS != IOT_WebSocket_send(&sg_websocketssh.websocket_ctx, packet, header_len)) {
        sg_websocketssh.disconnect = true;
    }
}

static void _websocket_ssh_proc_pong()
{
    /* disconnect websocket */
    sg_websocketssh.wait_pong  = false;
    sg_websocketssh.ping_count = 0;
    countdown(&sg_websocketssh.ping_timer, 30);
}

static void _websocket_ssh_proc_rawdata(char *token, char *payload, int payload_len)
{
    _local_ssh_send(token, payload, payload_len);
}

static int _websocket_ssh_proc_verifydevcie(void *mqtt_client)
{
    Qcloud_IoT_Client *client    = (Qcloud_IoT_Client *)mqtt_client;
    int                timestamp = HAL_Timer_current_sec();
    srand((unsigned)time(NULL));
    int nonce = rand() % 99999999999 + 1000;

    if (sg_websocketssh.verify_success == true) {
        return QCLOUD_RET_SUCCESS;
    }

    if (!expired(&sg_websocketssh.verify_timer)) {
        return QCLOUD_RET_SUCCESS;
    }

    if (sg_websocketssh.verify_count > 3) {
        sg_websocketssh.disconnect = true;
        Log_e("verify timeout disconn websocket");
        return QCLOUD_RET_SUCCESS;
    }

    char  sign_str[256];
    char *productid  = client->device_info.product_id;
    char *devicename = client->device_info.device_name;

    int sign_str_len = HAL_Snprintf(sign_str, sizeof(sign_str), "productId_%s_device_%s_timestamp_%d_rand_%d",
                                    productid, devicename, timestamp, nonce);

#ifdef AUTH_MODE_CERT
    char *signmethod = "rsasha256";
    char *privatekey = HAL_TLS_Get_PrivateKey_FromFile(client->key_file_path);
    if (privatekey == NULL) {
        Log_e("private key get fail");
        return QCLOUD_ERR_FAILURE;
    }

    int   sign_len = HAL_TLS_Get_RSASHA256_Result_Len(privatekey);
    char *sign     = HAL_Malloc(sign_len);
    if (NULL == sign) {
        Log_e("malloc sign fail");
        HAL_TLS_Destory_PrivateKey(privatekey);
        return QCLOUD_ERR_FAILURE;
    }

    char *sign_base64 = HAL_Malloc(sign_len * 2);
    if (NULL == sign) {
        Log_e("malloc sign base64 fail");
        HAL_TLS_Destory_PrivateKey(privatekey);
        HAL_Free(sign);
        return QCLOUD_ERR_FAILURE;
    }

    HAL_TLS_Calc_Sign_RSASHA256(privatekey, sign_str, sign_str_len, sign);
    HAL_TLS_Destory_PrivateKey(privatekey);
#else
    char *signmethod        = "hmacsha1";
    char *device_secret_key = client->device_info.device_secret;
    char  sign[20];
    int   sign_len = 20;
    char  sign_base64[40];
    utils_hmac_sha1_hex(sign_str, sign_str_len, sign, device_secret_key, strlen(device_secret_key));
#endif

    size_t out_len = 0;
    qcloud_iot_utils_base64encode((unsigned char *)sign_base64, sign_len * 2, &out_len, (unsigned char *)sign,
                                  sign_len);
    sign_base64[out_len] = '\0';

    char *payload     = sg_websocket_ssh_pakcet + WEBSOCKET_SSH_HEADER_MAX_LEN;
    int   payload_len = HAL_Snprintf(payload, sizeof(sg_websocket_ssh_pakcet) - WEBSOCKET_SSH_HEADER_MAX_LEN,
                                   "{\"productId\":\"%s\",\"deviceName\":\"%s\",\"timestamp\":%d,\"rand\":\"%d\","
                                   "\"version\":\"1.0\",\"signMethod\":\"%s\",\"sign\":\"%s\"}",
                                   productid, devicename, timestamp, nonce, signmethod, sign_base64);

    HAL_Snprintf(sign, sizeof(sign), "ws-token-%d", nonce);
    int header_len = _websocket_ssh_construct_msgheader(sg_websocket_ssh_pakcet, WEBSOCKET_SSH_HEADER_MAX_LEN, sign,
                                                        VERIFY_DEVICE, payload_len);

    sg_websocketssh.verify_success = false;
    countdown(&sg_websocketssh.verify_timer, 5);

    memmove(sg_websocket_ssh_pakcet + header_len, payload, payload_len);

    Log_d("send packet:%s", sg_websocket_ssh_pakcet);
    if (QCLOUD_RET_SUCCESS !=
        IOT_WebSocket_send(&sg_websocketssh.websocket_ctx, sg_websocket_ssh_pakcet, payload_len + header_len)) {
        sg_websocketssh.disconnect = true;
    }

#ifdef AUTH_MODE_CERT
    HAL_Free(sign);
    HAL_Free(sign_base64);
#endif

    return QCLOUD_RET_SUCCESS;
}

static void _websocket_ssh_proc_verifyresult(char *token, int code)
{
    if (code != 0) {
        /* disconnect websocket */
        sg_websocketssh.disconnect = true;
        Log_d("verify failed");
        return;
    }

    sg_websocketssh.disconnect     = false;
    sg_websocketssh.verify_success = true;
    Log_e("verify success");
}

static void _qcloud_websocket_recv_data_error(void *iotwsctx)
{
    sg_websocketssh.disconnect = true;
}

static void _qcloud_websocket_recv_msg_callback(char *data, int datalen, void *iotwsctx)
{
    char *delimiter = strstr((char *)data, "\r\n");

    char *payload = NULL;
    if (delimiter != NULL) {
        *delimiter = '\0';
        payload    = (delimiter + sizeof("\r\n") - 1);
    }

    char *msg_type    = LITE_json_value_of("msgType", (char *)data);
    char *token       = LITE_json_value_of("token", (char *)data);
    char *payload_len = LITE_json_value_of("payloadLen", (char *)data);

    if (!msg_type) {
        goto exit;
    }

    int type = atoi(msg_type);
    if (type == NEW_SESSION) {
        _websocket_ssh_proc_newsession(token);
    } else if (type == RELEASE_SESSION) {
        _websocket_ssh_proc_releasesession(token);
    } else if (type == CMD_PONG) {
        _websocket_ssh_proc_pong();
    } else if (type == SSH_RAWDATA) {
        _websocket_ssh_proc_rawdata(token, payload, atoi(payload_len));
    } else if (type == VERIFY_DEVICE_RESP) {
        _websocket_ssh_proc_verifyresult(token, 0);
    }

exit:
    HAL_Free(msg_type);
    HAL_Free(payload_len);
    HAL_Free(token);
}

static int _qcloud_websocket_conn(void *mqtt_client)
{
    int ret = QCLOUD_RET_SUCCESS;

    IotWebsocketParams params;
    Qcloud_IoT_Client *client     = (Qcloud_IoT_Client *)mqtt_client;
    char *             productid  = client->device_info.product_id;
    char *             devicename = client->device_info.device_name;
    char *             region     = client->device_info.region;

    params.host       = iot_get_ssh_domain(region);
    params.uri        = REMOTE_WS_SSH_PATH;
    params.port       = DYN_REG_SERVER_PORT;
    params.ca_crt_dir = NULL;
#ifndef AUTH_WITH_NOTLS
    params.port       = DYN_REG_SERVER_PORT_TLS;
    params.ca_crt_dir = iot_wss_ssh_ca_get();
#endif

    char custom_header[256];
    HAL_Snprintf(custom_header, sizeof(custom_header), "Sec-Websocket-Protocol: %s+%s\r\n", productid, devicename);
    params.custom_header = custom_header;

    sg_websocketssh.websocket_ctx.send_data_err          = NULL;
    sg_websocketssh.websocket_ctx.recv_data_err          = _qcloud_websocket_recv_data_error;
    sg_websocketssh.websocket_ctx.proc_recv_msg_callback = _qcloud_websocket_recv_msg_callback;
    ret = IOT_WebSocket_connect(&params, &sg_websocketssh.websocket_ctx);
    if (ret != QCLOUD_RET_SUCCESS) {
        return ret;
    }

    return ret;
}

static int _qcloud_websocket_msg_yield()
{
    // recv from websocket
    int ret = IOT_WebSocket_recv(&sg_websocketssh.websocket_ctx);
    if (ret == QCLOUD_RET_SUCCESS) {
        _websocket_ssh_proc_ping();
    } else {
        sg_websocketssh.disconnect = true;
        ret                        = QCLOUD_ERR_FAILURE;
    }

    return ret;
}

static int _qcloud_websocket_disconnect()
{
    int ret = QCLOUD_RET_SUCCESS;
    IOT_WebSocket_disconn(&sg_websocketssh.websocket_ctx);
    return ret;
}

static void _qcloud_websocket_thread(void *mqtt_client)
{
    if (QCLOUD_RET_SUCCESS != _qcloud_websocket_conn(mqtt_client)) {
        Log_e("qcloud websocekt connect failed");
        /* report connect failed */
        goto exit;
    }
    IOT_Ssh_state_report(mqtt_client, true);

    while (!sg_websocketssh.disconnect) {
        _websocket_ssh_proc_verifydevcie(mqtt_client);
        if (QCLOUD_RET_SUCCESS != _qcloud_websocket_msg_yield()) {
            break;
        }
        _local_ssh_recv();
    }

    // disconnect
    _local_ssh_deinit_all();
    _qcloud_websocket_disconnect();

exit:
    IOT_Ssh_state_report(mqtt_client, false);
    memset(&sg_websocketssh, 0, sizeof(sg_websocketssh));
    sg_local_ssh_count = 1;
}

void IOT_QCLOUD_SSH_Stop()
{
    // destory thread
    sg_websocketssh.disconnect = true;
    while (sg_websocketssh.start) {
        HAL_SleepMs(500);
    }
}

void IOT_QCLOUD_SSH_Start(void *mqtt_client)
{
    // start ssh thread
    int ret = QCLOUD_RET_SUCCESS;

    if (sg_websocketssh.start) {
        IOT_QCLOUD_SSH_Stop();
    }

    memset(&sg_websocketssh, 0, sizeof(sg_websocketssh));
    sg_local_ssh_count = 1;

    sg_websocketssh.start      = true;
    sg_websocketssh.disconnect = false;

    static ThreadParams thread_params = {0};
    thread_params.thread_func         = _qcloud_websocket_thread;
    thread_params.thread_name         = "qcloud_websocket";
    thread_params.user_arg            = mqtt_client;
    thread_params.stack_size          = 8192;
    thread_params.priority            = 0;

    ret = HAL_ThreadCreate(&thread_params);
    if (QCLOUD_RET_SUCCESS == ret) {
        Log_d("create qcloud_ssh thread success!");
    } else {
        Log_e("create qcloud_ssh thread fail!");
        sg_websocketssh.start = false;
        IOT_Ssh_state_report(mqtt_client, false);
    }
}

#else
void IOT_QCLOUD_SSH_Start(void *mqtt_client)
{
    Log_e("please modify CMakeLists.txt or make.settings FEATURE_REMOTE_LOGIN_SSH to ON/Y");
}

void IOT_QCLOUD_SSH_Stop()
{
    Log_e("please modify CMakeLists.txt or make.settings FEATURE_REMOTE_LOGIN_SSH to ON/Y");
}
#ifdef __cplusplus
}
#endif

#endif
