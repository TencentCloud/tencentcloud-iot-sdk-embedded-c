#ifndef qcloud_iot_utils_base64_h
#define qcloud_iot_utils_base64_h

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdio.h>
    
#include "qcloud_iot_export_log.h"
#include "qcloud_iot_export_error.h"

int qcloud_iot_utils_base64encode( unsigned char *dst, size_t dlen, size_t *olen,
                                const unsigned char *src, size_t slen );

int qcloud_iot_utils_base64decode( unsigned char *dst, size_t dlen, size_t *olen,
                                const unsigned char *src, size_t slen );

#ifdef __cplusplus
}
#endif
#endif /* qcloud_iot_utils_base64_h */
