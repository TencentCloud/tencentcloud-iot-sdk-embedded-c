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

#ifndef IOT_OTA_LIB_H_
#define IOT_OTA_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <stdint.h>

#include "qcloud_iot_export_ota.h"

void *qcloud_otalib_md5_init(void);

void qcloud_otalib_md5_update(void *md5, const char *buf, size_t buf_len);

void qcloud_otalib_md5_finalize(void *md5, char *output_str);

void qcloud_otalib_md5_deinit(void *md5);

int qcloud_otalib_get_firmware_type(const char *json, char **type);

int qcloud_otalib_get_report_version_result(const char *json);

/**
 * @brief 根据@json解析出来固件下载的相关信息
 *
 * @param json          JSON字符串
 * @param type          下发升级消息的时候，type为update_firmware
 * @param url           解析得到的下载url
 * @param version       解析得到的下载固件版本
 * @param md5           解析得到的下载固件MD5
 * @param fileSize      下载固件的大小
 * @return              返回QCLOUD_ERR_SUCCESS表示成功
 */
int qcloud_otalib_get_params(const char *json, char **type, char **url, char **version, char *md5,
                     uint32_t *fileSize);


/**
 * @brief 根据@id、@version生成固件信息，然后复制到@buf
 *
 * @param buf       固件输出信息
 * @param bufLen    固件输出信息长度
 * @param id        固件id
 * @param version   固件版本
 * @return          返回QCLOUD_ERR_SUCCESS表示成功
 */
int qcloud_otalib_gen_info_msg(char *buf, size_t bufLen, uint32_t id, const char *version);


/**
 * @brief 根据@id，@reportType生成报告信息
 *
 * @param buf           固件报告信息
 * @param bufLen        固件报告信息长度
 * @param id            固件id
 * @param version       固件版本
 * @param progress      固件下载进度
 * @param reportType    报告类型
 * @return              返回QCLOUD_ERR_SUCCESS表示成功
 */
int qcloud_otalib_gen_report_msg(char *buf, size_t bufLen, uint32_t id, const char *version, int progress, IOT_OTAReportType reportType);

#ifdef __cplusplus
}
#endif

#endif /* IOT_OTA_LIB_H_ */
