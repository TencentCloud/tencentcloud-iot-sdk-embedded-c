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

#ifndef QCLOUD_IOT_EXPORT_COAP_H_
#define QCLOUD_IOT_EXPORT_COAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/**
 * @brief CoAP needResp类型消息回调处理函数指针定义
 *
 * @param message CoAP消息
 * @param userContext 发送消息时携带的用户上下文
 */
typedef void (*OnRespCallback)(void *message, void *userContext);

/**
 * CoAP 事件类型
 */
typedef enum {
	COAP_EVENT_RECEIVE_ACK = 0,					// 收到消息ACK

	COAP_EVENT_RECEIVE_RESPCONTENT = 1,			// 收到消息回包响应

	COAP_EVENT_UNAUTHORIZED = -1,				// 设备鉴权失败或鉴权token过期

	COAP_EVENT_FORBIDDEN = -2,					// 设备topic无权限

	COAP_EVENT_INTERNAL_SERVER_ERROR = -3,		// 服务端错误

	COAP_EVENT_ACK_TIMEOUT = -4,				// 消息ACK超时

	COAP_EVENT_SEPRESP_TIMEOUT = -5,			// 消息回包响应超时
} CoAPEventType;

typedef struct {
    /* 事件类型 */
    CoAPEventType  event_type;
    /* 事件消息 or 内容消息 */
    void *message;

} CoAPEventMessage;

typedef void (*CoAPEventHandleFunc)(void *pcontext, CoAPEventMessage *message);

/* The structure of COAP event handle */
typedef struct {
    CoAPEventHandleFunc      h_fp;
    void                    *context;
} CoAPEventHandler;

/**
 * @brief COAP发布消息结构体
 */
typedef struct {
    bool          				need_resp;			// true: 消息与第三方业务后台交互，等待业务回包，暂不支持；false: 表示数据上报，不需要业务回包
    char        				*pay_load;     		// COAP 消息负载
    size_t     					pay_load_len;  		// MQTT 消息负载长度
    void						*user_context;		// 用于用户传递的消息上下文，SDK不做处理在callback时返回
    OnRespCallback 				resp_callback;		// 消息回包callback，仅对need_resp: true 有效
} SendMsgParams;

#define DEFAULT_SENDMSG_PARAMS {false, NULL, 0, NULL, NULL}

/**
 * CoAP客户端创建所需初始化参数结构体
 */
typedef struct {
    /* 设备基础信息 */
    char                        *product_id;              // 产品名称
    char                        *device_name;             // 设备名称

#ifdef AUTH_MODE_CERT
    char                        *cert_file;              // 客户端证书文件路径
    char                        *key_file;               // 客户端私钥文件路径
#else
    char                        *device_secret;                    // 对称加密密钥
#endif

    uint32_t					command_timeout;		 // coap消息等待回包ACK/RESP超时

    unsigned char               max_retry_count;         // CoAP消息列表最大节点数

    CoAPEventHandler            event_handle;            // 事件回调

} CoAPInitParams;

#ifdef AUTH_MODE_CERT
	#define DEFAULT_COAPINIT_PARAMS { NULL, NULL, NULL, NULL, 2000, 5, {0}}
#else
    #define DEFAULT_COAPINIT_PARAMS { NULL, NULL, NULL, 2000, 5, {0}}
#endif

/**
 * @brief 构造COAPClient并完成连接和鉴权
 *
 * @param pParams COAP协议连接接入与连接维持阶段所需要的参数
 *
 * @return 返回 NULL: 构造失败; 非NULL为COAPClient对象指针
 */
void* IOT_COAP_Construct(CoAPInitParams *pParams);

/**
 * @brief 关闭COAP连接并销毁COAPClient
 *
 * @param pClient COAPClient对象地址
 *
 */
void  IOT_COAP_Destroy(void **pClient);

/**
 * @brief 在当前线程为底层COAP客户端让出一定CPU执行时间
 *
 * 在这段时间内, COAP客户端会用于处理消息接收
 *
 * @param pClient    COAP Client结构体
 * @param timeout_ms Yield操作超时时间
 * @return 返回QCLOUD_ERR_SUCCESS, 表示成功, 返回负数, 表示COAP网络出错
 */
int   IOT_COAP_Yield(void *pClient, uint32_t timeout_ms);

/**
 * @brief 发布COAP消息
 *
 * @param pClient 			COAP客户端结构体
 * @param topicName 		主题名
 * @param sendParams 		消息参数
 * @return > 0, 消息Id; < 0, 表示失败
 */
int   IOT_COAP_SendMessage(void *pClient, char *topicName, SendMsgParams *sendParams);

/**
 * @brief 获取COAP Response消息msgId
 *
 * @param pMessage  COAP Response消息
 * @return 	> 0, 消息id; < 0, 表示失败
 */
int   IOT_COAP_GetMessageId(void *pMessage);

/**
 * @brief 获取COAP Response消息内容
 *
 * @param pMessage 			COAP Response消息
 * @param payload 			消息负载
 * @param payloadLen 		消息内容长度
 * @return QCLOUD_ERR_SUCCESS, 表示成功; < 0, 表示失败
 */
int   IOT_COAP_GetMessagePayload(void *pMessage, char **payload, int *payloadLen);

/**
 * @brief 获取COAP Response消息错误码
 *
 * @param pMessage  COAP Response消息
 * @return 	COAPEventType 错误码转换为event type，简单标识错误原因
 */
int   IOT_COAP_GetMessageCode(void *pMessage);

#ifdef __cplusplus
}
#endif

#endif /* QCLOUD_IOT_EXPORT_COAP_H_ */
