#ifndef MQTT_CLIENT_C_IOT_CONFIG_H_H
#define MQTT_CLIENT_C_IOT_CONFIG_H_H

// 这些信息可以从控制台获取得到
#define DOOR_QCLOUD_IOT_MQTT_CLIENT_ID         "144115205260725516"               // 设备ID, 必须保持唯一
#define DOOR_QCLOUD_IOT_MQTT_PASSWORD          "DEezIpsCLiZfbhWLGQEp3w=="         // 设备连接云端的密码
#define DOOR_QCLOUD_IOT_MY_PRODUCT_NAME        "timen"                            // 产品名称, 与云端同步设备状态时需要
#define DOOR_QCLOUD_IOT_MY_DEVICE_NAME         "timen_door"                       // 设备名称, 与云端同步设备状态时需要
#define DOOR_QCLOUD_IOT_CA_FILENAME            "root-ca.crt"                      // CA根证书文件名
#define DOOR_QCLOUD_IOT_CERT_FILENAME          "timen_door_cert.crt"              // 客户端证书文件名
#define DOOR_QCLOUD_IOT_KEY_FILENAME           "timen_door_private.key"           // 客户端私钥文件名
#define DOOR_QCLOUD_IOT_PSK                    "dVvgJSjcOKC/C80iGF8WqQ=="         // 客户端psk密钥
#define DOOR_QCLOUD_IOT_IS_ASYMC_ENCRYPTION    1                                  // 客户端是否使用非对称加密方式


#define AIR_CONDITION_QCLOUD_IOT_MQTT_CLIENT_ID         "144115205259735503"                     // 设备ID, 必须保持唯一
#define AIR_CONDITION_QCLOUD_IOT_MQTT_PASSWORD          "EVwPO3bjv2266qFzSJkLwg=="               // 设备连接云端的密码
#define AIR_CONDITION_QCLOUD_IOT_MY_PRODUCT_NAME        "timen"                                  // 产品名称, 与云端同步设备状态时需要
#define AIR_CONDITION_QCLOUD_IOT_MY_DEVICE_NAME         "timen_air_condition"                    // 设备名称, 与云端同步设备状态时需要
#define AIR_CONDITION_QCLOUD_IOT_CA_FILENAME            "root-ca.crt"                            // CA根证书文件名
#define AIR_CONDITION_QCLOUD_IOT_CERT_FILENAME          "timen_air_condition_cert.crt"           // 客户端证书文件名
#define AIR_CONDITION_QCLOUD_IOT_KEY_FILENAME           "timen_air_condition_private.key"        // 客户端私钥文件名
#define AIR_CONDITION_QCLOUD_IOT_PSK                    "PTN27rDijrpsD6wLWjxZHw=="               // 客户端psk密钥
#define AIR_CONDITION_QCLOUD_IOT_IS_ASYMC_ENCRYPTION    1                                        // 客户端是否使用非对称加密方式

#endif //MQTT_CLIENT_C_IOT_CONFIG_H_H
