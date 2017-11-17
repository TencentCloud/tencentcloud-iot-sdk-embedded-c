#ifndef MQTT_CLIENT_C_QCLOUD_IOT_MQTT_CLIENT_H
#define MQTT_CLIENT_C_QCLOUD_IOT_MQTT_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"
#include "qcloud_iot_sdk_impl_internal.h"

#include "qcloud_iot_utils_timer.h"
#include "mqtt_client_net.h"

/**
 * @brief MQTT Message Type
 */
typedef enum msgTypes {
	RESERVED    = 0,     // Reserved
	CONNECT     = 1,     // Client request to connect to Server
	CONNACK     = 2,     // Connect Acknowledgment
	PUBLISH     = 3,     // Publish message
	PUBACK      = 4,     // Publish Acknowledgment
	PUBREC      = 5,     // Publish Received
	PUBREL      = 6,     // Publish Release
	PUBCOMP     = 7,     // Publish Complete
	SUBSCRIBE   = 8,     // Client Subscribe request
	SUBACK      = 9,     // Subscribe Acknowledgment
	UNSUBSCRIBE = 10,    // Client Unsubscribe request
	UNSUBACK    = 11,    // Unsubscribe Acknowledgment
	PINGREQ     = 12,    // PING Request
	PINGRESP    = 13,    // PING Response
	DISCONNECT  = 14     // Client is Disconnecting
	//RESERVED    = 0, // Reserved
} MessageTypes;


/**
 * Bitfields for the MQTT header byte.
 */
typedef union {
	unsigned char byte;	                /**< the whole byte */
#if defined(REVERSED)
	struct {
		unsigned int type : 4;			/**< message type nibble */
		unsigned int dup : 1;				/**< DUP flag bit */
		unsigned int qos : 2;				/**< QoS value, 0, 1 or 2 */
		unsigned int retain : 1;		/**< retained flag bit */
	} bits;
#else
	struct {
		unsigned int retain : 1;		/**< retained flag bit */
		unsigned int qos : 2;				/**< QoS value, 0, 1 or 2 */
		unsigned int dup : 1;				/**< DUP flag bit */
		unsigned int type : 4;			/**< message type nibble */
	} bits;
#endif
} MQTTHeader;

/**
 * @brief MQTT 遗嘱消息参数结构体定义
 *
 * 当客户端异常断开连接, 若客户端在连接时有设置遗嘱消息, 服务端会发布遗嘱消息。
 */
typedef struct {
    char        struct_id[4];      // The eyecatcher for this structure. must be MQTW
    uint8_t     struct_version;    // 结构体 0
    char        *topicName;        // 遗嘱消息主题名
    char        *message;          // 遗嘱消息负载部分数据
    uint8_t     retained;          // 遗嘱消息retain标志位
    QoS         qos;               // 遗嘱消息qos标志位
} WillOptions;

/**
 * MQTT遗嘱消息结构体默认值定义
 */
#define DefaultWillOptions { {'M', 'Q', 'T', 'W'}, 0, NULL, NULL, 0, QOS0 }

/**
 * @brief MQTT 连接参数结构体定义
 *
 */
typedef struct {
    char            			*client_id;             // 客户端标识符, 请保持唯一
    char            			*username;              // 用户名
    char            			*password;              // 密码

    char            			struct_id[4];           // The eyecatcher for this structure.  must be MQTC.
    uint8_t         			struct_version;         // 结构体版本号, 必须为0
    uint8_t         			MQTTVersion;            // MQTT版本协议号 4 = 3.1.1

    uint16_t        			keep_alive_interval;    // 心跳周期, 单位: s
    uint8_t         			clean_session;          // 清理会话标志位, 具体含义请参考MQTT协议说明文档3.1.2.4小结

    uint8_t                   	auto_connect_enable;                 // 是否开启自动重连

    OnDisconnectHandler       	on_disconnect_handler;               // 连接断开或丢失的回调函数指针

} MQTTConnectParams;

/**
 * MQTT连接参数结构体默认值定义
 */
#define DEFAULT_MQTTCONNECT_PARAMS { NULL, NULL, NULL, {'M', 'Q', 'T', 'C'}, 0, 4, 240, 1, 0, NULL}
/**
 * @brief 订阅主题对应的消息处理结构体定义
 */
typedef struct MessageHandlers {
    const char              *topicFilter;               // 订阅主题名, 可包含通配符
    OnMessageHandler        messageHandler;             // 订阅主题消息回调函数指针
    void                    *pMessageHandlerData;       // 用户数据, 通过回调函数返回
    QoS                     qos;                        // 服务质量等级
} MessageHandlers;

/**
 * @brief IoT Client结构体定义
 */
