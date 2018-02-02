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

static void _iot_disconnect_callback(Qcloud_IoT_Client *pClient)
{

    if (NULL != pClient->event_handle.h_fp) {
        MQTTEventMsg msg;
        msg.event_type = MQTT_EVENT_DISCONNECT;
        msg.msg = NULL;

        pClient->event_handle.h_fp(pClient, pClient->event_handle.context, &msg);
    }
}

static void _reconnect_callback(Qcloud_IoT_Client* pClient) 
{
    if (NULL != pClient->event_handle.h_fp) {
        MQTTEventMsg msg;
        msg.event_type = MQTT_EVENT_RECONNECT;
        msg.msg = NULL;

        pClient->event_handle.h_fp(pClient, pClient->event_handle.context, &msg);
    }
}

/**
 * @brief 处理非手动断开连接的情况
 *
 * @param pClient
 * @return
 */
static int _handle_disconnect(Qcloud_IoT_Client *pClient) {
    IOT_FUNC_ENTRY;
    int rc;

    if (0 == get_client_conn_state(pClient)) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_NO_CONN);
    }

    rc = qcloud_iot_mqtt_disconnect(pClient);
    // 若断开连接失败, 强制断开底层TLS层连接
    if (rc != QCLOUD_ERR_SUCCESS) {
        pClient->network_stack.disconnect(&(pClient->network_stack));
        set_client_conn_state(pClient, NOTCONNECTED);
    }

    _iot_disconnect_callback(pClient);

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
            Log_e("attempt to reconnect success.");
            _reconnect_callback(pClient);
            IOT_FUNC_EXIT_RC(rc);
        }
        else {
            Log_e("attempt to reconnect failed, errCode: %d", rc);
            rc = QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT;
        }
    }

    pClient->current_reconnect_wait_interval *= 2;

    if (MAX_RECONNECT_WAIT_INTERVAL < pClient->current_reconnect_wait_interval) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_RECONNECT_TIMEOUT);
    }
    countdown_ms(&(pClient->reconnect_delay_timer), pClient->current_reconnect_wait_interval);

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
    rc = serialize_packet_with_zero_payload(pClient->write_buf, pClient->write_buf_size, PINGREQ, &serialized_len);
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

    HAL_MutexLock(pClient->lock_generic);
    pClient->is_ping_outstanding = 1;
    /* start a timer to wait for PINGRESP from server */
    countdown(&pClient->ping_timer, pClient->options.keep_alive_interval / 2);
    HAL_MutexUnlock(pClient->lock_generic);

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
    if (!get_client_conn_state(pClient) && pClient->was_manually_disconnected == 1) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_MANUALLY_DISCONNECTED);
    }

    // 2. 检查连接是否断开, 自动连接是否开启
    if (!get_client_conn_state(pClient) && pClient->options.auto_connect_enable == 0) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_NO_CONN);
    }

    InitTimer(&timer);
    countdown_ms(&timer, timeout_ms);

    // 3. 循环读取消息以及心跳包管理
    while (!expired(&timer)) {
        if (!get_client_conn_state(pClient)) {
            if (pClient->current_reconnect_wait_interval > MAX_RECONNECT_WAIT_INTERVAL) {
                rc = QCLOUD_ERR_MQTT_RECONNECT_TIMEOUT;
                break;
            }
            rc = _handle_reconnect(pClient);

            continue;
        }

        rc = cycle_for_read(pClient, &timer, &packet_type, 0);

        if (rc == QCLOUD_ERR_SUCCESS) {
            /* check list of wait publish ACK to remove node that is ACKED or timeout */
            qcloud_iot_mqtt_pub_info_proc(pClient);

            /* check list of wait subscribe(or unsubscribe) ACK to remove node that is ACKED or timeout */
            qcloud_iot_mqtt_sub_info_proc(pClient);

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
                pClient->current_reconnect_wait_interval = MIN_RECONNECT_WAIT_INTERVAL;
                countdown_ms(&(pClient->reconnect_delay_timer), pClient->current_reconnect_wait_interval);

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

/**
 * @brief puback等待超时检测
 *
 * @param pClient MQTTClient对象
 *
 */
int qcloud_iot_mqtt_pub_info_proc(Qcloud_IoT_Client *pClient)
{
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);


    HAL_MutexLock(pClient->lock_list_pub);
    do {
        if (0 == pClient->list_pub_wait_ack->len) {
            break;
        }

        ListIterator *iter;
        ListNode *node = NULL;
        ListNode *temp_node = NULL;

        if (NULL == (iter = list_iterator_new(pClient->list_pub_wait_ack, LIST_TAIL))) {
            Log_e("new list failed");
            break;
        }

        for (;;) {
            node = list_iterator_next(iter);

            if (NULL != temp_node) {
                list_remove(pClient->list_pub_wait_ack, temp_node);
                temp_node = NULL;
            }

            if (NULL == node) {
                break; /* end of list */
            }

            QcloudIotPubInfo *repubInfo = (QcloudIotPubInfo *) node->val;
            if (NULL == repubInfo) {
                Log_e("node's value is invalid!");
                temp_node = node;
                continue;
            }

            /* remove invalid node */
            if (MQTT_NODE_STATE_INVALID == repubInfo->node_state) {
                temp_node = node;
                continue;
            }

            if (!pClient->is_connected) {
                continue;
            }

            /* check the request if timeout or not */
            if (left_ms(&repubInfo->pub_start_time) > 0) {
                continue;
            }

            /* If wait ACK timeout, republish */
            HAL_MutexUnlock(pClient->lock_list_pub);
            /* 重发机制交给上层用户二次开发, 这里先把超时的节点从列表中移除 */
            temp_node = node;

            countdown_ms(&repubInfo->pub_start_time, pClient->command_timeout_ms);
            HAL_MutexLock(pClient->lock_list_pub);

                /* 通知外部网络已经断开 */
            if (NULL != pClient->event_handle.h_fp) {
                MQTTEventMsg msg;
                msg.event_type = MQTT_EVENT_PUBLISH_TIMEOUT;
                msg.msg = (void *)(uintptr_t)repubInfo->msg_id;
                pClient->event_handle.h_fp(pClient, pClient->event_handle.context, &msg);
            }
        }

        list_iterator_destroy(iter);

    } while (0);

    HAL_MutexUnlock(pClient->lock_list_pub);

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

/**
 * @brief suback等待超时检测
 *
 * @param pClient MQTTClient对象
 *
 */
int qcloud_iot_mqtt_sub_info_proc(Qcloud_IoT_Client *pClient)
{
    IOT_FUNC_ENTRY;
    int rc = QCLOUD_ERR_SUCCESS;

    if (!pClient) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_INVAL);
    }

    HAL_MutexLock(pClient->lock_list_sub);
    do {
        if (0 == pClient->list_sub_wait_ack->len) {
            break;
        }

        ListIterator *iter;
        ListNode *node = NULL;
        ListNode *temp_node = NULL;
        uint16_t packet_id = 0;
        MessageTypes msg_type;

        if (NULL == (iter = list_iterator_new(pClient->list_sub_wait_ack, LIST_TAIL))) {
            Log_e("new list failed");
            HAL_MutexUnlock(pClient->lock_list_sub);
            IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
        }

        for (;;) {
            node = list_iterator_next(iter);

            if (NULL != temp_node) {
                list_remove(pClient->list_sub_wait_ack, temp_node);
                temp_node = NULL;
            }

            if (NULL == node) {
                break; /* end of list */
            }

            QcloudIotSubInfo *sub_info = (QcloudIotSubInfo *) node->val;
            if (NULL == sub_info) {
                Log_e("node's value is invalid!");
                temp_node = node;
                continue;
            }

            /* remove invalid node */
            if (MQTT_NODE_STATE_INVALID == sub_info->node_state) {
                temp_node = node;
                continue;
            }

            if (pClient->is_connected <= 0) {
                continue;
            }

            /* check the request if timeout or not */
            if (left_ms(&sub_info->sub_start_time) > 0) {
                continue;
            }

            /* When arrive here, it means timeout to wait ACK */
            packet_id = sub_info->msg_id;
            msg_type = sub_info->type;

            /* Wait MQTT SUBSCRIBE ACK timeout */
            if (NULL != pClient->event_handle.h_fp) {
                MQTTEventMsg msg;

                if (SUBSCRIBE == msg_type) {
                    /* subscribe timeout */
                    msg.event_type = MQTT_EVENT_SUBCRIBE_TIMEOUT;
                    msg.msg = (void *)(uintptr_t)packet_id;
                } else { 
                    /* unsubscribe timeout */
                    msg.event_type = MQTT_EVENT_UNSUBCRIBE_TIMEOUT;
                    msg.msg = (void *)(uintptr_t)packet_id;
                }

                pClient->event_handle.h_fp(pClient, pClient->event_handle.context, &msg);
            }

            temp_node = node;
        }

        list_iterator_destroy(iter);

    } while (0);

    HAL_MutexUnlock(pClient->lock_list_sub);

    IOT_FUNC_EXIT_RC(rc);
}

#ifdef __cplusplus
}
#endif
