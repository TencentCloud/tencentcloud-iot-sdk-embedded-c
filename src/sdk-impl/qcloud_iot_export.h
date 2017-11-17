#ifndef QCLOUD_IOT_EXPORT_H
#define QCLOUD_IOT_EXPORT_H

#ifdef __cplusplus
extern "C" {
#endif

/********** Config **********/
// IoT C-SDK APPID
#define QCLOUD_IOT_DEVICE_SDK_APPID                                "10011117"
    
// MQTT连接相关信息
#define QCLOUD_IOT_MQTT_KEEP_ALIVE_INTERNAL                         240                                     // MQTT心跳消息发送周期, 单位:s
#define QCLOUD_IOT_MQTT_COMMAND_TIMEOUT                             2000                                    // MQTT 阻塞调用(包括连接, 订阅, 发布等)的超时时间, 单位:ms 建议2000ms
#define QCLOUD_IOT_TLS_HANDSHAKE_TIMEOUT                            5000                                    // TLS连接握手超时时间, 单位:ms
    
// 云端虚拟设备初始化信息
#define CLOUD_RX_BUF_LEN                                            QCLOUD_IOT_MQTT_RX_BUF_LEN + 1          // 接收云端返回的JSON文档的buffer大小
#define MAX_SIZE_OF_CLIENT_ID                                       80                                      // 设备ID的最大长度, 必须保持唯一
#define MAX_SIZE_OF_CLIENT_TOKEN                                    MAX_SIZE_OF_CLIENT_ID + 10              // 一个clientToken的最大长度
#define MAX_SIZE_OF_JSON_WITH_CLIENT_TOKEN                          MAX_SIZE_OF_CLIENT_TOKEN + 20           // 一个仅包含clientToken字段的JSON文档的最大长度
#define MAX_APPENDING_REQUEST_AT_ANY_GIVEN_TIME                     10                                      // 在任意给定时间内, 处于appending状态的请求最大个数
#define MAX_DEVICE_HANDLED_AT_ANY_GIVEN_TIME                        10                                      // 在任意给定时间内, 可以同时操作设备的最大个数
#define MAX_JSON_TOKEN_EXPECTED                                     120                                     // JSON文档中JSONToken的最大个数
#define MAX_SIZE_OF_CLOUD_TOPIC_WITHOUT_DEVICE_NAME                 60                                      // 除设备名称之外, 云端保留主题的最大长度。
#define MAX_SIZE_OF_PRODUCT_NAME                                    20                                      // 产品名称的最大长度, 可根据实际情况增大
#define MAX_SIZE_OF_DEVICE_NAME                                     20                                      // 设备名称的最大长度, 可根据实际情况增大
#define MAX_SIZE_OF_CLOUD_TOPIC                                     128                                     // 云端保留主题的最大长度

// MQTT
#define QCLOUD_IOT_MQTT_TX_BUF_LEN                                  512                                     // MQTT消息发送buffer大小
#define QCLOUD_IOT_MQTT_RX_BUF_LEN                                  512                                     // MQTT消息接收buffer大小
#define MAX_PACKET_ID               								65535          							// 报文id最大值
#define MAX_MESSAGE_HANDLERS        								5              							// 订阅主题的最大个数
#define MIN_RECONNECT_WAIT_INTERVAL 								1000           							// 重连最小等待时间
#define MAX_RECONNECT_WAIT_INTERVAL 								60000          							// 重连最大等待时间
#define MIN_COMMAND_TIMEOUT         								500            							// MQTT报文最小超时时间
#define MAX_COMMAND_TIMEOUT         								5000           							// MQTT报文最大超时时间
#define DEFAULT_COMMAND_TIMEOUT     								2000           							// 默认MQTT报文超时时间

typedef struct {
	char product_name[MAX_SIZE_OF_PRODUCT_NAME];
	char device_name[MAX_SIZE_OF_DEVICE_NAME];
	char client_id[MAX_SIZE_OF_CLIENT_ID];
} DeviceInfo;
    
#include "qcloud_iot_export_log.h"
#include "qcloud_iot_export_error.h"
#include "qcloud_iot_export_mqtt.h"
#include "qcloud_iot_export_mqtt_json.h"
#include "qcloud_iot_export_shadow.h"
#include "qcloud_iot_export_shadow_json.h"
    
#ifdef __cplusplus
}
#endif

#endif /* QCLOUD_IOT_EXPORT_H */
