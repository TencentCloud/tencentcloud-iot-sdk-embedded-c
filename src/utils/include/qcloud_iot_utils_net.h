#ifndef qcloud_iot_utils_net_h
#define qcloud_iot_utils_net_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "qcloud_iot_import.h"

/**
 * @brief 网络结构类型
 *
 * 定义一个网络结构类型, 具体定义如下面所示
 */
typedef struct Network Network;

/**
 * @brief 网络操作相关的结构体定义
 *
 * 定义了底层网络相关的操作, 包括连接, 读/写数据, 断开连接等
 */
struct Network {
    int (*connect)(Network *);
    
    int (*read)(Network *, unsigned char *, size_t, int, size_t *);
    
    int (*write)(Network *, unsigned char *, size_t, int, size_t *);
    
    void (*disconnect)(Network *);
    
    int (*is_connected)(Network *);
    
    uintptr_t handle;   // 连接句柄:0，尚未连接; 非0，已经连接
    
    TLSConnectParams tlsConnectParams;
    
};


int utils_net_read(Network *pNetwork, unsigned char *pMsg, size_t len, int timeout_ms, size_t *read_len);
int utils_net_write(Network *pNetwork, unsigned char *pMsg, size_t len, int timeout_ms, size_t *written_len);
void utils_net_disconnect(Network *pNetwork);
int utils_net_connect(Network *pNetwork);
int utils_net_init(Network *pNetwork);
    
#ifdef __cplusplus
}
#endif
#endif /* qcloud_iot_utils_net_h */
