#ifndef QCLOUD_IOT_EXPORT_NBIOT_H_
#define QCLOUD_IOT_EXPORT_NBIOT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define MAX_SIZE_MESSAGE 1024
#define MAX_SIZE_PAYLOAD 500
#define MAX_SIZE_TOPIC   64
#define TLV_COUNT        5

typedef struct nbiotSetMessage
{
    uint8_t  address;
    uint8_t  version;
    uint8_t  sigType;
    uint32_t expiry;
    uint8_t  qos;
    char*    topic;
    char*    payload;
    char*    key;
}NBIoTSetMessage;

typedef struct nbiotGetMessage
{
    uint8_t  address;
    uint8_t  version;
    uint8_t  sigType;
    uint32_t expiry;
    uint8_t  qos;
    char*    topic;
    char*    payload;
    //char*    token;
}NBIoTGetMessage;

/**
 * @brief nbiot平台上行消息编码接口
 *
 * 将 nbiot 终端需要发送的消息，通过此接口生成一串十六进制的信息发送到 nbiot 平台
 *
 * @param msg      生成的十六进制上行消息编码
 * @param length   十六进制上行消息的长度，以字节为单位
 * @param nbiotMsg nbiot终端发送消息的结构体，包括 address、 version、签名类型、过期时间、传输质量、topic、payload、key等信息
 * @return 返回QCLOUD_ERR_SUCCESS, 表示成功, 返回QCLOUD_ERR_FAILURE, 表示失败
 */
int IOT_NB_setMessage(unsigned char* msg, unsigned int* length, NBIoTSetMessage* nbiotMsg);

/**
 * @brief nbiot平台下行消息解码接口
 *
 * 将 nbiot 终端接收的一串十六进制的信息解析出来
 *
 * @param nbiotMsg nbiot终端解析出来的消息内容结构体，包括 address、version、签名类型、过期时间、传输质量、topic、payload等信息
 * @param msg      nbiot平台发送到设备端的十六进制下行消息编码
 * @return 返回QCLOUD_ERR_SUCCESS, 表示成功, 返回QCLOUD_ERR_FAILURE, 表示失败
 */
int IOT_NB_getMessage(NBIoTGetMessage* nbiotMsg, unsigned char* msg);

#ifdef __cplusplus
}
#endif

#endif /* QCLOUD_IOT_EXPORT_NBIOT_H_ */