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

#ifdef __cplusplus
extern "C" {
#endif

#include "qcloud_iot_export_log.h"

#include <string.h>

#include "qcloud_iot_import.h"

#define MAX_LOG_MSG_LEN 			(255)

#if defined(__linux__)
	#undef  MAX_LOG_MSG_LEN
	#define MAX_LOG_MSG_LEN                  (1023)
#endif

static char *level_str[] = {
    "DBG", "INF", "WRN", "ERR",
};

static LogMessageHandler sg_log_message_handler= NULL;

LOG_LEVEL g_log_level = INFO;

static char *_get_filename(const char *p)
{
    char ch = '/';
    char *q = strrchr(p,ch) + 1;
    return q;
}

void IOT_Log_Set_Level(LOG_LEVEL logLevel) {
    g_log_level = logLevel;
}

LOG_LEVEL IOT_Log_Get_Level() {
    return g_log_level;
}

void IOT_Log_Set_MessageHandler(LogMessageHandler handler) {
	sg_log_message_handler = handler;
}

void Log_writter(const char *file, const char *func, const int line, const int level, const char *fmt, ...)
{
	if (level < g_log_level) {
		return;
	}

	char *file_name = _get_filename(file);

	if (sg_log_message_handler) {
		static char sg_text_buf[MAX_LOG_MSG_LEN + 1];
		char		*tmp_buf = sg_text_buf;
		char        *o = tmp_buf;
	    memset(tmp_buf, 0, sizeof(sg_text_buf));

	    o += HAL_Snprintf(o, sizeof(sg_text_buf), "%s|%s|%s|%s(%d): ", level_str[level], HAL_Timer_current(), file_name, func, line);

	    va_list     ap;
	    va_start(ap, fmt);
	    o += vsnprintf(o, MAX_LOG_MSG_LEN - 2 - strlen(tmp_buf), fmt, ap);
	    va_end(ap);

	    strcat(tmp_buf, "\n");

		if (sg_log_message_handler(tmp_buf)) {
			return;
		}
	}

    HAL_Printf("%s|%s|%s|%s(%d): ", level_str[level], HAL_Timer_current(), file_name, func, line);

    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    HAL_Printf("\r\n");

    return;
}

#ifdef __cplusplus
}
#endif
