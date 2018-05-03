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

#ifndef IOT_COAP_CLIENT_H_
#define IOT_COAP_CLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <sys/types.h>

#include "qcloud_iot_export.h"

#include "coap_client_net.h"

#include "utils_net.h"
#include "utils_list.h"
#include "utils_timer.h"

/* COAP协议版本号  */
#define COAP_MSG_VER                           						(0x01)

/* COAP消息初始token */
#define COAP_MSG_INIT_TOKEN     									(0x01020304)

/* CoAP ACK/RESP最小超时时间 */
#define MIN_COMMAND_TIMEOUT         								(500)

/* CoAP ACK/RESP最大超时时间 */
#define MAX_COMMAND_TIMEOUT         								(5000)

/* CoAP 最大链接ID的长度 */
#define COAP_MAX_CONN_ID_LEN                                        (6)

/* Message id最大值 */
#define COAP_MSG_MAX_MSG_ID                    						((1 << 16) - 1)

/* Topic最大长度 */
#define URI_PATH_MAX_LEN           									(128)

/* CoAP 鉴权成功 */
#define COAP_TRUE    												(1)

/* CoAP 鉴权失败 */
#define COAP_FALSE   												(0)

/* 设备鉴权URI，该URI唯一，禁止设备创建该URI topic */
#define COAP_AUTH_URI												("txauth9w0BAQsFA")

/* token最大长度 */
#define COAP_MSG_MAX_TOKEN_LEN                 						(8)

/* COAP 消息最大code class */
#define COAP_MSG_MAX_CODE_CLASS                						(7)

/* COAP 消息最大code detail */
#define COAP_MSG_MAX_CODE_DETAIL               						(31)

/* 获取Option num字段 */
#define COAP_MSG_OPTION_NUM(option)                					((option)->option_num)

/* 获取Next Option */
#define COAP_MSG_OP_NEXT(option)               						((option)->next)

/* 判断COAP消息是否为空 */
#define COAP_MSG_IS_EMPTY(message)                 					(((message)->code_class == 0) && ((message)->code_detail == 0))

/* 判断COAP消息是否为空 ACK*/
#define COAP_MSG_IS_EMPTY_ACK(message)                 					(((message)->code_class == 2) && ((message)->code_detail == 3))

/* 判断COAP消息是否为RESP */
#define COAP_MSG_IS_EMPTY_RSP(message)                 					(((message)->code_class == 2) && ((message)->code_detail == 5))

/**
 *  @brief COAP消息分类
 */
typedef enum {
    COAP_MSG_REQ = 0,                                                           // 请求包
    COAP_MSG_SUCCESS = 2,                                                       // 成功回包
    COAP_MSG_CLIENT_ERR = 4,                                                    // 客户端错误回包
    COAP_MSG_SERVER_ERR = 5,                                                    // 后台错误回包
    COAP_MSG_SDKINTERNAL_ERR = 6,
} CoAPMessageClass;

/**
 *  @brief COAP消息请求方法
 */
typedef enum {
    COAP_MSG_GET = 1,					//GET请求
    COAP_MSG_POST = 2,					//POST请求
    COAP_MSG_PUT = 3,
    COAP_MSG_DELETE =4
} CoAPRequestMethod;

