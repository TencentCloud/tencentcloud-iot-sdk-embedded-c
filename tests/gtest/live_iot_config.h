#ifndef MQTT_CLIENT_C_IOT_CONFIG_H_H
#define MQTT_CLIENT_C_IOT_CONFIG_H_H

// 这些信息可以从控制台获取得到
#define DOOR_QCLOUD_IOT_MQTT_CLIENT_ID         "144115205260725199"               // 设备ID, 必须保持唯一
#define DOOR_QCLOUD_IOT_MQTT_PASSWORD          "P6kqEgo1e4EHpSAjchsIow=="         // 设备连接云端的密码
#define DOOR_QCLOUD_IOT_MY_PRODUCT_NAME        "MyDoor"                           // 产品名称, 与云端同步设备状态时需要
#define DOOR_QCLOUD_IOT_MY_DEVICE_NAME         "Door"                             // 设备名称, 与云端同步设备状态时需要
#define DOOR_QCLOUD_IOT_CA_FILENAME            "root-ca.crt"                      // CA根证书文件名
#define DOOR_QCLOUD_IOT_CERT_FILENAME          "Door_cert.crt"                    // 客户端证书文件名
#define DOOR_QCLOUD_IOT_KEY_FILENAME           "Door_private.key"                 // 客户端私钥文件名
#define DOOR_QCLOUD_IOT_PSK                    "2VR11nF7aDMidgm62Zq1yg=="         // 客户端psk密钥
#define DOOR_QCLOUD_IOT_IS_ASYNC_ENCRYPTION    1                                  // 客户端是否使用非对称加密方式

#define AIR_CONDITION_QCLOUD_IOT_MQTT_CLIENT_ID         "144115205260725198"                     // 设备ID, 必须保持唯一
#define AIR_CONDITION_QCLOUD_IOT_MQTT_PASSWORD          "wvQTRvQPsLEpEmnBF9mUZw=="               // 设备连接云端的密码
#define AIR_CONDITION_QCLOUD_IOT_MY_PRODUCT_NAME        "MyAirCondition"                         // 产品名称, 与云端同步设备状态时需要
#define AIR_CONDITION_QCLOUD_IOT_MY_DEVICE_NAME         "AirCondition"                           // 设备名称, 与云端同步设备状态时需要
#define AIR_CONDITION_QCLOUD_IOT_CA_FILENAME            "root-ca.crt"                            // CA根证书文件名
#define AIR_CONDITION_QCLOUD_IOT_CERT_FILENAME          "AirCondition_cert.crt"                  // 客户端证书文件名
#define AIR_CONDITION_QCLOUD_IOT_KEY_FILENAME           "AirCondition_private.key"               // 客户端私钥文件名
#define AIR_CONDITION_QCLOUD_IOT_PSK                    "INek0auRS/oX3LS7PYSKPg=="               // 客户端psk密钥
#define AIR_CONDITION_QCLOUD_IOT_IS_ASYNC_ENCRYPTION    1                                        // 客户端是否使用非对称加密方式

#endif //MQTT_CLIENT_C_IOT_CONFIG_H_H
