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

#include "stm32l4xx_hal.h"

static char now_time_str[20] = {0};

int HAL_Timer_set_systime_sec(size_t timestamp_sec)
{
#ifndef PLATFORM_HAS_TIME_FUNCS
    RTC_DATE_TIME date_time;
    date_time.ms = 0;
    timestamp_to_date(timestamp_sec, &date_time, 8);
    // set RTC Time note hw rtc year base
#endif
    return -1;
}

int HAL_Timer_set_systime_ms(size_t timestamp_ms)
{
#ifndef PLATFORM_HAS_TIME_FUNCS
    RTC_DATE_TIME date_time;
    date_time.ms = timestamp_ms % 1000;
    timestamp_to_date(timestamp_ms / 1000, &date_time, 8);
    // set RTC Time note hw rtc year base
#endif
    return -1;
}

uint64_t HAL_GetTimeMs(void)
{
    return HAL_GetTick();
}

/*Get timestamp*/
long HAL_Timer_current_sec(void)
{
    // return GetTimeStampByAt(NULL);

    return HAL_GetTimeMs() / 1000;
}

char *HAL_Timer_current(void)
{
    long time_sec;

    time_sec = HAL_Timer_current_sec();
    memset(now_time_str, 0, 20);
    snprintf(now_time_str, 20, "%ld", time_sec);

    return now_time_str;
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