typedef struct Client {
    uint8_t                  is_connected;                                  // 网络是否连接
    uint8_t                  was_manually_disconnected;                     // 是否手动断开连接
    uint8_t                  is_ping_outstanding;                           // 心跳包是否未完成, 即未收到服务器响应

    uint16_t                 next_packetId;                                 // MQTT报文标识符
    uint32_t                 command_timeout_ms;                            // MQTT消息超时时间, 单位:ms

    uint32_t                 current_reconnect_waitInterval;                // MQTT重连周期, 单位:ms
    uint32_t                 counter_network_disconnected;                  // 网络断开连接次数

    size_t                   buf_size;                                      // 消息接收buffer长度
    size_t                   read_buf_size;                                 // 消息接收buffer长度
    unsigned char            buf[QCLOUD_IOT_MQTT_TX_BUF_LEN];               // MQTT消息发送buffer
    unsigned char            read_buf[QCLOUD_IOT_MQTT_RX_BUF_LEN];          // MQTT消息接收buffer

    MQTTConnectParams        options;                                       // 连接相关参数

    Network                  network_stack;                                 // MQTT底层使用的网络参数

    Timer                    ping_timer;                                    // MQTT心跳包发送定时器
    Timer                    reconnect_delay_timer;                         // MQTT重连定时器, 判断是否已到重连时间

    MessageHandlers          message_handlers[MAX_MESSAGE_HANDLERS];        // 订阅主题对应的消息处理结构数组

} Qcloud_IoT_Client;

/**
 * @brief MQTT协议版本
 */
typedef enum {
    MQTT_3_1_1 = 4
} MQTT_VERSION;

/**
 * @brief 对结构体Client进行初始化
 *
 * @param pClient MQTT客户端结构体
 * @param pParams MQTT客户端初始化参数
 * @return 返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int qcloud_iot_mqtt_init(Qcloud_IoT_Client *pClient, MQTTInitParams *pParams);

/**
 * @brief 建立基于TLS的MQTT连接
 *
 * 注意: Client ID不能为NULL或空字符串
 *
 * @param pClient MQTT客户端结构体
 * @param pParams MQTT连接相关参数, 可参考MQTT协议说明
 * @return 返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int qcloud_iot_mqtt_connect(Qcloud_IoT_Client *pClient, MQTTConnectParams *pParams);

/**
 * @brief 与服务器重新建立MQTT连接
 *
 * 1. 与服务器重新建立MQTT连接
 * 2. 连接成功后, 重新订阅之前的订阅过的主题
 *
 * @param pClient MQTT Client结构体
 *
 * @return 返回QCLOUD_ERR_MQTT_RECONNECTED, 表示重连成功
 */
int qcloud_iot_mqtt_attempt_reconnect(Qcloud_IoT_Client *pClient);

/**
 * @brief 断开MQTT连接
 *
 * @param pClient MQTT Client结构体
 * @return 返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int qcloud_iot_mqtt_disconnect(Qcloud_IoT_Client *pClient);

/**
 * @brief 发布MQTT消息
 *
 * @param pClient MQTT客户端结构体
 * @param topicName 主题名
 * @param pParams 发布参数
 * @return 返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int qcloud_iot_mqtt_publish(Qcloud_IoT_Client *pClient, char *topicName, PublishParams *pParams);

/**
 * @brief 订阅MQTT主题
 *
 * @param pClient MQTT客户端结构体
 * @param topicFilter 主题过滤器, 可参考MQTT协议说明 4.7
 * @param pParams 订阅参数
 * @return 返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int qcloud_iot_mqtt_subscribe(Qcloud_IoT_Client *pClient, char *topicFilter, SubscribeParams *pParams);

/**
 * @brief 重新订阅断开连接之前已订阅的主题
 *
 * @param pClient MQTT客户端结构体
 * @return 返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int qcloud_iot_mqtt_resubscribe(Qcloud_IoT_Client *pClient);

/**
 * @brief 取消订阅已订阅的MQTT主题
 *
 * @param pClient MQTT客户端结构体
 * @param topicFilter  主题过滤器, 可参考MQTT协议说明 4.7
 * @return 返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int qcloud_iot_mqtt_unsubscribe(Qcloud_IoT_Client *pClient, char *topicFilter);

/**
 * @brief 在当前线程为底层MQTT客户端让出一定CPU执行时间
 *
 * 在这段时间内, MQTT客户端会用用处理消息接收, 以及发送PING报文, 监控网络状态
 *
 * @param pClient    MQTT Client结构体
 * @param timeout_ms Yield操作超时时间
 * @return 返回QCLOUD_ERR_SUCCESS, 表示成功, 返回QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT, 表示正在重连
 */
