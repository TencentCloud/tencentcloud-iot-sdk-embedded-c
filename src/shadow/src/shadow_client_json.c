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

#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include "shadow_client_json.h"

#include "jsmn.h"
#include "device.h"
#include "qcloud_iot_utils_json.h"


/**
 * 如果设备属性的值不为OBJECT类型, 则根据JSON文档中的值去更新指定设备属性
 *
 * @param pJsonString  JSON串
 * @param pProperty    设备属性
 * @param token        JSONToken
 * @return             返回QCLOUD_ERR_SUCCESS, 表示更新成功
 */
static int _update_value_if_no_object(const char *pJsonString, DeviceProperty *pProperty, jsmntok_t token) {

    int rc = QCLOUD_ERR_SUCCESS;

    if (pProperty->type == JBOOL) {
        rc = get_boolean(pProperty->data, pJsonString, &token);
    } else if (pProperty->type == JINT32) {
        rc = get_int32(pProperty->data, pJsonString, &token);
    } else if (pProperty->type == JINT16) {
        rc = get_int16(pProperty->data, pJsonString, &token);
    } else if (pProperty->type == JINT8) {
        rc = get_int8(pProperty->data, pJsonString, &token);
    } else if (pProperty->type == JUINT32) {
        rc = get_uint32(pProperty->data, pJsonString, &token);
    } else if (pProperty->type == JUINT16) {
        rc = get_uint16(pProperty->data, pJsonString, &token);
    } else if (pProperty->type == JUINT8) {
        rc = get_uint8(pProperty->data, pJsonString, &token);
    } else if (pProperty->type == JFLOAT) {
        rc = get_float(pProperty->data, pJsonString, &token);
    } else if (pProperty->type == JDOUBLE) {
        rc = get_double(pProperty->data, pJsonString, &token);
    }

    return rc;
}

/**
 * 为JSON文档增加client token字段
 *
 * @param pJsonDocument             JSON串
 * @param maxSizeOfJsonDocument     JSON串最大长度
 * @return                          添加字段的长度
 */
static int32_t _add_client_token(char *pJsonDocument, size_t maxSizeOfJsonDocument, uint32_t *tokenNumber) {

    int32_t rc_of_snprintf = HAL_Snprintf(pJsonDocument, maxSizeOfJsonDocument, "%s-%u", iot_device_info_get()->product_id, (*tokenNumber)++);

    return rc_of_snprintf;
}

/**
 * @brief 检查函数snprintf的返回值
 *
 * @param returnCode       函数snprintf的返回值
 * @param maxSizeOfWrite   可写最大字节数
 * @return                 返回QCLOUD_ERR_JSON, 表示出错; 返回QCLOUD_ERR_JSON_BUFFER_TRUNCATED, 表示截断
 */
static inline int _check_snprintf_return(int32_t returnCode, size_t maxSizeOfWrite) {

    if (returnCode >= maxSizeOfWrite) {
        return QCLOUD_ERR_JSON_BUFFER_TRUNCATED;
    } else if (returnCode < 0) { // 写入出错
        return QCLOUD_ERR_JSON;
    }

    return QCLOUD_ERR_SUCCESS;
}

/**
 * @brief 检查函数snprintf的返回值
 *
 * @param pJsonDoc          JSON文档
 * @param tokenCount        JSONToken的个数
 * @param pProperty         设备属性
 * @param pDataLength       JSON文档key对应的value的长度
 * @param pDataPosition     JSON文档中value在JSON串中的位置
 * @return                  成功返回true，否则返回false
 */
static bool _prase_shadow_operation_delta_key_in_state(const char *pJsonDoc, int32_t tokenCount,
                               DeviceProperty *pProperty, uint32_t *pDataLength, int32_t *pDataPosition)
{
    int32_t i;
    
    for (i = 1; i < tokenCount; i++) {
        if (jsoneq(pJsonDoc, &(tokens[i]), pProperty->key) == 0) {
            jsmntok_t dataToken = tokens[i + 1];
            uint32_t dataLength = dataToken.end - dataToken.start;
            _update_value_if_no_object(pJsonDoc, pProperty, dataToken);
            *pDataPosition += dataToken.start;
            *pDataLength = dataLength;
            return true;
        }
    }
    return false;
}

