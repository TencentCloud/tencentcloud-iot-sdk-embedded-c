/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Sergio R. Caprile - non-blocking packet read functions for stream transport
 *******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "mqtt_client.h"

#define MAX_NO_OF_REMAINING_LENGTH_BYTES 4

uint16_t get_next_packet_id(Qcloud_IoT_Client *c) {
    return c->next_packetId = (uint16_t) ((MAX_PACKET_ID == c->next_packetId) ? 1 : (c->next_packetId + 1));
}

/**
 * Encodes the message length according to the MQTT algorithm
 * @param buf the buffer into which the encoded data is written
 * @param length the length to be encoded
 * @return the number of bytes written to buffer
 */
size_t mqtt_write_packet_rem_len(unsigned char *buf, uint32_t length) {
    size_t outLen = 0;
    unsigned char encodeByte;

    IOT_FUNC_ENTRY;
    do {
        encodeByte = (unsigned char) (length % 128);
        length /= 128;
        /* if there are more digits to encode, set the top bit of this digit */
        if (length > 0) {
            encodeByte |= 0x80;
        }
        buf[outLen++] = encodeByte;
    } while (length > 0);

    IOT_FUNC_EXIT_RC(outLen);
}

size_t get_mqtt_packet_len(size_t rem_len) {
    rem_len += 1; /* header byte */

    /* now remaining_length field */
    if (rem_len < 128) {
        rem_len += 1;
    } else if (rem_len < 16384) {
        rem_len += 2;
    } else if (rem_len < 2097151) {
        rem_len += 3;
    } else {
        rem_len += 4;
    }

    return rem_len;
}

/**
 * Decodes the message length according to the MQTT algorithm
 * @param getcharfn pointer to function to read the next character from the data source
 * @param value the decoded length returned
 * @return the number of bytes read from the socket
 */
static int _decode_packet_rem_len_from_buf_read(uint32_t (*getcharfn)(unsigned char *, uint32_t), uint32_t *value,
                                                uint32_t *readBytesLen) {
    unsigned char c;
    uint32_t multiplier = 1;
    uint32_t len = 0;
    uint32_t getLen = 0;

    IOT_FUNC_ENTRY;
    *value = 0;
    do {
        if (++len > MAX_NO_OF_REMAINING_LENGTH_BYTES) {
            /* bad data */
            IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_PACKET_READ);
        }
        getLen = (*getcharfn)(&c, 1);
        if (1 != getLen) {
            IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
        }
        *value += (c & 127) * multiplier;
        multiplier *= 128;
    } while ((c & 128) != 0);

    *readBytesLen = len;

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

static unsigned char *bufptr;
uint32_t bufchar(unsigned char *c, uint32_t count) {
    uint32_t i;

    for (i = 0; i < count; ++i) {
        *c = *bufptr++;
    }

    return count;
}

int mqtt_read_packet_rem_len_form_buf(unsigned char *buf, uint32_t *value, uint32_t *readBytesLen) {
    bufptr = buf;
    return _decode_packet_rem_len_from_buf_read(bufchar, value, readBytesLen);
}

/**
 * Calculates uint16 packet id from two bytes read from the input buffer
 * @param pptr pointer to the input buffer - incremented by the number of bytes used & returned
 * @return the value calculated
 */
uint16_t mqtt_read_uint16_t(unsigned char **pptr) {
    unsigned char *ptr = *pptr;
    uint8_t firstByte = (uint8_t) (*ptr);
    uint8_t secondByte = (uint8_t) (*(ptr + 1));
    uint16_t len = (uint16_t) (secondByte + (256 * firstByte));
    *pptr += 2;
    return len;
}

/**
 * Reads one character from the input buffer.
 * @param pptr pointer to the input buffer - incremented by the number of bytes used & returned
 * @return the character read
 */
unsigned char mqtt_read_char(unsigned char **pptr) {
    unsigned char c = **pptr;
    (*pptr)++;
    return c;
}

