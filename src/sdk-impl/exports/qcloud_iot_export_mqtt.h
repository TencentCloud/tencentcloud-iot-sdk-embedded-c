#ifndef QCLOUD_IOT_EXPORT_MQTT_H
#define QCLOUD_IOT_EXPORT_MQTT_H

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
    QoS         qos;          // MQTT 服务质量等级
    uint8_t     retained;     // RETAIN 标识位
    uint8_t     dup;          // DUP 标识位
    uint16_t    id;           // MQTT 消息标识符
    void        *payload;     // MQTT 消息负载
    size_t      payload_len;  // MQTT 消息负载长度
} MQTTMessage;

typedef MQTTMessage PublishParams;

#define DefaultPubParams {QOS0, 0, 0, 0, NULL, 0}

/**
 * @brief MQTT 连接断开回调处理函数指针定义
 */
typedef void (*OnDisconnectHandler)(void);

/**
 * @brief MQTT PUBLISH 消息回调处理函数指针定义
 */
typedef void (*OnMessageHandler)(char *topicName, uint16_t topicNameLen, MQTTMessage *message, void *pApplicationHandlerData);

/**
 * @brief 订阅主题的结构体定义
 */
typedef struct {
    QoS                     qos;                    // 服务质量等级, 目前支持QOS0和QOS1
    OnMessageHandler        on_message_handler;     // 接收已订阅消息的回调函数
    void                    *pUserdata;             // 用户数据, 通过callback返回
} SubscribeParams;

/**
 * MQTT 订阅报文默认参数
 */
#define DEFAULT_SUB_PARAMS {QOS0, NULL, NULL}

typedef struct {
	/**
	 * 设备基础信息
	 */
    char            			*client_id;            	// 客户端标识符, 请保持唯一
    char            			*username;             	// 用户名
    char            			*password;             	// 密码
    char 						*product_name;			// 产品名称
    char 						*device_name;			// 设备名称

	/**
	 * 非对称加密使用
	 */
    char                        *ca_file;                // 根证书文件路径, 用于校验服务端证书
    char                        *cert_file;              // 客户端证书文件路径
    char                        *key_file;               // 客户端私钥文件路径

    /**
     * 对称加密使用
     */
    char                      	*psk;                    // 对称加密密钥

    uint32_t					command_timeout;		 // 发布订阅信令读写超时时间
    uint32_t					connect_timeout;		 // 连接超时时间，tls表示握手超时时间
    uint16_t					keep_alive_interval;	 // 心跳周期, 单位: s

    uint8_t         			clean_session;			 // 清理会话标志位

    uint8_t                   	auto_connect_enable;     // 是否开启自动重连 1:启用自动重连 0：不启用自动重连  建议为1

    bool						is_asymc_encryption;	 // 是否使用非对称加密 1:非对称 0:对称

    OnDisconnectHandler       	on_disconnect_handler;   // 连接断开或丢失的回调函数指针

} MQTTInitParams;

/**
 * MQTT初始化参数结构体默认值定义
 */
#define DEFAULT_MQTTINIT_PARAMS { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 2000, 5000, 240, 1, 0, 0, NULL}

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
 * @return 返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int IOT_MQTT_Publish(void *pClient, char *topicName, PublishParams *pParams);

/**
 * @brief 订阅MQTT主题
 *
 * @param pClient MQTT客户端结构体
 * @param topicFilter 主题过滤器, 可参考MQTT协议说明 4.7
 * @param pParams 订阅参数
 * @return 返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int IOT_MQTT_Subscribe(void *pClient, char *topicFilter, SubscribeParams *pParams);

/**
 * @brief 取消订阅已订阅的MQTT主题
 *
 * @param pClient MQTT客户端结构体
 * @param topicFilter  主题过滤器, 可参考MQTT协议说明 4.7
 * @return 返回QCLOUD_ERR_SUCCESS, 表示成功
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

#endif /* QCLOUD_IOT_EXPORT_MQTT_H */