int put_json_node(char *jsonBuffer, size_t sizeOfBuffer, const char *pKey, void *pData, JsonDataType type) {

    int rc;
    int32_t rc_of_snprintf = 0;
    size_t remain_size = 0;

    if ((remain_size = sizeOfBuffer - strlen(jsonBuffer)) <= 1) {
        return QCLOUD_ERR_JSON_BUFFER_TOO_SMALL;
    }

    rc_of_snprintf = HAL_Snprintf(jsonBuffer + strlen(jsonBuffer), remain_size, "\"%s\":", pKey);
    rc = _check_snprintf_return(rc_of_snprintf, remain_size);
    if (rc != QCLOUD_ERR_SUCCESS) {
        return rc;
    }

    if ((remain_size = sizeOfBuffer - strlen(jsonBuffer)) <= 1) {
        return QCLOUD_ERR_JSON_BUFFER_TOO_SMALL;
    }

    if (pData == NULL) {
        rc_of_snprintf = HAL_Snprintf(jsonBuffer + strlen(jsonBuffer), remain_size, "null,");
    } else {
        if (type == JINT32) {
            rc_of_snprintf = HAL_Snprintf(jsonBuffer + strlen(jsonBuffer), remain_size, "%"
                                      PRIi32
                                      ",", *(int32_t *) (pData));
        } else if (type == JINT16) {
            rc_of_snprintf = HAL_Snprintf(jsonBuffer + strlen(jsonBuffer), remain_size, "%"
                                      PRIi16
                                      ",", *(int16_t *) (pData));
        } else if (type == JINT8) {
            rc_of_snprintf = HAL_Snprintf(jsonBuffer + strlen(jsonBuffer), remain_size, "%"
                                      PRIi8
                                      ",", *(int8_t *) (pData));
        } else if (type == JUINT32) {
            rc_of_snprintf = HAL_Snprintf(jsonBuffer + strlen(jsonBuffer), remain_size, "%"
                                      PRIu32
                                      ",", *(uint32_t *) (pData));
        } else if (type == JUINT16) {
            rc_of_snprintf = HAL_Snprintf(jsonBuffer + strlen(jsonBuffer), remain_size, "%"
                                      PRIu16
                                      ",", *(uint16_t *) (pData));
        } else if (type == JUINT8) {
            rc_of_snprintf = HAL_Snprintf(jsonBuffer + strlen(jsonBuffer), remain_size, "%"
                                      PRIu8
                                      ",", *(uint8_t *) (pData));
        } else if (type == JDOUBLE) {
            rc_of_snprintf = HAL_Snprintf(jsonBuffer + strlen(jsonBuffer), remain_size, "%f,", *(double *) (pData));
        } else if (type == JFLOAT) {
            rc_of_snprintf = HAL_Snprintf(jsonBuffer + strlen(jsonBuffer), remain_size, "%f,", *(float *) (pData));
        } else if (type == JBOOL) {
            rc_of_snprintf = HAL_Snprintf(jsonBuffer + strlen(jsonBuffer), remain_size, "%s,",
                                      *(bool *) (pData) ? "true" : "false");
        } else if (type == JSTRING) {
            rc_of_snprintf = HAL_Snprintf(jsonBuffer + strlen(jsonBuffer), remain_size, "\"%s\",", (char *) (pData));
        } else if (type == JOBJECT) {
            rc_of_snprintf = HAL_Snprintf(jsonBuffer + strlen(jsonBuffer), remain_size, "%s,", (char *) (pData));
        }
    }

    rc = _check_snprintf_return(rc_of_snprintf, remain_size);

    return rc;
}

int generate_client_token(char *pStrBuffer, size_t sizeOfBuffer, uint32_t *tokenNumber) {
    return _add_client_token(pStrBuffer, sizeOfBuffer, tokenNumber);
}

void build_empty_json(uint32_t *tokenNumber, char *pJsonBuffer) {
    sprintf(pJsonBuffer, "{\"clientToken\":\"");
    sprintf(pJsonBuffer + strlen(pJsonBuffer), "%s-%u", iot_device_info_get()->product_id, (*tokenNumber)++);
    sprintf(pJsonBuffer + strlen(pJsonBuffer), "\"}");
}

bool parse_client_token(const char *pJsonDoc, int32_t tokenCount, char *pClientToken) {

    int32_t i;

    if (tokenCount <= 0) {
        if (check_and_parse_json(pJsonDoc, &tokenCount, NULL) == false) {
            return false;
        }
    }

    for (i = 1; i < tokenCount; i++) {
        if (jsoneq(pJsonDoc, &tokens[i], CLIENT_TOKEN_FIELD) == 0) {
            jsmntok_t token = tokens[i + 1];
            uint8_t length = token.end - token.start;
            strncpy(pClientToken, pJsonDoc + token.start, length);
            pClientToken[length] = '\0';
            return true;
        }
    }

    return false;
}

bool parse_version_num(const char *pJsonDoc, int32_t tokenCount, uint32_t *pVersionNumber) {

    int32_t i;
    int rc;

    if (tokenCount <= 0) {
        if (check_and_parse_json(pJsonDoc, &tokenCount, NULL) == false) {
            return false;
        }
    }

    for (i = 1; i < tokenCount; i++) {
        if (jsoneq(pJsonDoc, &(tokens[i]), VERSION_FIELD) == 0) {
            jsmntok_t dataToken = tokens[i + 1];
            rc = get_uint32(pVersionNumber, pJsonDoc, &dataToken);
            if (rc == QCLOUD_ERR_SUCCESS) {
                return true;
            }
        }
    }
    return false;
}

bool parse_error_code(const char *pJsonDoc, int32_t tokenCount, uint16_t *pErrorCode) {

    int32_t i;
    int rc;

    if (tokenCount <= 0) {
        if (check_and_parse_json(pJsonDoc, &tokenCount, NULL) == false) {
            return false;
        }
    }

    for (i = 1; i < tokenCount; i++) {
        if (jsoneq(pJsonDoc, &(tokens[i]), CODE_FIELD) == 0) {
            jsmntok_t dataToken = tokens[i + 1];
            rc = get_uint16(pErrorCode, pJsonDoc, &dataToken);
            if (rc == QCLOUD_ERR_SUCCESS) {
                return true;
            }
        }
    }
    return false;
}

