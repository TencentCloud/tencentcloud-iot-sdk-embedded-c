#include <qcloud_iot_mqtt_client_interface.h>
#include <string.h>
#include <unit_mock_tls_params.h>

#include "unit_helper_functions.h"

#define CONNACK_PACKET_SIZE 4
#define CONNACK_SUBACK_PACKET_SIZE 9
#define PUBACK_PACKET_SIZE 4
#define SUBACK_PACKET_SIZE 5
#define UNSUBACK_PACKET_SIZE 4
#define PINGRESP_PACKET_SIZE 2

void ResetInvalidParameters(void) {
    invalidEndpointFilter = NULL;
    invalidRootCAPathFilter = NULL;
    invalidCertPathFilter = NULL;
    invalidPrivKeyPathFilter = NULL;
    invalidPortFilter = 0;
}

/**
 * 设置初始化参数
 *
 * @param pParams
 * @param pHost
 * @param port
 * @param enableAutoReconnect
 * @param callback
 */
void MQTTInitParamsSetup(MQTTConnectParams *pParams, char *pHost,
                           uint16_t port, bool enableAutoReconnect, OnDisconnectHandler callback) {
    pParams->auto_connect_enable = enableAutoReconnect;
    pParams->on_disconnect_handler = callback;
    pParams->ca_file = QCLOUD_IOT_CA_FILENAME;
    pParams->cert_file = QCLOUD_IOT_CERT_FILENAME;
    pParams->key_file = QCLOUD_IOT_KEY_FILENAME;
}

void ConnectMQTTParamsSetup_Detailed(MQTTConnectParams *params, char *pClientID, QoS qos, bool isCleanSession,
                                     bool isWillMsgPresent, char *pWillTopicName, char *pWillMessage,
                                     char *pUsername, char *pPassword) {
    params->keep_alive_interval = 10;
    params->clean_session = isCleanSession;
    params->MQTTVersion = MQTT_3_1_1;
    params->client_id = pClientID;
    params->username = pUsername;
    params->password = pPassword;
}

/**
 * 设置连接参数
 *
 * @param pParams
 * @param pClientId
 */
void ConnectParamsSetup(MQTTConnectParams *pParams, char *pClientId) {
    pParams->MQTTVersion = MQTT_3_1_1;
    pParams->client_id = pClientId;
    pParams->keep_alive_interval = QCLOUD_IOT_MQTT_KEEP_ALIVE_INTERNAL;
}


/**
 * 设置CONNACK
 *
 *
 * @param pParams ConnectParams
 * @param sessionPresent Session Present flag
 * @param connackResponseCode Connect Return code
 */
void setTLSRxBufferForConnack(MQTTConnectParams *pParams, unsigned char sessionPresent, unsigned char connackResponseCode) {
    RxBuffer.NoMsgFlag = false;

    if (pParams->clean_session) {
        sessionPresent = 0;
    }

    RxBuffer.pBuffer[0] = (unsigned char) (0x20); // type CONNACK
    RxBuffer.pBuffer[1] = (unsigned char) (0x02); // remain length
    RxBuffer.pBuffer[2] = sessionPresent;         // session present flag
    RxBuffer.pBuffer[3] = connackResponseCode;    // connect return code

    RxBuffer.len = CONNACK_PACKET_SIZE;
    RxIndex = 0;
}

void setTLSRxBufferForPuback(void) {
    size_t i;

    RxBuffer.NoMsgFlag = true;
    RxBuffer.len = PUBACK_PACKET_SIZE;
    RxIndex = 0;

    for (i = 0; i < RxBuffer.BufMaxSize; i++) {
        RxBuffer.pBuffer[i] = 0;
    }

    RxBuffer.pBuffer[0] = (unsigned char) (0x40);
    RxBuffer.pBuffer[1] = (unsigned char) (0x02);
    RxBuffer.pBuffer[2] = (unsigned char) (0x02);
    RxBuffer.pBuffer[3] = (unsigned char) (0x00);
    RxBuffer.NoMsgFlag = false;
}

