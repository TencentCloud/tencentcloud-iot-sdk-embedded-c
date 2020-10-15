/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2018-2020 THL A29 Limited, a Tencent company. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"
#include "utils_getopt.h"
#include "lite-utils.h"

typedef struct {
    int baud_rate;
    int data_bits;
    int parity;
    int stop_bit;
    int thread_sleep_time;
} ConfigData;

static ConfigData sg_config_data       = {9600, 8, 0, 1, 1000};
static bool       sg_config_arrived = false;

static void _on_config_proc_handler(void *client, int config_reply_errcode, char *config_json, int config_json_len)
{
    int rc;
    Log_i("config message arrived , config data: %s", config_json);

    if (REMOTE_CONFIG_ERRCODE_SUCCESS != config_reply_errcode) {
        Log_i("remote config data arrived!!! error code is %d not 0", config_reply_errcode);
        return;
    } else if (0 == config_json_len) {
        Log_i("remote config data arrived!!! config data is null but error code is 0");
        return;
    }

    char *data_rate         = LITE_json_value_of("baud rate", config_json);
    char *data_bits         = LITE_json_value_of("data bits", config_json);
    char *parity            = LITE_json_value_of("parity", config_json);
    char *stop_bit          = LITE_json_value_of("stop bit", config_json);
    char *thread_sleep_time = LITE_json_value_of("thread sleep", config_json);

    if (NULL != data_rate) {
        rc = LITE_get_int32(&sg_config_data.baud_rate, data_rate);
        HAL_Free(data_rate);
    }

    if (NULL != data_bits) {
        rc = LITE_get_int32(&sg_config_data.data_bits, data_bits);
        HAL_Free(data_bits);
    }

    if (NULL != parity) {
        rc = LITE_get_int32(&sg_config_data.parity, parity);
        HAL_Free(parity);
    }

    if (NULL != stop_bit) {
        rc = LITE_get_int32(&sg_config_data.stop_bit, stop_bit);
        HAL_Free(stop_bit);
    }

    if (NULL != thread_sleep_time) {
        rc = LITE_get_int32(&sg_config_data.thread_sleep_time, thread_sleep_time);

        sg_config_data.thread_sleep_time = (sg_config_data.thread_sleep_time > 1000) ? sg_config_data.thread_sleep_time : 1000;

        HAL_Free(thread_sleep_time);
    }

    sg_config_arrived = true;

    Log_i("config message arrived , proc end: %d, %d, %d, %d, %d", sg_config_data.baud_rate, sg_config_data.data_bits,
          sg_config_data.parity, sg_config_data.stop_bit, sg_config_data.thread_sleep_time);

    (void)rc;

    return;
}

// MQTT event callback
static void _mqtt_event_handler(void *pclient, void *handle_context, MQTTEventMsg *msg)
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
                  mqtt_messge->topic_len, mqtt_messge->ptopic, mqtt_messge->payload_len, mqtt_messge->payload);
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
        default:
            Log_i("Should NOT arrive here.");
            break;
    }
}

// Setup MQTT construct parameters
static int _setup_connect_init_params(MQTTInitParams *initParams, DeviceInfo *device_info)
{
    initParams->product_id  = device_info->product_id;
    initParams->device_name = device_info->device_name;

#ifdef AUTH_MODE_CERT
    char  certs_dir[16] = "certs";
    char  current_path[128];
    char *cwd = getcwd(current_path, sizeof(current_path));

    if (cwd == NULL) {
        Log_e("getcwd return NULL");
        return QCLOUD_ERR_FAILURE;
    }

#ifdef WIN32
    HAL_Snprintf(initParams->cert_file, FILE_PATH_MAX_LEN, "%s\\%s\\%s", current_path, certs_dir,
                 device_info->dev_cert_file_name);
    HAL_Snprintf(initParams->key_file, FILE_PATH_MAX_LEN, "%s\\%s\\%s", current_path, certs_dir,
                 device_info->dev_key_file_name);
#else
    HAL_Snprintf(initParams->cert_file, FILE_PATH_MAX_LEN, "%s/%s/%s", current_path, certs_dir,
                 device_info->dev_cert_file_name);
    HAL_Snprintf(initParams->key_file, FILE_PATH_MAX_LEN, "%s/%s/%s", current_path, certs_dir,
                 device_info->dev_key_file_name);
#endif

#else
    initParams->device_secret = device_info->device_secret;
#endif

    initParams->command_timeout        = QCLOUD_IOT_MQTT_COMMAND_TIMEOUT;
    initParams->keep_alive_interval_ms = QCLOUD_IOT_MQTT_KEEP_ALIVE_INTERNAL;

    initParams->auto_connect_enable  = 1;
    initParams->event_handle.h_fp    = _mqtt_event_handler;
    initParams->event_handle.context = NULL;

    return QCLOUD_RET_SUCCESS;
}

