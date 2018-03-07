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
#include <time.h>

#include "mqtt_client.h"

#include "qcloud_iot_utils_list.h"


#define MAX_NO_OF_REMAINING_LENGTH_BYTES 4

/* return: 0, identical; NOT 0, different. */
static int _check_handle_is_identical(SubTopicHandle *sub_handle1, SubTopicHandle *sub_handle2)
{
    if (!sub_handle1 || !sub_handle2) {
        return 1;
    }

    int topic_name_Len = strlen(sub_handle1->topic_filter);

    if (topic_name_Len != strlen(sub_handle2->topic_filter)) {
        return 1;
    }

    if (0 != strncmp(sub_handle1->topic_filter, sub_handle2->topic_filter, topic_name_Len)) {
        return 1;
    }

    if (sub_handle1->message_handler != sub_handle2->message_handler) {
        return 1;
    }

    return 0;
}

uint16_t get_next_packet_id(Qcloud_IoT_Client *pClient) {
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);

    HAL_MutexLock(pClient->lock_generic);
    pClient->next_packet_id = (uint16_t) ((MAX_PACKET_ID == pClient->next_packet_id) ? 1 : (pClient->next_packet_id + 1));
    HAL_MutexUnlock(pClient->lock_generic);

    IOT_FUNC_EXIT_RC(pClient->next_packet_id);
}

void get_next_conn_id(MQTTConnectParams *options) {
	int i;
	srand((unsigned)time(0));
	for (i = 0; i < MAX_CONN_ID_LEN - 1; i++) {
		int flag = rand() % 3;
		switch(flag) {
			case 0:
				options->conn_id[i] = (rand() % 26) + 'a';
				break;
			case 1:
				options->conn_id[i] = (rand() % 26) + 'A';
				break;
			case 2:
				options->conn_id[i] = (rand() % 10) + '0';
				break;
		}
	}

	options->conn_id[MAX_CONN_ID_LEN - 1] = '\0';
}

/**
 * Encodes the message length according to the MQTT algorithm
 * @param buf the buffer into which the encoded data is written
 * @param length the length to be encoded
 * @return the number of bytes written to buffer
 */
size_t mqtt_write_packet_rem_len(unsigned char *buf, uint32_t length) {
    IOT_FUNC_ENTRY;

    size_t outLen = 0;

    do {
        unsigned char encodeByte;
        encodeByte = (unsigned char) (length % 128);
        length /= 128;
        /* if there are more digits to encode, set the top bit of this digit */
        if (length > 0) {
            encodeByte |= 0x80;
        }
        buf[outLen++] = encodeByte;
    } while (length > 0);

    IOT_FUNC_EXIT_RC((int)outLen);
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
    IOT_FUNC_ENTRY;

    unsigned char c;
    uint32_t multiplier = 1;
    uint32_t len = 0;
    *value = 0;
    do {
        if (++len > MAX_NO_OF_REMAINING_LENGTH_BYTES) {
            /* bad data */
            IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_PACKET_READ);
        }
        uint32_t getLen = 0;
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
            Log_e("deserialize_ack_packet failure! ack_code = 0x%02x", ack_code);
            IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
        }
    }

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

/**
  * Deserializes the supplied (wire) buffer into suback data
  * @param packet_id returned integer - the MQTT packet identifier
  * @param max_count - the maximum number of members allowed in the grantedQoSs array
  * @param count returned integer - number of members in the grantedQoSs array
  * @param grantedQoSs returned array of integers - the granted qualities of service
  * @param buf the raw buffer data, of the correct length determined by the remaining length field
  * @param buf_len the length in bytes of the data in the supplied buffer
  * @return error code.  1 is success, 0 is failure
  */
