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

#ifndef IOT_SHADOW_CLIENT_JSON_H_
#define IOT_SHADOW_CLIENT_JSON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>

#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"

#define VERSION_FIELD          		"version"
#define TYPE_FIELD	         		"type"
#define CLIENT_TOKEN_FIELD     		"clientToken"
#define RESULT_FIELD	       		"result"

#define OPERATION_DELTA        		"delta"
#define OPERATION_GET				"get"
#define OPERATION_UPDATE			"update"

#define PAYLOAD_STATE				"payload.state"
#define PAYLOAD_VERSION				"payload.version"
#define PAYLOAD_STATE_DELTA			"payload.state.delta"

#define REPLY_CODE					"code"
#define REPLY_STATUS				"status"


/**
 * 将一个JSON节点写入到JSON串中
 *
 * @param jsonBuffer   	JSON串
 * @param sizeOfBuffer  可写入大小
 * @param pKey          JSON节点的key
 * @param pData         JSON节点的value
 * @param type          JSON节点value的数据类型
 * @return              返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int put_json_node(char *jsonBuffer, size_t sizeOfBuffer, const char *pKey, void *pData, JsonDataType type);

/**
 * 将一个JSON节点写入到JSON串中,物模型对bool类型的处理有区分。
 *
 * @param jsonBuffer   	JSON串
 * @param sizeOfBuffer  可写入大小
 * @param pKey          JSON节点的key
 * @param pData         JSON节点的value
 * @param type          JSON节点value的数据类型
 * @return              返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int event_put_json_node(char *jsonBuffer, size_t sizeOfBuffer, const char *pKey, void *pData, JsonDataType type);


/**
 * @brief 返回一个ClientToken
 *
 * @param pStrBuffer    存储ClientToken的字符串缓冲区
 * @param sizeOfBuffer  缓冲区大小
 * @param tokenNumber   shadow的token值，函数内部每次执行完会自增
 * @return              返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int generate_client_token(char *pStrBuffer, size_t sizeOfBuffer, uint32_t *tokenNumber);

/**
 * @brief 为GET和DELETE请求构造一个只带有clientToken字段的JSON文档
 *
 * @param tokenNumber   shadow的token值，函数内部每次执行完会自增
 * @param pJsonBuffer 存储JSON文档的字符串缓冲区
 */
void build_empty_json(uint32_t *tokenNumber, char *pJsonBuffer);

/**
 * @brief 从JSON文档中解析出clientToken字段
 *
 * @param pJsonDoc       待解析的JSON文档
 * @param pClientToken   ClientToken字段
 * @return               返回true, 表示解析成功
 */
bool parse_client_token(char *pJsonDoc, char **pClientToken);

/**
 * @brief 从JSON文档中解析出status字段,事件回复
 *
 * @param pJsonDoc       待解析的JSON文档
 * @param pStatus   	 status字段
 * @return               返回true, 表示解析成功
 */
bool parse_status_return(char *pJsonDoc, char **pStatus);

/**
 * @brief 从JSON文档中解析出code字段,事件回复
 *
 * @param pJsonDoc       待解析的JSON文档
 * @param pCode   		 Code字段
 * @return               返回true, 表示解析成功
 */
bool parse_code_return(char *pJsonDoc, int32_t *pCode);

/**
 * @brief 从JSON文档中解析出version字段
 *
 * @param pJsonDoc        待解析的JSON文档
 * @param pVersionNumber  JSON文档版本号
 * @return                返回true, 表示解析成功
 */
bool parse_version_num(char *pJsonDoc, uint32_t *pVersionNumber);

/**
 * @brief 从JSON文档中解析出state字段
 *
 * @param pJsonDoc         待解析的JSON文档
 * @param pErrorMessage    响应返回错误提示消息
 * @return                 返回true, 表示解析成功
 */
bool parse_shadow_state(char *pJsonDoc, char **pState);

/**
 * @brief 从JSON文档中解析出type字段
 *
 * @param pJsonDoc         	待解析的JSON文档
 * @param pType    			输出tyde字段
 * @return                 	返回true, 表示解析成功
 */
bool parse_shadow_operation_type(char *pJsonDoc, char **pType);

/**
 * @brief 从JSON文档中解析出result字段
 *
 * @param pJsonDoc         	待解析的JSON文档
 * @param pResultCode    	操作结果标志码
 * @return                 	返回true, 表示解析成功
 */
bool parse_shadow_operation_result_code(char *pJsonDoc, int16_t *pResultCode);

/**
 * @brief 从JSON文档中解析出delta字段, dalta type
 *
 * @param pJsonDoc         	待解析的JSON文档
 * @param pDelta    		delta字段对应的value
 * @return                 	返回true, 表示解析成功
 */
bool parse_shadow_operation_delta(char *pJsonDoc, char **pDelta);

/**
 * @brief 从JSON文档中解析出delta字段	, get/update type
 *
 * @param pJsonDoc         	待解析的JSON文档
 * @param pDelta    		delta字段对应的value
 * @return                 	返回true, 表示解析成功
 */
bool parse_shadow_operation_get(char *pJsonDoc, char **pDelta);


/**
 * @brief 如果JSON文档中的key与某个设备属性的key匹配的话, 则更新该设备属性, 该设备属性的值不能为OBJECT类型
 *
 * @param pJsonDoc       JSON文档
 * @param pProperty      设备属性
 * @return               返回true, 表示成功
 */
bool update_value_if_key_match(char *pJsonDoc, DeviceProperty *pProperty);

#ifdef __cplusplus
}
#endif

#endif //IOT_SHADOW_CLIENT_JSON_H_
