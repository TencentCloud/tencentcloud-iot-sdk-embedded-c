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
 *    Ian Craggs - fix for https://bugs.eclipse.org/bugs/show_bug.cgi?id=453144
 *******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "mqtt_client.h"

/**
 * @param mqttstring the MQTTString structure into which the data is to be read
 * @param pptr pointer to the output buffer - incremented by the number of bytes used & returned
 * @param enddata pointer to the end of the data: do not read beyond
 * @return SUCCESS if successful, FAILURE if not
 */
static int _read_string_with_len(char **string, uint16_t *stringLen, unsigned char **pptr, unsigned char *enddata) {
    int rc = QCLOUD_ERR_FAILURE;

    /* the first two bytes are the length of the string */
    /* enough length to read the integer? */
    if (enddata - (*pptr) > 1) {
        *stringLen = mqtt_read_uint16_t(pptr); /* increments pptr to point past length */
        if (&(*pptr)[*stringLen] <= enddata) {
            *string = (char *) *pptr;
            *pptr += *stringLen;
            rc = QCLOUD_ERR_SUCCESS;
        }
    }

    return rc;
}

/**
  * Determines the length of the MQTT publish packet that would be produced using the supplied parameters
  * @param qos the MQTT QoS of the publish (packetid is omitted for QoS 0)
  * @param topicName the topic name to be used in the publish  
  * @param payload_len the length of the payload to be sent
  * @return the length of buffer needed to contain the serialized version of the packet
  */
static uint32_t _get_publish_packet_len(uint8_t qos, char *topicName, size_t payload_len) {
    size_t len = 0;

    len += 2 + strlen(topicName) + payload_len;
    if (qos > 0) {
        len += 2; /* packetid */
    }
    return (uint32_t) len;
}

/**
  * Deserializes the supplied (wire) buffer into publish data
  * @param dup returned integer - the MQTT dup flag
  * @param qos returned integer - the MQTT QoS value
  * @param retained returned integer - the MQTT retained flag
  * @param packet_id returned integer - the MQTT packet identifier
  * @param topicName returned MQTTString - the MQTT topic in the publish
  * @param payload returned byte buffer - the MQTT publish payload
  * @param payload_len returned integer - the length of the MQTT payload
  * @param buf the raw buffer data, of the correct length determined by the remaining length field
  * @param buf_len the length in bytes of the data in the supplied buffer
  * @return error code.  1 is success
  */
int deserialize_publish_packet(uint8_t *dup, QoS *qos, uint8_t *retained, uint16_t *packet_id, char **topicName,
                               uint16_t *topicNameLen,unsigned char **payload, size_t *payload_len, unsigned char *buf, size_t buf_len) {
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(dup, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(qos, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(retained, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(packet_id, QCLOUD_ERR_INVAL);

    MQTTHeader header = {0};
    unsigned char *curdata = buf;
    unsigned char *enddata = NULL;
    int rc;
    uint32_t decodedLen = 0;
    uint32_t readBytesLen = 0;

    /* Publish header size is at least four bytes.
     * Fixed header is two bytes.
     * Variable header size depends on QoS And Topic Name.
     * QoS level 0 doesn't have a message identifier (0 - 2 bytes)
     * Topic Name length fields decide size of topic name field (at least 2 bytes)
     * MQTT v3.1.1 Specification 3.3.1 */
    if (4 > buf_len) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_BUF_TOO_SHORT);
    }

    header.byte = mqtt_read_char(&curdata);
    if (PUBLISH != header.bits.type) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    *dup = header.bits.dup;
    *qos = (QoS) header.bits.qos;
    *retained = header.bits.retain;

    /* read remaining length */
    rc = mqtt_read_packet_rem_len_form_buf(curdata, &decodedLen, &readBytesLen);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }
    curdata += (readBytesLen);
    enddata = curdata + decodedLen;

    /* do we have enough data to read the protocol version byte? */
    if (QCLOUD_ERR_SUCCESS != _read_string_with_len(topicName, topicNameLen, &curdata, enddata) || (0 > (enddata - curdata))) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    if (QOS0 != *qos) {
        *packet_id = mqtt_read_uint16_t(&curdata);
    }

    *payload_len = (size_t) (enddata - curdata);
    *payload = curdata;

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

/**
  * Serializes the ack packet into the supplied buffer.
  * @param buf the buffer into which the packet will be serialized
  * @param buf_len the length in bytes of the supplied buffer
  * @param packet_type the MQTT packet type: 1.PUBACK; 2.PUBREL; 3.PUBCOMP
  * @param dup the MQTT dup flag
  * @param packet_id the MQTT packet identifier
  * @return serialized length, or error if 0
  */
int serialize_pub_ack_packet(unsigned char *buf, size_t buf_len, MessageTypes packet_type, uint8_t dup,
                             uint16_t packet_id,
                             uint32_t *serialized_len) {
    IOT_FUNC_ENTRY;
    POINTER_SANITY_CHECK(buf, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(serialized_len, QCLOUD_ERR_INVAL);

    MQTTHeader header = {0};
    unsigned char *ptr = buf;
    QoS requestQoS = (PUBREL == packet_type) ? QOS1 : QOS0;  // 详见 MQTT协议说明 3.6.1小结
    int rc = mqtt_init_packet_header(&header, packet_type, requestQoS, dup, 0);

    /* Minimum byte length required by ACK headers is
     * 2 for fixed and 2 for variable part */
    if (4 > buf_len) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_BUF_TOO_SHORT);
    }

    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }
    mqtt_write_char(&ptr, header.byte); /* write header */

    ptr += mqtt_write_packet_rem_len(ptr, 2); /* write remaining length */
    mqtt_write_uint_16(&ptr, packet_id);
    *serialized_len = (uint32_t) (ptr - buf);

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}


