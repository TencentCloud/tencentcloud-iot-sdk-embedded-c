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

#ifndef QCLOUD_IOT_EXPORT_MQTT_H_
#define QCLOUD_IOT_EXPORT_MQTT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief 服务质量等级
 *
 * 服务质量等级表示PUBLISH消息分发的质量等级
 */
typedef enum _QoS {
    QOS0 = 0,    // 至多分发一次
    QOS1 = 1,    // 至少分发一次, 消息的接收者需回复PUBACK报文
    QOS2 = 2     // 仅分发一次, 目前腾讯物联云不支持该等级
} QoS;

/**
 * @brief 发布或接收已订阅消息的结构体定义
 */
typedef struct {
    QoS         			qos;          // MQTT 服务质量等级
    uint8_t     			retained;     // RETAIN 标识位
    uint8_t     			dup;          // DUP 标识位
    uint16_t    			id;           // MQTT 消息标识符

    const char  			*ptopic;      // MQTT topic
    size_t      			topic_len;    // topic 长度

    void        			*payload;     // MQTT 消息负载
    size_t      			payload_len;  // MQTT 消息负载长度
} MQTTMessage;

typedef MQTTMessage PublishParams;

#define DEFAULT_PUB_PARAMS {QOS0, 0, 0, 0, NULL, 0, NULL, 0}

typedef enum {

    /* 未定义事件 */
    MQTT_EVENT_UNDEF = 0,

    /* MQTT 断开连接 */
    MQTT_EVENT_DISCONNECT = 1,

    /* MQTT 重连 */
    MQTT_EVENT_RECONNECT = 2,

    /* 订阅成功 */
    MQTT_EVENT_SUBCRIBE_SUCCESS = 3,

    /* 订阅超时 */
    MQTT_EVENT_SUBCRIBE_TIMEOUT = 4,

    /* 订阅失败 */
    MQTT_EVENT_SUBCRIBE_NACK = 5,

    /* 取消订阅成功 */
    MQTT_EVENT_UNSUBCRIBE_SUCCESS = 6,

    /* 取消订阅超时 */
    MQTT_EVENT_UNSUBCRIBE_TIMEOUT = 7,

    /* 取消订阅失败 */
    MQTT_EVENT_UNSUBCRIBE_NACK = 8,

    /* 发布成功 */
    MQTT_EVENT_PUBLISH_SUCCESS = 9,

    /* 发布超时 */
    MQTT_EVENT_PUBLISH_TIMEOUT = 10,

    /* 发布失败 */
    MQTT_EVENT_PUBLISH_NACK = 11,

    /* SDK订阅的topic收到后台push消息 */
    MQTT_EVENT_PUBLISH_RECVEIVED = 12,

    /* MQTT client destroy */
    MQTT_EVENT_CLIENT_DESTROY = 13,

    /* 取消订阅 */
    MQTT_EVENT_UNSUBSCRIBE = 14,

} MQTTEventType;

/**
 * @brief MQTT SUBSCRIBE 消息回调处理函数指针定义
 */
typedef void (*OnMessageHandler)(void *pClient, MQTTMessage *message, void *pUserData);

/**
 * @brief MQTT SUBSCRIBE 事件回调处理函数指针定义
 */
typedef void (*OnSubEventHandler)(void *pClient, MQTTEventType event_type, void *pUserData);

/**
 * @brief 订阅主题的结构体定义
 */
typedef struct {
    QoS                     qos;                    // 服务质量等级, 目前支持QOS0和QOS1
    OnMessageHandler        on_message_handler;     // 接收已订阅消息的回调函数
    OnSubEventHandler       on_sub_event_handler;       // 接收该订阅消息事件的回调函数
    void                    *user_data;             // 用户数据, 通过callback返回
} SubscribeParams;

/**
 * MQTT 订阅报文默认参数
 */
#define DEFAULT_SUB_PARAMS {QOS0, NULL, NULL, NULL}


typedef struct {
    /* 事件类型 */
    MQTTEventType  event_type;

    void *msg;
} MQTTEventMsg;

/**
 * @brief 定义了函数指针的数据类型. 当相关事件发生时，将调用这种类型的函数.
 *
 * @param context, the program context
 * @param pclient, the MQTT client
 * @param msg, the event message.
 *
 * @return none
 */