typedef enum {
	/* CoAP Success Response code detail */
	COAP_MSG_CODE_201_CREATED                    = 01,  /* Mapping to CoAP codeClass.codeDetail 2.01 */
	COAP_MSG_CODE_202_DELETED                    = 02,  /* Mapping to CoAP codeClass.codeDetail 2.02 */
	COAP_MSG_CODE_203_VALID                      = 03,  /* Mapping to CoAP codeClass.codeDetail 2.03 */
	COAP_MSG_CODE_204_CHANGED                    = 04,  /* Mapping to CoAP codeClass.codeDetail 2.04 */
	COAP_MSG_CODE_205_CONTENT                    = 05,  /* Mapping to CoAP codeClass.codeDetail 2.05 */
	COAP_MSG_CODE_231_CONTINUE                   = 31,  /* Mapping to CoAP codeClass.codeDetail 2.31 */

	/* CoAP Client Error Response code detail */
	COAP_MSG_CODE_400_BAD_REQUEST                = 00,  /* Mapping to CoAP codeClass.codeDetail 4.00 */
	COAP_MSG_CODE_401_UNAUTHORIZED               = 01,  /* Mapping to CoAP codeClass.codeDetail 4.01 */
	COAP_MSG_CODE_402_BAD_OPTION                 = 02,  /* Mapping to CoAP codeClass.codeDetail 4.02 */
	COAP_MSG_CODE_403_FORBIDDEN                  = 03,  /* Mapping to CoAP codeClass.codeDetail 4.03 */
	COAP_MSG_CODE_404_NOT_FOUND                  = 04,  /* Mapping to CoAP codeClass.codeDetail 4.04 */
	COAP_MSG_CODE_405_METHOD_NOT_ALLOWED         = 05,  /* Mapping to CoAP codeClass.codeDetail 4.05 */
	COAP_MSG_CODE_406_NOT_ACCEPTABLE             = 06,  /* Mapping to CoAP codeClass.codeDetail 4.06 */
	COAP_MSG_CODE_408_REQUEST_ENTITY_INCOMPLETE  = 8,   /* Mapping to CoAP codeClass.codeDetail 4.08 */
	COAP_MSG_CODE_412_PRECONDITION_FAILED        = 12,  /* Mapping to CoAP codeClass.codeDetail 4.12 */
	COAP_MSG_CODE_413_REQUEST_ENTITY_TOO_LARGE   = 13,  /* Mapping to CoAP codeClass.codeDetail 4.13 */
	COAP_MSG_CODE_415_UNSUPPORTED_CONTENT_FORMAT = 15,  /* Mapping to CoAP codeClass.codeDetail 4.15 */

	/* CoAP Server Error Response code detail */
	COAP_MSG_CODE_500_INTERNAL_SERVER_ERROR      = 00,  /* Mapping to CoAP codeClass.codeDetail 5.00 */
	COAP_MSG_CODE_501_NOT_IMPLEMENTED            = 01,  /* Mapping to CoAP codeClass.codeDetail 5.01 */
	COAP_MSG_CODE_502_BAD_GATEWAY                = 02,  /* Mapping to CoAP codeClass.codeDetail 5.02 */
	COAP_MSG_CODE_503_SERVICE_UNAVAILABLE        = 03,  /* Mapping to CoAP codeClass.codeDetail 5.03 */
	COAP_MSG_CODE_504_GATEWAY_TIMEOUT            = 04,  /* Mapping to CoAP codeClass.codeDetail 5.04 */
	COAP_MSG_CODE_505_PROXYING_NOT_SUPPORTED     = 05,  /* Mapping to CoAP codeClass.codeDetail 5.05 */

	COAP_MSG_CODE_600_TIMEOUT					 = 00,  /* Mapping to self define CoAP codeClass.codeDetail 6.00 */
} CoAPRespCodeDetail;

/**
 *  @brief Option number enumeration
 */
typedef enum {
    COAP_MSG_IF_MATCH = 1,                                                      // If-Match option number
    COAP_MSG_URI_HOST = 3,                                                      // URI-Host option number
    COAP_MSG_ETAG = 4,                                                          // Entity-Tag option number
    COAP_MSG_IF_NONE_MATCH = 5,                                                 // If-None-Match option number
    COAP_MSG_URI_PORT = 7,                                                      // URI-Port option number
    COAP_MSG_LOCATION_PATH = 8,                                                 // Location-Path option number
    COAP_MSG_URI_PATH = 11,                                                     // URI-Path option number
    COAP_MSG_CONTENT_FORMAT = 12,                                               // Content-Format option number
    COAP_MSG_MAX_AGE = 14,                                                      // Max-Age option number
    COAP_MSG_URI_QUERY = 15,                                                    // URI-Query option number
    COAP_MSG_ACCEPT = 17,                                                       // Accept option number
    COAP_MSG_LOCATION_QUERY = 20,                                               // Location-Query option number
    COAP_MSG_BLOCK2 = 23,                                                       // Block2 option number
    COAP_MSG_BLOCK1 = 27,                                                       // Block1 option number
    COAP_MSG_SIZE2 = 28,                                                        // Size2 option number
    COAP_MSG_PROXY_URI = 35,                                                    // Proxy-URI option number
    COAP_MSG_PROXY_SCHEME = 39,                                                 // Proxy-Scheme option number
    COAP_MSG_SIZE1 = 60,                                                        // Size1 option number

    COAP_MSG_AUTH_TOKEN = 61,													// 设备鉴权token option number
    COAP_MSG_NEED_RESP = 62,													// CoAP消息是否需要content response option number
} CoAPMsgOptionNum;

/* CoAP Client结构体定义 */
typedef struct Client {
	char									is_authed;											// CoAP Client是否已鉴权
    char					 	            conn_id[COAP_MAX_CONN_ID_LEN];						// 标识唯一一条CoAP连接

	unsigned int         					message_token;										// 标识请求响应条消息对应

	char                					*auth_token;										// 鉴权token
	int                  					auth_token_len;

    uint16_t                 				next_msg_id;                                		// COAP报文标识符

    size_t                   				send_buf_size;                                		// 发送消息buffer长度
    size_t                   				read_buf_size;                                 		// 消息接收buffer长度

    unsigned char							send_buf[COAP_SENDMSG_MAX_BUFLEN];
    unsigned char							recv_buf[COAP_RECVMSG_MAX_BUFLEN];

    void                     				*lock_send_buf;                          			// 输出流的锁
    void                     				*lock_list_wait_ack;                                // 等待发布消息ack列表的锁

    Network                  				network_stack;										// 网络调用数据结构

    uint32_t                 				command_timeout_ms;                                	// CoAP消息超时时间, 单位:ms

    List*									message_list;										// 请求消息列表

    unsigned char               			max_retry_count;         							// CoAP消息最大重试次数

    CoAPEventHandler            			event_handle;            							// 事件回调

} CoAPClient;

