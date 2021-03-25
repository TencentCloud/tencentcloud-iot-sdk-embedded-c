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

#ifndef QCLOUD_IOT_EXPORT_RESOURCE_H_
#define QCLOUD_IOT_EXPORT_RESOURCE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "qcloud_iot_import.h"

typedef enum {
    QCLOUD_RESOURCE_ERRCODE_FAIL_E            = -1,
    QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E   = -2,
    QCLOUD_RESOURCE_ERRCODE_INVALID_STATE_E   = -3,
    QCLOUD_RESOURCE_ERRCODE_STR_TOO_LONG_     = -4,
    QCLOUD_RESOURCE_ERRCODE_FETCH_FAILED_E    = -5,
    QCLOUD_RESOURCE_ERRCODE_FETCH_NOT_EXIST_E = -6,
    QCLOUD_RESOURCE_ERRCODE_FETCH_AUTH_FAIL_E = -7,
    QCLOUD_RESOURCE_ERRCODE_FETCH_TIMEOUT_E   = -8,
    QCLOUD_RESOURCE_ERRCODE_NOMEM_E           = -9,
    QCLOUD_RESOURCE_ERRCODE_OSC_FAILED_E      = -10,
    QCLOUD_RESOURCE_ERRCODE_REPORT_VERSION_E  = -11,
    QCLOUD_RESOURCE_ERRCODE_NONE_E            = 0
} QCLOUD_RESOURCE_ERR_CODE_E;

typedef enum {
    QCLOUD_RESOURCE_STATE_UNINITED_E = 0, /* un-inited */
    QCLOUD_RESOURCE_STATE_INITED_E,       /* inited */
    QCLOUD_RESOURCE_STATE_START_E,     /* start */
    QCLOUD_RESOURCE_STATE_END_E,      /* end*/
    QCLOUD_RESOURCE_STATE_DISCONNECTED_E  /* disconnected */
} QCLOUD_RESOURCE_STATE_CODE_E;

typedef enum {
    QCLOUD_RESOURCE_RESULTCODE_SUCCESS_E       = 0,
    QCLOUD_RESOURCE_RESULTCODE_TIMEOUT_E       = -1,
    QCLOUD_RESOURCE_RESULTCODE_FILE_NOTEXIST_E = -2,
    QCLOUD_RESOURCE_RESULTCODE_SIGN_INVALID_E  = -3,
    QCLOUD_RESOURCE_RESULTCODE_MD5_NOTMATCH_E  = -4
} QCLOUD_RESOURCE_RESULTCODE_E;

typedef enum {
    QCLOUD_RESOURCE_REPORT_UPLOADING_E,
    QCLOUD_RESOURCE_REPORT_DOWNLOADING_E,
    QCLOUD_RESOURCE_REPORT_SUCCESS_E,
    QCLOUD_RESOURCE_REPORT_FAILED_E
} QCLOUD_RESOURCE_REPORT_E;

typedef enum {
    QCLOUD_IOT_RESOURCE_FETCHED_SIZE_E,
    QCLOUD_IOT_RESOURCE_SIZE_E,
    QCLOUD_IOT_RESOURCE_MD5SUM_E,
    QCLOUD_IOT_RESOURCE_NAME_E,
    QCLOUD_IOT_RESOURCE_MD5CHECK_E,
    QCLOUD_IOT_RESOURCE_UPLOADED_SIZE_E,
    QCLOUD_IOT_RESOURCE_CALCMD5_E
} QCLOUD_IOT_RESOURCE_CMDTYPE_E;

/**
 * @brief Init Resource download & upload
 *
 *
 * @param product_id:   product Id
 * @param device_name:  device name
 * @param mqtt_client:  mqtt client handle
 *
 * @return a void resource handle when success, or NULL otherwise
 */
void *QCLOUD_IOT_RESOURCE_Init(const char *product_id, const char *device_name, void *mqtt_client);

