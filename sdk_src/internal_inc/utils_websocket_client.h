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

#ifndef UTILS_WEBSOCKET_CLIENT_H_
#define UTILS_WEBSOCKET_CLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void *userdata;
    void (*recv_data_err)(void *iotwsctx);
    void (*send_data_err)(void *iotwsctx);
    void (*proc_recv_msg_callback)(char *data, int datalen, void *iotwsctx);
    void *  wslay_ctx;
    Network network_stack;
} UtilsIotWSClientCtx;

int  Utils_WSClient_connect(const char *protocol, UtilsIotWSClientCtx *ctx, const char *ws_custom_header);
int  Utils_WSClient_send(UtilsIotWSClientCtx *ctx, char *data, int data_len);
int  Utils_WSClient_recv(UtilsIotWSClientCtx *ctx);
void Utils_WSClient_disconn(UtilsIotWSClientCtx *ctx);

#ifdef __cplusplus
}
#endif

#endif /* UTILS_WEBSOCKET_CLIENT_H_ */