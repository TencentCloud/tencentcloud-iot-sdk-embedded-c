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

#ifndef SYSTEM_MQTT_SSH_PROXY_H_
#define SYSTEM_MQTT_SSH_PROXY_H_

#ifdef __cplusplus
extern "C" {
#endif

int IOT_Ssh_state_report(void *pClient, int state);

void IOT_QCLOUD_SSH_Start(void *mqtt_client);

void IOT_QCLOUD_SSH_Stop();

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_MQTT_SSH_PROXY_H_ */