/**
 * Writes one character to an output buffer.
 * @param pptr pointer to the output buffer - incremented by the number of bytes used & returned
 * @param c the character to write
 */
void mqtt_write_char(unsigned char **pptr, unsigned char c) {
    **pptr = c;
    (*pptr)++;
}

/**
 * Writes an integer as 2 bytes to an output buffer.
 * @param pptr pointer to the output buffer - incremented by the number of bytes used & returned
 * @param anInt the integer to write
 */
void mqtt_write_uint_16(unsigned char **pptr, uint16_t anInt) {
    **pptr = (unsigned char) (anInt / 256);
    (*pptr)++;
    **pptr = (unsigned char) (anInt % 256);
    (*pptr)++;
}

/**
 * Writes a "UTF" string to an output buffer.  Converts C string to length-delimited.
 * @param pptr pointer to the output buffer - incremented by the number of bytes used & returned
 * @param string the C string to write
 */
void mqtt_write_utf8_string(unsigned char **pptr, const char *string) {
    size_t len = strlen(string);
    mqtt_write_uint_16(pptr, (uint16_t) len);
    memcpy(*pptr, string, len);
    *pptr += len;
}

/**
 * Initialize the MQTTHeader structure. Used to ensure that Header bits are
 * always initialized using the proper mappings. No Endianness issues here since
 * the individual fields are all less than a byte. Also generates no warnings since
 * all fields are initialized using hex constants
 */
int mqtt_init_packet_header(MQTTHeader *header, MessageTypes message_type,
                            QoS qos, uint8_t dup, uint8_t retained)
{
    POINTER_SANITY_CHECK(header, QCLOUD_ERR_INVAL);

    /* Set all bits to zero */
    header->byte = 0;
    switch (message_type) {
        case RESERVED:
            /* Should never happen */
            return QCLOUD_ERR_MQTT_UNKNOWN;
        case CONNECT:
            header->bits.type = 0x01;
            break;
        case CONNACK:
            header->bits.type = 0x02;
            break;
        case PUBLISH:
            header->bits.type = 0x03;
            break;
        case PUBACK:
            header->bits.type = 0x04;
            break;
        case PUBREC:
            header->bits.type = 0x05;
            break;
        case PUBREL:
            header->bits.type = 0x06;
            break;
        case PUBCOMP:
            header->bits.type = 0x07;
            break;
        case SUBSCRIBE:
            header->bits.type = 0x08;
            break;
        case SUBACK:
            header->bits.type = 0x09;
            break;
        case UNSUBSCRIBE:
            header->bits.type = 0x0A;
            break;
        case UNSUBACK:
            header->bits.type = 0x0B;
            break;
        case PINGREQ:
            header->bits.type = 0x0C;
            break;
        case PINGRESP:
            header->bits.type = 0x0D;
            break;
        case DISCONNECT:
            header->bits.type = 0x0E;
            break;
        default:
            /* Should never happen */
            return QCLOUD_ERR_MQTT_UNKNOWN;
    }

    header->bits.dup = (1 == dup) ? 0x01 : 0x00;
    switch (qos) {
        case QOS0:
            header->bits.qos = 0x00;
            break;
        case QOS1:
            header->bits.qos = 0x01;
            break;
        case QOS2:
            header->bits.qos = 0x02;
            break;
        default:
            /* Using QOS0 as default */
            header->bits.qos = 0x00;
            break;
    }

    header->bits.retain = (1 == retained) ? 0x01 : 0x00;

    return QCLOUD_ERR_SUCCESS;
}

/**
  * Deserializes the supplied (wire) buffer into an ack
  * @param packet_type returned integer - the MQTT packet type
  * @param dup returned integer - the MQTT dup flag
  * @param packet_id returned integer - the MQTT packet identifier
  * @param buf the raw buffer data, of the correct length determined by the remaining length field
  * @param buf_len the length in bytes of the data in the supplied buffer
  * @return error code.  1 is success, 0 is failure
  */