bool parse_error_message(const char *pJsonDoc, int32_t tokenCount, char *pErrorMessage) {

    int32_t i;
    int rc;

    if (tokenCount <= 0) {
        if (check_and_parse_json(pJsonDoc, &tokenCount, NULL) == false) {
            return false;
        }
    }

    for (i = 1; i < tokenCount; i++) {
        if (jsoneq(pJsonDoc, &(tokens[i]), MESSAGE_FIELD) == 0) {
            jsmntok_t dataToken = tokens[i + 1];
            rc = get_string(pErrorMessage, pJsonDoc, &dataToken);
            if (rc == QCLOUD_ERR_SUCCESS) {
                return true;
            }
        }
    }
    return false;
}

bool parse_shadow_state(const char *pJsonDoc, int32_t tokenCount, char *pState)
{
    int32_t i;

    if (tokenCount <= 0) {
        if (check_and_parse_json(pJsonDoc, &tokenCount, NULL) == false) {
            return false;
        }
    }

    for (i = 1; i < tokenCount; i++) {
        if (jsoneq(pJsonDoc, &tokens[i], STATE_FIELD) == 0) {
            int size = 0;
            size = tokens[i+1].end - tokens[i+1].start;
            if (pState != NULL && memcpy(pState, pJsonDoc + tokens[i+1].start, size) != NULL)
            {
                pState[size] = '\0';
            }
            return true;
        }
    }

    return false;
}

bool parse_shadow_operation_type(const char *pJsonDoc, int32_t tokenCount, char *pType)
{
    int32_t i;
    int rc;

    if (tokenCount <= 0) {
        if (check_and_parse_json(pJsonDoc, &tokenCount, NULL) == false) {
            return false;
        }
    }

    for (i = 1; i < tokenCount; i++) {
        if (jsoneq(pJsonDoc, &tokens[i], OPERATION_TYPE) == 0) {
            jsmntok_t dataToken = tokens[i + 1];
            rc = get_string(pType, pJsonDoc, &dataToken);
            if (rc == QCLOUD_ERR_SUCCESS) {
                return true;
            }
        }
    }

    return false;
}

bool parse_shadow_operation_result_code(const char *pJsonDoc, int32_t tokenCount, int16_t *pResultCode)
{
    int32_t i;
    int rc;

    if (tokenCount <= 0) {
        if (check_and_parse_json(pJsonDoc, &tokenCount, NULL) == false) {
            return false;
        }
    }

    for (i = 1; i < tokenCount; i++) {
        if (jsoneq(pJsonDoc, &(tokens[i]), OPERATION_RESULT) == 0) {
            jsmntok_t dataToken = tokens[i + 1];
            rc = get_int16(pResultCode, pJsonDoc, &dataToken);
            if (rc == QCLOUD_ERR_SUCCESS) {
                return true;
            }
        }
    }

    return false;
}

bool parse_shadow_operation_delta(const char *pJsonDoc, int32_t tokenCount, char *pDelta)
{
    int32_t i;
    char state_json[QCLOUD_IOT_MQTT_RX_BUF_LEN] = {0};
    if (parse_shadow_state(pJsonDoc, tokenCount, state_json) == false) return false;

    int32_t state_json_token_cnt;
    if (check_and_parse_json(state_json, &state_json_token_cnt, NULL) == false) return false;

    for (i = 1; i < state_json_token_cnt; i++) {
        if (jsoneq(state_json, &tokens[i], OPERATION_DELTA) == 0) {
            int size = 0;
            size = tokens[i+1].end - tokens[i+1].start;
            if (pDelta != NULL && memcpy(pDelta, state_json + tokens[i+1].start, size) != NULL)
            {
                pDelta[size] = '\0';
            }
            return true;
        }
    }
    return false;
}

bool update_value_if_key_match(const char *pJsonDoc, int32_t tokenCount,
                               DeviceProperty *pProperty, uint32_t *pDataLength, int32_t *pDataPosition) {
    int32_t i;

    char state_json[QCLOUD_IOT_MQTT_RX_BUF_LEN] = {0};
    for (i = 1; i < tokenCount; i++) {
        if (jsoneq(pJsonDoc, &(tokens[i]), STATE_FIELD) == 0)
        {
            jsmntok_t dataToken = tokens[i + 1];
            uint32_t dataLength = dataToken.end - dataToken.start;
            *pDataPosition = dataToken.start;
            if (memcpy(state_json, pJsonDoc + dataToken.start, dataLength) != NULL) 
            {
                int32_t token_count;
                if(check_and_parse_json(state_json, &token_count, NULL)) {
                    bool rc = _prase_shadow_operation_delta_key_in_state(state_json, token_count, pProperty, pDataLength, pDataPosition);
                    return rc;
                }
            }
        }
        
    }

    return false;
}

#ifdef __cplusplus
}
#endif
