#ifdef __cplusplus
extern "C" {
#endif

#include "qcloud_iot_utils_httpc.h"
    
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
    
#include "qcloud_iot_utils_timer.h"
#include "qcloud_iot_export_log.h"
#include "qcloud_iot_export_error.h"

#define HTTP_CLIENT_MIN(x,y) (((x)<(y))?(x):(y))
#define HTTP_CLIENT_MAX(x,y) (((x)>(y))?(x):(y))

#define HTTP_CLIENT_AUTHB_SIZE     128

#define HTTP_CLIENT_CHUNK_SIZE     1024
#define HTTP_CLIENT_SEND_BUF_SIZE  1024

#define HTTP_CLIENT_MAX_HOST_LEN   64
#define HTTP_CLIENT_MAX_URL_LEN    1024
    
#define HTTP_RETRIEVE_MORE_DATA   (1)
    
#if defined(MBEDTLS_DEBUG_C)
    #define DEBUG_LEVEL 2
#endif
    

static void _http_client_base64enc(char *out, const char *in)
{
    const char code[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    int i = 0, x = 0, l = 0;

    for (; *in; in++) {
        x = x << 8 | *in;
        for (l += 8; l >= 6; l -= 6) {
            out[i++] = code[(x >> (l - 6)) & 0x3f];
        }
    }
    if (l > 0) {
        x <<= 6 - l;
        out[i++] = code[x & 0x3f];
    }
    for (; i % 4;) {
        out[i++] = '=';
    }
    out[i] = '\0';
}

static int _http_client_parse_url(const char *url, char *scheme, uint32_t max_scheme_len, char *host, uint32_t maxhost_len,
    int *port, char *path, uint32_t max_path_len)
{
    char *scheme_ptr = (char *) url;
    char *host_ptr = (char *) strstr(url, "://");
    uint32_t host_len = 0;
    uint32_t path_len;
    
    char *path_ptr;
    char *fragment_ptr;

    if (host_ptr == NULL) {
        Log_e("Could not find host");
        return QCLOUD_ERR_HTTP_PARSE;
    }

    if (max_scheme_len < host_ptr - scheme_ptr + 1) {
        Log_e("Scheme str is too small (%u >= %u)", max_scheme_len, (uint32_t)(host_ptr - scheme_ptr + 1));
        return QCLOUD_ERR_HTTP_PARSE;
    }
    memcpy(scheme, scheme_ptr, host_ptr - scheme_ptr);
    scheme[host_ptr - scheme_ptr] = '\0';

    host_ptr += 3;

    *port = 0;

    path_ptr = strchr(host_ptr, '/');
    if (NULL == path_ptr) {
//        Log_e("http_client_parse_url invalid path");
//        return QCLOUD_ERR_HTTP_PARSE;
        
        // 兼容用户请求url没有输入路径"/"，如：https://www.qq.com , 这里在最后加上"/"：https://www.qq.com/
        path_ptr = scheme_ptr + (int)strlen(url);
        host_len = path_ptr - host_ptr;
        memcpy(host, host_ptr, host_len);
        host[host_len] = '\0';
        
        memcpy(path, "/", 1);
        path[1] = '\0';

        return QCLOUD_ERR_SUCCESS;
    }
    
    if (host_len == 0) {
        host_len = path_ptr - host_ptr;
    }
    
    if (maxhost_len < host_len + 1) {
        Log_e("Host str is too long (host_len(%d) >= max_len(%d))", host_len + 1, maxhost_len);
        return QCLOUD_ERR_HTTP_PARSE;
    }
    memcpy(host, host_ptr, host_len);
    host[host_len] = '\0';
    
    fragment_ptr = strchr(host_ptr, '#');
    if (fragment_ptr != NULL) {
        path_len = fragment_ptr - path_ptr;
    } else {
        path_len = strlen(path_ptr);
    }
    
    if (max_path_len < path_len + 1) {
        Log_e("Path str is too small (%d >= %d)", max_path_len, path_len + 1);
        return QCLOUD_ERR_HTTP_PARSE;
    }
    
    memcpy(path, path_ptr, path_len);
    
    path[path_len] = '\0';

    return QCLOUD_ERR_SUCCESS;
}

static void _str_tolower(const char *source, char *dest) {
    unsigned long len = strlen(source);
    int i;
    for (i = 0; i < len; i++) {
        dest[i] = tolower(source[i]);
    }
}

/**
 * @brief 根据HTTP/HTTPS采取默认端口80/443
 *
 * @param url   请求的url
 * @param port  端口结果
 * @return 返回QCLOUD_ERR_SUCCESS, 表示设置成功
 */
static int _http_client_get_port_with_protocol(const char *url, uint32_t *port) {
    const char *host_ptr = (const char *) strstr(url, "://");
    uint32_t pro_len = 0;
    if (host_ptr == NULL) {
        Log_e("Could not find host");
        return QCLOUD_ERR_HTTP_PARSE;
    }
    char pro_str[6] = {0};
    char low_pro_str[6] = {0};
    pro_len = host_ptr - url;
    strncpy(pro_str, url, pro_len);
    
    _str_tolower(pro_str, low_pro_str);
    
    if (pro_len == 4 && strcmp(low_pro_str, "http") == 0) {
        *port = 80;
    } else if (pro_len == 5 && strcmp(low_pro_str, "https") == 0) {
        *port = 443;
    } else {
        Log_e("http get default port failure");
        return QCLOUD_ERR_HTTP_PARSE;
    }
    return QCLOUD_ERR_SUCCESS;
}

static int _http_client_parse_host(const char *url, char *host, uint32_t host_max_len)
{
    const char *host_ptr = (const char *) strstr(url, "://");
    uint32_t host_len = 0;
    char *path_ptr;

    if (host_ptr == NULL) {
        Log_e("Could not find host");
        return QCLOUD_ERR_HTTP_PARSE;
    }
    host_ptr += 3;
    
    uint32_t pro_len = 0;
    pro_len = host_ptr - url;

    path_ptr = strchr(host_ptr, '/');
    if (host_len == 0) {
        if (path_ptr != NULL)
            host_len = path_ptr - host_ptr;
        else
            host_len = strlen(url) - pro_len;
    }

    if (host_max_len < host_len + 1) {
        Log_e("Host str is too small (%d >= %d)", host_max_len, host_len + 1);
        return QCLOUD_ERR_HTTP_PARSE;
    }
    memcpy(host, host_ptr, host_len);
    host[host_len] = '\0';

    return QCLOUD_ERR_SUCCESS;
}

int httpclient_conn(http_client_t *client)
{
    if (0 != client->network_stack.connect(&client->network_stack)) {
        Log_e("establish connection failed");
        return QCLOUD_ERR_FAILURE;
    }
    
    return QCLOUD_ERR_SUCCESS;
}

/**
 * @brief 拼接发送的数据
 *
 * @param client    http client
 * @param send_buf  发送数据buffer
 * @param send_idx  标志send_buf数据结束的位置
 * @param buf       需要被发送的数据，拼接到send_buf中
 * @param len       buf的长度
 * @return 返回QCLOUD_ERR_SUCCESS, 表示设置成功
 */
static int _http_client_get_info(http_client_t *client, unsigned char *send_buf, int *send_idx, char *buf, uint32_t len)
{
    int rc = QCLOUD_ERR_SUCCESS;
    int cp_len;
    int idx = *send_idx;

    if (len == 0) {
        len = strlen(buf);
    }

    do {
        if ((HTTP_CLIENT_SEND_BUF_SIZE - idx) >= len) {
            cp_len = len;
        } else {
            cp_len = HTTP_CLIENT_SEND_BUF_SIZE - idx;
        }

        memcpy(send_buf + idx, buf, cp_len);
        idx += cp_len;
        len -= cp_len;

        if (idx == HTTP_CLIENT_SEND_BUF_SIZE) {
            size_t byte_written_len = 0;
            rc = client->network_stack.write(&(client->network_stack), send_buf, HTTP_CLIENT_SEND_BUF_SIZE, 5000, &byte_written_len);
            if (byte_written_len) {
                return (byte_written_len);
            }
        }
    } while (len);

    *send_idx = idx;
    return rc;
}
    
/*
static void _http_client_set_custom_header(http_client_t *client, char *header)
{
    client->header = header;
}

static int _http_client_basic_auth(http_client_t *client, char *user, char *password)
{
    if ((strlen(user) + strlen(password)) >= HTTP_CLIENT_AUTHB_SIZE) {
        return QCLOUD_ERR_FAILURE;
    }
    client->auth_user = user;
    client->auth_password = password;
    return QCLOUD_ERR_SUCCESS;
}
*/

static int _http_client_send_auth(http_client_t *client, unsigned char *send_buf, int *send_idx)
{
    char b_auth[(int)((HTTP_CLIENT_AUTHB_SIZE + 3) * 4 / 3 + 1)];
    char base64buff[HTTP_CLIENT_AUTHB_SIZE + 3];

    _http_client_get_info(client, send_buf, send_idx, "Authorization: Basic ", 0);
    sprintf(base64buff, "%s:%s", client->auth_user, client->auth_password);
    Log_d("bAuth: %s", base64buff) ;
    _http_client_base64enc(b_auth, base64buff);
    b_auth[strlen(b_auth) + 1] = '\0';
    b_auth[strlen(b_auth)] = '\n';
    Log_d("b_auth:%s", b_auth) ;
    _http_client_get_info(client, send_buf, send_idx, b_auth, 0);
    return QCLOUD_ERR_SUCCESS;
}
    
/**
 * @brief 根据请求url和method，拼接请求头
 *
 * @param client        http client
 * @param url           请求url
 * @param method        请求方法
 * @param client_data   http数据负载
 * @return              返回QCLOUD_ERR_SUCCESS, 表示设置成功
 */
static int _http_client_send_header(http_client_t *client, const char *url, int method, http_client_data_t *client_data)
{
    char scheme[8] = { 0 };
    char host[HTTP_CLIENT_MAX_HOST_LEN] = { 0 };
    char path[HTTP_CLIENT_MAX_URL_LEN] = { 0 };
    int len;
    unsigned char send_buf[HTTP_CLIENT_SEND_BUF_SIZE] = { 0 };
    char buf[HTTP_CLIENT_SEND_BUF_SIZE] = { 0 };
    char *meth = (method == HTTP_GET) ? "GET" : (method == HTTP_POST) ? "POST" :
                 (method == HTTP_PUT) ? "PUT" : (method == HTTP_DELETE) ? "DELETE" :
                 (method == HTTP_HEAD) ? "HEAD" : "";
    int rc;
    int port;
    
    int res = _http_client_parse_url(url, scheme, sizeof(scheme), host, sizeof(host), &port, path, sizeof(path));
    if (res != QCLOUD_ERR_SUCCESS) {
        Log_e("httpclient_parse_url returned %d", res);
        return res;
    }
    
    if (strcmp(scheme, "http") == 0) {
        
    } else if (strcmp(scheme, "https") == 0) {
        
    }
    
    memset(send_buf, 0, HTTP_CLIENT_SEND_BUF_SIZE);
    len = 0;
    
    sprintf (buf, "%s %s HTTP/1.1\r\nHost: %s\r\n", meth, path, host);
    rc = _http_client_get_info(client, send_buf, &len, buf, strlen(buf));
    if (rc) {
        Log_e("Could not write request");
        return QCLOUD_ERR_HTTP_CONN;
    }
    
    if (client->auth_user) {
        _http_client_send_auth(client, send_buf, &len);
    }
    
    if (client->header) {
        _http_client_get_info(client, send_buf, &len, (char *) client->header, strlen(client->header));
    }
    
    if (client_data->post_buf != NULL && HTTP_GET != method) {
        sprintf(buf, "Content-Length: %d\r\n", client_data->post_buf_len);
        _http_client_get_info(client, send_buf, &len, buf, strlen(buf));
        
        if (client_data->post_content_type != NULL) {
            sprintf(buf, "Content-Type: %s\r\n", client_data->post_content_type);
            _http_client_get_info(client, send_buf, &len, buf, strlen(buf));
        }
    }
    
    _http_client_get_info(client, send_buf, &len, "\r\n", 0);
    
    Log_d("REQUEST:\n%s", send_buf);
    size_t written_len = 0;
    rc = client->network_stack.write(&client->network_stack, send_buf, len, 5000, &written_len);
    if (written_len > 0) {
        Log_d("Written %lu bytes", written_len);
    } else if (written_len == 0) {
        Log_e("rc == 0,Connection was closed by server");
        return QCLOUD_ERR_HTTP_CLOSED; /* Connection was closed by server */
    } else {
        Log_e("Connection error (send returned %lu)", written_len);
        return QCLOUD_ERR_HTTP_CONN;
    }
    
    return QCLOUD_ERR_SUCCESS;
}
    
/**
 * @brief 发送post的请求体数据
 *
 * @param client        http client
 * @param client_data   http数据负载
 * @return              返回QCLOUD_ERR_SUCCESS, 表示设置成功
 */
static int _http_client_send_userdata(http_client_t *client, http_client_data_t *client_data)
{
    int rc = 0;
    
    if (client_data->post_buf && client_data->post_buf_len) {
        Log_d("client_data->post_buf: %s", client_data->post_buf);
        {
            size_t written_len = 0;
            rc = client->network_stack.write(&client->network_stack, (unsigned char *)client_data->post_buf, client_data->post_buf_len, 5000,  &written_len);
            if (written_len > 0) {
                Log_d("Written %d bytes", rc);
            } else if (written_len == 0) {
                Log_e("written_len == 0,Connection was closed by server");
                return QCLOUD_ERR_HTTP_CLOSED;
            } else {
                Log_e("Connection error (send returned %lu)", written_len);
                return QCLOUD_ERR_HTTP_CONN;
            }
        }
    }
    
    return QCLOUD_ERR_SUCCESS;
}
    
/**
 * @brief 读取http数据
 *
 * @param client        http client
 * @param buf           数据buffer
 * @param min_len       读取数据的最小长度
 * @param max_len       读取数据的最大长度
 * @param p_read_len    成功读取到的数据的长度
 * @param timeout_ms    超时时间
 * @param client_data   http数据负载
 * @return              返回QCLOUD_ERR_SUCCESS, 表示设置成功
 */
static int _http_client_recv(http_client_t *client, char *buf, int min_len, int max_len, int *p_read_len, uint32_t timeout_ms)
{
    int rc = 0;
    Timer timer;
    
    InitTimer(&timer);
    countdown_ms(&timer, timeout_ms);
    
    *p_read_len = 0;
    
    rc = client->network_stack.read(&client->network_stack, (unsigned char *)buf, max_len, left_ms(&timer), (size_t *)p_read_len);
    
    if (*p_read_len > 0) {
//        *p_read_len = rc;
    } else if (*p_read_len == 0) {
        return 0;
    } else if (-1 == rc) {
        Log_i("Connection closed.");
        return QCLOUD_ERR_HTTP_CONN;
    } else if (rc != 0) {
        Log_e("Connection error rc = %d (recv returned %d)", rc, *p_read_len);
        return QCLOUD_ERR_HTTP_CONN;
    }
    Log_d("%u bytes has been read", *p_read_len);
    return QCLOUD_ERR_SUCCESS;
}
    
static int _http_client_retrieve_content(http_client_t *client, char *data, int len, uint32_t timeout_ms,
                                             http_client_data_t *client_data)
{
    int count = 0;
    int templen = 0;
    int crlf_pos;
    Timer timer;
    
    InitTimer(&timer);
    countdown_ms(&timer, timeout_ms);
    
    /* Receive data */
    Log_d("Current data: %s", data);
    
    client_data->is_more = true;
    
    if (client_data->response_content_len == -1 && client_data->is_chunked == false) {
        while (true) {
            int rc, max_len;
            if (count + len < client_data->response_buf_len - 1) {
                memcpy(client_data->response_buf + count, data, len);
                count += len;
                client_data->response_buf[count] = '\0';
            } else {
                memcpy(client_data->response_buf + count, data, client_data->response_buf_len - 1 - count);
                client_data->response_buf[client_data->response_buf_len - 1] = '\0';
                return HTTP_RETRIEVE_MORE_DATA;
            }
            
            max_len = HTTP_CLIENT_MIN(HTTP_CLIENT_CHUNK_SIZE - 1, client_data->response_buf_len - 1 - count);
            rc = _http_client_recv(client, data, 1, max_len, &len, left_ms(&timer));
            
            /* Receive data */
            Log_d("data len: %d %d", len, count);
            
            if (rc == QCLOUD_ERR_HTTP_CONN) {
                Log_d("_http_client_recv error");
                return rc;
            }
            
            if (len == 0) {
                /* read no more data */
                Log_d("no more len == 0");
                client_data->is_more = false;
                return QCLOUD_ERR_SUCCESS;
            }
        }
    }
    
    while (true) {
        uint32_t readLen = 0;
        
        if (client_data->is_chunked && client_data->retrieve_len <= 0) {
            /* Read chunk header */
            bool foundCrlf;
            int n;
            do {
                foundCrlf = false;
                crlf_pos = 0;
                data[len] = 0;
                if (len >= 2) {
                    for (; crlf_pos < len - 2; crlf_pos++) {
                        if (data[crlf_pos] == '\r' && data[crlf_pos + 1] == '\n') {
                            foundCrlf = true;
                            break;
                        }
                    }
                }
                if (!foundCrlf) {
                    /* Try to read more */
                    if (len < HTTP_CLIENT_CHUNK_SIZE) {
                        int new_trf_len, rc;
                        rc = _http_client_recv(client,
                                                data + len,
                                                0,
                                                HTTP_CLIENT_CHUNK_SIZE - len - 1,
                                                &new_trf_len,
                                                left_ms(&timer));
                        len += new_trf_len;
                        if (rc == QCLOUD_ERR_HTTP_CONN) {
                            return rc;
                        } else {
                            continue;
                        }
                    } else {
                        return QCLOUD_ERR_HTTP;
                    }
                }
            } while (!foundCrlf);
            data[crlf_pos] = '\0';
            n = sscanf(data, "%x", &readLen);/* chunk length */
            client_data->retrieve_len = readLen;
            client_data->response_content_len += client_data->retrieve_len;
            if (n != 1) {
                Log_e("Could not read chunk length");
                return QCLOUD_ERR_HTTP_UNRESOLVED_DNS;
            }
            
            memmove(data, &data[crlf_pos + 2], len - (crlf_pos + 2));
            len -= (crlf_pos + 2);
            
            if (readLen == 0) {
                /* Last chunk */
                client_data->is_more = false;
                Log_d("no more (last chunk)");
                break;
            }
        } else {
            readLen = client_data->retrieve_len;
        }
        
        Log_d("Total-Payload: %d Bytes; Read: %d Bytes", readLen, len);
        
        do {
            templen = HTTP_CLIENT_MIN(len, readLen);
            if (count + templen < client_data->response_buf_len - 1) {
                memcpy(client_data->response_buf + count, data, templen);
                count += templen;
                client_data->response_buf[count] = '\0';
                client_data->retrieve_len -= templen;
            } else {
                memcpy(client_data->response_buf + count, data, client_data->response_buf_len - 1 - count);
                client_data->response_buf[client_data->response_buf_len - 1] = '\0';
                client_data->retrieve_len -= (client_data->response_buf_len - 1 - count);
                return HTTP_RETRIEVE_MORE_DATA;
            }
            
            if (len > readLen) {
                Log_d("memmove %d %d %d\n", readLen, len, client_data->retrieve_len);
                memmove(data, &data[readLen], len - readLen); /* chunk case, read between two chunks */
                len -= readLen;
                readLen = 0;
                client_data->retrieve_len = 0;
            } else {
                readLen -= len;
            }
            
            if (readLen) {
                int rc;
                int max_len = HTTP_CLIENT_MIN(HTTP_CLIENT_CHUNK_SIZE - 1, client_data->response_buf_len - 1 - count);
                max_len = HTTP_CLIENT_MIN(max_len, readLen);
                rc = _http_client_recv(client, data, 1, max_len, &len, left_ms(&timer));
                if (rc == QCLOUD_ERR_HTTP_CONN) {
                    return rc;
                }
            }
        } while (readLen);
        
        if (client_data->is_chunked) {
            if (len < 2) {
                int new_trf_len, rc;
                /* Read missing chars to find end of chunk */
                rc = _http_client_recv(client, data + len, 2 - len, HTTP_CLIENT_CHUNK_SIZE - len - 1, &new_trf_len,
                                        left_ms(&timer));
                if (rc == QCLOUD_ERR_HTTP_CONN) {
                    return rc;
                }
                len += new_trf_len;
            }
            // TODO: 解决第二块chunk不能正常解析的问题
            if (len) {
                int new_trf_len, rc;
                rc = _http_client_recv(client, data, 1, HTTP_CLIENT_CHUNK_SIZE - 1, &new_trf_len,
                                       left_ms(&timer));
                
                data[new_trf_len] = 0;
                
                if (rc == QCLOUD_ERR_HTTP_CONN) {
                    return rc;
                }
                len = new_trf_len;
            }
            
            if ((data[0] != '\r') || (data[1] != '\n')) {
                Log_e("Format error, %s", data); /* after memmove, the beginning of next chunk */
                return QCLOUD_ERR_HTTP_UNRESOLVED_DNS;
            }
            memmove(data, &data[2], len - 2); /* remove the \r\n */
            len -= 2;
        } else {
            Log_d("no more (content-length)");
            client_data->is_more = false;
            break;
        }
        
    }
    
    return QCLOUD_ERR_SUCCESS;
}
    
/**
 * @brief 解析response body数据
 *
 * @param client        http_client_t数据
 * @param data          读取到的数据
 * @param len           读取到的数据的长度
 * @param timeout_ms    读取数据的超时时间
 * @param client_data   http数据负载
 * @return              返回QCLOUD_ERR_SUCCESS, 表示设置成功
 */
static int _http_client_response_parse(http_client_t *client, char *data, int len, uint32_t timeout_ms, http_client_data_t *client_data)
{
    int crlf_pos;
    Timer timer;
    
    InitTimer(&timer);
    countdown_ms(&timer, timeout_ms);
    
    client_data->response_content_len = -1;
    
    char *crlf_ptr = strstr(data, "\r\n");
    if (crlf_ptr == NULL) {
        Log_e("\\r\\n not found");
        return QCLOUD_ERR_HTTP_UNRESOLVED_DNS;
    }
    
    crlf_pos = crlf_ptr - data;
    data[crlf_pos] = '\0';
    
    if (sscanf(data, "HTTP/%*d.%*d %d %*[^\r\n]", &(client->response_code)) != 1) {
        Log_e("Not a correct HTTP answer : %s\n", data);
        return QCLOUD_ERR_HTTP_UNRESOLVED_DNS;
    }
    
    if ((client->response_code < 200) || (client->response_code >= 400)) {
        Log_w("Response code %d", client->response_code);
    }
    
    Log_d("Reading headers : %s", data);
    
    // 移除null终止字符
    memmove(data, &data[crlf_pos + 2], len - (crlf_pos + 2) + 1);
    len -= (crlf_pos + 2);
    
    client_data->is_chunked = false;
    
    while (true) {
        char key[32];
        char value[32];
        int n;
        
        key[31] = '\0';
        value[31] = '\0';
        
        crlf_ptr = strstr(data, "\r\n");
        if (crlf_ptr == NULL) {
            if (len < HTTP_CLIENT_CHUNK_SIZE - 1) {
                int new_trf_len, rc;
                rc = _http_client_recv(client, data + len, 1, HTTP_CLIENT_CHUNK_SIZE - len - 1, &new_trf_len, left_ms(&timer));
                len += new_trf_len;
                data[len] = '\0';
                Log_d("Read %d chars; In buf: [%s]", new_trf_len, data);
                if (rc == QCLOUD_ERR_HTTP_CONN) {
                    return rc;
                } else {
                    continue;
                }
            } else {
                Log_d("header len(%d) > chunksize(%d)| data: %s", len, HTTP_CLIENT_CHUNK_SIZE - 1, data);
                return QCLOUD_ERR_HTTP;
            }
        }
        
        crlf_pos = crlf_ptr - data;
        if (crlf_pos == 0) {
            // 响应头的终结点
            // 移除null终止字符
            memmove(data, &data[2], len - 2 + 1);
            len -= 2;
            break;
        }
        
        data[crlf_pos] = '\0';
        
        n = sscanf(data, "%31[^:]: %31[^\r\n]", key, value);
        if (n == 2) {
            Log_d("Read header : %s: %s", key, value);
            if (!strcmp(key, "Content-Length")) {
                sscanf(value, "%d", &(client_data->response_content_len));
                client_data->retrieve_len = client_data->response_content_len;
            } else if (!strcmp(key, "Transfer-Encoding")) {
                if (!strcmp(value, "Chunked") || !strcmp(value, "chunked")) {
                    client_data->is_chunked = true;
                    client_data->response_content_len = 0;
                    client_data->retrieve_len = 0;
                }
            }
            // 移除null终止字符
            memmove(data, &data[crlf_pos + 2], len - (crlf_pos + 2) + 1);
            len -= (crlf_pos + 2);
            
        } else {
            Log_e("Could not parse header");
            return QCLOUD_ERR_HTTP;
        }
    }
    
    return _http_client_retrieve_content(client, data, len, left_ms(&timer), client_data);
}

static int _http_client_connect(http_client_t *client)
{
    int rc = QCLOUD_ERR_SUCCESS;
    if (QCLOUD_ERR_SUCCESS != client->network_stack.connect(&client->network_stack)) {
        Log_e("establish connection failed");
        return QCLOUD_ERR_HTTP_CONN;
    }

    IOT_FUNC_EXIT_RC(rc);
}

static int _http_client_send_request(http_client_t *client, const char *url, int method, http_client_data_t *client_data)
{    
    int rc = QCLOUD_ERR_HTTP_CONN;

    if (0 == client->network_stack.handle) {
        Log_d("not connection have been established");
        return rc;
    }

    rc = _http_client_send_header(client, url, method, client_data);
    if (rc != 0) {
        Log_e("httpclient_send_header is error,rc = %d", rc);
        return rc;
    }

    if (method == HTTP_POST || method == HTTP_PUT) {
        rc = _http_client_send_userdata(client, client_data);
    }

    return rc;
}
    
    
/**
 * @brief 接收http返回的数据
 *
 * @param client        http client
 * @param timeout_ms    读取数据的超时时间
 * @param client_data   http数据负载
 * @return              返回QCLOUD_ERR_SUCCESS, 表示设置成功
 */
static int _http_client_recv_response(http_client_t *client, uint32_t timeout_ms, http_client_data_t *client_data)
{
    int reclen = 0, rc = QCLOUD_ERR_HTTP_CONN;
    char buf[HTTP_CLIENT_CHUNK_SIZE] = { 0 };
    Timer timer;
    
    InitTimer(&timer);
    countdown_ms(&timer, timeout_ms);
    
    if (0 == client->network_stack.handle) {
        Log_d("not connection have been established");
        return rc;
    }
    
    if (client_data->is_more) {
        client_data->response_buf[0] = '\0';
        rc = _http_client_retrieve_content(client, buf, reclen, left_ms(&timer), client_data);
    } else {
        client_data->is_more = 1;
        rc = _http_client_recv(client, buf, 1, HTTP_CLIENT_CHUNK_SIZE - 1, &reclen, left_ms(&timer));
        if (rc != 0) {
            return rc;
        }
        
        buf[reclen] = '\0';
        
        if (reclen) {
            Log_d("RESPONSE:\n%s", buf);
            rc = _http_client_response_parse(client, buf, reclen, left_ms(&timer), client_data);
        }
    }
    
    return rc;
}

static void _http_client_close(http_client_t *client)
{
    if (client->network_stack.handle != 0) {
        client->network_stack.disconnect(&client->network_stack);
    }
}
    
static void _get_ip_addr(const char *host_name, char *ip_addr)
{
    /*通过域名得到相应的ip地址*/
    struct hostent *host = gethostbyname(host_name);//此函数将会访问DNS服务器
    if (!host)
    {
        ip_addr = NULL;
        return;
    }
    
    int i;
    for (i = 0; host->h_addr_list[i]; i++)
    {
        strcpy(ip_addr, inet_ntoa( * (struct in_addr*) host->h_addr_list[i]));
        break;
    }
}

//static int _utils_get_response_code(http_client_t *client)
//{
//    return client->response_code;
//}
    
static int _qcloud_iot_http_network_init(Network *pNetwork, const char *host, int port, const char *ca_crt_dir)
{
    int rc = QCLOUD_ERR_SUCCESS;
    if (pNetwork == NULL) {
        return QCLOUD_ERR_INVAL;
    }
    
    pNetwork->tlsConnectParams.host = host;
    pNetwork->tlsConnectParams.port = port;
    pNetwork->tlsConnectParams.ca_file = ca_crt_dir;
    pNetwork->tlsConnectParams.is_asymc_encryption = true;
    
    rc = utils_net_init(pNetwork);
    
    return rc;
}
    
int qcloud_http_client_common(http_client_t *client, const char *url, int port, const char *ca_crt_dir, int method,
                              uint32_t timeout_ms,
                              http_client_data_t *client_data)
{
    Timer timer;
    int rc = 0;
    char host[HTTP_CLIENT_MAX_HOST_LEN] = {0};
    if (client->network_stack.handle == 0) {
        _http_client_parse_host(url, host, sizeof(host));
        
        char ip_addr[16] = {0};
        _get_ip_addr(host, ip_addr);
        
        rc = _qcloud_iot_http_network_init(&client->network_stack, ip_addr, port, ca_crt_dir);
        if (rc != QCLOUD_ERR_SUCCESS)
            return rc;

        rc = _http_client_connect(client);
        if (0 != rc) {
            Log_e("http_client_connect is error,rc = %d", rc);
            _http_client_close(client);
            return rc;
        }

        rc = _http_client_send_request(client, url, method, client_data);
        if (QCLOUD_ERR_SUCCESS != rc) {
            Log_e("http_client_send_request is error,rc = %d", rc);
            _http_client_close(client);
            return rc;
        }
    }
    InitTimer(&timer);
    countdown_ms(&timer, (unsigned int) timeout_ms);
    
    if ((NULL != client_data->response_buf)
        && (0 != client_data->response_buf_len)) {
        rc = _http_client_recv_response(client, left_ms(&timer), client_data);
        if (rc < 0) {
            Log_e("http_client_recv_response is error,rc = %d", rc);
            _http_client_close(client);
            return rc;
        }
    }
    Log_d("response_buf:\n%s", client_data->response_buf);
    if (! client_data->is_more) {
        Log_d("close http channel");
        _http_client_close(client);
    }
    
    return 0;
}
    
int qcloud_iot_get(http_client_t *client,
                    const char *url,
                    const char *ca_crt_dir,
                    uint32_t timeout_ms,
                    http_client_data_t *client_data)
{
    uint32_t port;
    int rc;
    rc = _http_client_get_port_with_protocol(url, &port);
    if (rc != QCLOUD_ERR_SUCCESS) {
        return rc;
    }
    return qcloud_http_client_common(client, url, port, ca_crt_dir, HTTP_GET, timeout_ms, client_data);
}

int qcloud_iot_post(http_client_t *client,
                     const char *url,
                     const char *ca_crt_dir,
                     uint32_t timeout_ms,
                     http_client_data_t *client_data)
{
    uint32_t port;
    int rc;
    rc = _http_client_get_port_with_protocol(url, &port);
    if (rc != QCLOUD_ERR_SUCCESS) {
        return rc;
    }
    return qcloud_http_client_common(client, url, port, ca_crt_dir, HTTP_POST, timeout_ms, client_data);
}

    
#ifdef __cplusplus
}
#endif
