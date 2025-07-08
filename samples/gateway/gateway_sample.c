/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2018-2020 Tencent. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qcloud_iot_export.h"
#include "utils_getopt.h"

static bool sg_unbind_all_subdev = false;

void _event_handler(void *client, void *context, MQTTEventMsg *msg)
{
    MQTTMessage *mqtt_messge = (MQTTMessage *)msg->msg;
    uintptr_t    packet_id   = (uintptr_t)msg->msg;

    switch (msg->event_type) {
        case MQTT_EVENT_UNDEF:
            Log_i("undefined event occur.");
            break;

        case MQTT_EVENT_DISCONNECT:
            Log_i("MQTT disconnect.");
            break;

        case MQTT_EVENT_RECONNECT:
            Log_i("MQTT reconnect.");
            break;

        case MQTT_EVENT_PUBLISH_RECVEIVED:
            Log_i("topic message arrived but without any related handle: topic=%.*s, topic_msg=%.*s",
                  mqtt_messge->topic_len, STRING_PTR_PRINT_SANITY_CHECK(mqtt_messge->ptopic), mqtt_messge->payload_len,
                  STRING_PTR_PRINT_SANITY_CHECK(mqtt_messge->payload));
            break;
        case MQTT_EVENT_SUBCRIBE_SUCCESS:
            Log_i("subscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_SUBCRIBE_TIMEOUT:
            Log_i("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_SUBCRIBE_NACK:
            Log_i("subscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_UNSUBCRIBE_SUCCESS:
            Log_i("unsubscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
            Log_i("unsubscribe timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_UNSUBCRIBE_NACK:
            Log_i("unsubscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_PUBLISH_SUCCESS:
            Log_i("publish success, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_PUBLISH_TIMEOUT:
            Log_i("publish timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_PUBLISH_NACK:
            Log_i("publish nack, packet-id=%u", (unsigned int)packet_id);
            break;
        case MQTT_EVENT_GATEWAY_SEARCH:
            Log_d("gateway search subdev status:%d", *(int32_t *)(msg->msg));
            break;
        case MQTT_EVENT_GATEWAY_UNBIND_ALL:
            sg_unbind_all_subdev = true;
            Log_d("gateway all subdev have been unbind");
            break;
        default:
            Log_i("Should NOT arrive here.");
            break;
    }
}

static void _message_handler(void *client, MQTTMessage *message, void *user_data)
{
    if (message == NULL) {
        return;
    }

    Log_i("Receive Message With topicName:%.*s, payload:%.*s", (int)message->topic_len,
          STRING_PTR_PRINT_SANITY_CHECK(message->ptopic), (int)message->payload_len,
          STRING_PTR_PRINT_SANITY_CHECK((char *)message->payload));
}

static int _setup_gw_init_params(GatewayInitParam *gw_init_params, GatewayDeviceInfo *gw_dev_info)
{
    MQTTInitParams *init_params = &gw_init_params->init_param;
    DeviceInfo *    dev_info    = &gw_dev_info->gw_info;
    init_params->region         = dev_info->region;
    init_params->product_id     = dev_info->product_id;
    init_params->device_name    = dev_info->device_name;

#ifdef AUTH_MODE_CERT
    char  certs_dir[16] = "certs";
    char  current_path[128];
    char *cwd = getcwd(current_path, sizeof(current_path));

    if (cwd == NULL) {
        Log_e("getcwd return NULL");
        return QCLOUD_ERR_FAILURE;
    }

#ifdef WIN32
    HAL_Snprintf(init_params->cert_file, FILE_PATH_MAX_LEN, "%s\\%s\\%s", current_path, certs_dir,
                 STRING_PTR_PRINT_SANITY_CHECK(dev_info->dev_cert_file_name));
    HAL_Snprintf(init_params->key_file, FILE_PATH_MAX_LEN, "%s\\%s\\%s", current_path, certs_dir,
                 STRING_PTR_PRINT_SANITY_CHECK(dev_info->dev_key_file_name));
#else
    HAL_Snprintf(init_params->cert_file, FILE_PATH_MAX_LEN, "%s/%s/%s", current_path, certs_dir,
                 STRING_PTR_PRINT_SANITY_CHECK(dev_info->dev_cert_file_name));
    HAL_Snprintf(init_params->key_file, FILE_PATH_MAX_LEN, "%s/%s/%s", current_path, certs_dir,
                 STRING_PTR_PRINT_SANITY_CHECK(dev_info->dev_key_file_name));
#endif

#else
    init_params->device_secret = dev_info->device_secret;
#endif

    init_params->command_timeout        = QCLOUD_IOT_MQTT_COMMAND_TIMEOUT;
    init_params->keep_alive_interval_ms = QCLOUD_IOT_MQTT_KEEP_ALIVE_INTERNAL;

    init_params->auto_connect_enable  = 1;
    init_params->event_handle.h_fp    = _event_handler;
    init_params->event_handle.context = NULL;

    return QCLOUD_RET_SUCCESS;
}

// subscribe subdev MQTT topic and wait for sub result
static int _subscribe_subdev_topic_wait_result(void *client, char *topic_keyword, QoS qos, GatewayParam *gw_info)
{
    char topic_name[128] = {0};
    int  size            = HAL_Snprintf(
        topic_name, sizeof(topic_name), "%s/%s/%s", STRING_PTR_PRINT_SANITY_CHECK(gw_info->subdev_product_id),
        STRING_PTR_PRINT_SANITY_CHECK(gw_info->subdev_device_name), STRING_PTR_PRINT_SANITY_CHECK(topic_keyword));
    if (size < 0 || size > sizeof(topic_name) - 1) {
        Log_e("topic content length not enough! content size:%d  buf size:%d", size, (int)sizeof(topic_name));
        return QCLOUD_ERR_FAILURE;
    }

    SubscribeParams sub_params    = DEFAULT_SUB_PARAMS;
    sub_params.qos                = qos;
    sub_params.on_message_handler = _message_handler;

    int rc = IOT_Gateway_Subscribe(client, topic_name, &sub_params);
    if (rc < 0) {
        Log_e("IOT_Gateway_Subscribe fail.");
        return rc;
    }

    int wait_cnt = 10;
    while (!IOT_Gateway_IsSubReady(client, topic_name) && (wait_cnt > 0)) {
        // wait for subscription result
        rc = IOT_Gateway_Yield(client, 1000);
        if (rc) {
            Log_e("MQTT error: %d", rc);
            return rc;
        }
        wait_cnt--;
    }

    if (wait_cnt > 0) {
        return QCLOUD_RET_SUCCESS;
    } else {
        Log_e("wait for subscribe result timeout!");
        return QCLOUD_ERR_FAILURE;
    }
}

// publish MQTT msg
static int _publish_subdev_msg(void *client, char *topic_keyword, QoS qos, GatewayParam *gw_info)
{
    char topic_name[128] = {0};
    int  size            = HAL_Snprintf(
        topic_name, sizeof(topic_name), "%s/%s/%s", STRING_PTR_PRINT_SANITY_CHECK(gw_info->subdev_product_id),
        STRING_PTR_PRINT_SANITY_CHECK(gw_info->subdev_device_name), STRING_PTR_PRINT_SANITY_CHECK(topic_keyword));
    if (size < 0 || size > sizeof(topic_name) - 1) {
        Log_e("topic content length not enough! content size:%d  buf size:%d", size, (int)sizeof(topic_name));
        return QCLOUD_ERR_FAILURE;
    }

    static int    test_count = 0;
    PublishParams pub_params = DEFAULT_PUB_PARAMS;
    pub_params.qos           = qos;

    char topic_content[128] = {0};
    size = HAL_Snprintf(topic_content, sizeof(topic_content), "{\"action\": \"gateway_test\", \"count\": \"%d\"}",
                        test_count++);
    if (size < 0 || size > sizeof(topic_content) - 1) {
        Log_e("payload content length not enough! content size:%d  buf size:%d", size, (int)sizeof(topic_content));
        return -3;
    }

    pub_params.payload     = topic_content;
    pub_params.payload_len = strlen(topic_content);

    return IOT_Gateway_Publish(client, topic_name, &pub_params);
}

static void _get_gw_subdev_bindlist(void *client, GatewayParam *gw_param)
{
    SubdevBindList  bindlist;
    SubdevBindInfo *cur_bindinfo = NULL;
    int             rc;

    bindlist.bindlist_head = NULL;
    bindlist.bind_num      = 0;

    rc = IOT_Gateway_Subdev_GetBindList(client, gw_param, &bindlist);
    if (QCLOUD_RET_SUCCESS != rc) {
        Log_e("get sub device bind list failed: %d", rc);
    } else {
        cur_bindinfo = bindlist.bindlist_head;
        HAL_Printf("bind list sub device nums: %d\r\n", bindlist.bind_num);
        if (NULL == cur_bindinfo) {
            Log_e("gateway no bind sub device on cloud platform");
        } else {
            HAL_Printf("bind list sub device info:\r\n");
            int count = 1;
            while (cur_bindinfo) {
                HAL_Printf("sub device: %05d  product_id: % -11s  device_name: % -49s\r\n", count,
                           STRING_PTR_PRINT_SANITY_CHECK(cur_bindinfo->product_id),
                           STRING_PTR_PRINT_SANITY_CHECK(cur_bindinfo->device_name));
                cur_bindinfo = cur_bindinfo->next;
                count += 1;
            }
            // destory bind list
            IOT_Gateway_Subdev_DestoryBindList(&bindlist);
        }
    }

    return;
}

// for reply code, pls check https://cloud.tencent.com/document/product/634/45960
#define GATEWAY_RC_REPEAT_BIND 809
static char *new_subdev_file = NULL;
static int   sg_loop_count   = 5;

static int parse_arguments(int argc, char **argv)
{
    int c;
    while ((c = utils_getopt(argc, argv, "c:b:l:")) != EOF) switch (c) {
            case 'c':
                if (HAL_SetDevInfoFile(utils_optarg))
                    return -1;
                break;

            case 'b':
                new_subdev_file = utils_optarg;
                break;

            case 'l':
                sg_loop_count = atoi(utils_optarg);
                if (sg_loop_count > 10000)
                    sg_loop_count = 10000;
                else if (sg_loop_count < 0)
                    sg_loop_count = 1;
                break;

            default:
                HAL_Printf(
                    "usage: %s [options]\n"
                    "  [-c <config file for DeviceInfo>] \n"
                    "  [-b <config file for new SubDevice to bind>] \n"
                    "  [-l <loop count>] \n",
                    argv[0]);
                return -1;
        }
    return 0;
}

int main(int argc, char **argv)
{
    int   rc     = QCLOUD_ERR_FAILURE;
    void *client = NULL;

    IOT_Log_Set_Level(eLOG_DEBUG);

    // parse arguments for device info file and loop test;
    rc = parse_arguments(argc, argv);
    if (rc != QCLOUD_RET_SUCCESS) {
        Log_e("parse arguments error, rc = %d", rc);
        return rc;
    }

    GatewayDeviceInfo gw_dev_info;
    rc = HAL_GetGwDevInfo((void *)&gw_dev_info);
    if (QCLOUD_RET_SUCCESS != rc) {
        Log_e("Get gateway dev info err,rc:%d", rc);
        return rc;
    }

    GatewayInitParam init_params = DEFAULT_GATEWAY_INIT_PARAMS;
    _setup_gw_init_params(&init_params, &gw_dev_info);

    client = IOT_Gateway_Construct(&init_params);
    if (client == NULL) {
        Log_e("client constructed failed.");
        return QCLOUD_ERR_FAILURE;
    }

    // make sub-device online
    GatewayParam gw_param = DEFAULT_GATEWAY_PARAMS;
    gw_param.product_id   = gw_dev_info.gw_info.product_id;
    gw_param.device_name  = gw_dev_info.gw_info.device_name;

    DeviceInfo *sub_dev_info = gw_dev_info.sub_dev_info;

    // to bind a new sub device
    DeviceInfo new_sub_dev;
    memset(&new_sub_dev, 0, sizeof(DeviceInfo));
    if (new_subdev_file) {
        do {
            rc = HAL_GetDevInfoFromFile(new_subdev_file, &new_sub_dev);
            if (rc) {
                Log_e("get devinfo from file failed: %d", rc);
                break;
            }

            rc = IOT_Gateway_Subdev_Bind(client, &gw_param, &new_sub_dev);
            if (rc == QCLOUD_RET_SUCCESS || rc == GATEWAY_RC_REPEAT_BIND) {
                Log_i("bind sub-device %s-%s success", new_sub_dev.product_id, new_sub_dev.device_name);
                sub_dev_info = &new_sub_dev;
                break;
            } else {
                Log_e("bind subdev failed: %d", rc);
            }
        } while (0);
    }

    gw_param.subdev_product_id  = sub_dev_info->product_id;
    gw_param.subdev_device_name = sub_dev_info->device_name;

#if 0
    // one GatewayParam only support one sub-device
    // use more GatewayParam to add more sub-device
    GatewayParam gw_param1 = DEFAULT_GATEWAY_PARAMS;;
    gw_param1.product_id = gw_dev_info.gw_info.product_id;
    gw_param1.device_name = gw_dev_info.gw_info.device_name;

    gw_param1.subdev_product_id = "SUB-PRODUCT";
    gw_param1.subdev_device_name = "SUB-DEVICE";
#endif

    // get bind list from cloud platform
    _get_gw_subdev_bindlist(client, &gw_param);

    rc = IOT_Gateway_Subdev_Online(client, &gw_param);
    if (rc != QCLOUD_RET_SUCCESS) {
        Log_e("IOT_Gateway_Subdev_Online fail.");
        return rc;
    }

    // gateway proxy loop for sub-device
    // subscribe sub-device topic
    rc = _subscribe_subdev_topic_wait_result(client, "data", QOS0, &gw_param);
    if (rc) {
        Log_e("Subdev Subscribe Topic Failed: %d", rc);
        return rc;
    }

    do {
        // get bind list from cloud platform
        if (sg_loop_count % 60 == 0) {
            _get_gw_subdev_bindlist(client, &gw_param);
        }

        // publish msg to sub-device topic
        rc = _publish_subdev_msg(client, "data", QOS1, &gw_param);
        if (rc < 0) {
            Log_e("IOT_Gateway_Publish fail.");
        }

        rc = IOT_Gateway_Yield(client, 200);

        if (rc == QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT) {
            HAL_SleepMs(1000);
            continue;
        } else if (rc != QCLOUD_RET_SUCCESS && rc != QCLOUD_RET_MQTT_RECONNECTED) {
            Log_e("exit with error: %d", rc);
            break;
        }

        if (true == sg_unbind_all_subdev) {
            Log_d("gateway all subdev have been unbind by cloud platform stop publish subdev msg");
            sg_unbind_all_subdev = false;
            break;
        }

        HAL_SleepMs(1000);

    } while (--sg_loop_count > 0);

    // make sub-device offline
    rc = IOT_Gateway_Subdev_Offline(client, &gw_param);
    if (rc != QCLOUD_RET_SUCCESS) {
        Log_e("IOT_Gateway_Subdev_Offline fail.");
        return rc;
    }

    if (new_subdev_file) {
        rc = IOT_Gateway_Subdev_Unbind(client, &gw_param, &new_sub_dev);
        if (rc) {
            Log_e("unbind failed: %d", rc);
        }
    }

    rc = IOT_Gateway_Destroy(client);

    return rc;
}
