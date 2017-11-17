#ifndef QCLOUD_IOT_EXPORT_LOG_H
#define QCLOUD_IOT_EXPORT_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

/**
 * 日志输出等级
 */
typedef enum {
    DEBUG,
    INFO,
    WARN,
    ERROR
} LOG_LEVEL;

/**
 * 全局日志级别输出标志, 只输出小于或等于该等级的日志信息
 */
extern LOG_LEVEL g_log_level;

typedef bool (*LogMessageHandler)(const char* message);

/**
 * @brief
 *
 * @param
 */
void IOT_Log_Set_Level(LOG_LEVEL level);

/**
 * @brief 获取当前日志等级
 *
 * @return
 */
LOG_LEVEL IOT_Log_Get_Level();

/**
 * @brief 设置日志回调函数，用户接管日志内容用于输出到文件等操作
 *
 * @param handler 回调函数指针
 *
 */
void IOT_Log_Set_MessageHandler(LogMessageHandler handler);

/**
 * @brief 日志打印函数，默认打印到标准输出，当用户设置日志打印handler时，回调handler
 *
 * @param file 源文件名
 * @param func 函数名
 * @param line 行号
 * @param level 日志等级
 */
void Log_writter(const char *file, const char *func, const int line, const int level, const char *fmt, ...);

#define Log_d(args...) Log_writter(__FILE__, __FUNCTION__, __LINE__, DEBUG, args)
#define Log_i(args...) Log_writter(__FILE__, __FUNCTION__, __LINE__, INFO, args)
#define Log_w(args...) Log_writter(__FILE__, __FUNCTION__, __LINE__, WARN, args)
#define Log_e(args...) Log_writter(__FILE__, __FUNCTION__, __LINE__, ERROR, args)

#ifdef IOT_SDK_TRACE
	#define IOT_FUNC_ENTRY    \
		{\
		printf("FUNC_ENTRY:   %s L#%d \n", __FUNCTION__, __LINE__);  \
		}
	#define IOT_FUNC_EXIT    \
		{\
		printf("FUNC_EXIT:   %s L#%d \n", __FUNCTION__, __LINE__);  \
		}
	#define IOT_FUNC_EXIT_RC(x)    \
		{\
		printf("FUNC_EXIT:   %s L#%d Return Code : %d \n", __FUNCTION__, __LINE__, x);  \
		return x; \
		}
#else
	#define IOT_FUNC_ENTRY
	#define IOT_FUNC_EXIT
	#define IOT_FUNC_EXIT_RC(x)     \
		{\
			return x; \
		}
#endif

#ifdef __cplusplus
}
#endif

#endif /* QCLOUD_IOT_EXPORT_LOG_H */
