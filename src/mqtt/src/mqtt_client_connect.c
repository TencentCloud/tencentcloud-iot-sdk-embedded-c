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
 *    Allan Stockdill-Mander/Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "mqtt_client.h"

typedef union {
    uint8_t all;    /**< all connect flags */
#if defined(REVERSED)
    struct
    {
        unsigned int username : 1;			/**< 3.1 user name */
        unsigned int password : 1; 			/**< 3.1 password */
        unsigned int willRetain : 1;		/**< will retain setting */
        unsigned int willQoS : 2;				/**< will QoS value */
        unsigned int will : 1;			    /**< will flag */
        unsigned int cleansession : 1;	  /**< clean session flag */
        unsigned int : 1;	  	          /**< unused */
    } bits;
#else
    struct {
        unsigned int : 1;                            /**< unused */
        unsigned int cleansession : 1;      /**< cleansession flag */
        unsigned int will : 1;                /**< will flag */
        unsigned int willQoS : 2;                /**< will QoS value */
        unsigned int willRetain : 1;        /**< will retain setting */
        unsigned int password : 1;            /**< 3.1 password */
        unsigned int username : 1;            /**< 3.1 user name */
    } bits;
#endif
} MQTTConnectFlags;    /**< connect flags byte */

typedef union {
    uint8_t all;    /**< all connack flags */
#if defined(REVERSED)
    struct
    {
        unsigned int sessionpresent : 1;    /**< session present flag */
        unsigned int : 7;	  	          /**< unused */
    } bits;
#else
    struct {
        unsigned int : 7;                    /**< unused */
        unsigned int sessionpresent : 1;    /**< session present flag */
    } bits;
#endif
} MQTTConnackFlags;    /**< connack flags byte */

/**
 * Connect return code
 */
typedef enum {
    CONNACK_CONNECTION_ACCEPTED = 0,    // connection accepted
    CONANCK_UNACCEPTABLE_PROTOCOL_VERSION_ERROR = 1,    // connection refused: unaccpeted protocol verison
    CONNACK_IDENTIFIER_REJECTED_ERROR = 2,    // connection refused: identifier rejected
    CONNACK_SERVER_UNAVAILABLE_ERROR = 3,    // connection refused: server unavailable
    CONNACK_BAD_USERDATA_ERROR = 4,    // connection refused: bad user name or password
    CONNACK_NOT_AUTHORIZED_ERROR = 5     // connection refused: not authorized
} MQTTConnackReturnCodes;

/**
  * Determines the length of the MQTT connect packet that would be produced using the supplied connect options.
  * @param options the options to be used to build the connect packet
  * @param the length of buffer needed to contain the serialized version of the packet
  * @return int indicating function execution status
  */
static uint32_t _get_packet_connect_rem_len(MQTTConnectParams *options) {
    size_t len = 0;
    /* variable depending on MQTT or MQIsdp */
    if (3 == options->MQTTVersion) {
        len = 12;
    } else if (4 == options->MQTTVersion) {
        len = 10;
    }

    len += strlen(options->client_id) + 2;

    if (options->username) {
        len += strlen(options->username) + 2;
    }

    if (options->password) {
        len += strlen(options->password) + 2;
    }

    return (uint32_t) len;
}

static void _copy_connect_params(MQTTConnectParams *destination, MQTTConnectParams *source) {

	POINTER_SANITY_CHECK_RTN(destination);
	POINTER_SANITY_CHECK_RTN(source);

    destination->MQTTVersion = source->MQTTVersion;
    destination->client_id = source->client_id;
    destination->username = source->username;
    destination->password = source->password;
    destination->keep_alive_interval = source->keep_alive_interval;
    destination->clean_session = source->clean_session;
    destination->auto_connect_enable = source->auto_connect_enable;
    destination->on_disconnect_handler = source->on_disconnect_handler;
}

/**
  * Serializes the connect options into the buffer.
  * @param buf the buffer into which the packet will be serialized
  * @param len the length in bytes of the supplied buffer
  * @param options the options to be used to build the connect packet
  * @param serialized length
  * @return int indicating function execution status
  */