/**
 * @brief send resource upload request to get upload url
 *
 *
 * @param handle:         resource handle
 * @param resource_name:  upload resource name
 * @param resource_size:  upload resource size
 * @param md5sum       :  upload resource md5sum; 32B lowercase hex
 *
 * @return  >= 0 send success, > 0 is publish packetid, other send fail
 */
int QCLOUD_IOT_RESOURCE_Upload_Request(void *handle, char *resource_name, int resource_size, char *md5sum);

/**
 * @brief send get resource download task, check offline download, download failed etc
 *
 *
 * @param handle:         resource handle
 *
 * @return  >= 0 send success, > 0 is publish packetid, other send fail
 */
int QCLOUD_IOT_RESOURCE_GetDownloadTask(void *handle);

/**
 * @brief Deinit resource handle download & upload
 *
 * @param handle: resource handle
 *
 * @return QCLOUD_RET_SUCCESS when success, or err code for failure
 */
int QCLOUD_IOT_RESOURCE_DeInit(void *handle);

/**
 * @brief Setup HTTP connection and prepare resource download
 *
 * @param handle: resource handle
 * @param offset: offset of resource downloaded
 * @param size: size of resource
 *
 * @return QCLOUD_RET_SUCCESS when success, or err code for failure
 */
int QCLOUD_IOT_RESOURCE_StartDownload(void *handle, uint32_t offset, uint32_t size);

/**
 * @brief Setup HTTP connection send http put request and prepare resource upload
 *
 * @param handle: resource handle
 * @param buf:    don't null, not send
 *
 * @return QCLOUD_RET_SUCCESS when success, or err code for failure
 */
int QCLOUD_IOT_RESOURCE_StartUpload(void *handle, char *buf);

/**
 * @brief Update MD5 calc value of local downloaded resource
 *
 * @param handle: resource handle
 * @param buff:   buffer to downloaded resource
 * @param size:   size of buffer
 *
 */
void QCLOUD_IOT_RESOURCE_UpdateDownloadClientMd5(void *handle, char *buff, uint32_t size);

/**
 * @brief reset download MD5 handle
 *
 * @param handle: resource handle
 *
 */
int QCLOUD_IOT_RESOURCE_DownloadResetClientMD5(void *handle);

/**
 * @brief reset upload MD5 handle
 *
 * @param handle: resource handle
 *
 */
int QCLOUD_IOT_RESOURCE_UploadResetClientMD5(void *handle);

/**
 * @brief Update uplaod MD5 calc value
 *
 * @param handle: resource handle
 * @param buff:   buffer to upload resource
 * @param size:   size of buffer
 *
 */
void QCLOUD_IOT_RESOURCE_UploadMd5Update(void *handle, char *buf, int buf_len);

/**
 * @brief finish uplaod MD5 calc value ,get finsh md5sum >= 33B
 *
 * @param handle:  resource handle
 * @param md5_str: output md5sum, 32B lowercase hex
 *
 */
void QCLOUD_IOT_RESOURCE_Upload_Md5_Finish(void *handle, char *md5_str);

/**
 * @brief Report resource download success to server
 *
 * @param handle:         resource handle
 * @param resource_name:  download resource name, can is null
 *
 * @return (>=0) when success, > 0 is publish packetid, or err code (<0) for failure
 */
int QCLOUD_IOT_RESOURCE_ReportDownloadSuccess(void *handle, char *resource_name);

/**
 * @brief Report resource upload success to server
 *
 * @param handle:         resource handle
 * @param resource_name:  upload resource name, can is null
 *
 * @return = 0 success, other for failure
 */
int QCLOUD_IOT_RESOURCE_ReportUploadSuccess(void *handle, char *resource_name);

/**
 * @brief Report resource download fail to server
 *
 * @param handle:         resource handle
 * @param resource_name:  download resource name, can is null
 *
 * @return (>=0) when success, > 0 is publish packetid, or err code (<0) for failure
 */
int QCLOUD_IOT_RESOURCE_ReportDownloadFail(void *handle, char *resource_name);

