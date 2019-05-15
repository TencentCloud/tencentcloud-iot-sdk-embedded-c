/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef QLCOUD_IOT_EXPORT_DYNREG_H_ 
#define QLCOUD_IOT_EXPORT_DYNREG_H_

#ifdef __cplusplus
extern "C" {

#endif
/* 设备动态注册domain*/
#define  DYN_REG_SERVER_URL     	"gateway.tencentdevices.com"   

#define  DYN_REG_SERVER_PORT   		80
#define  DYN_REG_SERVER_PORT_TLS    443
#define  REG_URL_MAX_LEN			(128)
#define  DYN_REG_SIGN_LEN 		 	(64)
#define  DYN_BUFF_DATA_MORE			(10)
#define  BASE64_ENCODE_OUT_LEN(x) 	(((x+3)*4)/3)
#define  DYN_REG_RES_HTTP_TIMEOUT_MS  (2000)
#define  FILE_PATH_MAX_LEN			(128)

#ifdef AUTH_MODE_CERT
#define  DYN_RESPONSE_BUFF_LEN	 	(5*1024)
#define  DECODE_BUFF_LEN			(5*1024)
#else
#define  DYN_RESPONSE_BUFF_LEN	 	(256)
#define  DECODE_BUFF_LEN			(256)
#endif



int qcloud_iot_dyn_reg_dev(DeviceInfo *pDevInfo);


#ifdef __cplusplus
}
#endif

#endif //IOT_MQTT_CLIENT_H_