void setTLSRxBufferForSuback(QoS qos) {

    RxBuffer.NoMsgFlag = false;
    RxBuffer.pBuffer[0] = (unsigned char) (0x90);
    RxBuffer.pBuffer[1] = (unsigned char) (0x2 + 1);
    // Variable header - packet identifier
    RxBuffer.pBuffer[2] = (unsigned char) (2);
    RxBuffer.pBuffer[3] = (unsigned char) (0);
    // payload
    RxBuffer.pBuffer[4] = (unsigned char) (qos);

    RxBuffer.len = SUBACK_PACKET_SIZE;
    RxIndex = 0;
}

void setTLSRxBufferForDoubleSuback(QoS qos) {

    RxBuffer.NoMsgFlag = false;
    RxBuffer.pBuffer[0] = (unsigned char) (0x90);
    RxBuffer.pBuffer[1] = (unsigned char) (0x2 + 1);
    // Variable header - packet identifier
    RxBuffer.pBuffer[2] = (unsigned char) (2);
    RxBuffer.pBuffer[3] = (unsigned char) (0);
    // payload
    RxBuffer.pBuffer[4] = (unsigned char) (qos);

    RxBuffer.pBuffer[5] = (unsigned char) (0x90);
    RxBuffer.pBuffer[6] = (unsigned char) (0x2 + 1);
    // Variable header - packet identifier
    RxBuffer.pBuffer[7] = (unsigned char) (2);
    RxBuffer.pBuffer[8] = (unsigned char) (0);
    // payload
    RxBuffer.pBuffer[9] = (unsigned char) (qos);

    RxBuffer.len = SUBACK_PACKET_SIZE * 2;
    RxIndex = 0;
}

void setTLSRxBufferForUnsuback() {
    RxBuffer.NoMsgFlag = false;
    RxBuffer.pBuffer[0] = (unsigned char) (0xB0);
    RxBuffer.pBuffer[1] = (unsigned char) (0x02);
    // Variable header - packet identifier
    RxBuffer.pBuffer[2] = (unsigned char) (2);
    RxBuffer.pBuffer[3] = (unsigned char) (0);
    // No payload
    RxBuffer.len = UNSUBACK_PACKET_SIZE;
    RxIndex = 0;
}

void setTLSRxBufferForPingresp() {
    RxBuffer.NoMsgFlag = false;
    RxBuffer.pBuffer[0] = (unsigned char) (0xD0);
    RxBuffer.pBuffer[1] = (unsigned char) (0x00);
    RxBuffer.len = PINGRESP_PACKET_SIZE;
    RxIndex = 0;
}

void setTLSRxBufferForConnackAndSuback(MQTTConnectParams *conParams, unsigned char sessionPresent, QoS qos) {

    RxBuffer.NoMsgFlag = false;

    if(conParams->clean_session) {
        sessionPresent = 0;
    }

    RxBuffer.pBuffer[0] = (unsigned char) (0x20);
    RxBuffer.pBuffer[1] = (unsigned char) (0x02);
    RxBuffer.pBuffer[2] = sessionPresent;
    RxBuffer.pBuffer[3] = (unsigned char) (0x0);

    RxBuffer.pBuffer[4] = (unsigned char) (0x90);
    RxBuffer.pBuffer[5] = (unsigned char) (0x2 + 1);
    // Variable header - packet identifier
    RxBuffer.pBuffer[6] = (unsigned char) (2);
    RxBuffer.pBuffer[7] = (unsigned char) (0);
    // payload
    RxBuffer.pBuffer[8] = (unsigned char) (qos);

    RxBuffer.len = CONNACK_SUBACK_PACKET_SIZE;
    RxIndex = 0;
}

