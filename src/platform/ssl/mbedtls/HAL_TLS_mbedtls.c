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

#include <stdint.h>
#include <string.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "qcloud_iot_import.h"
#include "qcloud_iot_export_error.h"
#include "qcloud_iot_export_log.h"
#include "qcloud_iot_sdk_impl_internal.h"

#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"

#include "utils_timer.h"

#ifndef AUTH_MODE_CERT
static const int ciphersuites[] = { MBEDTLS_TLS_PSK_WITH_AES_128_CBC_SHA, MBEDTLS_TLS_PSK_WITH_AES_256_CBC_SHA, 0 };
#endif
    
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
 * @brief 释放mbedtls开辟的内存
 */
static void _free_mebedtls(TLSDataParams *pParams)
{
    mbedtls_net_free(&(pParams->socket_fd));
    mbedtls_x509_crt_free(&(pParams->client_cert));
    mbedtls_x509_crt_free(&(pParams->ca_cert));
    mbedtls_pk_free(&(pParams->private_key));
    mbedtls_ssl_free(&(pParams->ssl));
    mbedtls_ssl_config_free(&(pParams->ssl_conf));
    mbedtls_ctr_drbg_free(&(pParams->ctr_drbg));
    mbedtls_entropy_free(&(pParams->entropy));
    
    HAL_Free(pParams);
}

/**
 * @brief mbedtls库初始化
 *
 * 1. 执行mbedtls库相关初始化函数
 * 2. 随机数生成器
 * 3. 加载CA证书, 客户端证书及私钥文件/设置psk
 *
 * @param pDataParams       TLS连接相关数据结构
 * @param pConnectParams    TLS证书密钥相关
 * @return                  返回QCLOUD_ERR_SUCCESS, 表示成功
 */
static int _mbedtls_client_init(TLSDataParams *pDataParams, TLSConnectParams *pConnectParams) {

    int ret = QCLOUD_ERR_SUCCESS;
    mbedtls_net_init(&(pDataParams->socket_fd));
    mbedtls_ssl_init(&(pDataParams->ssl));
    mbedtls_ssl_config_init(&(pDataParams->ssl_conf));
    mbedtls_ctr_drbg_init(&(pDataParams->ctr_drbg));
    mbedtls_x509_crt_init(&(pDataParams->ca_cert));
    mbedtls_x509_crt_init(&(pDataParams->client_cert));
    mbedtls_pk_init(&(pDataParams->private_key));

    mbedtls_entropy_init(&(pDataParams->entropy));
    // 随机数, 增加custom参数, 目前为NULL
    if ((ret = mbedtls_ctr_drbg_seed(&(pDataParams->ctr_drbg), mbedtls_entropy_func,
                                     &(pDataParams->entropy), NULL, 0)) != 0) {
        Log_e("mbedtls_ctr_drbg_seed failed returned 0x%04x", ret<0?-ret:ret);
        return QCLOUD_ERR_SSL_INIT;
    }
    
    if (pConnectParams->ca_crt != NULL)
    {
        if ((ret = mbedtls_x509_crt_parse(&(pDataParams->ca_cert), (const unsigned char *)pConnectParams->ca_crt,
            (pConnectParams->ca_crt_len + 1)))) {
            Log_e("parse ca crt failed returned 0x%04x", ret<0?-ret:ret);
            return QCLOUD_ERR_SSL_CERT;
        }
    }

#ifdef AUTH_MODE_CERT
    if (pConnectParams->cert_file != NULL && pConnectParams->key_file != NULL) {
            if ((ret = mbedtls_x509_crt_parse_file(&(pDataParams->client_cert), pConnectParams->cert_file)) != 0) {
            Log_e("load client cert file failed returned 0x%x", ret<0?-ret:ret);
            return QCLOUD_ERR_SSL_CERT;
        }

        if ((ret = mbedtls_pk_parse_keyfile(&(pDataParams->private_key), pConnectParams->key_file, "")) != 0) {
            Log_e("load client key file failed returned 0x%x", ret<0?-ret:ret);
            return QCLOUD_ERR_SSL_CERT;
        }
    } else {
        Log_d("cert_file/key_file is empty!|cert_file=%s|key_file=%s", pConnectParams->cert_file, pConnectParams->key_file);
    }
#else
	if (pConnectParams->psk != NULL && pConnectParams->psk_id !=NULL) {
        const char *psk_id = pConnectParams->psk_id;
        ret = mbedtls_ssl_conf_psk(&(pDataParams->ssl_conf), (unsigned char *)pConnectParams->psk, pConnectParams->psk_length,
                                    (const unsigned char *) psk_id, strlen( psk_id ));
    } else {
        Log_d("psk/pskid is empty!|psk=%s|psd_id=%s", pConnectParams->psk, pConnectParams->psk_id);
    }
	
	if (0 != ret) {
		Log_e("mbedtls_ssl_conf_psk fail: 0x%x", ret<0?-ret:ret);
		return ret;
	}
#endif

    return QCLOUD_ERR_SUCCESS;
}