typedef void (*MQTTEventHandleFun)(void *pclient, void *handle_context, MQTTEventMsg *msg);


/* The structure of MQTT event handle */
typedef struct {
    MQTTEventHandleFun      h_fp;
    void                    *context;
} MQTTEventHandler;

typedef struct {
	/**
	 * 设备基础信息
	 */
    char 						*product_id;			// 产品名称
    char 						*device_name;			// 设备名称

#ifdef AUTH_MODE_CERT
	/**
	 * 非对称加密使用
	 */
    char                        *cert_file;              // 客户端证书文件路径
    char                        *key_file;               // 客户端私钥文件路径
#else
    char                        *device_secret;
#endif

    uint32_t					command_timeout;		 // 发布订阅信令读写超时时间 ms
    uint32_t					keep_alive_interval_ms;	 // 心跳周期, 单位: ms

    uint8_t         			clean_session;			 // 清理会话标志位

    uint8_t                   	auto_connect_enable;     // 是否开启自动重连 1:启用自动重连 0：不启用自动重连  建议为1

    MQTTEventHandler            event_handle;            // 事件回调

} MQTTInitParams;

/**
 * MQTT初始化参数结构体默认值定义
 */
#ifdef AUTH_MODE_CERT
	#define DEFAULT_MQTTINIT_PARAMS { NULL, NULL, NULL, NULL, 2000, 240 * 1000, 1, 1, {0}}
#else
    #define DEFAULT_MQTTINIT_PARAMS { NULL, NULL, NULL, 2000, 240 * 1000, 1, 1, {0}}
#endif

/**
 * @brief 构造MQTTClient并完成MQTT连接
 *
 * @param pInitParams MQTT协议连接接入与连接维持阶段所需要的参数
 *
 * @return 返回NULL: 构造失败
 */
void* IOT_MQTT_Construct(MQTTInitParams *pParams);

/**
 * @brief 关闭MQTT连接并销毁MQTTClient
 *
 * @param pClient MQTTClient对象地址
 *
 * @return 返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int IOT_MQTT_Destroy(void **pClient);

/**
 * @brief 在当前线程为底层MQTT客户端让出一定CPU执行时间
 *
 * 在这段时间内, MQTT客户端会用用处理消息接收, 以及发送PING报文, 监控网络状态
 *
 * @param pClient    MQTT Client结构体
 * @param timeout_ms Yield操作超时时间
 * @return 返回QCLOUD_ERR_SUCCESS, 表示成功, 返回QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT, 表示正在重连
 */
int IOT_MQTT_Yield(void *pClient, uint32_t timeout_ms);

/**
 * @brief 发布MQTT消息
 *
 * @param pClient MQTT客户端结构体
 * @param topicName 主题名
 * @param pParams 发布参数
 * @return < 0  :   表示失败
 *         >= 0 :   返回唯一的packet id 
 */
int IOT_MQTT_Publish(void *pClient, char *topicName, PublishParams *pParams);

/**
 * @brief 订阅MQTT主题
 *
 * @param pClient MQTT客户端结构体
 * @param topicFilter 主题过滤器, 可参考MQTT协议说明 4.7
 * @param pParams 订阅参数
 * @return <  0  :   表示失败
 *         >= 0 :   返回唯一的packet id 
 */
int IOT_MQTT_Subscribe(void *pClient, char *topicFilter, SubscribeParams *pParams);

/**
 * @brief 取消订阅已订阅的MQTT主题
 *
 * @param pClient MQTT客户端结构体
 * @param topicFilter  主题过滤器, 可参考MQTT协议说明 4.7
 * @return <  0  :   表示失败
 *         >= 0 :   返回唯一的packet id
 */
int IOT_MQTT_Unsubscribe(void *pClient, char *topicFilter);

/**
 * @brief 客户端目前是否已连接
 *
 * @param pClient MQTT Client结构体
 * @return 返回true, 表示客户端已连接
 */
bool IOT_MQTT_IsConnected(void *pClient);

#ifdef __cplusplus
}
#endif

#endif /* QCLOUD_IOT_EXPORT_MQTT_H_ */