int deserialize_ack_packet(uint8_t *packet_type, uint8_t *dup, uint16_t *packet_id, unsigned char *buf, size_t buf_len) {
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(packet_type, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(dup, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(packet_id, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(buf, QCLOUD_ERR_INVAL);

    int rc;
    MQTTHeader header = {0};
    unsigned char *curdata = buf;
    unsigned char *enddata = NULL;
    uint32_t decodedLen = 0, readBytesLen = 0;

    /* PUBACK fixed header size is two bytes, variable header is 2 bytes, MQTT v3.1.1 Specification 3.4.1 */
    if (4 > buf_len) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_BUF_TOO_SHORT);
    }

    header.byte = mqtt_read_char(&curdata);
    *dup = header.bits.dup;
    *packet_type = header.bits.type;

    /* read remaining length */
    rc = mqtt_read_packet_rem_len_form_buf(curdata, &decodedLen, &readBytesLen);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }
    curdata += (readBytesLen);
    enddata = curdata + decodedLen;

    if (enddata - curdata < 2) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    *packet_id = mqtt_read_uint16_t(&curdata);
    
    // 返回错误码处理
    if (enddata - curdata >= 1) {
        unsigned char ack_code = mqtt_read_char(&curdata);
        if (ack_code != 0) {
            Log_e("deserialize_ack_packet failure! ack_code = %02x", ack_code);
            IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
        }
    }

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

/**
  * Serializes a 0-length packet into the supplied buffer, ready for writing to a socket
  * @param buf the buffer into which the packet will be serialized
  * @param buf_len the length in bytes of the supplied buffer, to avoid overruns
  * @param packettype the message type
  * @param serialized length
  * @return int indicating function execution status
  */
int serialize_packet_with_zero_payload(unsigned char *buf, size_t buf_len, MessageTypes packetType, uint32_t *serialized_len) {
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(buf, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(serialized_len, QCLOUD_ERR_INVAL);

    MQTTHeader header = {0};
    unsigned char *ptr = buf;
    int rc;

    /* Buffer should have at least 2 bytes for the header */
    if (4 > buf_len) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_BUF_TOO_SHORT);
    }

    rc = mqtt_init_packet_header(&header, packetType, QOS0, 0, 0);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    /* write header */
    mqtt_write_char(&ptr, header.byte);

    /* write remaining length */
    ptr += mqtt_write_packet_rem_len(ptr, 0);
    *serialized_len = (uint32_t) (ptr - buf);

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

int send_mqtt_packet(Qcloud_IoT_Client *pClient, size_t length, Timer *timer) {
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(timer, QCLOUD_ERR_INVAL);

    int rc = QCLOUD_ERR_SUCCESS;
    size_t sentLen = 0, sent = 0;

    if (length >= pClient->buf_size) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_BUF_TOO_SHORT);
    }

    while (sent < length && !expired(timer)) {
        rc = pClient->network_stack.write(&(pClient->network_stack), &pClient->buf[sent], length, left_ms(timer), &sentLen);
        if (rc != QCLOUD_ERR_SUCCESS) {
            /* there was an error writing the data */
            break;
        }
        sent = sent + sentLen;
    }

    if (sent == length) {
        /* record the fact that we have successfully sent the packet */
        //countdown(&c->ping_timer, c->keep_alive_interval);
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
    }

    IOT_FUNC_EXIT_RC(rc);
}

/**
 * @brief 解析报文的剩余长度字段
 *
 * 每从网络中读取一个字节, 按照MQTT协议算法计算剩余长度
 *
 * @param pClient Client结构体
 * @param value   剩余长度
 * @param timeout 超时时间
 * @return
 */