/**
 * @brief 建立TCP连接
 *
 * @param socket_fd  Socket描述符
 * @param host       服务器主机名
 * @param port       服务器端口地址
 * @return 返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int _mbedtls_tcp_connect(mbedtls_net_context *socket_fd, const char *host, int port) {
    int ret = 0;
    char port_str[6];
    HAL_Snprintf(port_str, 6, "%d", port);
    if ((ret = mbedtls_net_connect(socket_fd, host, port_str, MBEDTLS_NET_PROTO_TCP)) != 0) {

        Log_e("tcp connect failed returned 0x%04x errno: %d", ret<0?-ret:ret, errno);

        switch (ret) {
            case MBEDTLS_ERR_NET_SOCKET_FAILED:
                return QCLOUD_ERR_TCP_SOCKET_FAILED;
            case MBEDTLS_ERR_NET_UNKNOWN_HOST:
                return QCLOUD_ERR_TCP_UNKNOWN_HOST;
            default:
                return QCLOUD_ERR_TCP_CONNECT;
        }

    }

    if ((ret = mbedtls_net_set_block(socket_fd)) != 0) {
        Log_e("set block faliled returned 0x%04x", ret<0?-ret:ret);
        return QCLOUD_ERR_TCP_CONNECT;
    }

    return QCLOUD_ERR_SUCCESS;
}

/**
 * @brief 在该函数中可对服务端证书进行自定义的校验
 *
 * 这种行为发生在握手过程中, 一般是校验连接服务器的主机名与服务器证书中的CN或SAN的域名信息是否一致
 * 不过, mbedtls库已经实现该功能, 可以参考函数 `mbedtls_x509_crt_verify_with_profile`
 *
 * @param hostname 连接服务器的主机名
 * @param crt x509格式的证书
 * @param depth
 * @param flags
 * @return
 */
int _qcloud_server_certificate_verify(void *hostname, mbedtls_x509_crt *crt, int depth, uint32_t *flags) {
    return *flags;
}

uintptr_t HAL_TLS_Connect(TLSConnectParams *pConnectParams, const char *host, int port)
{
    int ret = 0;

    TLSDataParams * pDataParams = (TLSDataParams *)HAL_Malloc(sizeof(TLSDataParams));
    
    if ((ret = _mbedtls_client_init(pDataParams, pConnectParams)) != QCLOUD_ERR_SUCCESS) {
        goto error;
    }

    Log_d("Connecting to /%s/%d...", host, port);
    if ((ret = _mbedtls_tcp_connect(&(pDataParams->socket_fd), host, port)) != QCLOUD_ERR_SUCCESS) {
        goto error;
    }

    Log_d("Setting up the SSL/TLS structure...");
    if ((ret = mbedtls_ssl_config_defaults(&(pDataParams->ssl_conf), MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        Log_e("mbedtls_ssl_config_defaults failed returned 0x%04x", ret<0?-ret:ret);
        goto error;
    }

    mbedtls_ssl_conf_verify(&(pDataParams->ssl_conf), _qcloud_server_certificate_verify, (void *)host);

    mbedtls_ssl_conf_authmode(&(pDataParams->ssl_conf), MBEDTLS_SSL_VERIFY_REQUIRED);

    mbedtls_ssl_conf_rng(&(pDataParams->ssl_conf), mbedtls_ctr_drbg_random, &(pDataParams->ctr_drbg));

    mbedtls_ssl_conf_ca_chain(&(pDataParams->ssl_conf), &(pDataParams->ca_cert), NULL);
    if ((ret = mbedtls_ssl_conf_own_cert(&(pDataParams->ssl_conf),
                                         &(pDataParams->client_cert), &(pDataParams->private_key))) != 0) {
        Log_e("mbedtls_ssl_conf_own_cert failed returned 0x%04x", ret<0?-ret:ret);
        goto error;
    }

    mbedtls_ssl_conf_read_timeout(&(pDataParams->ssl_conf), pConnectParams->timeout_ms);
    if ((ret = mbedtls_ssl_setup(&(pDataParams->ssl), &(pDataParams->ssl_conf))) != 0) {
        Log_e("mbedtls_ssl_setup failed returned 0x%04x", ret<0?-ret:ret);
        goto error;
    }

#ifndef AUTH_MODE_CERT
    // 选择加密套件代码，以后不通加密方式合并端口的时候可以用到
    if(pConnectParams->psk != NULL) {
        mbedtls_ssl_conf_ciphersuites(&(pDataParams->ssl_conf), ciphersuites);
    }
#endif

    // Set the hostname to check against the received server certificate and sni
    if ((ret = mbedtls_ssl_set_hostname(&(pDataParams->ssl), host)) != 0) {
        Log_e("mbedtls_ssl_set_hostname failed returned 0x%04x", ret<0?-ret:ret);
        goto error;
    }

    mbedtls_ssl_set_bio(&(pDataParams->ssl), &(pDataParams->socket_fd), mbedtls_net_send, mbedtls_net_recv,
                        mbedtls_net_recv_timeout);

    Log_d("Performing the SSL/TLS handshake...");
    while ((ret = mbedtls_ssl_handshake(&(pDataParams->ssl))) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            Log_e("mbedtls_ssl_handshake failed returned 0x%04x", ret<0?-ret:ret);
            if (ret == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED) {
                Log_e("Unable to verify the server's certificate");
            }
            goto error;
        }
    }

    if ((ret = mbedtls_ssl_get_verify_result(&(pDataParams->ssl))) != 0) {
        Log_e("mbedtls_ssl_get_verify_result failed returned 0x%04x", ret<0?-ret:ret);
        goto error;
    }

    mbedtls_ssl_conf_read_timeout(&(pDataParams->ssl_conf), 100);

    Log_i("connected with /%s/%d...", host, port);
    
    return (uintptr_t)pDataParams;

error:
    _free_mebedtls(pDataParams);
    return 0;
}

void HAL_TLS_Disconnect(uintptr_t handle) {
    if ((uintptr_t)NULL == handle) {
        Log_d("handle is NULL");
        return;
    }
    TLSDataParams *pParams = (TLSDataParams *)handle;
    int ret = 0;
    do {
        ret = mbedtls_ssl_close_notify(&(pParams->ssl));
    } while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);

    mbedtls_net_free(&(pParams->socket_fd));
    mbedtls_x509_crt_free(&(pParams->client_cert));
    mbedtls_x509_crt_free(&(pParams->ca_cert));
    mbedtls_pk_free(&(pParams->private_key));
    mbedtls_ssl_free(&(pParams->ssl));
    mbedtls_ssl_config_free(&(pParams->ssl_conf));
    mbedtls_ctr_drbg_free(&(pParams->ctr_drbg));
    mbedtls_entropy_free(&(pParams->entropy));
    
    HAL_Free((void *)handle);
}

