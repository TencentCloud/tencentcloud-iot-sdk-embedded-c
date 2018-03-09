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

#ifndef QCLOUD_IOT_EXPORT_OTA_H_
#define QCLOUD_IOT_EXPORT_OTA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "qcloud_iot_import.h"

#define OTA_CH_SIGNAL_MQTT      (1)
#define OTA_CH_SIGNAL_COAP      (0)
#define OTA_CH_FETCH_HTTP       (1)

typedef enum {

    IOT_OTA_ERR_FAIL = -1,
    IOT_OTA_ERR_INVALID_PARAM = -2,
    IOT_OTA_ERR_INVALID_STATE = -3,
    IOT_OTA_ERR_STR_TOO_LONG = -4,
    IOT_OTA_ERR_FETCH_FAILED = -5,
    IOT_OTA_ERR_FETCH_NOT_EXIST = -6,
    IOT_OTA_ERR_FETCH_AUTH_FAIL = -7,
    IOT_OTA_ERR_FETCH_TIMEOUT = -8,
    IOT_OTA_ERR_NOMEM = -9,
    IOT_OTA_ERR_OSC_FAILED = -10,
    IOT_OTA_ERR_REPORT_VERSION = -11, 
    IOT_OTA_ERR_NONE = 0

} IOT_OTA_Error_Code;


/* OTA状态 */
typedef enum {
    IOT_OTAS_UNINITED = 0,  /* 未初始化 */
    IOT_OTAS_INITED,        /* 初始化完成 */
    IOT_OTAS_FETCHING,      /* 正在下载固件 */
    IOT_OTAS_FETCHED,       /* 固件下载完成 */
    IOT_OTAS_DISCONNECTED   /* 连接已经断开 */
} IOT_OTA_State_Code;


/* OTA进度 */
typedef enum {

    /* 固件升级失败 */
    IOT_OTAP_BURN_FAILED = -4,

    /* 固件校验失败 */
    IOT_OTAP_CHECK_FALIED = -3,

    /* 固件下载失败 */
    IOT_OTAP_FETCH_FAILED = -2,

    /* 初始化失败 */
    IOT_OTAP_GENERAL_FAILED = -1,


    /* [0, 100], 进度百分比区间 */

    /* 进度的最小百分比 */
    IOT_OTAP_FETCH_PERCENTAGE_MIN = 0,

    /* 进度的最大百分比 */
    IOT_OTAP_FETCH_PERCENTAGE_MAX = 100

} IOT_OTA_Progress_Code;


typedef enum {

    IOT_OTAG_FETCHED_SIZE,     /* 固件已经下载的大小 */
    IOT_OTAG_FILE_SIZE,        /* 固件总大小 */
    IOT_OTAG_MD5SUM,           /* md5(字符串类型) */
    IOT_OTAG_VERSION,          /* 版本号(字符串类型)t */
    IOT_OTAG_CHECK_FIRMWARE    /* 对固件进行校验 */

} IOT_OTA_CmdType;

typedef enum {

    IOT_OTAR_DOWNLOAD_TIMEOUT = -1,
    IOT_OTAR_FILE_NOT_EXIST = -2,
    IOT_OTAR_AUTH_FAIL = -3,
    IOT_OTAR_MD5_NOT_MATCH = -4,
    IOT_OTAR_UPGRADE_FAIL = -5,
    IOT_OTAR_NONE = 0,
    IOT_OTAR_DOWNLOAD_BEGIN = 1,
    IOT_OTAR_DOWNLOADING = 2,
    IOT_OTAR_UPGRADE_BEGIN = 3,
    IOT_OTAR_UPGRADE_SUCCESS = 4,

} IOT_OTAReportType;


/**
 * @brief 初始化OTA模块和返回句柄
 *        MQTT客户端必须在调用此接口之前进行初始化
 *
 * @param product_id:   指定产品ID
 * @param device_name:  指定设备名
 * @param ch_signal:    指定的信号通道.
 *
 * @see None.
 */
void *IOT_OTA_Init(const char *product_id, const char *device_name, void *ch_signal);


/**
 * @brief 释放OTA相关的资源
 *        如果在下载之后没有调用重新启动，则必须调用该接口以释放资源
 *
 * @param handle: 指定OTA模块
 *
 * @retval   0 : 成功
 * @retval < 0 : 失败，返回具体错误码
 */
int IOT_OTA_Destroy(void *handle);


/**
 * @brief 向OTA服务器报告固件版本信息。
 *        NOTE: 进行OTA前请保证先上报一次本地固件的版本信息，以便服务器获取到设备目前的固件信息
 *
 * @param handle:   指定OTA模块
 * @param version:  以字符串格式指定固件版本
 *
 * @retval > 0 : 对应publish的packet id
 * @retval < 0 : 失败，返回具体错误码
 */
