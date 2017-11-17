#ifndef MQTT_CLIENT_C_UNIT_HELPER_FUNCTIONS_H
#define MQTT_CLIENT_C_UNIT_HELPER_FUNCTIONS_H

#include <qcloud_iot_mqtt_client.h>
#include "iot_unit_config.h"


#define QCLOUD_THINGS_TOPIC ""//"$qcloud/devices/"
#define SHADOW_TOPIC "$shadow/"
#define ACCEPTED_TOPIC "/accepted"
#define REJECTED_TOPIC "/rejected"
#define UPDATE_TOPIC "update"
#define GET_TOPIC "get"
#define DELETE_TOPIC "delete"


#define GET_ACCEPTED_TOPIC SHADOW_TOPIC QCLOUD_IOT_MY_PRODUCT_NAME QCLOUD_IOT_MY_DEVICE_NAME GET_TOPIC ACCEPTED_TOPIC
#define GET_REJECTED_TOPIC SHADOW_TOPIC QCLOUD_IOT_MY_PRODUCT_NAME QCLOUD_IOT_MY_DEVICE_NAME GET_TOPIC REJECTED_TOPIC
#define GET_PUB_TOPIC SHADOW_TOPIC QCLOUD_IOT_MY_PRODUCT_NAME QCLOUD_IOT_MY_DEVICE_NAME GET_TOPIC

#define DELETE_ACCEPTED_TOPIC SHADOW_TOPIC QCLOUD_IOT_MY_PRODUCT_NAME QCLOUD_IOT_MY_DEVICE_NAME DELETE_TOPIC ACCEPTED_TOPIC
#define DELETE_REJECTED_TOPIC SHADOW_TOPIC QCLOUD_IOT_MY_PRODUCT_NAME QCLOUD_IOT_MY_DEVICE_NAME DELETE_TOPIC REJECTED_TOPIC

#define UPDATE_ACCEPTED_TOPIC SHADOW_TOPIC QCLOUD_IOT_MY_PRODUCT_NAME QCLOUD_IOT_MY_DEVICE_NAME UPDATE_TOPIC ACCEPTED_TOPIC
#define UPDATE_REJECTED_TOPIC SHADOW_TOPIC QCLOUD_IOT_MY_PRODUCT_NAME QCLOUD_IOT_MY_DEVICE_NAME UPDATE_TOPIC REJECTED_TOPIC

typedef struct {
    unsigned char PacketType;        // 报文类型: CONNECT  0x10
    unsigned int RemainingLength;    // 剩余长度
    unsigned int ProtocolLength;     // 协议名长度 0x0004
    unsigned char ProtocolName[4];   // 协议名
    unsigned int ProtocolLevel;      // 协议版本号
    unsigned char ConnectFlag;       // 连接标志位
    unsigned int KeepAlive;          // 心跳周期
} ConnectBufferProofread;

void MQTTInitParamsSetup(MQTTConnectParams *pParams, char *pHost,
                           uint16_t port, bool enableAutoReconnect, OnDisconnectHandler callback);

void ConnectMQTTParamsSetup_Detailed(MQTTConnectParams *params, char *pClientID, QoS qos, bool isCleanSession,
                                     bool isWillMsgPresent, char *pWillTopicName, char *pWillMessage,
                                     char *pUsername, char *pPassword);

void ConnectParamsSetup(MQTTConnectParams *pParams, char *pClientId);

void setTLSRxBufferForConnack(MQTTConnectParams *pParams, unsigned char sessionPresent, unsigned char connackResponseCode);

void setTLSRxBufferForPuback();

void setTLSRxBufferForSuback(QoS qos);

void setTLSRxBufferForDoubleSuback(QoS qos);

void setTLSRxBufferForUnsuback();

void setTLSRxBufferForPingresp();

void setTLSRxBufferForConnackAndSuback(MQTTConnectParams *conParams, unsigned char sessionPresent, QoS qos);

void resetTlsBuffer();

bool isConnectTxBufFlagCorrect(MQTTConnectParams *settings, ConnectBufferProofread *readRes);

unsigned char *connectTxBufferHeaderParser(ConnectBufferProofread *params, unsigned char *buf);

void printPrfrdParams(ConnectBufferProofread *params);

bool isConnectTxBufPayloadCorrect(MQTTConnectParams *settings, unsigned char *payloadBuf);

void setTLSRxBufferWithMsgOnSubscribedTopic(char *topic, size_t topic_len, QoS qos, char *pMsg);

void setTLSRxBufferDelay(int seconds, int microseconds);

unsigned char isLastTLSTxMessagePuback();

unsigned char isLastTLSTxMessagePingreq();

unsigned char isLastTLSTxMessageDisconnect();
#endif //MQTT_CLIENT_C_UNIT_HELPER_FUNCTIONS_H
