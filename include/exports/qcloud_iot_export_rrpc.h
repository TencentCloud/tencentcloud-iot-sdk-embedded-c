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

#ifndef QCLOUD_IOT_EXPORT_RRPC_H_
#define QCLOUD_IOT_EXPORT_RRPC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "qcloud_iot_import.h"

#ifdef RRPC_ENABLED

/* Max len of status in reply msg*/
#define MAX_RRPC_REPLY_STATUS_LEN 64

/* Max len of process id*/
#define MAX_RRPC_PROCESS_ID_LEN 16

typedef enum _eRRPCReplyCode_ {
    eRRPC_SUCCESS = 0,
    eRRPC_FAIL    = -1,
} eRRPCReplyCode;

/**
 * @brief rrpc msg reply parameter
 */
typedef struct _sRRPCReplyPara {
    eRRPCReplyCode code;                                   // reply code. 0:success, ~0:failed
    char           status_msg[MAX_RRPC_REPLY_STATUS_LEN];  // reply message
    char *         user_data;                              // content of user reply
    size_t         user_data_len;                          // length of user data
} sRRPCReplyPara;

/**
 * @brief RRPC message callback
 */
typedef void (*OnRRPCMessageCallback)(void *pClient, const char *msg, uint32_t msgLen);

/**
 * @brief Subscribe rrpc topic with message callback
 *
 * @param pClient pointer of handle to MQTT client
 * @param callback rrpc message callback
 * @return  QCLOUD_RET_SUCCESS when success, otherwise fail
 */
int IOT_RRPC_Init(void *pClient, OnRRPCMessageCallback callback);

/**
 * @brief  reply to the rrpc msg
 * @param pClient       handle to mqtt client
 * @param processId     process id
 * @param pJsonDoc      data buffer for reply
 * @param sizeOfBuffer  length of data buffer
 * @param replyPara     rrpc reply info
 * @return	        QCLOUD_RET_SUCCESS when success, or err code
 * for failure
 */
int IOT_RRPC_Reply(void *pClient, char *pJsonDoc, size_t sizeOfBuffer, sRRPCReplyPara *replyPara);

#endif

#ifdef __cplusplus
}
#endif

#endif  // QCLOUD_IOT_EXPORT_RRPC_H_