void resetTlsBuffer() {
    size_t i;
    RxBuffer.len = 0;
    RxBuffer.NoMsgFlag = true;

    for (i = 0; i < RxBuffer.BufMaxSize; i++) {
        RxBuffer.pBuffer[i] = 0;
    }

    RxIndex = 0;
    RxBuffer.expiry_time.tv_sec = 0;
    RxBuffer.expiry_time.tv_usec = 0;

    TxBuffer.len = 0;
    for (i = 0; i < TxBuffer.BufMaxSize; i++) {
        TxBuffer.pBuffer[i] = 0;
    }

}

void setTLSRxBufferDelay(int seconds, int microseconds) {
    struct timeval now, duration, result;
    duration.tv_sec = seconds;
    duration.tv_usec = microseconds;

    gettimeofday(&now, NULL);
    timeradd(&now, &duration, &result);
    RxBuffer.expiry_time.tv_sec = result.tv_sec;
    RxBuffer.expiry_time.tv_usec = result.tv_usec;
}

unsigned char *connectTxBufferHeaderParser(ConnectBufferProofread *params, unsigned char *buf) {
    unsigned char *op = buf;
    // Get packet type
    unsigned char *ele1 = op;
    unsigned int multiplier = 1;
    int cnt = 0;
    unsigned char *ele2;
    unsigned int i;
    unsigned char *op2;
    params->PacketType = *ele1;
    op++;
    // Get remaining length (length bytes more than 4 bytes are ignored)
    params->RemainingLength = 0;
    do {
        ele2 = op;
        params->RemainingLength += ((unsigned int) (*ele2 & (0x7F)) * multiplier);  // & 127
        multiplier *= 128;
        cnt++;
        op++;
    } while ((*ele2 & (0x80)) != 0 && cnt < 4);
    // At this point, op should be updated to the start address of the next chunk of information
    // Get protocol length
    params->ProtocolLength = 0;
    params->ProtocolLength += (256 * (unsigned int) (*op++)); // Length MSB, 最高有效位（the Most Significant Bit）
    params->ProtocolLength += (unsigned int) (*op++);         // Length LSB, 最低有效位（Least Significant Bit)
    // Get protocol name
    for (i = 0; i < params->ProtocolLength; i++) {
        params->ProtocolName[i] = *op;
        op++;
    }
    // Get protocol level
    params->ProtocolLevel = (unsigned int) (*op++);
    // Get connect flags
    params->ConnectFlag = (*op++);
    // Get _mqtt_keep_alive
    op2 = op;
    params->KeepAlive = 0;
    params->KeepAlive += (256 * (unsigned int) (*op2++)); // get rid of the sign bit
    op++;
    params->KeepAlive += (unsigned int) (*op2++);
    op++;

    return op;
}

/**
 * 判断调用MQTT连接时,写缓存中的标志位是否正确
 *
 * @param settings 连接配置参数
 * @param readRes 固定头部+可变头部
 *
 * @return
 */
bool isConnectTxBufFlagCorrect(MQTTConnectParams *settings, ConnectBufferProofread *readRes) {
    bool ret = true;
    int i;
    unsigned char myByte[8]; // Construct our own connect flag byte according to the settings
    myByte[0] = (unsigned char) (settings->username == NULL ? 0 : 1); // User Name Flag
    myByte[1] = (unsigned char) (settings->password == NULL ? 0 : 1); // Password Flag
    myByte[2] = 0; // Will Retain
    // QoS
    if (settings->will.qos == QOS1) {
        myByte[3] = 0;
        myByte[4] = 1;
    } else if (settings->will.qos == QOS2) {
        myByte[3] = 1;
        myByte[4] = 0;
    } else {    // default QoS is QOS0
        myByte[3] = 0;
        myByte[4] = 0;
    }
    //
    myByte[5] = (unsigned char) settings->will_flag; // Will Flag
    myByte[6] = (unsigned char) settings->clean_session; // Clean Session
    myByte[7] = 0; // Retained
    //
    for (i = 0; i < 8; i++) {
        if (myByte[i] != (unsigned char) (((readRes->ConnectFlag) >> (7 - i)) & 0x01)) {
            printf("ex %x ac %x\n", (unsigned char) (((readRes->ConnectFlag) >> (7 - i)) & 0x01) + '0', myByte[i]);
            ret = false;
            break;
        }
    }
    return ret;
}