static int _decode_packet_rem_len_with_net_read(Qcloud_IoT_Client *pClient, uint32_t *value, uint32_t timeout) {
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(value, QCLOUD_ERR_INVAL);

    unsigned char i;
    uint32_t multiplier = 1;
    uint32_t len = 0;
    size_t read_len = 0;

    *value = 0;

    do {
        if (++len > MAX_NO_OF_REMAINING_LENGTH_BYTES) {
            /* bad data */
            IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_PACKET_READ)
        }

        if ((pClient->network_stack.read(&(pClient->network_stack), &i, 1, (int) timeout, &read_len)) !=
            QCLOUD_ERR_SUCCESS) {
            /* The value argument is the important value. len is just used temporarily
             * and never used by the calling function for anything else */
            IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
        }

        *value += ((i & 127) * multiplier);
        multiplier *= 128;
    } while ((i & 128) != 0);

    /* The value argument is the important value. len is just used temporarily
     * and never used by the calling function for anything else */
    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

/**
 * @brief 从底层SSL/TCP层读取报文数据
 *
 * 1. 读取第一个字节确定报文的类型;
 * 2. 读取剩余长度字段, 最大为四个字节; 剩余长度表示可变包头和有效负载的长度
 * 3. 根据剩余长度, 读取剩下的数据, 包括可变包头和有效负荷
 *
 * @param pClient        Client结构体
 * @param timer          定时器
 * @param packet_type    报文类型
 * @return
 */
static int _read_mqtt_packet(Qcloud_IoT_Client *pClient, Timer *timer, uint8_t *packet_type) {
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(timer, QCLOUD_ERR_INVAL);

    MQTTHeader header = {0};
    uint32_t len = 0;
    uint32_t rem_len = 0;
    size_t total_bytes_read = 0;
    size_t bytes_to_be_read = 0;
    size_t read_len = 0;
    int32_t ret_val = 0;
    int rc;

    // 1. 读取报文固定头部的第一个字节
    rc = pClient->network_stack.read(&(pClient->network_stack), pClient->read_buf, 1, left_ms(timer), &read_len);
    if (rc == QCLOUD_ERR_SSL_NOTHING_TO_READ) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_NOTHING_TO_READ);
    } else if (rc != QCLOUD_ERR_SUCCESS) {
        IOT_FUNC_EXIT_RC(rc);
    }

    len = 1;

    // 2. 读取报文固定头部剩余长度部分
    rc = _decode_packet_rem_len_with_net_read(pClient, &rem_len, (uint32_t) left_ms(timer));
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    // 如果读缓冲区的大小小于报文的剩余长度, 报文会被丢弃
    if (rem_len >= pClient->read_buf_size) {
        bytes_to_be_read = pClient->read_buf_size;
        do {
            ret_val = pClient->network_stack.read(&(pClient->network_stack), pClient->read_buf, bytes_to_be_read, left_ms(timer),
                                               &read_len);
            if (ret_val == QCLOUD_ERR_SUCCESS) {
                total_bytes_read += read_len;
                if ((rem_len - total_bytes_read) >= pClient->read_buf_size) {
                    bytes_to_be_read = pClient->read_buf_size;
                } else {
                    bytes_to_be_read = rem_len - total_bytes_read;
                }
            }
        } while (total_bytes_read < rem_len && ret_val == QCLOUD_ERR_SUCCESS);
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_BUF_TOO_SHORT);
    }

    // 将剩余长度写入读缓冲区
    len += mqtt_write_packet_rem_len(pClient->read_buf + 1, rem_len);

    // 3. 读取报文的剩余部分数据
    if (rem_len > 0 &&
        (pClient->network_stack.read(&(pClient->network_stack), pClient->read_buf + len, rem_len, left_ms(timer), &read_len) !=
         QCLOUD_ERR_SUCCESS)) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    header.byte = pClient->read_buf[0];
    *packet_type = header.bits.type;

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

/**
 * @brief 消息主题是否相同
 *
 * @param topicFilter
 * @param topicName
 * @return
 */
static uint8_t _is_topic_equals(char *topicFilter, char *topicName) {
    return (uint8_t) (strlen(topicFilter) == strlen(topicName) && strcmp(topicFilter, topicName));
}

