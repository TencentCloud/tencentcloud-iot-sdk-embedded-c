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

#ifndef MQTT_CLIENT_C_IOT_UNIT_TEST_H
#define MQTT_CLIENT_C_IOT_UNIT_TEST_H

// 这些信息可以从控制台获取得到
#define QCLOUD_IOT_MY_PRODUCT_ID            "27N9Y8PCPO"                                                // 产品名称, 与云端同步设备状态时需要
#define QCLOUD_IOT_MY_DEVICE_NAME           "unittest0"                                                  // 设备名称, 与云端同步设备状态时需要
#define QCLOUD_IOT_CA_FILENAME              "root-ca.crt"                                               // CA根证书文件名
#define QCLOUD_IOT_CERT_FILENAME            "unittest_cert.crt"                                         // 客户端证书文件名
#define QCLOUD_IOT_KEY_FILENAME             "unittest_private.key"                                      // 客户端私钥文件名

#endif //MQTT_CLIENT_C_IOT_UNIT_TEST_H
