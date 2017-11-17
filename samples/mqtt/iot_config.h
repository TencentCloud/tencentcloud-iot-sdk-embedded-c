#ifndef MQTT_CLIENT_C_IOT_CONFIG_H_H
#define MQTT_CLIENT_C_IOT_CONFIG_H_H

// 这些信息可以从控制台获取得到
#define QCLOUD_IOT_MQTT_CLIENT_ID         "YOUR_CLIENT_ID"                     				// 设备ID, 必须保持唯一
#define QCLOUD_IOT_MQTT_PASSWORD          "YOUR_DEVICE_PASSWORD"               				// 设备连接云端的密码
#define QCLOUD_IOT_MY_PRODUCT_NAME        "YOUR_PRODUCT_NAME"                            	// 产品名称, 与云端同步设备状态时需要
#define QCLOUD_IOT_MY_DEVICE_NAME         "YOUR_DEVICE_NAME"                       			// 设备名称, 与云端同步设备状态时需要
#define QCLOUD_IOT_CA_FILENAME            "root-ca.crt"                            			// CA根证书文件名
#define QCLOUD_IOT_CERT_FILENAME          "YOUR_DEVICE_NAME_cert.crt"                		// 客户端证书文件名
#define QCLOUD_IOT_KEY_FILENAME           "YOUR_DEVICE_NAME_private.key"             		// 客户端私钥文件名
#define QCLOUD_IOT_PSK                    "YOUR_DEVICE_PSK"               					// 客户端psk密钥
#define QCLOUD_IOT_IS_ASYMC_ENCRYPTION    1                                                 // 客户端是否使用非对称加密方式

#endif //MQTT_CLIENT_C_IOT_CONFIG_H_H