/**
 *  @brief CoAP Option结构体
 */
typedef struct coap_msg_op
{
    unsigned short							option_num;                                         // Option number
    unsigned 								val_len;                                            // Option length
    char 									*val;                                               // Pointer to a buffer containing the option value
    struct coap_msg_op 						*next;                                              // Pointer to the next option structure in the list
} CoAPMsgOption;

/**
 *  @brief CoAP Option 链表结构
 */
typedef struct
{
	CoAPMsgOption 							*first;                                             // Pointer to the first option structure in the list
	CoAPMsgOption 							*last;                                              // Pointer to the last option structure in the list
} CoAPMsgOptionList;

/**
 * @brief CoAP消息列表中节点状态
 */
typedef enum {
    COAP_NODE_STATE_NORMANL = 0,
    COAP_NODE_STATE_INVALID,
} CoAPNodeState;

typedef struct {
    CoAPNodeState           				node_state;         								/* 节点状态 */
	void                    				*user_context;
	unsigned short           				msg_id;
	char                     				acked;
	unsigned char            				token_len;
	unsigned char            				token[COAP_MSG_MAX_TOKEN_LEN];
	unsigned char            				retrans_count;
	Timer           						start_time;
	unsigned char           				*message;
	unsigned int             				msglen;
	OnRespCallback       	 				handler;
} CoAPMsgSendInfo;

/**
 *  @brief COAP消息类型
 */
typedef enum {
    COAP_MSG_CON = 0x0,                                                         /**< 需要等待回包确认的消息 */
    COAP_MSG_NON = 0x1,                                                         /**< 无需等待回包确认的消息 */
    COAP_MSG_ACK = 0x2,                                                         /**< ACK消息 */
    COAP_MSG_RST = 0x3                                                          /**< Reset消息 */
} CoAPMsgType;

/**
 *  @brief CoAP消息结构体
 */
typedef struct
{
    unsigned 								version;                                            // CoAP协议版本号
    CoAPMsgType 							type;                                               // 消息类型

    unsigned 								code_class;                                         // Code class
    unsigned 								code_detail;                                        // Code detail

    unsigned short 							msg_id;                                             // 消息id

    char 									*pay_load;                                          // 消息负载
    size_t 									pay_load_len;                                       // 消息负载长度

    char 									token[COAP_MSG_MAX_TOKEN_LEN];                      // 消息token
    unsigned 								token_len;                                          // 消息token长度

    CoAPMsgOptionList 						op_list;                                            // Option链表

    OnRespCallback							handler;											// CoAP Response消息回调处理函数
    void									*user_context;										// 用户上下文
} CoAPMessage;

#define DEFAULT_COAP_MESSAGE {COAP_MSG_VER, COAP_MSG_CON, COAP_MSG_REQ, COAP_MSG_POST, 0, NULL, 0, {0}, 0, {0}, NULL, NULL}

/**
 * @brief 对CoAPClient对象字段进行初始化
 *
 * @param pClient CoAP客户端结构体
 * @param pParams CoAP客户端初始化参数
 * @return 返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int qcloud_iot_coap_init(CoAPClient *pClient, CoAPInitParams *pParams);

/**
 * @brief 生成下一条CoAPMessage 消息Id
 *
 * @param pClient CoAP客户端结构体
 * @return 消息Id
 */
uint16_t get_next_coap_msg_id(CoAPClient *pClient);

/**
 * @brief 生成下一条CoAPMessage 消息token
 *
 * @param pClient CoAP客户端结构体
 * @param[in,out] tokenData 消息token返回值
 * @return token长度
 */
unsigned int get_coap_message_token(CoAPClient *client, char *tokenData);

/**
 * @brief 设置消息类型
 *
 * @param message CoAP消息对象
 * @param
 * @return QCLOUD_ERR_SUCCESS: 设置成功; < 0: 设置失败
 */
int coap_message_type_set(CoAPMessage *message, unsigned type);

/**
 * @brief 设置消息code
 *
 * @param message CoAP消息对象
 * @param code_class CoAPMessageClass
 * @param code_detail
 * @return QCLOUD_ERR_SUCCESS: 设置成功; < 0: 设置失败
 */
