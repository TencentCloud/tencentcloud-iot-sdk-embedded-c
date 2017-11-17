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
 *******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "mqtt_client.h"

/**
 * @brief get the free index in Message Handlers
 *
 * Return MAX_MESSAGE_HANDLERS value if no free index is available
 *
 * @param pClient
 * @return
 */
static uint32_t _get_free_index_in_handlers(Qcloud_IoT_Client *pClient) {
    uint32_t itr;
    for (itr = 0; itr < MAX_MESSAGE_HANDLERS; itr++) {
        if (pClient->message_handlers[itr].topicFilter == NULL) {
            break;
        }
    }

    return itr;
}

/**
  * Determines the length of the MQTT subscribe packet that would be produced using the supplied parameters
  * @param count the number of topic filter strings in topicFilters
  * @param topicFilters the array of topic filter strings to be used in the publish
  * @return the length of buffer needed to contain the serialized version of the packet
  */
static uint32_t _get_subscribe_packet_rem_len(uint32_t count, char **topicFilters) {
    size_t i;
    size_t len = 2; /* packetid */

    for (i = 0; i < count; ++i) {
        len += 2 + strlen(*topicFilters + i) + 1; /* length + topic + req_qos */
    }

    return (uint32_t) len;
}

/**
  * Serializes the supplied subscribe data into the supplied buffer, ready for sending
  * @param buf the buffer into which the packet will be serialized
  * @param buf_len the length in bytes of the supplied bufferr
  * @param dup integer - the MQTT dup flag
  * @param packet_id integer - the MQTT packet identifier
  * @param count - number of members in the topicFilters and reqQos arrays
  * @param topicFilters - array of topic filter names
  * @param requestedQoSs - array of requested QoS
  * @return the length of the serialized data.  <= 0 indicates error
  */
static int _serialize_subscribe_packet(unsigned char *buf, size_t buf_len, uint8_t dup, uint16_t packet_id, uint32_t count,
                                char **topicFilters, QoS *requestedQoSs, uint32_t *serialized_len) {
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(buf, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(serialized_len, QCLOUD_ERR_INVAL);

    unsigned char *ptr = buf;
    MQTTHeader header = {0};
    uint32_t rem_len = 0;
    uint32_t i = 0;
    int rc;

    // SUBSCRIBE报文的剩余长度 = 报文标识符(2 byte) + count * (长度字段(2 byte) + topicLen + qos(1 byte))
    rem_len = _get_subscribe_packet_rem_len(count, topicFilters);
    if (get_mqtt_packet_len(rem_len) > buf_len) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_BUF_TOO_SHORT);
    }
    // 初始化报文头部
    rc = mqtt_init_packet_header(&header, SUBSCRIBE, QOS1, dup, 0);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }
    // 写报文固定头部第一个字节
    mqtt_write_char(&ptr, header.byte);
    // 写报文固定头部剩余长度字段
    ptr += mqtt_write_packet_rem_len(ptr, rem_len);
    // 写可变头部: 报文标识符
    mqtt_write_uint_16(&ptr, packet_id);
    // 写报文的负载部分数据
    for (i = 0; i < count; ++i) {
        mqtt_write_utf8_string(&ptr, *topicFilters + i);
        mqtt_write_char(&ptr, (unsigned char) requestedQoSs[i]);
    }

    *serialized_len = (uint32_t) (ptr - buf);

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
static int _deserialize_suback_packet(uint16_t *packet_id, uint32_t max_count, uint32_t *count, QoS *grantedQoSs,
                               unsigned char *buf, size_t buf_len) {
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

int qcloud_iot_mqtt_subscribe(Qcloud_IoT_Client *pClient, char *topicFilter, SubscribeParams *pParams) {

    IOT_FUNC_ENTRY;
    int rc;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(pParams, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(pParams->on_message_handler, QCLOUD_ERR_INVAL);
    STRING_PTR_SANITY_CHECK(topicFilter, QCLOUD_ERR_INVAL);

    Timer timer;
    uint32_t len = 0;
    uint32_t freeIndex;
    uint32_t count = 0;
    QoS grantedQoS[3] = {QOS0, QOS0, QOS0};
    uint16_t packetId;
    
    size_t topicLen = strlen(topicFilter);
    if (topicLen > MAX_SIZE_OF_CLOUD_TOPIC) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MAX_TOPIC_LENGTH);
    }

    if (!pClient->is_connected) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_NO_CONN)
    }
    
    Log_d("topicName=%s|pUserdata=%s", topicFilter, (char *)pParams->pUserdata);

    InitTimer(&timer);
    countdown_ms(&timer, pClient->command_timeout_ms);

    // 序列化SUBSCRIBE报文
    rc = _serialize_subscribe_packet(pClient->buf, pClient->buf_size, 0, get_next_packet_id(pClient), 1, &topicFilter,
                                     &pParams->qos, &len);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    freeIndex = _get_free_index_in_handlers(pClient);
    if (MAX_MESSAGE_HANDLERS <= freeIndex) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_MAX_SUBSCRIPTIONS);
    }
    
    // 发送SUBSCRIBE报文
    rc = send_mqtt_packet(pClient, len, &timer);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    // 等待服务器中的SUBACK报文
    rc = wait_for_read(pClient, SUBACK, &timer);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    // 反序列化SUBACK报文
    rc = _deserialize_suback_packet(&packetId, 1, &count, grantedQoS, pClient->read_buf, pClient->read_buf_size);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    // 检查SUBACK报文中的返回码:0x00(QOS0, SUCCESS),0x01(QOS1, SUCCESS),0x02(QOS2, SUCCESS),0x80(Failure)
    if (grantedQoS[0] != pParams->qos) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_SUB);
    }

    pClient->message_handlers[freeIndex].topicFilter = topicFilter;
    pClient->message_handlers[freeIndex].messageHandler = pParams->on_message_handler;
    pClient->message_handlers[freeIndex].qos = pParams->qos;
    pClient->message_handlers[freeIndex].pMessageHandlerData = pParams->pUserdata;

    IOT_FUNC_EXIT_RC(rc);
}

