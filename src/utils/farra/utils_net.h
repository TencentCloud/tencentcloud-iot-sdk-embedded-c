/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef QCLOUD_IOT_UTILS_NET_H_
#define QCLOUD_IOT_UTILS_NET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "qcloud_iot_import.h"

/**
 * @brief 网络结构类型
 *
 * 定义一个网络结构类型, 具体定义如下面所示
 */
typedef struct Network Network;

/**
 * @brief 网络操作相关的结构体定义
 *
 * 定义了底层网络相关的操作, 包括连接, 读/写数据, 断开连接等
 */
struct Network {
    int (*connect)(Network *);

    int (*read)(Network *, unsigned char *, size_t, uint32_t, size_t *);

    int (*write)(Network *, unsigned char *, size_t, uint32_t, size_t *);

    void (*disconnect)(Network *);

    int (*is_connected)(Network *);

    uintptr_t handle;   // 连接句柄:0，尚未连接; 非0，已经连接

#ifndef AUTH_WITH_NOTLS
    SSLConnectParams ssl_connect_params;
#endif

    const char       *host;                 // 服务器地址
    int              port;                  // 服务器端口
};


int 	utils_net_read(Network *pNetwork, unsigned char *data, size_t datalen, uint32_t timeout_ms, size_t *read_len);
int 	utils_net_write(Network *pNetwork, unsigned char *data, size_t datalen, uint32_t timeout_ms, size_t *written_len);
void 	utils_net_disconnect(Network *pNetwork);
int 	utils_net_connect(Network *pNetwork);
int 	utils_net_init(Network *pNetwork);

#ifdef COAP_COMM_ENABLED
int 	utils_udp_net_read(Network *pNetwork, unsigned char *data, size_t datalen, uint32_t timeout_ms, size_t *read_len);
int 	utils_udp_net_write(Network *pNetwork, unsigned char *data, size_t datalen, uint32_t timeout_ms, size_t *written_len);
void 	utils_udp_net_disconnect(Network *pNetwork);
int 	utils_udp_net_connect(Network *pNetwork);
int 	utils_udp_net_init(Network *pNetwork);
#endif

#ifdef __cplusplus
}
#endif
#endif /* QCLOUD_IOT_UTILS_NET_H_ */