int coap_message_code_set(CoAPMessage *message, unsigned code_class, unsigned code_detail);

/**
 * @brief 设置消息Id
 *
 * @param message CoAP消息对象
 * @param msg_id
 * @return QCLOUD_ERR_SUCCESS: 设置成功; < 0: 设置失败
 */
int coap_message_id_set(CoAPMessage *message, unsigned msg_id);

/**
 * @brief 设置消息token
 *
 * @param message CoAP消息对象
 * @param buf token字符串
 * @param len token长度
 * @return QCLOUD_ERR_SUCCESS: 设置成功; < 0: 设置失败
 */
int coap_message_token_set(CoAPMessage *message, char *buf, size_t len);

/**
 * @brief 设置消息内容负责
 *
 * @param message CoAP消息对象
 * @param buf 消息负载
 * @param len 负载长度
 * @return QCLOUD_ERR_SUCCESS: 设置成功; < 0: 设置失败
 */
int coap_message_payload_set(CoAPMessage *message, char *buf, size_t len);

/**
 * @brief 增加消息option
 *
 * @param message CoAP消息对象
 * @param num option number
 * @param len option 长度
 * @param val option 字符串内容
 * @return QCLOUD_ERR_SUCCESS: 设置成功; < 0: 设置失败
 */
int coap_message_option_add(CoAPMessage *message, unsigned num, unsigned len, const char *val);

/**
 * @brief 设置响应消息回调函数
 *
 * @param message CoAP消息对象
 * @param callback 回调函数
 * @return QCLOUD_ERR_SUCCESS: 设置成功; < 0: 设置失败
 */
int coap_message_callback_set(CoAPMessage *message, OnRespCallback callback);

/**
 * @brief 设置用户上下文
 *
 * @param message CoAP消息对象
 * @param userContext 用户上下文
 * @return QCLOUD_ERR_SUCCESS: 设置成功; < 0: 设置失败
 */
int coap_message_context_set(CoAPMessage *message, void *userContext);

/**
 * @brief 根据option number/len/val 构造CoAPMsgOption对象
 *
 * @param num CoAP option number
 * @param len CoAP option string len
 * @param val CoAP option string value
 * @return QCLOUD_ERR_SUCCESS: 设置成功; < 0: 设置失败
 */
CoAPMsgOption* qcloud_iot_coap_option_init(unsigned num, unsigned len, const char *val);

/**
 *  @brief 销毁CoAPMessage对象
 *
 *  @param[in,out] message指针
 */
void coap_message_destroy(CoAPMessage *message);

/**
 * @brief CoAP消息网络读取、消息处理包裹函数
 *
 * @param pClient CoAPClient 对象指针
 * @param timeout_ms 读超时时长
 * @return QCLOUD_ERR_SUCCESS: 成功; < 0: 读循环失败
 */
int coap_message_cycle(CoAPClient *client, uint32_t timeout_ms);

/**
 * @brief CoAP消息网络发送
 *
 * @param client CoAPClient 对象指针
 * @param message 待发送的消息对象
 * @return QCLOUD_ERR_SUCCESS: 写入网络成功; < 0: 失败
 */
ssize_t coap_message_send(CoAPClient *client, CoAPMessage *message);

/**
 * @brief CoAP消息网络读取
 *
 * @param client CoAPClient 对象指针
 * @param timeout_ms 读超时时长
 * @return QCLOUD_ERR_SUCCESS: 写入网络成功; < 0: 失败
 */
ssize_t coap_message_recv(CoAPClient *client, uint32_t timeout_ms);

/**
 * @brief CoAPClient鉴权函数，发送鉴权消息并等待回包
 *
 * @param client CoAPClient 对象指针
 * @return QCLOUD_ERR_SUCCESS: 鉴权消息写入网络成功; < 0: 失败
 */
int  coap_client_auth(CoAPClient *client);

/**
 *  @brief Parse a message
 *
 *  @param[in,out] message Pointer to a message structure
 *  @param[in] buf Pointer to a buffer containing the message
 *  @param[in] len Length of the buffer
 *
 *  @returns Operation status
 *  @retval 0 Success
 *  @retval <0 Error
 */
ssize_t deserialize_coap_message(CoAPMessage *message, char *buf, size_t len);

/**
 *  @brief Format a message
 *
 *  @param[in] message Pointer to a message structure
 *  @param[out] buf Pointer to a buffer to contain the formatted message
 *  @param[in] len Length of the buffer
 *
 *  @returns Length of the formatted message or error code
 *  @retval >0 Length of the formatted message
 *  @retval <0 Error
 */
ssize_t serialize_coap_message(CoAPMessage *message, char *buf, size_t len);


void coap_msg_dump(CoAPMessage* msg);

#ifdef __cplusplus
}
#endif


#endif /* IOT_COAP_CLIENT_H_ */
