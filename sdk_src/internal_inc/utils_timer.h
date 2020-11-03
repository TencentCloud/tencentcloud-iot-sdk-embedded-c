/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2018-2020 THL A29 Limited, a Tencent company. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef QCLOUD_IOT_UTILS_TIMER_H_
#define QCLOUD_IOT_UTILS_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

// Add the platform specific timer includes to define the Timer struct
#include "qcloud_iot_import.h"

/**
 * Define RTC DATE TIME structure
 */
typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int sec;
    int ms;
} RTC_DATE_TIME;

/**
 * @brief Check if a timer is expired
 *
 * Call this function passing in a timer to check if that timer has expired.
 *
 * @param timer - pointer to the timer to be checked for expiration
 * @return bool - true = timer expired, false = timer not expired
 */
bool expired(Timer *timer);

/**
 * @brief Create a timer (milliseconds)
 *
 * Sets the timer to expire in a specified number of milliseconds.
 *
 * @param timer - pointer to the timer to be set to expire in milliseconds
 * @param timeout_ms - set the timer to expire in this number of milliseconds
 */
void countdown_ms(Timer *timer, unsigned int timeout_ms);

/**
 * @brief Create a timer (seconds)
 *
 * Sets the timer to expire in a specified number of seconds.
 *
 * @param timer - pointer to the timer to be set to expire in seconds
 * @param timeout - set the timer to expire in this number of seconds
 */
void countdown(Timer *timer, unsigned int timeout);

/**
 * @brief Check the time remaining on a give timer
 *
 * Checks the input timer and returns the number of milliseconds remaining on the timer.
 *
 * @param timer - pointer to the timer to be set to checked
 * @return int - milliseconds left on the countdown timer
 */
int left_ms(Timer *timer);

/**
 * @brief Initialize a timer
 *
 * Performs any initialization required to the timer passed in.
 *
 * @param timer - pointer to the timer to be initialized
 */
void InitTimer(Timer *timer);

/**
 * @brief Time stamp converted to date
 *
 * Time stamp converted to date
 *
 * @param timestamp_sec   timestamp second
 * @param date_time       output date_time
 * @param time_zone       +-time_zone, 8 is china time zone
 */
void timestamp_to_date(size_t timestamp_sec, RTC_DATE_TIME *date_time, int8_t time_zone);

#ifdef __cplusplus
}
#endif

#endif  // QCLOUD_IOT_UTILS_TIMER_H_