static int _serialize_connect_packet(unsigned char *buf, size_t buf_len, MQTTConnectParams *options, uint32_t *serialized_len) {
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(buf, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(options, QCLOUD_ERR_INVAL);
    STRING_PTR_SANITY_CHECK(options->client_id, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(serialized_len, QCLOUD_ERR_INVAL);

    unsigned char *ptr = buf;
    MQTTHeader header = {0};
    MQTTConnectFlags flags = {0};
    uint32_t rem_len = 0;
    int rc;

    // TODO: appid逻辑暂时先注释等后台稳定再打开
    int username_len = strlen(options->client_id) + strlen(QCLOUD_IOT_DEVICE_SDK_APPID) + 2;
    options->username = (char*)HAL_Malloc(username_len);
	sprintf(options->username, "%s;%s", options->client_id, QCLOUD_IOT_DEVICE_SDK_APPID);

    rem_len = _get_packet_connect_rem_len(options);
    if (get_mqtt_packet_len(rem_len) > buf_len) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_BUF_TOO_SHORT);
    }

    rc = mqtt_init_packet_header(&header, CONNECT, QOS0, 0, 0);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    // 报文固定头部第一个字节
    mqtt_write_char(&ptr, header.byte);

    // 报文固定头部剩余长度字段
    ptr += mqtt_write_packet_rem_len(ptr, rem_len);

    // 报文可变头部协议名 + 协议版本号
    if (4 == options->MQTTVersion) {
        mqtt_write_utf8_string(&ptr, "MQTT");
        mqtt_write_char(&ptr, (unsigned char) 4);
    } else {
        mqtt_write_utf8_string(&ptr, "MQIsdp");
        mqtt_write_char(&ptr, (unsigned char) 3);
    }

    // 报文可变头部连接标识位
    flags.all = 0;
    flags.bits.cleansession = (options->clean_session) ? 1 : 0;

    if (options->username != NULL) {
        flags.bits.username = 1;
    }
    if (options->password != NULL) {
        flags.bits.password = 1;
    }
    mqtt_write_char(&ptr, flags.all);

    // 报文可变头部心跳周期/保持连接, 一个以秒为单位的时间间隔, 表示为一个16位的字
    mqtt_write_uint_16(&ptr, options->keep_alive_interval);

    // 有效负载部分: 客户端标识符
    mqtt_write_utf8_string(&ptr, options->client_id);

    // 用户名
    if (flags.bits.username) {
        mqtt_write_utf8_string(&ptr, options->username);
    }

    // 密码
    if (flags.bits.password) {
        mqtt_write_utf8_string(&ptr, options->password);
    }

    *serialized_len = (uint32_t) (ptr - buf);
    
    HAL_Free(options->username);

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

/**
  * Deserializes the supplied (wire) buffer into connack data - return code
  * @param sessionPresent the session present flag returned (only for MQTT 3.1.1)
  * @param connack_rc returned integer value of the connack return code
  * @param buf the raw buffer data, of the correct length determined by the remaining length field
  * @param buflen the length in bytes of the data in the supplied buffer
  * @return int indicating function execution status
  */
static int _deserialize_connack_packet(uint8_t *sessionPresent, int *connack_rc, unsigned char *buf, size_t buflen) {
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(sessionPresent, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(connack_rc, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(buf, QCLOUD_ERR_INVAL);

    MQTTHeader header = {0};
    unsigned char *curdata = buf;
    unsigned char *enddata = NULL;
    int rc;
    uint32_t decodedLen = 0, readBytesLen = 0;
    MQTTConnackFlags flags = {0};
    unsigned char connack_rc_char;

    // CONNACK 头部大小是固定的2字节长度, 可变头部也是两个字节的长度, 无有效负载
    if (4 > buflen) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_BUF_TOO_SHORT);
    }

    // 读取固定头部第一个字节
    header.byte = mqtt_read_char(&curdata);
    if (CONNACK != header.bits.type) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    // 读取固定头部剩余长度字段
    rc = mqtt_read_packet_rem_len_form_buf(curdata, &decodedLen, &readBytesLen);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }
    curdata += (readBytesLen);
    enddata = curdata + decodedLen;
    if (enddata - curdata != 2) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    // 读取可变头部-连接确认标志 参考MQTT协议说明文档3.2.2.1小结
    flags.all = mqtt_read_char(&curdata);
    *sessionPresent = flags.bits.sessionpresent;

    // 读取可变头部-连接返回码 参考MQTT协议说明文档3.2.2.3小结
    connack_rc_char = mqtt_read_char(&curdata);
    switch (connack_rc_char) {
        case CONNACK_CONNECTION_ACCEPTED:
            *connack_rc = QCLOUD_ERR_MQTT_CONNACK_CONNECTION_ACCEPTED;
            break;
        case CONANCK_UNACCEPTABLE_PROTOCOL_VERSION_ERROR:
            *connack_rc = QCLOUD_ERR_MQTT_CONANCK_UNACCEPTABLE_PROTOCOL_VERSION;
            break;
        case CONNACK_IDENTIFIER_REJECTED_ERROR:
            *connack_rc = QCLOUD_ERR_MQTT_CONNACK_IDENTIFIER_REJECTED;
            break;
        case CONNACK_SERVER_UNAVAILABLE_ERROR:
            *connack_rc = QCLOUD_ERR_MQTT_CONNACK_SERVER_UNAVAILABLE;
            break;
        case CONNACK_BAD_USERDATA_ERROR:
            *connack_rc = QCLOUD_ERR_MQTT_CONNACK_BAD_USERDATA;
            break;
        case CONNACK_NOT_AUTHORIZED_ERROR:
            *connack_rc = QCLOUD_ERR_MQTT_CONNACK_NOT_AUTHORIZED;
            break;
        default:
            *connack_rc = QCLOUD_ERR_MQTT_CONNACK_UNKNOWN;
            break;
    }

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

/**
 * @brief 与服务器建立MQTT连接
 *
 * @param pClient
 * @param options
 * @return
 */
static int _mqtt_connect(Qcloud_IoT_Client *pClient, MQTTConnectParams *options) {

    IOT_FUNC_ENTRY;

    Timer connect_timer;
    int connack_rc = QCLOUD_ERR_FAILURE, rc = QCLOUD_ERR_FAILURE;
    uint8_t sessionPresent = 0;
    uint32_t len = 0;

    InitTimer(&connect_timer);
    countdown_ms(&connect_timer, pClient->command_timeout_ms);

    if (NULL != options) {
        _copy_connect_params(&(pClient->options), options);
    }

    // 建立TLS连接
    rc = pClient->network_stack.connect(&(pClient->network_stack));
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    // 序列化CONNECT报文
    rc = _serialize_connect_packet(pClient->buf, pClient->buf_size, &pClient->options, &len);
    if (QCLOUD_ERR_SUCCESS != rc || 0 == len) {
        IOT_FUNC_EXIT_RC(rc);
    }

    // 发送CONNECT报文
    rc = send_mqtt_packet(pClient, len, &connect_timer);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    // 阻塞等待CONNACK的报文,
    rc = wait_for_read(pClient, CONNACK, &connect_timer);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    // 反序列化CONNACK包, 检查返回码
    rc = _deserialize_connack_packet(&sessionPresent, &connack_rc, pClient->read_buf, pClient->read_buf_size);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    if (QCLOUD_ERR_MQTT_CONNACK_CONNECTION_ACCEPTED != connack_rc) {
        IOT_FUNC_EXIT_RC(connack_rc);
    }

    pClient->is_connected = 1;
    pClient->was_manually_disconnected = 0;
    pClient->is_ping_outstanding = 0;
    countdown(&pClient->ping_timer, pClient->options.keep_alive_interval);

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

int qcloud_iot_mqtt_connect(Qcloud_IoT_Client *pClient, MQTTConnectParams *pParams) {

    IOT_FUNC_ENTRY;
    int rc;
    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(pParams, QCLOUD_ERR_INVAL);

    // 如果MQTT连接已经建立, 不要重复发送CONNECT报文
    if (pClient->is_connected == 1) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_ALREADY_CONNECTED);
    }

    rc = _mqtt_connect(pClient, pParams);

    // 如果MQTT连接建立失败, 则断开底层的TLS连接
    if (rc != QCLOUD_ERR_SUCCESS) {
        pClient->network_stack.disconnect(&(pClient->network_stack));
    }

    IOT_FUNC_EXIT_RC(rc);
}

