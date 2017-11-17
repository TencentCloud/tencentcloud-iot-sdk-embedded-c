/*
 * qcloud_iot_shadow_json.c
 *
 *  Created on: 2017年10月25日
 *      Author: shockcao
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "qcloud_iot_export_shadow_json.h"

#include <string.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include "jsmn.h"
#include "shadow_client_json.h"
#include "qcloud_iot_export.h"
#include "qcloud_iot_json_utils.h"

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
 * 将一个JSON节点写入到JSON串中
 *
 * @param pJsonBuffer   JSON串
 * @param sizeOfBuffer  可写入大小
 * @param pKey          JSON节点的key
 * @param pData         JSON节点的value
 * @param type          JSON节点value的数据类型
 * @return              返回QCLOUD_ERR_SUCCESS, 表示成功
 */
static int _put_json_node(char *pJsonBuffer, size_t sizeOfBuffer, const char *pKey, void *pData, JsonDataType type) {

    int rc;
    int32_t rc_of_snprintf = 0;
    size_t remain_size = 0;

    if ((remain_size = sizeOfBuffer - strlen(pJsonBuffer)) <= 1) {
        return QCLOUD_ERR_JSON_BUFFER_TOO_SMALL;
    }

    rc_of_snprintf = snprintf(pJsonBuffer + strlen(pJsonBuffer), remain_size, "\"%s\":", pKey);
    rc = _check_snprintf_return(rc_of_snprintf, remain_size);
    if (rc != QCLOUD_ERR_SUCCESS) {
        return rc;
    }

    if ((remain_size = sizeOfBuffer - strlen(pJsonBuffer)) <= 1) {
        return QCLOUD_ERR_JSON_BUFFER_TOO_SMALL;
    }

    if (pData == NULL) {
        rc_of_snprintf = snprintf(pJsonBuffer + strlen(pJsonBuffer), remain_size, "null,");
    } else {
        if (type == JINT32) {
            rc_of_snprintf = snprintf(pJsonBuffer + strlen(pJsonBuffer), remain_size, "%"
                                      PRIi32
                                      ",", *(int32_t *) (pData));
        } else if (type == JINT16) {
            rc_of_snprintf = snprintf(pJsonBuffer + strlen(pJsonBuffer), remain_size, "%"
                                      PRIi16
                                      ",", *(int16_t *) (pData));
        } else if (type == JINT8) {
            rc_of_snprintf = snprintf(pJsonBuffer + strlen(pJsonBuffer), remain_size, "%"
                                      PRIi8
                                      ",", *(int8_t *) (pData));
        } else if (type == JUINT32) {
            rc_of_snprintf = snprintf(pJsonBuffer + strlen(pJsonBuffer), remain_size, "%"
            						  PRIu32
                                      ",", *(uint32_t *) (pData));
        } else if (type == JUINT16) {
            rc_of_snprintf = snprintf(pJsonBuffer + strlen(pJsonBuffer), remain_size, "%"
                                      PRIu16
                                      ",", *(uint16_t *) (pData));
        } else if (type == JUINT8) {
            rc_of_snprintf = snprintf(pJsonBuffer + strlen(pJsonBuffer), remain_size, "%"
                                      PRIu8
                                      ",", *(uint8_t *) (pData));
        } else if (type == JDOUBLE) {
            rc_of_snprintf = snprintf(pJsonBuffer + strlen(pJsonBuffer), remain_size, "%f,", *(double *) (pData));
        } else if (type == JFLOAT) {
            rc_of_snprintf = snprintf(pJsonBuffer + strlen(pJsonBuffer), remain_size, "%f,", *(float *) (pData));
        } else if (type == JBOOL) {
            rc_of_snprintf = snprintf(pJsonBuffer + strlen(pJsonBuffer), remain_size, "%s,",
                                      *(bool *) (pData) ? "true" : "false");
        } else if (type == JSTRING) {
            rc_of_snprintf = snprintf(pJsonBuffer + strlen(pJsonBuffer), remain_size, "\"%s\",", (char *) (pData));
        }
    }

    rc = _check_snprintf_return(rc_of_snprintf, remain_size);

    return rc;
}

