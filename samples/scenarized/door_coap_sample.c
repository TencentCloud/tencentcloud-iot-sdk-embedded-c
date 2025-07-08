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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"
#include "utils_getopt.h"

#define MAX_SIZE_OF_TOPIC_CONTENT 100

#define PROGRAM_NAME "door_coap_sample"

void printUsage()
{
    HAL_Printf(
        "usage: %s [options]\n"
        "   [-c <config file of DeviceInfo>] \n"
        "   [-t <target device name>]\n"
        "   [-a <action: come_home or leave_home>]\n",
        PROGRAM_NAME);
}

void event_handler(void *pcontext, CoAPEventMessage *message)
{
    switch (message->event_type) {
        case COAP_EVENT_RECEIVE_ACK:
            Log_i("message received ACK, msgid: %d", (unsigned)(uintptr_t)message->message);
            break;
        case COAP_EVENT_RECEIVE_RESPCONTENT: /* not supported currently */
            break;
        case COAP_EVENT_UNAUTHORIZED:
            Log_i("coap client auth token expired or invalid, msgid: %d", (unsigned)(uintptr_t)message->message);
            break;
        case COAP_EVENT_FORBIDDEN:
            Log_i("coap URI is invalid for this device, msgid: %d", (unsigned)(uintptr_t)message->message);
            break;
        case COAP_EVENT_INTERNAL_SERVER_ERROR:
            Log_i("coap server internal error, msgid: %d", (unsigned)(uintptr_t)message->message);
            break;
        case COAP_EVENT_ACK_TIMEOUT:
            Log_i("message receive ACK timeout, msgid: %d", (unsigned)(uintptr_t)message->message);
            break;
        case COAP_EVENT_SEPRESP_TIMEOUT: /* not supported currently */
            break;
        default:
            Log_e("unrecogonized event type: %d", message->event_type);
            break;
    }
}

static int _setup_connect_init_params(CoAPInitParams *initParams, DeviceInfo *device_info)
{
    initParams->product_id  = device_info->product_id;
    initParams->device_name = device_info->device_name;
    initParams->region      = device_info->region;

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
                 STRING_PTR_PRINT_SANITY_CHECK(device_info->dev_cert_file_name));
    HAL_Snprintf(initParams->key_file, FILE_PATH_MAX_LEN, "%s\\%s\\%s", current_path, certs_dir,
                 STRING_PTR_PRINT_SANITY_CHECK(device_info->dev_key_file_name));
#else
    HAL_Snprintf(initParams->cert_file, FILE_PATH_MAX_LEN, "%s/%s/%s", current_path, certs_dir,
                 STRING_PTR_PRINT_SANITY_CHECK(device_info->dev_cert_file_name));
    HAL_Snprintf(initParams->key_file, FILE_PATH_MAX_LEN, "%s/%s/%s", current_path, certs_dir,
                 STRING_PTR_PRINT_SANITY_CHECK(device_info->dev_key_file_name));
#endif

#else
    initParams->device_secret = device_info->device_secret;
#endif

    initParams->command_timeout   = QCLOUD_IOT_MQTT_COMMAND_TIMEOUT;
    initParams->event_handle.h_fp = event_handler;
    initParams->max_retry_count   = 3;

    return QCLOUD_RET_SUCCESS;
}

int main(int argc, char **argv)
{
    int  c;
    char action[16]                                      = {0};
    char target_device_name[MAX_SIZE_OF_DEVICE_NAME + 1] = {0};

    while ((c = utils_getopt(argc, argv, "c:t:a:")) != EOF) switch (c) {
            case 'c':
                if (HAL_SetDevInfoFile(utils_optarg))
                    return -1;
                break;

            case 't':
                strncpy(target_device_name, utils_optarg, MAX_SIZE_OF_DEVICE_NAME);
                break;

            case 'a':
                strncpy(action, utils_optarg, sizeof(action) - 1);
                break;

            default:
                printUsage();

                return -1;
        }

    int rc = QCLOUD_RET_SUCCESS;
    IOT_Log_Set_Level(eLOG_DEBUG);

    DeviceInfo device_info;
    memset(&device_info, 0, sizeof(device_info));
    rc                     = HAL_GetDevInfo((void *)&device_info);
    if (QCLOUD_RET_SUCCESS != rc) {
        Log_e("get device info failed: %d", rc);
        return rc;
    }

    CoAPInitParams init_params = DEFAULT_COAPINIT_PARAMS;
    rc                         = _setup_connect_init_params(&init_params, &device_info);
    if (rc != QCLOUD_RET_SUCCESS) {
        Log_e("init params err,rc=%d", rc);
        return rc;
    }

    void *coap_client = IOT_COAP_Construct(&init_params);
    if (coap_client == NULL) {
        Log_e("COAP Client construct failed.");
        return QCLOUD_ERR_FAILURE;
    } else {
        Log_e("%p", coap_client);
    }

    if (strcmp(action, "come_home") == 0 || strcmp(action, "leave_home") == 0) {
        char          topic_content[MAX_SIZE_OF_TOPIC_CONTENT + 1] = {0};
        SendMsgParams send_params                                  = DEFAULT_SENDMSG_PARAMS;

        int size = HAL_Snprintf(topic_content, sizeof(topic_content), "{\"action\": \"%s\", \"targetDevice\": \"%s\"}",
                                action, target_device_name);
        if (size < 0 || size > sizeof(topic_content) - 1) {
            Log_e("payload content length not enough! content size:%d  buf size:%d", size, (int)sizeof(topic_content));
            return -3;
        }

        send_params.pay_load     = topic_content;
        send_params.pay_load_len = strlen(topic_content);

        char topicName[128] = "";
        sprintf(topicName, "%s/%s/event", device_info.product_id, device_info.device_name);
        Log_i("topic name is %s", topicName);

        rc = IOT_COAP_SendMessage(coap_client, topicName, &send_params);
        if (rc < 0) {
            Log_e("client publish topic failed :%d.", rc);
        } else {
            Log_d("client topic has been sent, msg_id: %d", rc);
        }

        rc = IOT_COAP_Yield(coap_client, 200);

        if (rc != QCLOUD_RET_SUCCESS) {
            Log_e("exit with error: %d", rc);
        }
    } else {
        printUsage();
        return -2;
    }

    IOT_COAP_Destroy(&coap_client);

    return QCLOUD_RET_SUCCESS;
}