/**
 * 判断调用MQTT连接时,写缓存中的有效负荷是否正确
 *
 * @param settings  连接配置参数
 * @params payloadBuf 有效负荷写缓存中的数据
 *
 * @return
 */
bool isConnectTxBufPayloadCorrect(MQTTConnectParams *settings, unsigned char *payloadBuf) {
    bool ret = false;
    unsigned int i = 0;
    // Construct our own payload according to the settings to see if the real one matches with it
    unsigned int clientIdLen = (unsigned int) strlen(settings->client_id);
    unsigned int willTopicLen = 0;
    unsigned int willMsgLen = 0;
    unsigned int usernameLen = (unsigned int) (settings->username != NULL ? strlen(settings->username) : 0);
    unsigned int passwordLen = (unsigned int) (settings->password != NULL ? strlen(settings->password) : 0);


    if (settings->will_flag == true) {
        willTopicLen = (unsigned int) (settings->will.topicName != NULL ? strlen(settings->will.topicName) : 0);
        willMsgLen = (unsigned int) (settings->will.message != NULL ? strlen(settings->will.message) : 0);
    }

    unsigned int payloadLen = 2 + clientIdLen + 2 + willTopicLen + 2 + willMsgLen + 2 + usernameLen + 2 + passwordLen;

    unsigned char *payload = (unsigned char *) malloc(sizeof(char) * (payloadLen + 1));  // reserve 1 byte for '\0'
    unsigned char *op = payload;

    // 1. clientid + 两个字节长度
    *op = (unsigned char) (clientIdLen & 0x0FF00);  // MSB  最高有效位
    op++;
    *op = (unsigned char) (clientIdLen & 0x00FF);  // LSB 最低有效位
    op++;
    for (i = 0; i < clientIdLen; i++) {
        *op = (unsigned char) settings->client_id[i];
        op++;
    }

    // 2. willtopic + willMsg + 两个字节消息长度
    if (settings->will_flag == true) {

        *op = (unsigned char) (willMsgLen & 0x0FF00); // MSB
        op++;
        *op = (unsigned char) (willMsgLen & 0x00FF); // LSB
        op++;

        for (i = 0; i < willTopicLen; i++) {
            *op = (unsigned char) settings->will.topicName[i];
            op++;
        }

        *op = (unsigned char) (willMsgLen & 0x0FF00); // MSB
        op++;
        *op = (unsigned char) (willMsgLen & 0x00FF); // LSB
        op++;

        for (i = 0; i < willMsgLen; i++) {
            *op = (unsigned char) settings->will.message[i];
            op++;
        }
    }

    // 3. username
    if (settings->username != NULL) {
        *op = (unsigned char) (usernameLen & 0x0FF00);
        op++;
        *op = (unsigned char) (usernameLen & 0x00FF);
        op++;

        for (i = 0; i < usernameLen; i++) {
            *op = (unsigned char) settings->username[i];
            op++;
        }
    }
    // 4. password + 两个字节长度字段
    if (settings->password != NULL) {
        *op = (unsigned char) (passwordLen & 0x0FF00);
        op++;
        *op = (unsigned char) (passwordLen & 0x00FF);
        op++;

        for (i = 0; i < passwordLen; i++) {
            *op = (unsigned char) settings->password[i];
            op++;
        }
    }

    // 5. 结尾
    *op = '\0';

    ret = strcmp((const char *) payload, (const char *) payloadBuf) == 0 ? true : false;
    free(payload);

    return ret;
}

