/*
 * qcloud_iot_sdk_impl_internal.h
 *
 *  Created on: 2017年11月2日
 *      Author: shockcao
 */

#ifndef QCLOUD_IOT_SDK_IMPL_INTERNAL_H_
#define QCLOUD_IOT_SDK_IMPL_INTERNAL_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"

#define NUMBERIC_SANITY_CHECK(num, err) \
    do { \
        if (0 == (num)) { \
            Log_e("Invalid argument, numeric 0"); \
            return (err); \
        } \
    } while(0)

#define NUMBERIC_SANITY_CHECK_RTN(num) \
    do { \
        if (0 == (num)) { \
            Log_e("Invalid argument, numeric 0"); \
            return; \
        } \
    } while(0)

#define POINTER_SANITY_CHECK(ptr, err) \
    do { \
        if (NULL == (ptr)) { \
            Log_e("Invalid argument, %s = %p", #ptr, ptr); \
            return (err); \
        } \
    } while(0)

#define POINTER_SANITY_CHECK_RTN(ptr) \
    do { \
        if (NULL == (ptr)) { \
            Log_e("Invalid argument, %s = %p", #ptr, ptr); \
            return; \
        } \
    } while(0)

#define STRING_PTR_SANITY_CHECK(ptr, err) \
    do { \
        if (NULL == (ptr)) { \
            Log_e("Invalid argument, %s = %p", #ptr, (ptr)); \
            return (err); \
        } \
        if (0 == strlen((ptr))) { \
            Log_e("Invalid argument, %s = '%s'", #ptr, (ptr)); \
            return (err); \
        } \
    } while(0)

#define STRING_PTR_SANITY_CHECK_RTN(ptr) \
    do { \
        if (NULL == (ptr)) { \
            Log_e("Invalid argument, %s = %p", #ptr, (ptr)); \
            return; \
        } \
        if (0 == strlen((ptr))) { \
            Log_e("Invalid argument, %s = '%s'", #ptr, (ptr)); \
            return; \
        } \
    } while(0)

#if defined(__cplusplus)
}
#endif

#endif /* QCLOUD_IOT_SDK_IMPL_INTERNAL_H_ */