/**
 * @brief 消息主题匹配
 *
 * assume topic filter and name is in correct format
 * # can only be at end
 * + and # can only be next to separator
 *
 * @param topicFilter   订阅消息的主题名
 * @param topicName     收到消息的主题名, 不能包含通配符
 * @param topicNameLen  主题名的长度
 * @return
 */
static uint8_t _is_topic_matched(char *topicFilter, char *topicName, uint16_t topicNameLen) {
    char *curf = NULL;
    char *curn = NULL;
    char *curn_end = NULL;

    curf = topicFilter;
    curn = topicName;
    curn_end = curn + topicNameLen;

    while (*curf && (curn < curn_end)) {

        if (*curf == '+' && *curn == '/') {
            curf++;
            continue;
        }

        if (*curn == '/' && *curf != '/') {
            break;
        }

        if (*curf != '+' && *curf != '#' && *curf != *curn) {
            break;
        }

        if (*curf == '+') {
            /* skip until we meet the next separator, or end of string */
            char *nextpos = curn + 1;
            while (nextpos < curn_end && *nextpos != '/')
                nextpos = ++curn + 1;
        } else if (*curf == '#') {
            /* skip until end of string */
            curn = curn_end - 1;
        }

        curf++;
        curn++;
    };

    if (*curf == '\0') {
        return (uint8_t) (curn == curn_end);
    } else {
        return (uint8_t) ((*curf == '#') || *(curf + 1) == '#' || (*curf == '+' && *(curn - 1) == '/'));
    }
}

/**
 * @brief 终端收到服务器的的PUBLISH消息之后, 传递消息给消息回调处理函数
 *
 * @param pClient
 * @param topicName
 * @param message
 * @return
 */