int qcloud_iot_mqtt_attempt_reconnect(Qcloud_IoT_Client *pClient) {

    IOT_FUNC_ENTRY;

    int rc;
    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);

    Log_i("attempt to reconnect %s = %p", pClient, pClient);

    if (pClient->is_connected == 1) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_ALREADY_CONNECTED);
    }

    rc = qcloud_iot_mqtt_connect(pClient, &pClient->options);

    if (pClient->is_connected == 0) {
        IOT_FUNC_EXIT_RC(rc);
    }

    rc = qcloud_iot_mqtt_resubscribe(pClient);
    if (rc != QCLOUD_ERR_SUCCESS) {
        IOT_FUNC_EXIT_RC(rc);
    }

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_RECONNECTED);
}

int qcloud_iot_mqtt_disconnect(Qcloud_IoT_Client *pClient) {

    IOT_FUNC_ENTRY;

    int rc;
    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);

    Timer timer;
    uint32_t serialized_len = 0;

    if (pClient->is_connected == 0) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_NO_CONN);
    }

    // 1. 组disconnect包
    rc = serialize_packet_with_zero_payload(pClient->buf, pClient->buf_size, DISCONNECT, &serialized_len);
    if (rc != QCLOUD_ERR_SUCCESS) {
        IOT_FUNC_EXIT_RC(rc);
    }

    InitTimer(&timer);
    countdown_ms(&timer, pClient->command_timeout_ms);

    // 2. 发送disconnect包
    if (serialized_len > 0) {
        rc = send_mqtt_packet(pClient, serialized_len, &timer);
        if (rc != QCLOUD_ERR_SUCCESS) {
            IOT_FUNC_EXIT_RC(rc);
        }
    }

    // 3. 断开底层TCP连接, 并修改相关标识位
    pClient->network_stack.disconnect(&(pClient->network_stack));
    pClient->is_connected = 0;
    pClient->was_manually_disconnected = 1;

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

#ifdef __cplusplus
}
#endif


