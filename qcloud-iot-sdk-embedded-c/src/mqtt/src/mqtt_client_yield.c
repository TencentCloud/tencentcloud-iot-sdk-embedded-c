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

#include "mqtt_client.h"

/**
 * @brief 处理非手动断开连接的情况
 *
 * @param pClient
 * @return
 */
static int _handle_disconnect(Qcloud_IoT_Client *pClient) {
    IOT_FUNC_ENTRY;
    int rc;

    if (0 == pClient->is_connected) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_NO_CONN);
    }

    rc = qcloud_iot_mqtt_disconnect(pClient);
    // 若断开连接失败, 强制断开底层TLS层连接
    if (rc != QCLOUD_ERR_SUCCESS) {
        pClient->network_stack.disconnect(&(pClient->network_stack));
        pClient->is_connected = 0;
    }

    if (NULL != pClient->options.on_disconnect_handler) {
    	pClient->options.on_disconnect_handler();
    }

    // 非手动断开连接
    pClient->was_manually_disconnected = 0;
    IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_NO_CONN);
}

/**
 * @brief 处理自动重连的相关逻辑
 *
 * @param pClient
 * @return
 */
static int _handle_reconnect(Qcloud_IoT_Client *pClient) {
    IOT_FUNC_ENTRY;

    int8_t isPhysicalLayerConnected = 1;
    int rc = QCLOUD_ERR_MQTT_RECONNECTED;

    // 自动重连等待时间还未过期, 还未到重连的时候, 返回正在进行重连
    if (!expired(&(pClient->reconnect_delay_timer))) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT);
    }

    if (NULL != pClient->network_stack.is_connected) {
        isPhysicalLayerConnected = (int8_t) pClient->network_stack.is_connected(&(pClient->network_stack)); // always return 1
    }

    if (isPhysicalLayerConnected) {
        rc = qcloud_iot_mqtt_attempt_reconnect(pClient);
        if (rc == QCLOUD_ERR_MQTT_RECONNECTED) {
            IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_RECONNECTED);
        }
        else {
        	Log_e("attempt to reconnect failed, errCode: %d", rc);
        	rc = QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT;
        }
    }

    pClient->current_reconnect_waitInterval *= 2;

    if (MAX_RECONNECT_WAIT_INTERVAL < pClient->current_reconnect_waitInterval) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_RECONNECT_TIMEOUT);
    }
    countdown_ms(&(pClient->reconnect_delay_timer), pClient->current_reconnect_waitInterval);

    IOT_FUNC_EXIT_RC(rc);
}

/**
 * @brief 处理与服务器维持心跳的相关逻辑
 *
 * @param pClient
 * @return
 */
static int _mqtt_keep_alive(Qcloud_IoT_Client *pClient) {
    IOT_FUNC_ENTRY;

    int rc;
    Timer timer;
    uint32_t serialized_len = 0;

    if (0 == pClient->options.keep_alive_interval) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
    }

    if (!expired(&pClient->ping_timer)) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
    }

    if (pClient->is_ping_outstanding) {
        rc = _handle_disconnect(pClient);
        IOT_FUNC_EXIT_RC(rc);
    }

    /* there is no ping outstanding - send one */
    InitTimer(&timer);
    countdown_ms(&timer, pClient->command_timeout_ms);
    rc = serialize_packet_with_zero_payload(pClient->buf, pClient->buf_size, PINGREQ, &serialized_len);
    if (QCLOUD_ERR_SUCCESS != rc) {
        IOT_FUNC_EXIT_RC(rc);
    }

    /* send the ping packet */
    rc = send_mqtt_packet(pClient, serialized_len, &timer);
    if (QCLOUD_ERR_SUCCESS != rc) {
        //If sending a PING fails we can no longer determine if we are connected.  In this case we decide we are disconnected and begin reconnection attempts
        rc = _handle_disconnect(pClient);
        IOT_FUNC_EXIT_RC(rc);
    }

    pClient->is_ping_outstanding = 1;
    /* start a timer to wait for PINGRESP from server */
    countdown(&pClient->ping_timer, pClient->options.keep_alive_interval / 2);

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

int qcloud_iot_mqtt_yield(Qcloud_IoT_Client *pClient, uint32_t timeout_ms) {
    IOT_FUNC_ENTRY;

    int rc = QCLOUD_ERR_SUCCESS;
    Timer timer;
    uint8_t packet_type;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
    NUMBERIC_SANITY_CHECK(timeout_ms, QCLOUD_ERR_INVAL);

    // 1. 检查连接是否已经手动断开
    if (pClient->is_connected == 0 && pClient->was_manually_disconnected == 1) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_MANUALLY_DISCONNECTED);
    }

    // 2. 检查连接是否断开, 自动连接是否开启
    if (pClient->is_connected == 0 && pClient->options.auto_connect_enable == 0) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_NO_CONN);
    }

    InitTimer(&timer);
    countdown_ms(&timer, timeout_ms);

    // 3. 循环读取消息以及心跳包管理
    while (!expired(&timer)) {
        if (pClient->is_connected == 0) {
            if (pClient->current_reconnect_waitInterval > MAX_RECONNECT_WAIT_INTERVAL) {
                rc = QCLOUD_ERR_MQTT_RECONNECT_TIMEOUT;
                break;
            }
            rc = _handle_reconnect(pClient);

            continue;
        }

        rc = cycle_for_read(pClient, &timer, &packet_type);

        if (rc == QCLOUD_ERR_SUCCESS) {
            rc = _mqtt_keep_alive(pClient);
        } 
		else if (rc == QCLOUD_ERR_SSL_READ || rc == QCLOUD_ERR_SSL_READ_TIMEOUT) {
            rc = _handle_disconnect(pClient);
        }
		else {
			/**
			* 其它错误码处理暂无处理逻辑
			*/
		}

        if (rc == QCLOUD_ERR_MQTT_NO_CONN) {
            pClient->counter_network_disconnected++;

            if (pClient->options.auto_connect_enable == 1) {
                pClient->current_reconnect_waitInterval = MIN_RECONNECT_WAIT_INTERVAL;
                countdown_ms(&(pClient->reconnect_delay_timer), pClient->current_reconnect_waitInterval);

                // 如果超时时间到了,则会直接返回
                rc = QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT;
            } else {
                break;
            }
        } else if (rc != QCLOUD_ERR_SUCCESS) {
            break;
        }
    }

    IOT_FUNC_EXIT_RC(rc);
}

#ifdef __cplusplus
}
#endif