int qcloud_iot_mqtt_yield(Qcloud_IoT_Client *pClient, uint32_t timeout_ms);

/**
 * @brief 客户端自动重连是否开启
 *
 * @param pClient MQTT客户端结构体
 * @return 返回true, 表示客户端已开启自动重连功能
 */
bool qcloud_iot_mqtt_is_autoreconnect_enabled(Qcloud_IoT_Client *pClient);

/**
 * @brief 设置客户端连接断开回调处理函数
 *
 * @param pClient MQTT客户端结构体
 * @param on_disconnect_handler 连接断开回调函数
 * @return 返回QCLOUD_ERR_SUCCESS, 表示设置成功
 */
int qcloud_iot_mqtt_set_disconnect_handler(Qcloud_IoT_Client *pClient, OnDisconnectHandler on_disconnect_handler);

/**
 * @brief 设置客户端是否开启自动重连
 *
 * @param pClient MQTT客户端结构体
 * @param value 是否开启该功能
 * @return 返回QCLOUD_ERR_SUCCESS, 表示设置成功
 */
int qcloud_iot_mqtt_set_autoreconnect(Qcloud_IoT_Client *pClient, bool value);

/**
 * @brief 获取网络断开的次数
 *
 * @param pClient MQTT Client结构体
 * @return 返回客户端MQTT网络断开的次数
 */
int qcloud_iot_mqtt_get_network_disconnected_count(Qcloud_IoT_Client *pClient);

/**
 * @brief 重置连接断开次数
 *
 * @param pClient MQTT Client结构体
 * @return 返回QCLOUD_ERR_SUCCESS, 表示设置成功
 */
int qcloud_iot_mqtt_reset_network_disconnected_count(Qcloud_IoT_Client *pClient);

/**
 * @brief 获取报文标识符
 *
 * @param pClient
 * @return
 */
uint16_t get_next_packet_id(Qcloud_IoT_Client *pClient);

/**
 *
 * @param header
 * @param message_type
 * @param qos
 * @param dup
 * @param retained
 * @return
 */
int mqtt_init_packet_header(MQTTHeader *header, MessageTypes message_type, QoS qos, uint8_t dup, uint8_t retained);

/**
 * @brief 接收服务端消息
 *
 * @param pClient
 * @param timer
 * @param packet_type
 * @return
 */
int cycle_for_read(Qcloud_IoT_Client *pClient, Timer *timer, uint8_t *packet_type);

/**
 * @brief 发送报文数据
 *
 * @param pClient       Client结构体
 * @param length  报文长度
 * @param timer   定时器
 * @return
 */
int send_mqtt_packet(Qcloud_IoT_Client *pClient, size_t length, Timer *timer);

/**
 * @brief 等待指定类型的MQTT控制报文
 *
 * only used in single-threaded mode where one command at a time is in process
 *
 * @param c MQTT Client结构体
 * @param packet_type 控制报文类型
 * @param timer 定时器
 * @return
 */
int wait_for_read(Qcloud_IoT_Client *c, uint8_t packet_type, Timer *timer);

int deserialize_publish_packet(unsigned char *dup, QoS *qos, uint8_t *retained, uint16_t *packet_id, char **topicName,
                               uint16_t *topicNameLen, unsigned char **payload, size_t *payload_len, unsigned char *buf, size_t buf_len);

int serialize_pub_ack_packet(unsigned char *buf, size_t buf_len, MessageTypes packet_type, uint8_t dup,
                             uint16_t packet_id,
                             uint32_t *serialized_len);
int serialize_packet_with_zero_payload(unsigned char *buf, size_t buf_len, MessageTypes packetType, uint32_t *serialized_len);
int deserialize_ack_packet(uint8_t *packet_type, uint8_t *dup, uint16_t *packet_id, unsigned char *buf, size_t buf_len);

/**
 * @brief 根据剩余长度计算整个MQTT报文的长度
 *
 * @param rem_len    剩余长度
 * @return           整个MQTT报文的长度
 */
size_t get_mqtt_packet_len(size_t rem_len);

size_t mqtt_write_packet_rem_len(unsigned char *buf, uint32_t length);

int mqtt_read_packet_rem_len_form_buf(unsigned char *buf, uint32_t *value, uint32_t *readBytesLen);

uint16_t mqtt_read_uint16_t(unsigned char **pptr);

unsigned char mqtt_read_char(unsigned char **pptr);
void mqtt_write_char(unsigned char **pptr, unsigned char c);
void mqtt_write_uint_16(unsigned char **pptr, uint16_t anInt);
void mqtt_write_utf8_string(unsigned char **pptr, const char *string);

#ifdef __cplusplus
}
#endif

#endif //MQTT_CLIENT_C_QCLOUD_IOT_MQTT_CLIENT_H