/**
 * @brief 初始化一个JSON文档
 *
 * 本函数主要是为JSON文档添加state字段, 即 "{\"state\":{", 所以在生成JSON文档时, 请先调用该方法
 *
 * @param pJsonBuffer   为存储JSON文档准备的字符串缓冲区
 * @param sizeOfBuffer  缓冲区大小
 * @return              返回QCLOUD_ERR_SUCCESS, 表示初始化成功
 */
static int IOT_Shadow_JSON_Init(char *pJsonBuffer, size_t sizeOfBuffer) {

    if (pJsonBuffer == NULL) {
        return QCLOUD_ERR_INVAL;
    }

    int32_t rc_of_snprintf = snprintf(pJsonBuffer, sizeOfBuffer, "{\"state\":{");

    return _check_snprintf_return(rc_of_snprintf, sizeOfBuffer);
}

/**
 * @brief 在JSON文档中添加结尾部分的内容, 包括clientToken字段、version字段
 *
 * @param pJsonBuffer    为存储JSON文档准备的字符串缓冲区
 * @param sizeOfBuffer   缓冲区大小
 * @return               返回QCLOUD_ERR_SUCCESS, 表示成功
 */
static int IOT_Shadow_JSON_Finalize(char *pJsonBuffer, size_t sizeOfBuffer) {
	int rc;
	size_t remain_size = 0;
	int32_t rc_of_snprintf = 0;

	if (pJsonBuffer == NULL) {
		return QCLOUD_ERR_INVAL;
	}

	if ((remain_size = sizeOfBuffer - strlen(pJsonBuffer)) <= 1) {
		return QCLOUD_ERR_JSON_BUFFER_TOO_SMALL;
	}

	rc_of_snprintf = snprintf(pJsonBuffer + strlen(pJsonBuffer) - 1, remain_size, "}, \"%s\":\"", CLIENT_TOKEN_FIELD);
	rc = _check_snprintf_return(rc_of_snprintf, remain_size);
	Log_i("%d", rc);
	if (rc != QCLOUD_ERR_SUCCESS) {
		return rc;
	}

	if ((remain_size = sizeOfBuffer - strlen(pJsonBuffer)) <= 1) {
		return QCLOUD_ERR_JSON_BUFFER_TOO_SMALL;
	}

	rc_of_snprintf = generate_client_token(pJsonBuffer + strlen(pJsonBuffer), remain_size);
	rc = _check_snprintf_return(rc_of_snprintf, remain_size);

	if (rc != QCLOUD_ERR_SUCCESS) {
		return rc;
	}

	if ((remain_size = sizeOfBuffer - strlen(pJsonBuffer)) <= 1) {
		return QCLOUD_ERR_JSON_BUFFER_TOO_SMALL;
	}

	rc_of_snprintf = snprintf(pJsonBuffer + strlen(pJsonBuffer), remain_size, "\"}");
	rc = _check_snprintf_return(rc_of_snprintf, remain_size);

	return rc;
}

