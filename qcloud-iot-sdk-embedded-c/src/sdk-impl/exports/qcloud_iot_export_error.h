#ifndef QCLOUD_IOT_EXPORT_ERROR_H
#define QCLOUD_IOT_EXPORT_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * IOT MQTT SDK 错误码
 */
typedef enum {
    QCLOUD_ERR_MQTT_ALREADY_CONNECTED                        = 4,       // 表示与MQTT服务器已经建立连接
    QCLOUD_ERR_MQTT_CONNACK_CONNECTION_ACCEPTED              = 3,       // 表示服务器接受客户端MQTT连接
    QCLOUD_ERR_MQTT_MANUALLY_DISCONNECTED                    = 2,       // 表示与MQTT服务器已经手动断开
    QCLOUD_ERR_MQTT_RECONNECTED                              = 1,       // 表示与MQTT服务器重连成功

    QCLOUD_ERR_HTTP_CLOSED                                   = -3,      // 远程主机关闭连接
    QCLOUD_ERR_HTTP                                          = -4,      // HTTP未知错误
    QCLOUD_ERR_HTTP_PRTCL                                    = -5,      // 协议错误
    QCLOUD_ERR_HTTP_UNRESOLVED_DNS                           = -6,      // 域名解析失败
    QCLOUD_ERR_HTTP_PARSE                                    = -7,      // URL解析失败
    QCLOUD_ERR_HTTP_CONN                                     = -8,      // HTTP连接失败

    QCLOUD_ERR_SUCCESS                                       = 0,       // 表示成功返回
    QCLOUD_ERR_FAILURE                                       = -101,    // 表示失败返回
    QCLOUD_ERR_INVAL                                         = -102,    // 表示参数无效错误
    QCLOUD_ERR_MQTT_NO_CONN                                  = -103,    // 表示未与MQTT服务器建立连接或已经断开连接
    QCLOUD_ERR_MQTT_UNKNOWN                                  = -104,    // 表示MQTT相关的未知错误
    QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT                     = -105,    // 表示正在与MQTT服务重新建立连接
    QCLOUD_ERR_MQTT_RECONNECT_TIMEOUT                        = -106,    // 表示重连已经超时
    QCLOUD_ERR_MQTT_MAX_SUBSCRIPTIONS                        = -107,    // 表示超过可订阅的主题数
    QCLOUD_ERR_MQTT_SUB                                      = -108,    // 表示订阅主题失败, 即服务器拒绝
    QCLOUD_ERR_MQTT_NOTHING_TO_READ                          = -109,    // 表示无MQTT相关报文可以读取
    QCLOUD_ERR_MQTT_PACKET_READ                              = -110,    // 表示读取的MQTT报文有问题
    QCLOUD_ERR_MQTT_REQUEST_TIMEOUT                          = -111,    // 表示MQTT相关操作请求超时
    QCLOUD_ERR_MQTT_CONNACK_UNKNOWN                          = -112,    // 表示客户端MQTT连接未知错误
    QCLOUD_ERR_MQTT_CONANCK_UNACCEPTABLE_PROTOCOL_VERSION    = -113,    // 表示客户端MQTT版本错误
    QCLOUD_ERR_MQTT_CONNACK_IDENTIFIER_REJECTED              = -114,    // 表示客户端标识符错误
    QCLOUD_ERR_MQTT_CONNACK_SERVER_UNAVAILABLE               = -115,    // 表示服务器不可用
    QCLOUD_ERR_MQTT_CONNACK_BAD_USERDATA                     = -116,    // 表示客户端连接参数中的username或password错误
    QCLOUD_ERR_MQTT_CONNACK_NOT_AUTHORIZED                   = -117,    // 表示客户端连接认证失败
    QCLOUD_ERR_RX_MESSAGE_INVAL                              = -118,    // 表示收到的消息无效
    QCLOUD_ERR_BUF_TOO_SHORT                                 = -119,    // 表示消息接收缓冲区的长度小于消息的长度
    QCLOUD_ERR_TCP_SOCKET_FAILED                             = -120,    // 表示TCP连接建立套接字失败
    QCLOUD_ERR_TCP_UNKNOWN_HOST                              = -121,    // 表示无法通过主机名获取IP地址
    QCLOUD_ERR_TCP_CONNECT                                   = -122,    // 表示建立TCP连接失败
    QCLOUD_ERR_SSL_INIT                                      = -123,    // 表示SSL初始化失败
    QCLOUD_ERR_SSL_CERT                                      = -124,    // 表示SSL证书相关问题
    QCLOUD_ERR_SSL_CONNECT                                   = -125,    // 表示SSL连接失败
    QCLOUD_ERR_SSL_CONNECT_TIMEOUT                           = -126,    // 表示SSL连接超时
    QCLOUD_ERR_SSL_WRITE_TIMEOUT                             = -127,    // 表示SSL写超时
    QCLOUD_ERR_SSL_WRITE                                     = -128,    // 表示SSL写错误
    QCLOUD_ERR_SSL_READ_TIMEOUT                              = -129,    // 表示SSL读超时
    QCLOUD_ERR_SSL_READ                                      = -130,    // 表示SSL读错误
    QCLOUD_ERR_SSL_NOTHING_TO_READ                           = -131,    // 表示底层没有数据可以读取
    QCLOUD_ERR_JSON_PARSE                                    = -132,    // 表示JSON解析错误
    QCLOUD_ERR_JSON_BUFFER_TRUNCATED                         = -133,    // 表示JSON文档会被截断
    QCLOUD_ERR_JSON_BUFFER_TOO_SMALL                         = -134,    // 表示存储JSON文档的缓冲区太小
    QCLOUD_ERR_JSON                                          = -135,    // 表示JSON文档生成错误
    QCLOUD_ERR_MAX_JSON_TOKEN                                = -136,    // 表示超过JSON文档中的最大Token数
    QCLOUD_ERR_MAX_APPENDING_REQUEST                         = -137,    // 表示超过同时最大的文档请求
    QCLOUD_ERR_MAX_TOPIC_LENGTH                              = -138,    // 表示超过规定最大的topic长度
} IoT_Error_Code;

#ifdef __cplusplus
}
#endif

#endif /* QCLOUD_IOT_EXPORT_ERROR_H */
