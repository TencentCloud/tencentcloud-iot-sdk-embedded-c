#ifdef __cplusplus
extern "C" {
#endif

#include "mqtt_client_net.h"

    
/**
 * TODO: 需要看下怎么去实现
 *
 * @brief 用于检查TLS层连接是否还存在
 *
 * @param pNetwork
 * @return
 */
int qcloud_iot_mqtt_tls_is_connected(Network *pNetwork) {
    return 1;
}

/**
 * @brief 初始化Network结构体
 *
 * @param pNetwork
 * @param pConnectParams
 * @return
 */
int qcloud_iot_mqtt_network_init(Network *pNetwork) {
    int rc;
    rc = utils_net_init(pNetwork);
    pNetwork->is_connected = qcloud_iot_mqtt_tls_is_connected;

    return rc;
}

#ifdef __cplusplus
}
#endif
