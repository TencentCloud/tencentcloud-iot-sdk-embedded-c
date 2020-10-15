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

#ifndef QCLOUD_IOT_EXPORT_REMOTE_CONFIG_H_
#define QCLOUD_IOT_EXPORT_REMOTE_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define REMOTE_CONFIG_ERRCODE_SUCCESS 0     // get config success
#define REMOTE_CONFIG_ERRCODE_DISABLE 1001  // Cloud platform has been disabled remote config

#define REMOTE_CONFIG_JSON_BUFFER_MIN_LEN 60 // config json buffer sys need len 

/**
 * @brief proc config data, subscribe set struct ConfigSubscirbeUserData
 *
 * @param client               MQTTClient pointer
 * @param config_reply_errcode Config ErrCode; ERRCODE_REMOTE_CONFIG_SUCCESS can proc config_json other config_json is invalid
 * @param config_json          config json data string, format is {xxxx}; user config data at cloud platform, pointer is 
                               struct ConfigSubscirbeUserData json_buffer member
 * @param config_json_len      config json data length, 
 * @return                     void
 */
typedef void (*OnConfigProcHandler)(void *client, int config_reply_errcode, char *config_json, int config_json_len);

typedef struct {
    OnConfigProcHandler on_config_proc;  // config proc callback when message arrived
    int                 json_buffer_len; // sys json data need 60 B + config json data max len 
    char *				json_buffer;     // save json buffer [sys json data + config json data] ,sdk proc recv buff
                                         // save config json data as OnConfigProcHandler functions config_json parameter
} ConfigSubscirbeUserData;

/**
 * @brief subscribe config topic $config/operation/result/${productID}/${deviceName}
 *
 * @param client               MQTTClient pointer
 * @param config_sub_userdata  Config topic subscribe userdata
 * @param subscribe_timeout    wait cloud platform reply max time, ms
 * @return                     >= QCLOUD_RET_SUCCESS for success
 *                             otherwise, failure
 */
int IOT_Subscribe_Config(void *client, ConfigSubscirbeUserData *config_sub_userdata, int subscribe_timeout);

/**
 * @brief get config from MQTT server   $config/operation/${productID}/${deviceName}
 *
 * @param client            MQTTClient pointer
 * @param json_buffer       store create json string
 * @param buffer_size       pJsonBuffer len min 20B
 * @param reply_timeout		wait cloud platform reply max time, ms
 * @return                  QCLOUD_RET_SUCCESS for success
 *                          otherwise, failure
 */
int IOT_Get_Config(void *client, char *json_buffer, int buffer_size, int reply_timeout);

#ifdef __cplusplus
}
#endif

#endif /* QCLOUD_IOT_EXPORT_CONFIG_H_ */