int IOT_OTA_ReportVersion(void *handle, const char *version);


/**
 * @brief 向OTA服务器报告详细进度 
 *
 * @param handle:       指定OTA模块
 * @param progress:     下载进度
 * @param reportType:   指定当前的下载状态
 *
 * @retval 0 : 成功
 * @retval < 0 : 失败，返回具体错误码
 */
// int IOT_OTA_ReportProgress(void *handle, IOT_OTA_Progress_Code progress, IOT_OTAReportType reportType);

/**
 * @brief 当进行升级前，需要向OTA服务器上报即将升级的状态
 *
 * @param handle:   指定OTA模块
 *
 * @retval > 0 : 对应publish的packet id
 * @retval < 0 : 失败，返回具体错误码
 */
int IOT_OTA_ReportUpgradeBegin(void *handle);

/**
 * @brief 当升级成功之后，需要向OTA服务器上报升级成功的状态
 *
 * @param handle:   指定OTA模块
 * @param version:  即将升级的固件信息
 *
 * @retval > 0 : 对应publish的packet id
 * @retval < 0 : 失败，返回具体错误码
 */
int IOT_OTA_ReportUpgradeSuccess(void *handle, const char *version);

/**
 * @brief 当升级失败之后，需要向OTA服务器上报升级失败的状态
 *
 * @param handle:   指定OTA模块
 * @param version:  即将升级的固件信息
 *
 * @retval > 0 : 对应publish的packet id
 * @retval < 0 : 失败，返回具体错误码
 */
int IOT_OTA_ReportUpgradeFail(void *handle, const char *version);

/**
 * @brief 检查是否处于下载固件的状态
 *
 * @param handle: 指定OTA模块
 *
 * @retval 1 : Yes.
 * @retval 0 : No.
 */
int IOT_OTA_IsFetching(void *handle);


/**
 * @brief 检查固件是否已经下载完成
 *
 * @param handle: 指定OTA模块
 *
 * @retval 1 : Yes.
 * @retval 0 : No.
 */
int IOT_OTA_IsFetchFinish(void *handle);


/**
 * @brief 从具有特定超时值的远程服务器获取固件
 *        注意:如果你想要下载的更快，那么应该给出更大的“buf”
 *
 * @param handle:       指定OTA模块
 * @param buf:          指定存储固件数据的空间
 * @param buf_len:      用字节指定“buf”的长度
 * @param timeout_s:    在秒中指定超时值
 *
 * @retval      < 0 : 对应的错误码
 * @retval        0 : 在“timeout_s”超时期间没有任何数据被下载
 * @retval (0, len] : 在“timeout_s”超时时间内以字节的方式下载数据的长度
 */
int IOT_OTA_FetchYield(void *handle, char *buf, uint32_t buf_len, uint32_t timeout_s);


/**
 * @brief 获取指定的OTA信息
 *        通过这个接口，您可以获得诸如状态、文件大小、文件的md5等信息
 *
 * @param handle:   指定OTA模块
 * @param type:     指定您想要的信息，请参见详细信息“IOT_OTA_CmdType”
 * @param buf:      为数据交换指定缓冲区
 * @param buf_len:  在字节中指定“buf”的长度
 * @return
      NOTE:
      1) 如果 type==IOT_OTAG_FETCHED_SIZE, 'buf'需要传入 uint32_t 类型指针, 'buf_len'需指定为 4
      2) 如果 type==IOT_OTAG_FILE_SIZE, 'buf'需要传入 uint32_t 类型指针, 'buf_len'需指定为 4
      3) 如果 type==IOT_OTAG_MD5SUM, 'buf'需要传入 buffer, 'buf_len需指定为 33
      4) 如果 type==IOT_OTAG_VERSION, 'buf'需要传入 buffer, 'buf_len'需指定为 OTA_VERSION_LEN_MAX
      5) 如果 type==IOT_OTAG_CHECK_FIRMWARE, 'buf'需要传入 uint32_t 类型指针, 'buf_len'需指定为 4
         0, 固件MD5校验不通过, 固件是无效的; 1, 固件是有效的.
 *
 * @retval   0 : 执行成功
 * @retval < 0 : 执行失败，返回对应的错误码
 */
int IOT_OTA_Ioctl(void *handle, IOT_OTA_CmdType type, void *buf, size_t buf_len);


/**
 * @brief 得到最后一个错误代码
 *
 * @param handle: 指定OTA模块
 *
 * @return 对应错误的错误码.
 */
int IOT_OTA_GetLastError(void *handle);

#ifdef __cplusplus
}
#endif

#endif /* QCLOUD_IOT_EXPORT_OTA_H_ */
