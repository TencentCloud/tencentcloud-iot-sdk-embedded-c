/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2018-2020 Tencent. All rights reserved.

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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qcloud_iot_import.h"
#include "utils_timer.h"

#define PLATFORM_HAS_TIME_FUNCS
//#define PLATFORM_HAS_CMSIS

#ifdef PLATFORM_HAS_TIME_FUNCS
#include <sys/time.h>
#include <time.h>
#endif

#ifdef PLATFORM_HAS_CMSIS
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"
#endif

int HAL_Timer_set_systime_sec(size_t timestamp_sec)
{
#if defined PLATFORM_HAS_TIME_FUNCS
    struct timeval stime;
    stime.tv_sec  = timestamp_sec;
    stime.tv_usec = 0;
    if (0 != settimeofday(&stime, NULL)) {
        return -1;
    }
    return 0;
#else
    RTC_DATE_TIME date_time;
    date_time.ms = 0;
    timestamp_to_date(timestamp_sec, &date_time, 8);
    // set RTC Time note hw rtc year base
    return -1;
#endif
}

int HAL_Timer_set_systime_ms(size_t timestamp_ms)
{
#if defined PLATFORM_HAS_TIME_FUNCS
    struct timeval stime;
    stime.tv_sec  = (timestamp_ms / 1000);
    stime.tv_usec = ((timestamp_ms % 1000) * 1000);

    if (0 != settimeofday(&stime, NULL)) {
        return -1;
    }
    return 0;
#else
    RTC_DATE_TIME date_time;
    date_time.ms = timestamp_ms % 1000;
    timestamp_to_date(timestamp_ms / 1000, &date_time, 8);
    // set RTC Time note hw rtc year base
    return -1;
#endif
}

uint64_t HAL_GetTimeMs(void)
{
#if defined PLATFORM_HAS_TIME_FUNCS
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;

#elif defined PLATFORM_HAS_CMSIS
    return HAL_GetTick();
#endif
}

/*Get timestamp*/
long HAL_Timer_current_sec(void)
{
    return HAL_GetTimeMs() / 1000;
}

char *HAL_Timer_current(char *time_str)
{
#if defined PLATFORM_HAS_TIME_FUNCS
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t    now_time = tv.tv_sec;
    struct tm tm_tmp   = *localtime(&now_time);
    strftime(time_str, TIME_FORMAT_STR_LEN, "%F %T", &tm_tmp);
    return time_str;
#else
    long time_sec;
    time_sec = HAL_Timer_current_sec();
    memset(time_str, 0, TIME_FORMAT_STR_LEN);
    snprintf(time_str, TIME_FORMAT_STR_LEN, "%ld", time_sec);
    return time_str;
#endif
}

bool HAL_Timer_expired(Timer *timer)
{
    uint64_t now_ts;

    now_ts = HAL_GetTimeMs();

    return (now_ts > timer->end_time) ? true : false;
}

void HAL_Timer_countdown_ms(Timer *timer, unsigned int timeout_ms)
{
    timer->end_time = HAL_GetTimeMs();
    timer->end_time += timeout_ms;
}

void HAL_Timer_countdown(Timer *timer, unsigned int timeout)
{
    timer->end_time = HAL_GetTimeMs();
    timer->end_time += timeout * 1000;
}

int HAL_Timer_remain(Timer *timer)
{
    return (int)(timer->end_time - HAL_GetTimeMs());
}

void HAL_Timer_init(Timer *timer)
{
    timer->end_time = 0;
}

#ifdef __cplusplus
}
#endif
