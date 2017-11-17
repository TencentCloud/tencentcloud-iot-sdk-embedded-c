#ifndef MQTT_CLIENT_C_QCLOUD_IOT_MQTT_NET_H
#define MQTT_CLIENT_C_QCLOUD_IOT_MQTT_NET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "qcloud_iot_utils_net.h"

/**
 * @brief 初始化TLS实现
 *
 * @param pNetwork 网络操作相关结构体
 * @return 返回0, 表示初始化成功
 */
int qcloud_iot_mqtt_network_init(Network *pNetwork);

#ifdef __cplusplus
}
#endif

#endif //MQTT_CLIENT_C_QCLOUD_IOT_MQTT_NET_H