int qcloud_iot_mqtt_resubscribe(Qcloud_IoT_Client *pClient) {
    IOT_FUNC_ENTRY;
    int rc;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);

    Timer timer;
    uint32_t len = 0;
    uint32_t count = 0;
    QoS grantedQoS[3] = {QOS0, QOS0, QOS0};
    uint16_t packetId;
    uint32_t itr = 0;
    char *topic = NULL;

    if (NULL == pClient) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_INVAL);
    }

    if (!pClient->is_connected) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_NO_CONN);
    }

    for (itr = 0; itr < MAX_MESSAGE_HANDLERS; itr++) {
        topic = (char *) pClient->message_handlers[itr].topicFilter;
        if (topic == NULL) {
            continue;
        }

        InitTimer(&timer);
        countdown_ms(&timer, pClient->command_timeout_ms);

        rc = _serialize_subscribe_packet(pClient->buf, pClient->buf_size, 0, get_next_packet_id(pClient), 1,
                                         &topic, &(pClient->message_handlers[itr].qos), &len);
        if (QCLOUD_ERR_SUCCESS != rc) {
            IOT_FUNC_EXIT_RC(rc);
        }

        rc = send_mqtt_packet(pClient, len, &timer);
        if (QCLOUD_ERR_SUCCESS != rc) {
            IOT_FUNC_EXIT_RC(rc);
        }

        rc = wait_for_read(pClient, SUBACK, &timer);
        if (QCLOUD_ERR_SUCCESS != rc) {
            IOT_FUNC_EXIT_RC(rc);
        }

        rc = _deserialize_suback_packet(&packetId, 1, &count, grantedQoS, pClient->read_buf, pClient->read_buf_size);
        if (QCLOUD_ERR_SUCCESS != rc) {
            IOT_FUNC_EXIT_RC(rc);
        }
    }

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

#ifdef __cplusplus
}
#endif