static bool sg_loop_test = false;
static int  parse_arguments(int argc, char **argv)
{
    int c;
    while ((c = utils_getopt(argc, argv, "c:l")) != EOF) switch (c) {
            case 'c':
                if (HAL_SetDevInfoFile(utils_optarg))
                    return -1;
                break;

            case 'l':
                sg_loop_test = true;
                break;

            default:
                HAL_Printf(
                    "usage: %s [options]\n"
                    "  [-c <config file for DeviceInfo>] \n"
                    "  [-l ] loop test or not\n",
                    argv[0]);
                return -1;
        }
    return 0;
}

int main(int argc, char **argv)
{
    int rc;
    // init log level
    IOT_Log_Set_Level(eLOG_DEBUG);

    // parse arguments for device info file and loop test;
    rc = parse_arguments(argc, argv);
    if (rc != QCLOUD_RET_SUCCESS) {
        Log_e("parse arguments error, rc = %d", rc);
        return rc;
    }

    DeviceInfo device_info = {0};
    rc                     = HAL_GetDevInfo((void *)&device_info);
    if (QCLOUD_RET_SUCCESS != rc) {
        Log_e("get device info failed: %d", rc);
        return rc;
    }

    // init connection
    MQTTInitParams init_params = DEFAULT_MQTTINIT_PARAMS;
    rc                         = _setup_connect_init_params(&init_params, &device_info);
    if (rc != QCLOUD_RET_SUCCESS) {
        Log_e("init params error, rc = %d", rc);
        return rc;
    }

    // create MQTT client and connect with server
    void *client = IOT_MQTT_Construct(&init_params);
    if (client != NULL) {
        Log_i("Cloud Device Construct Success");
    } else {
        Log_e("MQTT Construct failed!");
        return QCLOUD_ERR_FAILURE;
    }

    // subscribe config topic
    // example: config json data : {"baud rate":115200,"data bits":8,"stop bit":1,"parity":"NONE","thread sleep":1000}
    static ConfigSubscirbeUserData config_sub_userdata;
    config_sub_userdata.on_config_proc  = _on_config_proc_handler;
    config_sub_userdata.json_buffer_len = REMOTE_CONFIG_JSON_BUFFER_MIN_LEN + 88;
    config_sub_userdata.json_buffer     = HAL_Malloc(config_sub_userdata.json_buffer_len);
    if (NULL == config_sub_userdata.json_buffer) {
        Log_e("Malloc Error size:%d", config_sub_userdata.json_buffer_len);
        return QCLOUD_ERR_MALLOC;
    }

    for(int count = 0; count < 3; count++) {
        rc = IOT_Subscribe_Config(client, &config_sub_userdata, 5000);
        if (QCLOUD_RET_SUCCESS <= rc) {
            Log_i("config topic subscribe success, packetid:%d", rc);
    	    break;
        } else {
            Log_i("config topic subscribe failed, ret:%d, retry :%d", rc, count + 1);
        }
    }

    char json_buffer[32];
    int time_count = 0;
    rc = IOT_Get_Config(client, json_buffer, sizeof(json_buffer), 5000);
    if (rc != QCLOUD_RET_SUCCESS) {
        Log_e("get config failed ret: %d", rc);
    }

    do {
        rc = IOT_MQTT_Yield(client, 500);
        if (rc == QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT) {
            HAL_SleepMs(1000);
            continue;
        } else if (rc != QCLOUD_RET_SUCCESS && rc != QCLOUD_RET_MQTT_RECONNECTED) {
            Log_e("exit with error: %d", rc);
            break;
        }
		
        time_count++;
        if ((time_count % 120) == 0) {
            rc = IOT_Get_Config(client, json_buffer, sizeof(json_buffer), 5000);
            if (rc != QCLOUD_RET_SUCCESS) {
                Log_e("get config failed ret: %d", rc);
            }
        }

        if (sg_loop_test)
            HAL_SleepMs(sg_config_data.thread_sleep_time);

    } while (sg_loop_test);

    rc = IOT_MQTT_Destroy(&client);

    return rc;
}
