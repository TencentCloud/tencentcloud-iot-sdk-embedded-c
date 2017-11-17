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
  * Determines the length of the MQTT unsubscribe packet that would be produced using the supplied parameters
  * @param count the number of topic filter strings in topicFilters
  * @param topicFilters the array of topic filter strings to be used in the publish
  * @return the length of buffer needed to contain the serialized version of the packet
  */
static uint32_t _get_unsubscribe_packet_rem_len(uint32_t count, char **topicFilters) {
    size_t i;
    size_t len = 2; /* packetid */

    for (i = 0; i < count; ++i) {
        len += 2 + strlen(*topicFilters + i); /* length + topic*/
    }

    return (uint32_t) len;
}

/**
  * Serializes the supplied unsubscribe data into the supplied buffer, ready for sending
  * @param buf the raw buffer data, of the correct length determined by the remaining length field
  * @param buf_len the length in bytes of the data in the supplied buffer
  * @param dup integer - the MQTT dup flag
  * @param packet_id integer - the MQTT packet identifier
  * @param count - number of members in the topicFilters array
  * @param topicFilters - array of topic filter names
  * @param serialized_len - the length of the serialized data
  * @return int indicating function execution status
  */
static int _serialize_unsubscribe_packet(unsigned char *buf, size_t buf_len,
                                         uint8_t dup, uint16_t packet_id,
                                         uint32_t count, char **topicFilters,
                                         uint32_t *serialized_len) {
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(buf, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(serialized_len, QCLOUD_ERR_INVAL);

    unsigned char *ptr = buf;
    MQTTHeader header = {0};
    uint32_t rem_len = 0;
    uint32_t i = 0;
    int rc;

    rem_len = _get_unsubscribe_packet_rem_len(count, topicFilters);
    if (get_mqtt_packet_len(rem_len) > buf_len) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_BUF_TOO_SHORT);
    }

    rc = mqtt_init_packet_header(&header, UNSUBSCRIBE, QOS1, dup, 0);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }
    mqtt_write_char(&ptr, header.byte); /* write header */

    ptr += mqtt_write_packet_rem_len(ptr, rem_len); /* write remaining length */

    mqtt_write_uint_16(&ptr, packet_id);

    for (i = 0; i < count; ++i) {
        mqtt_write_utf8_string(&ptr, *topicFilters + i);
    }

    *serialized_len = (uint32_t) (ptr - buf);

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}


/**
  * Deserializes the supplied (wire) buffer into unsuback data
  * @param packet_id returned integer - the MQTT packet identifier
  * @param buf the raw buffer data, of the correct length determined by the remaining length field
  * @param buf_len the length in bytes of the data in the supplied buffer
  * @return int indicating function execution status
  */
static int _deserialize_unsuback_packet(uint16_t *packet_id, unsigned char *buf, size_t buf_len) {
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

int qcloud_iot_mqtt_unsubscribe(Qcloud_IoT_Client *pClient, char *topicFilter) {
    IOT_FUNC_ENTRY;
    int rc;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
    STRING_PTR_SANITY_CHECK(topicFilter, QCLOUD_ERR_INVAL);

    int i = 0;
    Timer timer;
    uint32_t len = 0;
    uint16_t packet_id;
    bool subscriptionExists = false;
    
    size_t topicLen = strlen(topicFilter);
    if (topicLen > MAX_SIZE_OF_CLOUD_TOPIC) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MAX_TOPIC_LENGTH);
    }
    
    Log_d("topicName=%s", topicFilter);

    /* Remove from message handler array */
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
        if (pClient->message_handlers[i].topicFilter != NULL &&
            (strcmp(pClient->message_handlers[i].topicFilter, topicFilter) == 0)) {
            pClient->message_handlers[i].topicFilter = NULL;
            /* We don't want to break here, if the same topic is registered
             * with 2 callbacks. Unlikely scenario */
            subscriptionExists = true;
        }
    }

    if (subscriptionExists == false) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }


    if (!pClient->is_connected) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_NO_CONN);
    }

    InitTimer(&timer);
    countdown_ms(&timer, pClient->command_timeout_ms);

    rc = _serialize_unsubscribe_packet(pClient->buf, pClient->buf_size, 0, get_next_packet_id(pClient), 1, &topicFilter,
                                       &len);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    /* send the unsubscribe packet */
    rc = send_mqtt_packet(pClient, len, &timer);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    rc = wait_for_read(pClient, UNSUBACK, &timer);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    rc = _deserialize_unsuback_packet(&packet_id, pClient->read_buf, pClient->read_buf_size);

    IOT_FUNC_EXIT_RC(rc);
}

#ifdef __cplusplus
}
#endif
