#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include "shadow_client_json.h"

#include "device.h"
#include "qcloud_iot_export.h"
#include "qcloud_iot_json_utils.h"

extern uint32_t client_token_num;

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
static int32_t _add_client_token(char *pJsonDocument, size_t maxSizeOfJsonDocument) {

    int32_t rc_of_snprintf = snprintf(pJsonDocument, maxSizeOfJsonDocument, "%s-%d", iot_device_info_get()->client_id, client_token_num++);

    return rc_of_snprintf;
}

int generate_client_token(char *pStrBuffer, size_t sizeOfBuffer) {
    return _add_client_token(pStrBuffer, sizeOfBuffer);
}

void build_empty_json(char *pJsonBuffer) {
    sprintf(pJsonBuffer, "{\"clientToken\":\"");
    sprintf(pJsonBuffer + strlen(pJsonBuffer), "%s-%d", iot_device_info_get()->client_id, client_token_num++);
    sprintf(pJsonBuffer + strlen(pJsonBuffer), "\"}");
}

//static jsmn_parser parser;
//static jsmntok_t tokens[MAX_JSON_TOKEN_EXPECTED];

//bool check_json_valid(const char *pJsonDoc) {
//    if (pJsonDoc == NULL) {
//        return false;
//    }
//
//    int32_t tokenCount;
//
//    jsmn_init(&parser);
//
//    tokenCount = jsmn_parse(&parser, pJsonDoc, strlen(pJsonDoc), tokens,
//                            sizeof(tokens) / sizeof(tokens[0]));
//
//    if (tokenCount < 0) {
//        Log_w("Failed to parse JSON: %d\n", tokenCount);
//        return false;
//    }
//
//    /* Assume the top-level element is an object */
//    if (tokenCount < 1 || tokens[0].type != JSMN_OBJECT) {
//        return false;
//    }
//
//    return true;
//}

//bool check_and_parse_json(const char *pJsonDoc, int32_t *pTokenCount, void **pJsonTokens) {
//    int32_t tokenCount = 0;
//    *pTokenCount = 0;
//
//    if (pJsonDoc == NULL) {
//        return false;
//    }
//
//    jsmn_init(&parser);
//
//    tokenCount = jsmn_parse(&parser, pJsonDoc, strlen(pJsonDoc), tokens, sizeof(tokens) / sizeof(tokens[0]));
//
//    if (tokenCount < 0) {
//        Log_w("jsmn_parse failed returned: %d\n", tokenCount);
//        return false;
//    }
//
//    /* Assume the top-level element is an object */
//    if (tokenCount < 1 || tokens[0].type != JSMN_OBJECT) {
//        return false;
//    }
//
//    if (pJsonTokens != NULL) {
//        *pJsonTokens = (void *) tokens;
//    }
//
//    *pTokenCount = tokenCount;
//
//    return true;
//}

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

bool update_value_if_key_match(const char *pJsonDoc, int32_t tokenCount,
                               DeviceProperty *pProperty, uint32_t *pDataLength, int32_t *pDataPosition) {
    int32_t i;

    for (i = 1; i < tokenCount; i++) {
        if (jsoneq(pJsonDoc, &(tokens[i]), pProperty->key) == 0) {
            jsmntok_t dataToken = tokens[i + 1];
            uint32_t dataLength = dataToken.end - dataToken.start;
            _update_value_if_no_object(pJsonDoc, pProperty, dataToken);
            *pDataPosition = dataToken.start;
            *pDataLength = dataLength;
            return true;
        }
    }

    return false;
}

#ifdef __cplusplus
}
#endif
