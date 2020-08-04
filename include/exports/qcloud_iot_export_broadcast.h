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

#ifndef QCLOUD_IOT_EXPORT_BROADCAST_H_
#define QCLOUD_IOT_EXPORT_BROADCAST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifdef BROADCAST_ENABLED

/**
 * @brief broadcast message callback
 */
typedef void (*OnBroadcastMessageCallback)(void *pClient, const char *msg, uint32_t msgLen);

/**
 * @brief Subscribe broadcast topic with message callback
 *
 * @param pClient pointer of handle to MQTT client
 * @param callback Broadcast message callback
 * @return  QCLOUD_RET_SUCCESS when success, otherwise fail
 */
int IOT_Broadcast_Subscribe(void *pClient, OnBroadcastMessageCallback callback);

#endif

#ifdef __cplusplus
}
#endif

#endif  // QCLOUD_IOT_EXPORT_BROADCAST_H_