void encodeRemainingLength(unsigned char *buf, size_t *st, size_t length) {
    unsigned char c;
    // watch out for the type of length, could be over flow. Limits = 256MB
    // No boundary check for 256MB!!
    do {
        c = (unsigned char) (length % 128);
        length /= 128;
        if (length > 0) c |= (unsigned char) (0x80); // If there is still another byte following
        buf[(*st)++] = c;
    } while (length > 0);
    // At this point, *st should be the next position for a new part of data in the packet
}

void setTLSRxBufferWithMsgOnSubscribedTopic(char *topic, size_t topic_len, QoS qos, char *pMsg) {
    size_t VariableLen = topic_len + 2 + 2;
    size_t i = 0, cursor = 0, packetIdStartLoc = 0, payloadStartLoc = 0, VarHeaderStartLoc = 0;
    size_t PayloadLen = strlen(pMsg) + 1;

    RxBuffer.NoMsgFlag = false;
    RxBuffer.pBuffer[0] = (unsigned char) (0x30 | ((qos << 1) & 0xF));// QoS1
    cursor++; // Move the cursor

    // Remaining Length
    // Translate the Remaining Length into packet bytes
    encodeRemainingLength(RxBuffer.pBuffer, &cursor, VariableLen + PayloadLen);

    VarHeaderStartLoc = cursor - 1;
    // Variable header
    RxBuffer.pBuffer[VarHeaderStartLoc + 1] = (unsigned char) ((topic_len & 0xFF00) >> 8);
    RxBuffer.pBuffer[VarHeaderStartLoc + 2] = (unsigned char) (topic_len & 0xFF);
    for (i = 0; i < topic_len; i++) {
        RxBuffer.pBuffer[VarHeaderStartLoc + 3 + i] = (unsigned char) topic[i];
    }

    packetIdStartLoc = VarHeaderStartLoc + topic_len + 2;
    payloadStartLoc = (packetIdStartLoc + 1);

    if (QOS0 != qos) {
        // packet id only for QoS 1 or 2
        RxBuffer.pBuffer[packetIdStartLoc + 1] = 2;
        RxBuffer.pBuffer[packetIdStartLoc + 2] = 3;
        payloadStartLoc = packetIdStartLoc + 3;
    }

    // payload
    for (i = 0; i < PayloadLen; i++) {
        RxBuffer.pBuffer[payloadStartLoc + i] = (unsigned char) pMsg[i];
    }

    RxBuffer.len = VariableLen + PayloadLen + 2; // 2 for fixed header
    RxIndex = 0;
    //printBuffer(RxBuffer.pBuffer, RxBuffer.len);
}

unsigned char isLastTLSTxMessagePuback() {

    return (unsigned char) (TxBuffer.pBuffer[0] == 0x40 ? 1 : 0);
}

unsigned char isLastTLSTxMessagePingreq() {
    return (unsigned char) (TxBuffer.pBuffer[0] == 0xC0 ? 1 : 0);
}

unsigned char isLastTLSTxMessageDisconnect() {
    return (unsigned char) (TxBuffer.pBuffer[0] == 0xE0 ? 1 : 0);
}

void printPrfrdParams(ConnectBufferProofread *params) {
    unsigned int i;
    printf("\n----------------\n");
    printf("PacketType: %x\n", params->PacketType);
    printf("RemainingLength: %u\n", params->RemainingLength);
    printf("ProtocolLength: %u\n", params->ProtocolLength);
    printf("ProtocolName: ");
    for (i = 0; i < params->ProtocolLength; i++) {
        printf("%c", params->ProtocolName[i]);
    }
    printf("\n");
    printf("ProtocolLevel: %u\n", params->ProtocolLevel);
    printf("ConnectFlag: %x\n", params->ConnectFlag);
    printf("KeepAliveInterval: %u\n", params->KeepAlive);
    printf("----------------\n");
}