/**
  * Serializes the supplied publish data into the supplied buffer, ready for sending
  * @param buf the buffer into which the packet will be serialized
  * @param buf_len the length in bytes of the supplied buffer
  * @param dup integer - the MQTT dup flag
  * @param qos integer - the MQTT QoS value
  * @param retained integer - the MQTT retained flag
  * @param packet_id integer - the MQTT packet identifier
  * @param topicName MQTTString - the MQTT topic in the publish
  * @param payload byte buffer - the MQTT publish payload
  * @param payload_len integer - the length of the MQTT payload
  * @return the length of the serialized data.  <= 0 indicates error
  */
static int _serialize_publish_packet(unsigned char *buf, size_t buf_len, uint8_t dup, QoS qos, uint8_t retained,
                                     uint16_t packet_id,
                                     char *topicName, unsigned char *payload, size_t payload_len,
                                     uint32_t *serialized_len) {
    IOT_FUNC_ENTRY;
    POINTER_SANITY_CHECK(buf, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(serialized_len, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(payload, QCLOUD_ERR_INVAL);

    unsigned char *ptr = buf;
    MQTTHeader header = {0};
    uint32_t rem_len = 0;
    int rc;

    rem_len = _get_publish_packet_len(qos, topicName, payload_len);
    if (get_mqtt_packet_len(rem_len) > buf_len) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_BUF_TOO_SHORT);
    }

    rc = mqtt_init_packet_header(&header, PUBLISH, qos, dup, retained);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    mqtt_write_char(&ptr, header.byte); /* write header */

    ptr += mqtt_write_packet_rem_len(ptr, rem_len); /* write remaining length */;

    mqtt_write_utf8_string(&ptr, topicName);   /* Variable Header: Topic Name */

    if (qos > 0) {
        mqtt_write_uint_16(&ptr, packet_id);  /* Variable Header: Topic Name */
    }

    memcpy(ptr, payload, payload_len);
    ptr += payload_len;

    *serialized_len = (uint32_t) (ptr - buf);

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

int qcloud_iot_mqtt_publish(Qcloud_IoT_Client *pClient, char *topicName, PublishParams *pParams) {
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(pParams, QCLOUD_ERR_INVAL);
    STRING_PTR_SANITY_CHECK(topicName, QCLOUD_ERR_INVAL);

    Timer timer;
    uint32_t len = 0;
    uint8_t waitForAck = 0;
    uint8_t packetType = PUBACK;
    uint16_t packet_id;
    uint8_t dup, type;
    int rc;
    
    size_t topicLen = strlen(topicName);
    if (topicLen > MAX_SIZE_OF_CLOUD_TOPIC) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MAX_TOPIC_LENGTH);
    }

    if (!pClient->is_connected) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_NO_CONN);
    }

    InitTimer(&timer);
    countdown_ms(&timer, pClient->command_timeout_ms);

    if (pParams->qos == QOS1 || pParams->qos == QOS2) {
        pParams->id = get_next_packet_id(pClient);
        if (IOT_Log_Get_Level() <= DEBUG) {
        	Log_d("publish topic seq=%d|topicName=%s|payload=%s", pParams->id, topicName, (char *)pParams->payload);
        }
        else {
        	Log_i("publish topic seq=%d|topicName=%s", pParams->id, topicName);
        }
        waitForAck = 1;
        if (pParams->qos == QOS2) {
            packetType = PUBCOMP;
        }
    }
    else {
    	if (IOT_Log_Get_Level() <= DEBUG) {
    		Log_d("publish topic topicName=%s|payload=%s", topicName, (char *)pParams->payload);
		}
		else {
			Log_i("publish topic topicName=%s", topicName);
		}
    }

    rc = _serialize_publish_packet(pClient->buf, pClient->buf_size, 0, pParams->qos, pParams->retained, pParams->id,
                                   topicName, (unsigned char *) pParams->payload, pParams->payload_len, &len);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    rc = send_mqtt_packet(pClient, len, &timer);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    if (waitForAck == 1) {
        rc = wait_for_read(pClient, packetType, &timer);
        if (QCLOUD_ERR_SUCCESS != rc) {
            IOT_FUNC_EXIT_RC(rc);
        }

        rc = deserialize_ack_packet(&type, &dup, &packet_id, pClient->read_buf, pClient->read_buf_size);
        if (QCLOUD_ERR_SUCCESS != rc) {
            IOT_FUNC_EXIT_RC(rc);
        }
    }

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

#ifdef __cplusplus
}
#endif