int deserialize_suback_packet(uint16_t *packet_id, uint32_t max_count, uint32_t *count,
                                     QoS *grantedQoSs, unsigned char *buf, size_t buf_len) 
{
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(packet_id, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(count, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(grantedQoSs, QCLOUD_ERR_INVAL);

    MQTTHeader header = {0};
    unsigned char *curdata = buf;
    unsigned char *enddata = NULL;
    int decodeRc;
    uint32_t decodedLen = 0;
    uint32_t readBytesLen = 0;

    // SUBACK头部大小为4字节, 负载部分至少为1字节QOS返回码
    if (5 > buf_len) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_BUF_TOO_SHORT);
    }
    // 读取报文固定头部的第一个字节
    header.byte = mqtt_read_char(&curdata);
    if (header.bits.type != SUBACK) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    // 读取报文固定头部的剩余长度
    decodeRc = mqtt_read_packet_rem_len_form_buf(curdata, &decodedLen, &readBytesLen);
    if (decodeRc != QCLOUD_ERR_SUCCESS) {
        IOT_FUNC_EXIT_RC(decodeRc);
    }

    curdata += (readBytesLen);
    enddata = curdata + decodedLen;
    if (enddata - curdata < 2) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    // 读取报文可变头部的报文标识符
    *packet_id = mqtt_read_uint16_t(&curdata);

    // 读取报文的负载部分
    *count = 0;
    while (curdata < enddata) {
        if (*count > max_count) {
            IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
        }
        grantedQoSs[(*count)++] = (QoS) mqtt_read_char(&curdata);
    }

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

/**
  * Deserializes the supplied (wire) buffer into unsuback data
  * @param packet_id returned integer - the MQTT packet identifier
  * @param buf the raw buffer data, of the correct length determined by the remaining length field
  * @param buf_len the length in bytes of the data in the supplied buffer
  * @return int indicating function execution status
  */
int deserialize_unsuback_packet(uint16_t *packet_id, unsigned char *buf, size_t buf_len) 
{
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(buf, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(packet_id, QCLOUD_ERR_INVAL);

    unsigned char type = 0;
    unsigned char dup = 0;
    int rc;

    rc = deserialize_ack_packet(&type, &dup, packet_id, buf, buf_len);
    if (QCLOUD_ERR_SUCCESS == rc && UNSUBACK != type) {
        rc = QCLOUD_ERR_FAILURE;
    }

    IOT_FUNC_EXIT_RC(rc);
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

    if (length >= pClient->write_buf_size) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_BUF_TOO_SHORT);
    }

    while (sent < length && !expired(timer)) {
        rc = pClient->network_stack.write(&(pClient->network_stack), &pClient->write_buf[sent], length, left_ms(timer), &sentLen);
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
    size_t read_len = 0;
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
        size_t total_bytes_read = 0;
        size_t bytes_to_be_read;
        int32_t ret_val = 0;

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
    if (rem_len > 0 && ((len + rem_len) > pClient->read_buf_size)) {
    	pClient->network_stack.read(&(pClient->network_stack), pClient->read_buf, rem_len, left_ms(timer), &read_len);
    	IOT_FUNC_EXIT_RC(QCLOUD_ERR_BUF_TOO_SHORT);
    }
    else {
        if (rem_len > 0 &&
            (pClient->network_stack.read(&(pClient->network_stack), pClient->read_buf + len, rem_len, left_ms(timer), &read_len) !=
             QCLOUD_ERR_SUCCESS)) {
            IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
        }
    }

    header.byte = pClient->read_buf[0];
    *packet_type = header.bits.type;

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

/**
 * @brief 消息主题是否相同
 *
 * @param topic_filter
 * @param topicName
 * @return
 */
static uint8_t _is_topic_equals(char *topic_filter, char *topicName) {
    return (uint8_t) (strlen(topic_filter) == strlen(topicName) && !strcmp(topic_filter, topicName));
}

/**
 * @brief 消息主题匹配
 *
 * assume topic filter and name is in correct format
 * # can only be at end
 * + and # can only be next to separator
 *
 * @param topic_filter   订阅消息的主题名
 * @param topicName     收到消息的主题名, 不能包含通配符
 * @param topicNameLen  主题名的长度
 * @return
 */
static uint8_t _is_topic_matched(char *topic_filter, char *topicName, uint16_t topicNameLen) {
    char *curf;
    char *curn;
    char *curn_end;

    curf = topic_filter;
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

    message->ptopic = topicName;
    message->topic_len = (size_t)topicNameLen;

    uint32_t i;
    int flag_matched = 0;
    
    HAL_MutexLock(pClient->lock_generic);
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
        if ((pClient->sub_handles[i].topic_filter != NULL)
            && (_is_topic_equals(topicName, (char *) pClient->sub_handles[i].topic_filter) ||
                _is_topic_matched((char *) pClient->sub_handles[i].topic_filter, topicName, topicNameLen)))
        {
            HAL_MutexUnlock(pClient->lock_generic);
            if (pClient->sub_handles[i].message_handler != NULL) {
                pClient->sub_handles[i].message_handler(pClient, message, pClient->sub_handles[i].message_handler_data);
                flag_matched = 1;
                IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
            }
            HAL_MutexLock(pClient->lock_generic);
        }
    }

    /* Message handler not found for topic */
    /* May be we do not care  change FAILURE  use SUCCESS*/
    HAL_MutexUnlock(pClient->lock_generic);

    if (0 == flag_matched) {
        Log_d("no matching any topic, call default handle function");

        if (NULL != pClient->event_handle.h_fp) {
            MQTTEventMsg msg;
            msg.event_type = MQTT_EVENT_PUBLISH_RECVEIVED;
            msg.msg = message;
            pClient->event_handle.h_fp(pClient, pClient->event_handle.context, &msg);
        }
    }

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

/**
 * @brief 从等待 publish ACK 的列表中，移除由 msdId 标记的元素
 *
 * @param c
 * @param msgId
 *
 * @return 0, success; NOT 0, fail;
 */
static int _mask_pubInfo_from(Qcloud_IoT_Client *c, uint16_t msgId)
{
    IOT_FUNC_ENTRY;

    if (!c) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    HAL_MutexLock(c->lock_list_pub);
    if (c->list_pub_wait_ack->len) {
        ListIterator *iter;
        ListNode *node = NULL;
        QcloudIotPubInfo *repubInfo = NULL;

        if (NULL == (iter = list_iterator_new(c->list_pub_wait_ack, LIST_TAIL))) {
            HAL_MutexUnlock(c->lock_list_pub);
            return QCLOUD_ERR_SUCCESS;
        }

        for (;;) {
            node = list_iterator_next(iter);

            if (NULL == node) {
                break;
            }

            repubInfo = (QcloudIotPubInfo *) node->val;
            if (NULL == repubInfo) {
                Log_e("node's value is invalid!");
                continue;
            }

            if (repubInfo->msg_id == msgId) {
                repubInfo->node_state = MQTT_NODE_STATE_INVALID; /* 标记为无效节点 */
            }
        }

        list_iterator_destroy(iter);
    }
    HAL_MutexUnlock(c->lock_list_pub);

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

/* 从等待 subscribe(unsubscribe) ACK 的列表中，移除由 msdId 标记的元素 */
/* 同时返回消息处理数据 messageHandler */
/* return: 0, success; NOT 0, fail; */
static int _mask_sub_info_from(Qcloud_IoT_Client *c, unsigned int msgId, SubTopicHandle *messageHandler)
{
    IOT_FUNC_ENTRY;

    if (NULL == c || NULL == messageHandler) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    HAL_MutexLock(c->lock_list_sub);
    if (c->list_sub_wait_ack->len) {
        ListIterator *iter;
        ListNode *node = NULL;
        QcloudIotSubInfo *sub_info = NULL;

        if (NULL == (iter = list_iterator_new(c->list_sub_wait_ack, LIST_TAIL))) {
            HAL_MutexUnlock(c->lock_list_sub);
            IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
        }

        for (;;) {
            node = list_iterator_next(iter);
            if (NULL == node) {
                break;
            }

            sub_info = (QcloudIotSubInfo *) node->val;
            if (NULL == sub_info) {
                Log_e("node's value is invalid!");
                continue;
            }

            if (sub_info->msg_id == msgId) {
                *messageHandler = sub_info->handler; /* return handle */
                sub_info->node_state = MQTT_NODE_STATE_INVALID; /* mark as invalid node */
            } 
        }

        list_iterator_destroy(iter);
    }
    HAL_MutexUnlock(c->lock_list_sub);

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

/**
 * @brief 终端收到服务器的的PUBACK消息之后, 处理收到的PUBACK报文
 */
static int _handle_puback_packet(Qcloud_IoT_Client *pClient, Timer *timer)
{
    IOT_FUNC_ENTRY;
    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(timer, QCLOUD_ERR_INVAL);

    uint16_t packet_id;
    uint8_t dup, type;
    int rc;

    rc = deserialize_ack_packet(&type, &dup, &packet_id, pClient->read_buf, pClient->read_buf_size);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    (void)_mask_pubInfo_from(pClient, packet_id);

    /* 调用回调函数，通知外部PUBLISH成功. */
    if (NULL != pClient->event_handle.h_fp) {
        MQTTEventMsg msg;
        msg.event_type = MQTT_EVENT_PUBLISH_SUCCESS;
        msg.msg = (void *)(uintptr_t)packet_id;
        pClient->event_handle.h_fp(pClient, pClient->event_handle.context, &msg);
    }

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

/**
 * @brief 终端收到服务器的的 SUBACK 消息之后, 处理收到的 SUBACK 报文
 */
static int _handle_suback_packet(Qcloud_IoT_Client *pClient, Timer *timer, QoS qos)
{
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(timer, QCLOUD_ERR_INVAL);

    uint32_t count = 0;
    uint16_t packet_id = 0;
    QoS grantedQoS[3] = {QOS0, QOS0, QOS0};
    int rc;

    // 反序列化SUBACK报文
    rc = deserialize_suback_packet(&packet_id, 1, &count, grantedQoS, pClient->read_buf, pClient->read_buf_size);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    int flag_dup = 0, i_free = -1;
    // 检查SUBACK报文中的返回码:0x00(QOS0, SUCCESS),0x01(QOS1, SUCCESS),0x02(QOS2, SUCCESS),0x80(Failure)
    if (grantedQoS[0] == 0x80) {
        MQTTEventMsg msg;

        Log_e("MQTT SUBSCRIBE failed, ack code is 0x80");

        msg.event_type = MQTT_EVENT_SUBCRIBE_NACK;
        msg.msg = (void *)(uintptr_t)packet_id;
        pClient->event_handle.h_fp(pClient, pClient->event_handle.context, &msg);

        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_SUB);
    }

    HAL_MutexLock(pClient->lock_generic);
    
    SubTopicHandle sub_handle;
    memset(&sub_handle, 0, sizeof(SubTopicHandle));
    (void)_mask_sub_info_from(pClient, (unsigned int)packet_id, &sub_handle);

    if (/*(NULL == sub_handle.message_handler) || */(NULL == sub_handle.topic_filter)) {
        Log_e("sub_handle is illegal, handle is null:%d, topic is null:%d", (NULL == sub_handle.message_handler), (NULL == sub_handle.topic_filter));
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_SUB);
    }

    int i;
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
        if ((NULL != pClient->sub_handles[i].topic_filter)) {
            if (0 == _check_handle_is_identical(&pClient->sub_handles[i], &sub_handle)) {
                
                flag_dup = 1;
                Log_e("There is a identical topic and related handle in list!");
                break;
            }
        } else {
            if (-1 == i_free) {
                i_free = i; /* record available element */
            }
        }
    }

    if (0 == flag_dup) {
        if (-1 == i_free) {
            Log_e("NOT more @sub_handles space!");
            HAL_MutexUnlock(pClient->lock_generic);
            IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
        } else {
            pClient->sub_handles[i_free].topic_filter = sub_handle.topic_filter;
            pClient->sub_handles[i_free].message_handler = sub_handle.message_handler;
            pClient->sub_handles[i_free].qos = sub_handle.qos;
            pClient->sub_handles[i_free].message_handler_data = sub_handle.message_handler_data;
        }
    }
    
    HAL_MutexUnlock(pClient->lock_generic);

    /* 调用回调函数，通知外部 SUBSCRIBE 成功. */
    if (NULL != pClient->event_handle.h_fp) {
        MQTTEventMsg msg;
        msg.event_type = MQTT_EVENT_SUBCRIBE_SUCCESS;
        msg.msg = (void *)(uintptr_t)packet_id;
        if (pClient->event_handle.h_fp != NULL)
            pClient->event_handle.h_fp(pClient, pClient->event_handle.context, &msg);
    }

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

/**
 * @brief 终端收到服务器的的 USUBACK 消息之后, 处理收到的 USUBACK 报文
 */
static int _handle_unsuback_packet(Qcloud_IoT_Client *pClient, Timer *timer)
{
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(timer, QCLOUD_ERR_INVAL);

    uint16_t packet_id = 0;

    int rc =  deserialize_unsuback_packet(&packet_id, pClient->read_buf, pClient->read_buf_size);
    if (rc != QCLOUD_ERR_SUCCESS) {
        IOT_FUNC_EXIT_RC(rc);
    }

    SubTopicHandle messageHandler;
    (void)_mask_sub_info_from(pClient, packet_id, &messageHandler);

    /* Remove from message handler array */
    HAL_MutexLock(pClient->lock_generic);
    int i;
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
        if ((pClient->sub_handles[i].topic_filter != NULL)
            && (0 == _check_handle_is_identical(&pClient->sub_handles[i], &messageHandler))) {
            memset(&pClient->sub_handles[i], 0, sizeof(SubTopicHandle));

            /* NOTE: in case of more than one register(subscribe) with different callback function,
             *       so we must keep continuously searching related message handle. */
        }
    }

    if (NULL != pClient->event_handle.h_fp) {
        MQTTEventMsg msg;
        msg.event_type = MQTT_EVENT_UNSUBCRIBE_SUCCESS;
        msg.msg = (void *)(uintptr_t)packet_id;

        pClient->event_handle.h_fp(pClient, pClient->event_handle.context, &msg);
    }

    HAL_MutexUnlock(pClient->lock_generic);

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

#ifdef MQTT_RMDUP_MSG_ENABLED

#define MQTT_MAX_REPEAT_BUF_LEN 50
static uint16_t sg_repeat_packet_id_buf[MQTT_MAX_REPEAT_BUF_LEN];

/**
 * @brief 判断packet_id缓存中是否已经存有传入的packet_id;
 */
static int _get_packet_id_in_repeat_buf(uint16_t packet_id)
{
    int i;
    for (i = 0; i < MQTT_MAX_REPEAT_BUF_LEN; ++i)
    {
        if (packet_id == sg_repeat_packet_id_buf[i])
        {
            return packet_id;
        }
    }
    return -1;
}

static void _add_packet_id_to_repeat_buf(uint16_t packet_id)
{
    static unsigned int current_packet_id_cnt = 0;
    if (_get_packet_id_in_repeat_buf(packet_id) < 0)
        return;

    sg_repeat_packet_id_buf[current_packet_id_cnt++] = packet_id;

    if (current_packet_id_cnt >= MQTT_MAX_REPEAT_BUF_LEN)
        current_packet_id_cnt = current_packet_id_cnt % 50;
}

void reset_repeat_packet_id_buffer(void)
{
    int i;
    for (i = 0; i < MQTT_MAX_REPEAT_BUF_LEN; ++i)
    {
        sg_repeat_packet_id_buf[i] = 0;
    }
}

#endif

/**
 * @brief 终端收到服务器的的PUBLISH消息之后, 处理收到的PUBLISH报文
 */
static int _handle_publish_packet(Qcloud_IoT_Client *pClient, Timer *timer) {
    IOT_FUNC_ENTRY;
    char *topic_name;
    uint16_t topic_len;
    MQTTMessage msg;
    int rc;
    uint32_t len = 0;

    rc = deserialize_publish_packet(&msg.dup, &msg.qos, &msg.retained, &msg.id, &topic_name, &topic_len, (unsigned char **) &msg.payload,
                                    &msg.payload_len, pClient->read_buf, pClient->read_buf_size);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }
    
    // 传过来的topicName没有截断，会把payload也带过来
    char fix_topic[MAX_SIZE_OF_CLOUD_TOPIC] = {0};
    memcpy(fix_topic, topic_name, topic_len);

    if (QOS0 == msg.qos)
    {
        rc = _deliver_message(pClient, fix_topic, topic_len, &msg);
        if (QCLOUD_ERR_SUCCESS != rc)
            IOT_FUNC_EXIT_RC(rc);

        /* No further processing required for QOS0 */
        IOT_FUNC_EXIT_RC(rc);

    } else {
#ifdef MQTT_RMDUP_MSG_ENABLED
        // 判断packet_id之前是否已经收到过
        int repeat_id = _get_packet_id_in_repeat_buf(msg.id);

        // 执行订阅消息的回调函数
        if (repeat_id < 0)
        {
#endif
            rc = _deliver_message(pClient, fix_topic, topic_len, &msg);
            if (QCLOUD_ERR_SUCCESS != rc)
                IOT_FUNC_EXIT_RC(rc);
#ifdef MQTT_RMDUP_MSG_ENABLED
        }
        _add_packet_id_to_repeat_buf(msg.id);
#endif
    }
    

    if (QOS1 == msg.qos) {
        rc = serialize_pub_ack_packet(pClient->write_buf, pClient->write_buf_size, PUBACK, 0, msg.id, &len);
    } else { /* Message is not QOS0 or 1 means only option left is QOS2 */
        rc = serialize_pub_ack_packet(pClient->write_buf, pClient->write_buf_size, PUBREC, 0, msg.id, &len);
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

    rc = serialize_pub_ack_packet(pClient->write_buf, pClient->write_buf_size, PUBREL, 0, packet_id, &len);
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

    HAL_MutexLock(pClient->lock_generic);
    pClient->is_ping_outstanding = 0;
    countdown(&pClient->ping_timer, pClient->options.keep_alive_interval);
    HAL_MutexUnlock(pClient->lock_generic);

    IOT_FUNC_EXIT;
}

int cycle_for_read(Qcloud_IoT_Client *pClient, Timer *timer, uint8_t *packet_type, QoS qos) {
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
            break;
        case PUBACK:
            rc = _handle_puback_packet(pClient, timer);
            break;
        case SUBACK:
            rc = _handle_suback_packet(pClient, timer, qos);
            break;
        case UNSUBACK:
            rc = _handle_unsuback_packet(pClient, timer);
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

int wait_for_read(Qcloud_IoT_Client *pClient, uint8_t packet_type, Timer *timer, QoS qos) {
	IOT_FUNC_ENTRY;
	int rc;
	uint8_t read_packet_type = 0;

	POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
	POINTER_SANITY_CHECK(timer, QCLOUD_ERR_INVAL);

	do {
		if (expired(timer)) {
			rc = QCLOUD_ERR_MQTT_REQUEST_TIMEOUT;
			break;
		}
		rc = cycle_for_read(pClient, timer, &read_packet_type, qos);
	} while (QCLOUD_ERR_SSL_READ_TIMEOUT != rc && QCLOUD_ERR_SSL_READ != rc && read_packet_type != packet_type);

	if (QCLOUD_ERR_MQTT_REQUEST_TIMEOUT != rc
		&& QCLOUD_ERR_SSL_READ_TIMEOUT != rc && QCLOUD_ERR_SSL_READ != rc && read_packet_type != packet_type) {
		IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
	}

	/* Something failed or we didn't receive the expected packet, return error code */
	IOT_FUNC_EXIT_RC(rc);
}

void set_client_conn_state(Qcloud_IoT_Client *pClient, uint8_t connected) {
    HAL_MutexLock(pClient->lock_generic);
    pClient->is_connected = connected;
    HAL_MutexUnlock(pClient->lock_generic);
}

uint8_t get_client_conn_state(Qcloud_IoT_Client *pClient) {
    IOT_FUNC_ENTRY;
	uint8_t is_connected = 0;
	HAL_MutexLock(pClient->lock_generic);
	is_connected = pClient->is_connected;
	HAL_MutexUnlock(pClient->lock_generic);
    IOT_FUNC_EXIT_RC(is_connected);
}

/*
 * @brief 向 subscribe(unsubscribe) ACK 等待列表中添加元素
 *
 *
 * return: 0, success; NOT 0, fail;
 */
int push_sub_info_to(Qcloud_IoT_Client *c, int len, unsigned short msgId, MessageTypes type,
                                   SubTopicHandle *handler, ListNode **node)
{
    IOT_FUNC_ENTRY;
    if (!c || !handler || !node) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_INVAL);
    }


    HAL_MutexLock(c->lock_list_sub);

    if (c->list_sub_wait_ack->len >= MAX_MESSAGE_HANDLERS) {
        HAL_MutexUnlock(c->lock_list_sub);
        Log_e("number of sub_info more than max!,size = %d", c->list_sub_wait_ack->len);
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_MAX_SUBSCRIPTIONS);
    }

    QcloudIotSubInfo *sub_info = (QcloudIotSubInfo *)HAL_Malloc(sizeof(
            QcloudIotSubInfo) + len);
    if (NULL == sub_info) {
        HAL_MutexUnlock(c->lock_list_sub);
        Log_e("run memory malloc is error!");
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    sub_info->node_state = MQTT_NODE_STATE_NORMANL;
    sub_info->msg_id = msgId;
    sub_info->len = len;

    InitTimer(&sub_info->sub_start_time);
    countdown_ms(&sub_info->sub_start_time, c->command_timeout_ms);

    sub_info->type = type;
    sub_info->handler = *handler;
    sub_info->buf = (unsigned char *)sub_info + sizeof(QcloudIotSubInfo);

    memcpy(sub_info->buf, c->write_buf, len);

    *node = list_node_new(sub_info);
    if (NULL == *node) {
        HAL_MutexUnlock(c->lock_list_sub);
        Log_e("run list_node_new is error!");
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    list_rpush(c->list_sub_wait_ack, *node);

    HAL_MutexUnlock(c->lock_list_sub);

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

#ifdef __cplusplus
}
#endif
