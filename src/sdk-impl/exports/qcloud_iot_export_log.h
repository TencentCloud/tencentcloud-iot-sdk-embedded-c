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

#ifndef QCLOUD_IOT_EXPORT_LOG_H_
#define QCLOUD_IOT_EXPORT_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

/**
 * 日志输出等级
 * 设置为DISABLE级别时表示不打印或不上传
 */
typedef enum {
    DISABLE = 0,
    ERROR = 1,
    WARN = 2,
    INFO = 3,
    DEBUG = 4
} LOG_LEVEL;

/**
 * 全局日志打印级别标志, 只打印输出小于或等于该等级的日志信息
 */
extern LOG_LEVEL g_log_print_level;

/**
 * 全局日志上传级别标志, 只上传小于或等于该等级的日志信息
 */
extern LOG_LEVEL g_log_upload_level;

/*用户自定义日志打印函数回调*/
typedef bool (*LogMessageHandler)(const char* message);

/*单条日志最大长度*/
#define MAX_LOG_MSG_LEN 			(255)

/* 
 * 日志上报功能相关参数， 影响内存和存储空间使用量及上报频率
 * 以下为默认推荐值
 */
// 日志上报缓冲区大小，用户可以根据需要调整，但应小于或等于MAX_HTTP_LOG_POST_SIZE
#define LOG_UPLOAD_BUFFER_SIZE      3000    
// 一次日志上报的最大post payload长度，不可修改
#define MAX_HTTP_LOG_POST_SIZE      5000
//日志上报失败后通过缓存到非易失性存储区的最大长度，可根据需要调整
#define MAX_LOG_SAVE_SIZE           (3*LOG_UPLOAD_BUFFER_SIZE)
//日志上报的频率，若上报缓冲区比较小，则需要提高上报频率，否则容易丢失日志
#define LOG_UPLOAD_INTERVAL_MS      2000    //milliseconds

/* 日志上报服务器地址，不可修改*/
#define LOG_UPLOAD_SERVER_URL     "http://devicelog.iot.cloud.tencent.com/cgi-bin/report-log"
#define LOG_UPLOAD_SERVER_PORT    80

/**
 * @brief 日志上报功能用户自定义回调函数，用于上报失败时的缓存和通讯恢复后的重传
 */
//缓存指定长度日志到非易失性存储，返回值为成功写入的长度
typedef size_t (*LogSaveFunc)(const char *msg, size_t wLen);
//从非易失性存储读取指定长度日志，返回值为成功读取的长度
typedef size_t (*LogReadFunc)(char *buff, size_t rLen);
//从非易失性存储删除缓存的日志，返回值为0删除成功，非0删除失败
typedef int (*LogDelFunc)();
//获取存储在非易失性存储中的log长度，返回0为没有缓存
typedef size_t (*LogGetSizeFunc)();

/**
 * @brief 日志上报功能初始化数据结构
 */
typedef struct {
    /*物联网产品信息*/
    const char      *product_id;
    const char      *device_name;
    /*校验key，采用PSK方式请传入产品密钥，证书方式请传入客户端证书全路径*/
    const char      *sign_key;
    /*用户自定义回调函数*/
    LogSaveFunc     save_func; 
    LogReadFunc     read_func; 
    LogDelFunc      del_func;
    LogGetSizeFunc  get_size_func;
} LogUploadInitParams;


/**
 * @brief 设置全局日志打印级别
 *
 * @param level 
 */
void IOT_Log_Set_Level(LOG_LEVEL level);

/**
 * @brief 获取当前日志打印等级
 *
 * @return
 */
LOG_LEVEL IOT_Log_Get_Level();

/**
 * @brief 设置全局日志上传级别
 *
 * @param level
 */
void IOT_Log_Set_Upload_Level(LOG_LEVEL level);

/**
 * @brief 获取当前日志上传等级
 *
 * @return
 */
LOG_LEVEL IOT_Log_Get_Upload_Level();


/**
 * @brief 设置日志回调函数，用户接管日志内容用于输出到文件等操作
 *
 * @param handler 回调函数指针
 *
 */
void IOT_Log_Set_MessageHandler(LogMessageHandler handler);


/**
 * @brief 设置日志上报功能相关信息
 *
 * @param product_id 产品ID
 * @param device_name 设备名称
 * @param sign_key 用于签名的key
 *
 */
void IOT_Log_Init_Uploader(LogUploadInitParams *init_params);


/**
 * @brief 触发一次日志上报
 *
 * @param force_upload 为真则强制进行上报，否则会根据预设间隔定时上报
 * @return 返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int IOT_Log_Upload(bool force_upload);


/**
 * @brief 日志打印及上报函数，默认打印到标准输出，当用户设置日志打印handler时，回调handler
 *
 * 当日志上报功能打开时，会将日志缓存后上报到云端日志服务器
 *
 * @param file 源文件名
 * @param func 函数名
 * @param line 行号
 * @param level 日志等级
 */
void Log_writter(const char *file, const char *func, const int line, const int level, const char *fmt, ...);

/* 日志打印及上报对外接口*/
#define Log_d(args...) Log_writter(__FILE__, __FUNCTION__, __LINE__, DEBUG, args)
#define Log_i(args...) Log_writter(__FILE__, __FUNCTION__, __LINE__, INFO, args)
#define Log_w(args...) Log_writter(__FILE__, __FUNCTION__, __LINE__, WARN, args)
#define Log_e(args...) Log_writter(__FILE__, __FUNCTION__, __LINE__, ERROR, args)

#ifdef IOT_DEBUG
	#define IOT_FUNC_ENTRY    \
		{\
		printf("FUNC_ENTRY:   %s L#%d \n", __FUNCTION__, __LINE__);  \
		}
	#define IOT_FUNC_EXIT    \
		{\
		printf("FUNC_EXIT:   %s L#%d \n", __FUNCTION__, __LINE__);  \
		return;\
		}
	#define IOT_FUNC_EXIT_RC(x)    \
		{\
		printf("FUNC_EXIT:   %s L#%d Return Code : %ld \n", __FUNCTION__, __LINE__, (long)(x));  \
		return x; \
		}
#else
	#define IOT_FUNC_ENTRY
	#define IOT_FUNC_EXIT 			\
		{\
			return;\
		}
	#define IOT_FUNC_EXIT_RC(x)     \
		{\
			return x; \
		}
#endif

//#define LOG_UPLOAD_DEBUG 
#ifdef LOG_UPLOAD_DEBUG
#define UPLOAD_DBG(fmt, ...)   HAL_Printf(">>LOG-DBG>>%s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#else
#define UPLOAD_DBG(...)
#endif
#define UPLOAD_ERR(fmt, ...)   HAL_Printf(">>LOG-ERR>>%s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__)


#ifdef __cplusplus
}
#endif

#endif /* QCLOUD_IOT_EXPORT_LOG_H_ */
