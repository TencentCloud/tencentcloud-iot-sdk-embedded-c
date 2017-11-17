#ifndef MQTT_CLIENT_C_QCLOUD_IOT_NETWORK_TLS_H
#define MQTT_CLIENT_C_QCLOUD_IOT_NETWORK_TLS_H

#include <stdbool.h>

#include "qcloud_iot_export_error.h"

typedef struct {

    char             *ca_file;              // ca证书文件路径, 用于认证服务端证书
    char             *cert_file;            // 客户端证书
    char             *key_file;             // 客户端私钥
    char             *psk;                  // 对称加密密钥
    char             *psk_id;               // psk密钥ID
    size_t           psk_length;            // psk长度
    char             *host;                 // MQTT服务器地址
    int              port;                  // MQTT服务器端口
    unsigned int     timeout_ms;            // SSL握手超时时间
    uint8_t			 is_asymc_encryption;
} TLSConnectParams;

/**
 * @brief 用于保存SSL连接相关数据结构
 */
typedef struct {
    mbedtls_net_context          socket_fd;        // socket文件描述符
    mbedtls_entropy_context      entropy;          // 保存熵配置
    mbedtls_ctr_drbg_context     ctr_drbg;         // 随机数生成器
    mbedtls_ssl_context          ssl;              // 保存SSL基本数据
    mbedtls_ssl_config           ssl_conf;         // TSL/TLS配置信息
    mbedtls_x509_crt             ca_cert;          // ca证书信息
    mbedtls_x509_crt             client_cert;      // 客户端证书信息
    mbedtls_pk_context           private_key;      // 客户端私钥信息

} TLSDataParams;

/**
 * @brief 系统调用select()返回值
 */
enum {
    SELECT_TIMEOUT = 0, // select()返回0, 表示已超时
    SELECT_ERROR   = -1   // select()返回-1, 表示错误, 具体看select()返回
};

/**
 * @brief 为MQTT客户端建立SSL连接
 *
 * 主要步骤如下:
 *     1. 初始化工作, 例如openssl库初始化, 相关证书文件加载等
 *     2. 建立TCP socket连接
 *     3. 建立SSL连接, 包括握手, 服务器证书检查等
 *
 * @param pConnectParams SSL连接初始化参数
 * @param pDataParams SSL连接相关数据结构
 * @return  返回0表示成功
 */
int HAL_TLS_Connect(TLSConnectParams *pConnectParams, TLSDataParams *pDataParams);

/**
 * @brief 断开SSL连接, 并释放相关对象资源
 *
 * @param pParams SSL连接参数
 */
void HAL_TLS_Disconnect(TLSDataParams *pParams);

/**
 * @brief 通过SSL连接写数据
 *
 * @param pParams     SSL连接参数
 * @param msg         写入数据
 * @param totalLen    写入数据长度
 * @param timeout_ms  超时时间, 单位:ms
 * @param written_len 已写入数据长度
 * @return 若写数据成功, 则返回写入数据的长度
 */
int HAL_TLS_Write(TLSDataParams *pParams, unsigned char *msg, size_t totalLen, int timeout_ms, size_t *written_len);

/**
 * @brief 通过SSL连接读数据
 *
 * @param pParams    SSL连接参数
 * @param msg        读取数据
 * @param totalLen   读取数据的长度
 * @param timeout_ms 超时时间, 单位:ms
 * @param read_len   已读取数据长度
 * @return 若读数据成功, 则返回读取数据的长度
 */
int HAL_TLS_Read(TLSDataParams *pParams, unsigned char *msg, size_t totalLen, int timeout_ms, size_t *read_len);

#endif //MQTT_CLIENT_C_QCLOUD_IOT_NETWORK_TLS_H
