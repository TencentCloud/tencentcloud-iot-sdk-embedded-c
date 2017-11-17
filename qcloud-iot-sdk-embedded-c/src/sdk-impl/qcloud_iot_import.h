#ifndef __QCLOUD_IOT_IMPORT_H__
#define __QCLOUD_IOT_IMPORT_H__
#if defined(__cplusplus)
extern "C" {
#endif
    
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
    
#define _IN_            /* 表明这是一个输入参数. */
#define _OU_            /* 表明这是一个输出参数. */

/**
 * @brief 创建互斥锁
 *
 * @return 创建失败返回NULL，成功返回Mutex指针
 */
void *HAL_MutexCreate(void);

/**
 * @brief 销毁互斥锁
 *
 * @param Mutex指针
 */
void HAL_MutexDestroy(_IN_ void *mutex);

/**
 * @brief 加锁
 *
 * @param Mutex指针
 */
void HAL_MutexLock(_IN_ void *mutex);

/**
 * @brief 释放锁
 *
 * @param Mutex指针
 */
void HAL_MutexUnlock(_IN_ void *mutex);
    
/**
 * @brief 分配一块的内存，返回一个指向块开始的指针.
 *
 * @param size   用字节指定块大小.
 * @return       一个指向block开头的指针.
 */
void *HAL_Malloc(_IN_ uint32_t size);
    
/**
 * @brief 释放内存块
 *
 * @param ptr   指向先前分配给平台malloc的内存块的指针.
 */
void HAL_Free(_IN_ void *ptr);
    
/**
 * @brief 将格式化的数据写入标准输出流中.
 *
 * @param fmt   要写入的文本的字符串, 它可以选择包含嵌入的格式指定符, 它指定了随后的参数如何转换为输出.
 * @param ...   变量参数列表.
 */
void HAL_Printf(_IN_ const char *fmt, ...);

/**
 * @brief 将格式化的数据写入字符串.
 *
 * @param str   目标字符串.
 * @param len   将被写入字符的最大长度
 * @param fmt   要编写的文本的格式，它可以选择包含嵌入的格式指定符, 它指定了随后的参数如何转换为输出.
 * @param ...   变量参数列表，用于格式化并插入到生成的字符串中，替换它们各自的指定符.
 * @return      成功写入字符串的字节数.
 */
int HAL_Snprintf(_IN_ char *str, const int len, const char *fmt, ...);
    
/**
 * @brief 检索自系统启动以来已运行的毫秒数.
 *
 * @return 返回毫秒数.
 */
uint32_t HAL_UptimeMs(void);

/**
 * @brief 休眠.
 *
 * @param ms 休眠的时长, 单位毫秒.
 */
void HAL_SleepMs(_IN_ uint32_t ms);
    
/**
 * 定义特定平台下的一个定时器结构体,
 */
struct Timer {
    struct timeval end_time;
};

typedef struct Timer Timer;

/**
 * @brief 判断定时器时间是否已经过期
 *
 * @param timer 定时器结构体
 * @return 返回1, 表示定时器已过期
 */
char HAL_Timer_expired(Timer *timer);

/**
 * @brief 根据定时器开始计时, 单位:ms
 *
 * @param timer 定时器结构体
 * @param timeout_ms 超时时间, 单位:ms
 */
void HAL_Timer_countdown_ms(Timer *timer, unsigned int timeout_ms);

/**
 * @brief 根据定时器开始计时, 单位:ms
 *
 * @param timer   定时器结构体
 * @param timeout 超时时间, 单位:s
 */
void HAL_Timer_countdown(Timer *timer, unsigned int timeout);

/**
 * @brief 检查给定定时器还剩下多少时间
 *
 * @param timer 定时器结构体
 * @return 返回剩余时间
 */
int HAL_Timer_remain(Timer *timer);

/**
 * @brief 初始化定时器结构体
 *
 * @param timer 定时器结构体
 */
void HAL_Timer_init(Timer *timer);

/**
 * @brief 获取当前时间格式化字符串 %Y-%m-%d %z %H:%M:%S
 *
 * @return 当前时间格式化字符串
 */
char* HAL_Timer_current(void);
    

/********** network **********/

/**
 * @brief TLS连接相关参数定义
 *
 * 在初始化时, 必须要将ca证书、客户端证书、客户端私钥文件及服务器域名带上来
 */
typedef struct {
	/**
	 * 非对称加密
	 */
    const char       *ca_file;              // ca证书文件路径, 用于认证服务端证书
    const char       *cert_file;            // 客户端证书
    const char       *key_file;             // 客户端私钥

    /**
     * 对称加密
     */
    const char       *psk;                  // 对称加密密钥
    const char       *psk_id;               // psk密钥ID
    size_t           psk_length;            // psk长度

    const char       *host;                 // 服务器地址
    int              port;                  // 服务器端口

    unsigned int     timeout_ms;            // SSL握手超时时间

    bool             is_asymc_encryption;   // 加密方式 0:对称加密 1:非对称加密

} TLSConnectParams;

/**
 * @brief 为MQTT客户端建立TLS连接
 *
 * 主要步骤如下:
 *     1. 初始化工作, 例如mbedtls库初始化, 相关证书文件加载等
 *     2. 建立TCP socket连接
 *     3. 建立SSL连接, 包括握手, 服务器证书检查等
 *
 * @param pConnectParams TLS连接初始化参数
 * @param pDataParams    TLS连接相关数据结构
 * @return  返回0 表示TLS连接成功
 */
uintptr_t HAL_TLS_Connect(TLSConnectParams *pConnectParams);

/**
 * @brief 断开TLS连接, 并释放相关对象资源
 *
 * @param pParams TLS连接参数
 */
void HAL_TLS_Disconnect(uintptr_t handle);

/**
 * @brief 通过TLS连接写数据
 *
 * @param pParams     TLS连接相关数据结构
 * @param msg         写入数据
 * @param totalLen    写入数据长度
 * @param timeout_ms  超时时间, 单位:ms
 * @param written_len 已写入数据长度
 * @return 若写数据成功, 则返回写入数据的长度
 */
int HAL_TLS_Write(uintptr_t handle, unsigned char *msg, size_t totalLen, int timeout_ms,
                                 size_t *written_len);

/**
 * @brief 通过TLS连接读数据
 *
 * @param pParams    TLS连接相关数据结构
 * @param msg        读取数据
 * @param totalLen   读取数据的长度
 * @param timeout_ms 超时时间, 单位:ms
 * @param read_len   已读取数据的长度
 * @return 若读数据成功, 则返回读取数据的长度
 */
int HAL_TLS_Read(uintptr_t handle, unsigned char *msg, size_t totalLen, int timeout_ms,
                                size_t *read_len);    
    
#if defined(__cplusplus)
}
#endif
#endif  /* __QCLOUD_IOT_IMPORT_H__ */