int HAL_TLS_Write(uintptr_t handle, unsigned char *msg, size_t totalLen, uint32_t timeout_ms,
                                 size_t *written_len)
{

    Timer timer;
    InitTimer(&timer);
    countdown_ms(&timer, (unsigned int) timeout_ms);
    size_t written_so_far;
    bool errorFlag = false;
    int write_rc = 0;
    
    TLSDataParams *pParams = (TLSDataParams *)handle;

    for (written_so_far = 0; written_so_far < totalLen && !expired(&timer); written_so_far += write_rc) {

        while (!expired(&timer) && (write_rc = mbedtls_ssl_write(&(pParams->ssl), msg + written_so_far, totalLen - written_so_far)) <= 0)
        {
            if (write_rc != MBEDTLS_ERR_SSL_WANT_READ && write_rc != MBEDTLS_ERR_SSL_WANT_WRITE) {
                Log_e("HAL_TLS_write failed 0x%04x", write_rc<0?-write_rc:write_rc);
                errorFlag = true;
                break;
            }
        }

        if (errorFlag) {
            break;
        }
    }

    *written_len = written_so_far;

    if (errorFlag) {
        return QCLOUD_ERR_SSL_WRITE;
    } else if (expired(&timer) && written_so_far != totalLen) {
        return QCLOUD_ERR_SSL_WRITE_TIMEOUT;
    }

    return QCLOUD_ERR_SUCCESS;
}

int HAL_TLS_Read(uintptr_t handle, unsigned char *msg, size_t totalLen, uint32_t timeout_ms, size_t *read_len) 
{

    //mbedtls_ssl_conf_read_timeout(&(pParams->ssl_conf), timeout_ms); TODO:每次调用这个方法会导致read阻塞, 超时也不返回
    // 这里使用非阻塞的方式, 具体的超时操作由上层做
    Timer timer;
    InitTimer(&timer);
    countdown_ms(&timer, (unsigned int) timeout_ms);
    *read_len = 0;
    
    TLSDataParams *pParams = (TLSDataParams *)handle;

    do {
        int read_rc = 0;
        read_rc = mbedtls_ssl_read(&(pParams->ssl), msg + *read_len, totalLen - *read_len);

        if (read_rc > 0) {
            *read_len += read_rc;
        } else if (read_rc == 0 || (read_rc != MBEDTLS_ERR_SSL_WANT_WRITE
                                    && read_rc != MBEDTLS_ERR_SSL_WANT_READ && read_rc != MBEDTLS_ERR_SSL_TIMEOUT)) {
            Log_e("cloud_iot_network_tls_read failed: 0x%04x", read_rc<0?-read_rc:read_rc);
            return QCLOUD_ERR_SSL_READ;
        }

        if (expired(&timer)) {
            break;
        }

    } while (*read_len < totalLen);

    if (totalLen == *read_len) {
        return QCLOUD_ERR_SUCCESS;
    }

    if (*read_len == 0) {
        return QCLOUD_ERR_SSL_NOTHING_TO_READ;
    } else {
        return QCLOUD_ERR_SSL_READ_TIMEOUT;
    }
}

#ifdef __cplusplus
}
#endif
