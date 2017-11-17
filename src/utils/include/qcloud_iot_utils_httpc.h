#ifndef QCLOUD_UTILS_HTTPC_H
#define QCLOUD_UTILS_HTTPC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "mqtt_client_net.h"
    

#define HTTP_PORT   80

#define HTTPS_PORT  443


typedef enum {
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT,
    HTTP_DELETE,
    HTTP_HEAD
} HttpRequstType;

typedef struct {
    int         remote_port;        // 端口号
    int         response_code;      // 响应码
    char        *header;            // 自定义头部
    char        *auth_user;         // 身份验证的用户名
    char        *auth_password;     // 身份验证的密码
    Network     network_stack;      
} http_client_t;

typedef struct {
    bool    is_more;                // 是否需要检索更多的数据
    bool    is_chunked;             // 响应数据是否以分块进行编码
    int     retrieve_len;           // 要检索的内容长度
    int     response_content_len;   // 响应内容长度
    int     post_buf_len;           // post data length
    int     response_buf_len;       // 响应包缓冲区长度
    char    *post_content_type;     // post数据的内容类型
    char    *post_buf;              // post的数据
    char    *response_buf;          // 存储响应数据的缓冲区
} http_client_data_t;

/**
 * @brief http 网络请求
 *
 * @param client        http client
 * @param url           请求url
 * @param ca_crt_dir    ca证书路径
 * @param method        请求方法
 * @param timeout_ms    超时时间
 * @param client_data   http数据负载
 * @return              返回QCLOUD_ERR_SUCCESS, 表示设置成功
 */
int qcloud_http_client_common(http_client_t *client, const char *url, int port, const char *ca_crt, int method,
                              uint32_t timeout_ms,
                              http_client_data_t *client_data);
    
/**
 * @brief http get请求
 *
 * @param client        http client
 * @param url           请求url
 * @param ca_crt_dir    ca证书路径
 * @param timeout_ms    超时时间
 * @param client_data   http数据负载
 * @return              返回QCLOUD_ERR_SUCCESS, 表示设置成功
 */
int qcloud_iot_get(http_client_t *client,
                     const char *url,
                     const char *ca_crt_dir,
                     uint32_t timeout_ms,
                     http_client_data_t *client_data);

/**
 * @brief http post请求
 *
 * @param client        http client
 * @param url           请求url
 * @param ca_crt_dir    ca证书路径
 * @param timeout_ms    超时时间
 * @param client_data   http数据负载
 * @return              返回QCLOUD_ERR_SUCCESS, 表示设置成功
 */
int qcloud_iot_post(http_client_t *client,
                      const char *url,
                      const char *ca_crt_dir,
                      uint32_t timeout_ms,
                      http_client_data_t *client_data);


#ifdef __cplusplus
}
#endif
#endif /* QCLOUD_UTILS_HTTPC_H */