static int _deliver_message(Qcloud_IoT_Client *pClient, char *topicName, uint16_t topicNameLen, MQTTMessage *message) {
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(topicName, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(message, QCLOUD_ERR_INVAL);

    uint32_t i;
    // we have to find the right message handler - indexed by topic
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
        if ((pClient->message_handlers[i].topicFilter != NULL)
            && (_is_topic_equals(topicName, (char *) pClient->message_handlers[i].topicFilter) ||
                _is_topic_matched((char *) pClient->message_handlers[i].topicFilter, topicName, topicNameLen))) {
            if (pClient->message_handlers[i].messageHandler != NULL) {
                pClient->message_handlers[i].messageHandler(topicName, topicNameLen, message, pClient->message_handlers[i].pMessageHandlerData);
                IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
            }
        }
    }

    /* Message handler not found for topic */
    /* May be we do not care  change FAILURE  use SUCCESS*/
    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

/**
 * @brief 终端收到服务器的的PUBLISH消息之后, 处理收到的PUBLISH报文
 *
 * @param pClient
 * @param timer
 * @return
 */
static int _handle_publish_packet(Qcloud_IoT_Client *pClient, Timer *timer) {
    IOT_FUNC_ENTRY;
    char *topicName;
    uint16_t topicNameLen;
    MQTTMessage msg;
    int rc;
    uint32_t len = 0;

    rc = deserialize_publish_packet(&msg.dup, &msg.qos, &msg.retained, &msg.id, &topicName, &topicNameLen, (unsigned char **) &msg.payload,
                                    &msg.payload_len, pClient->read_buf, pClient->read_buf_size);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }
    
    // 传过来的topicName没有截断，会把payload也带过来
    char fix_topic[MAX_SIZE_OF_CLOUD_TOPIC] = {0};
    memcpy(fix_topic, topicName, topicNameLen);
    rc = _deliver_message(pClient, fix_topic, topicNameLen, &msg);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    if (QOS0 == msg.qos) {
        /* No further processing required for QOS0 */
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
    }

    if (QOS1 == msg.qos) {
        rc = serialize_pub_ack_packet(pClient->buf, pClient->buf_size, PUBACK, 0, msg.id, &len);
    } else { /* Message is not QOS0 or 1 means only option left is QOS2 */
        rc = serialize_pub_ack_packet(pClient->buf, pClient->buf_size, PUBREC, 0, msg.id, &len);
    }

    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    rc = send_mqtt_packet(pClient, len, timer);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

/**
 * @brief 处理PUBREC报文, 并发送PUBREL报文, PUBLISH报文为QOS2时
 *
 * @param pClient
 * @param timer
 * @return
 */
static int _handle_pubrec_packet(Qcloud_IoT_Client *pClient, Timer *timer) {
    IOT_FUNC_ENTRY;
    uint16_t packet_id;
    unsigned char dup, type;
    int rc;
    uint32_t len;

    rc = deserialize_ack_packet(&type, &dup, &packet_id, pClient->read_buf, pClient->read_buf_size);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    rc = serialize_pub_ack_packet(pClient->buf, pClient->buf_size, PUBREL, 0, packet_id, &len);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    /* send the PUBREL packet */
    rc = send_mqtt_packet(pClient, len, timer);
    if (QCLOUD_ERR_SUCCESS != rc) {
        /* there was a problem */
        IOT_FUNC_EXIT_RC(rc);
    }

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

/**
 * @brief 处理服务器的心跳包回包
 *
 * @param pClient
 */
static void _handle_pingresp_packet(Qcloud_IoT_Client *pClient) {
    IOT_FUNC_ENTRY;
    pClient->is_ping_outstanding = 0;
    countdown(&pClient->ping_timer, pClient->options.keep_alive_interval);
    IOT_FUNC_EXIT;
}

int cycle_for_read(Qcloud_IoT_Client *pClient, Timer *timer, uint8_t *packet_type) {
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(timer, QCLOUD_ERR_INVAL);

    int rc;
    /* read the socket, see what work is due */
    rc = _read_mqtt_packet(pClient, timer, packet_type);
    if (QCLOUD_ERR_MQTT_NOTHING_TO_READ == rc) {
        /* Nothing to read, not a cycle failure */
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
    }

    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    switch (*packet_type) {
        case CONNACK:
        case PUBACK:
        case SUBACK:
        case UNSUBACK:
            break;
        case PUBLISH: {
            rc = _handle_publish_packet(pClient, timer);
            break;
        }
        case PUBREC: {
            rc = _handle_pubrec_packet(pClient, timer);
            break;
        }
        case PUBCOMP:
            break;
        case PINGRESP: {
            _handle_pingresp_packet(pClient);
            break;
        }
        default: {
            /* Either unknown packet type or Failure occurred
             * Should not happen */
            IOT_FUNC_EXIT_RC(QCLOUD_ERR_RX_MESSAGE_INVAL);
            break;
        }
    }

    IOT_FUNC_EXIT_RC(rc);
}

int wait_for_read(Qcloud_IoT_Client *pClient, uint8_t packet_type, Timer *timer) {
    IOT_FUNC_ENTRY;
    int rc;
    uint8_t read_packet_type = 0;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(timer, QCLOUD_ERR_INVAL);

    do {
        if (expired(timer)) {
            /* we timed out */
            rc = QCLOUD_ERR_MQTT_REQUEST_TIMEOUT;
            break;
        }
        rc = cycle_for_read(pClient, timer, &read_packet_type);
    } while (QCLOUD_ERR_SSL_READ_TIMEOUT != rc && QCLOUD_ERR_SSL_READ != rc && read_packet_type != packet_type);

    if (QCLOUD_ERR_MQTT_REQUEST_TIMEOUT != rc
        && QCLOUD_ERR_SSL_READ_TIMEOUT != rc && QCLOUD_ERR_SSL_READ != rc && read_packet_type != packet_type) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    /* Something failed or we didn't receive the expected packet, return error code */
    IOT_FUNC_EXIT_RC(rc);
}


#ifdef __cplusplus
}
#endif