/**
 * @brief Report resource upload fail to server
 *
 * @param handle:         resource handle
 * @param resource_name:  upload resource name, can is null
 *
 * @return = 0 success, other for failure
 */
int QCLOUD_IOT_RESOURCE_ReportUploadFail(void *handle, char *resource_name);

/**
 * @brief Check if has resource can fetching/downloading
 *
 * @param handle: resource handle
 *
 * @retval 1 : Yes.
 * @retval 0 : No.
 */
int QCLOUD_IOT_RESOURCE_IsStartDownload(void *handle);

/**
 * @brief Check if has resource can uploading
 *
 * @param handle: resource handle
 *
 * @retval 1 : Yes.
 * @retval 0 : No.
 */
int QCLOUD_IOT_RESOURCE_IsStartUpload(void *handle);

/**
 * @brief Download resource from HTTP server and save to buffer
 *
 * @param handle:       resource handle
 * @param buf:          buffer to store resource
 * @param buf_len:      length of buffer
 * @param timeout_s:    timeout value in second
 *
 * @retval      < 0 : error code
 * @retval        0 : no data is downloaded in this period and timeout happen
 * @retval (0, len] : size of the downloaded data
 */
int QCLOUD_IOT_RESOURCE_DownloadYield(void *handle, char *buf, uint32_t buf_len, uint32_t timeout_s);

/**
 * @brief upload resource send to HTTP server
 *
 * @param handle:       resource handle
 * @param buf:          buffer stored upload resource
 * @param buf_len:      length of buffer
 * @param timeout_s:    timeout value in second
 *
 * @retval      < 0 : error code
 * @retval        0 : no data is upload in this period and timeout happen
 * @retval (0, len] : size of the downloaded data
 */
int QCLOUD_IOT_RESOURCE_UploadYield(void *handle, char *buf, uint32_t buf_len, uint32_t timeout_s);

/**
 * @brief Check if resource fetching/downloading is finished
 *
 * @param handle: resource handle
 *
 * @retval 1 : Yes.
 * @retval 0 : No.
 */
int QCLOUD_IOT_RESOURCE_IsDownloadFinish(void *handle);

/**
 * @brief Check if resource uploading is finished
 *
 * @param handle: resource handle
 *
 * @retval 1 : Yes.
 * @retval 0 : No.
 */
int QCLOUD_IOT_RESOURCE_IsUploadFinish(void *handle);

/**
 * @brief Get resource download info (resource name, size, md5sum, check md5)
 *
 * @param handle:   resource handle
 * @param type:     type of info to get, refer to QCLOUD_IOT_RESOURCE_CMDTYPE_E
 * @param buf:      buffer for the data
 * @param buf_len:  length of buffer
 *
 * @retval   0 : success
 * @retval < 0 : error code for failure
 */
int QCLOUD_IOT_RESOURCE_DownloadIoctl(void *handle, QCLOUD_IOT_RESOURCE_CMDTYPE_E type, void *buf, size_t buf_len);

/**
 * @brief Get resource upload info (resource name, size, md5sum, check upload md5)
 *
 * @param handle:   resource handle
 * @param type:     type of info to get, refer to QCLOUD_IOT_RESOURCE_CMDTYPE_E
 * @param buf:      buffer for the data
 * @param buf_len:  length of buffer
 *
 * @retval   0 : success
 * @retval < 0 : error code for failure
 */
int QCLOUD_IOT_RESOURCE_UploadIoctl(void *handle, QCLOUD_IOT_RESOURCE_CMDTYPE_E type, void *buf, size_t buf_len);

/**
 * @brief Get error code of last upload operation
 *
 * @param handle:   resource handle
 *
 * @return error code
 */
int QCLOUD_IOT_RESOURCE_DownloadGetLastError(void *handle);

/**
 * @brief Get error code of last download operation
 *
 * @param handle:   resource handle
 *
 * @return error code
 */
int QCLOUD_IOT_RESOURCE_UploadGetLastError(void *handle);

#ifdef __cplusplus
}
#endif

#endif /* QCLOUD_IOT_EXPORT_RESOURCE_H_ */