int IOT_Shadow_JSON_ConstructReport(char *pJsonBuffer, size_t sizeOfBuffer, uint8_t count, ...) {
	int rc = IOT_Shadow_JSON_Init(pJsonBuffer, sizeOfBuffer);

	if (rc != QCLOUD_ERR_SUCCESS) {
		Log_e("shadow json init failed: %d", rc);
		return rc;
	}

    size_t remain_size = 0;
    int32_t rc_of_snprintf = 0;
    int8_t i;
    va_list pArgs;
    va_start(pArgs, count);
    DeviceProperty *pJsonNode;

    if (pJsonBuffer == NULL) {
        return QCLOUD_ERR_INVAL;
    }

    if ((remain_size = sizeOfBuffer - strlen(pJsonBuffer)) <= 1) {
        return QCLOUD_ERR_JSON_BUFFER_TOO_SMALL;
    }

    rc_of_snprintf = snprintf(pJsonBuffer + strlen(pJsonBuffer), remain_size, "\"reported\":{");
    rc = _check_snprintf_return(rc_of_snprintf, remain_size);

    if (rc != QCLOUD_ERR_SUCCESS) {
        return rc;
    }

    for (i = 0; i < count; i++) {
        pJsonNode = va_arg(pArgs, DeviceProperty *);
        if (pJsonNode != NULL && pJsonNode->key != NULL) {
            rc = _put_json_node(pJsonBuffer, remain_size, pJsonNode->key, pJsonNode->data, pJsonNode->type);

            if (rc != QCLOUD_ERR_SUCCESS) {
                return rc;
            }
        } else {
            return QCLOUD_ERR_INVAL;
        }
    }

    va_end(pArgs);
    if ((remain_size = sizeOfBuffer - strlen(pJsonBuffer)) <= 1) {
        return QCLOUD_ERR_JSON_BUFFER_TOO_SMALL;
    }
    rc_of_snprintf = snprintf(pJsonBuffer + strlen(pJsonBuffer) - 1, remain_size, "},");
    rc = _check_snprintf_return(rc_of_snprintf, remain_size);

	if (rc != QCLOUD_ERR_SUCCESS) {
		Log_e("shadow json add report failed: %d", rc);
		return rc;
	}

	rc = IOT_Shadow_JSON_Finalize(pJsonBuffer, sizeOfBuffer);
	if (rc != QCLOUD_ERR_SUCCESS) {
		Log_e("shadow json finalize failed: %d", rc);
	}

	return rc;
}

int IOT_Shadow_JSON_ConstructDesire(char *pJsonBuffer, size_t sizeOfBuffer, uint8_t count, ...) {
	int rc = IOT_Shadow_JSON_Init(pJsonBuffer, sizeOfBuffer);

	if (rc != QCLOUD_ERR_SUCCESS) {
		Log_e("shadow json init failed: %d", rc);
		return rc;
	}

    size_t remain_size = 0;
    int32_t rc_of_snprintf = 0;
    int8_t i;
    va_list pArgs;
    va_start(pArgs, count);
    DeviceProperty *pJsonNode;

    if (pJsonBuffer == NULL) {
        return QCLOUD_ERR_INVAL;
    }

    if ((remain_size = sizeOfBuffer - strlen(pJsonBuffer)) <= 1) {
        return QCLOUD_ERR_JSON_BUFFER_TOO_SMALL;
    }

    rc_of_snprintf = snprintf(pJsonBuffer + strlen(pJsonBuffer), remain_size, "\"desired\":{");
    rc = _check_snprintf_return(rc_of_snprintf, remain_size);

    if (rc != QCLOUD_ERR_SUCCESS) {
        return rc;
    }

    for (i = 0; i < count; i++) {
        pJsonNode = va_arg (pArgs, DeviceProperty *);
        if (pJsonNode != NULL && pJsonNode->key != NULL) {
            rc = _put_json_node(pJsonBuffer, remain_size, pJsonNode->key, pJsonNode->data, pJsonNode->type);
            if (rc != QCLOUD_ERR_SUCCESS) {
                return rc;
            }
        } else {
            return QCLOUD_ERR_INVAL;
        }
    }

    va_end(pArgs);
    if ((remain_size = sizeOfBuffer - strlen(pJsonBuffer)) <= 1) {
        return QCLOUD_ERR_JSON_BUFFER_TOO_SMALL;
    }
    // strlen(pJsonBuffer) - 1 是为了把最后一项的逗号去掉
    rc_of_snprintf = snprintf(pJsonBuffer + strlen(pJsonBuffer) - 1, remain_size, "},");
    rc = _check_snprintf_return(rc_of_snprintf, remain_size);

	if (rc != QCLOUD_ERR_SUCCESS) {
		Log_e("shadow json add desired failed: %d", rc);
		return rc;
	}

	rc = IOT_Shadow_JSON_Finalize(pJsonBuffer, sizeOfBuffer);
	return rc;
}

#ifdef __cplusplus
}
#endif

