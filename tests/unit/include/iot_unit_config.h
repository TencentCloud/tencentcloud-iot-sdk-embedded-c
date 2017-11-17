#ifndef MQTT_CLIENT_C_IOT_UNIT_TEST_H
#define MQTT_CLIENT_C_IOT_UNIT_TEST_H

// 这些信息可以从控制台获取得到
#define QCLOUD_IOT_MQTT_HOST              "mqtt.api.cobaya.cc"      // MQTT服务器地址
#define QCLOUD_IOT_MQTT_PORT              8883                      // MQTT服务器端口
#define QCLOUD_IOT_MQTT_CLIENT_ID         "integrationTest"         // 设备ID, 必须保持唯一
#define QCLOUD_IOT_MY_PRODUCT_NAME 		  "integrationTest/"         // 产品名称, 与云端同步设备状态时需要
#define QCLOUD_IOT_MY_DEVICE_NAME 		  "integrationTest"         // 设备名称, 与云端同步设备状态时需要
#define QCLOUD_IOT_CA_FILENAME            "ca-s-coby.crt"           // CA根证书文件名
#define QCLOUD_IOT_CERT_FILENAME          "MyLight_cert.crt"        // 客户端证书文件名
#define QCLOUD_IOT_KEY_FILENAME           "MyLight_private.key"     // 客户端私钥文件名

#endif //MQTT_CLIENT_C_IOT_UNIT_TEST_H
