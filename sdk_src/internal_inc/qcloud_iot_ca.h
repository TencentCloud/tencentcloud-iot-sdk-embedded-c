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

#ifndef IOT_CA_H_
#define IOT_CA_H_

#ifdef __cplusplus
extern "C" {
#endif

const char *iot_ca_get(void);

const char *iot_dynreg_https_ca_get(void);

const char *iot_https_ca_get(void);

const char *iot_wss_ssh_ca_get(void);

const char *iot_wss_mqtt_ca_get(void);

const char *iot_get_mqtt_domain(const char *region);

int iot_get_mqtt_port(void);

const char *iot_get_coap_domain(const char *region);

const char *iot_get_dyn_reg_domain(const char *region);

const char *iot_get_ws_ssh_domain(const char *region);

const char *iot_get_ssh_domain(const char *region);

#ifdef __cplusplus
}
#endif

#endif /* IOT_CA_H_ */
