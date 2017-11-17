#ifdef __cplusplus
extern "C" {
#endif

#include "qcloud_iot_json_utils.h"

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdbool.h>

int8_t jsoneq(const char *json, jsmntok_t *tok, const char *key) {
    if (tok->type == JSMN_STRING && (int) strlen(key) == tok->end - tok->start &&
        strncmp(json + tok->start, key, strlen(key)) == 0) {
        return 0;
    }
    return -1;
}

int get_uint32(uint32_t *i, const char *json, jsmntok_t *tok) {
    if (tok->type != JSMN_PRIMITIVE) {
        return QCLOUD_ERR_JSON_PARSE;
    }

    if (json[tok->start] == '-' || sscanf(json + tok->start, "%" SCNu32, i) != 1) {
        return QCLOUD_ERR_JSON_PARSE;
    }

    return QCLOUD_ERR_SUCCESS;
}

int get_uint16(uint16_t *i, const char *json, jsmntok_t *tok) {
    if (tok->type != JSMN_PRIMITIVE) {
        return QCLOUD_ERR_JSON_PARSE;
    }

    if (json[tok->start] == '-' || sscanf(json + tok->start, "%" SCNu16, i) != 1) {
        return QCLOUD_ERR_JSON_PARSE;
    }

    return QCLOUD_ERR_SUCCESS;
}

int get_uint8(uint8_t *i, const char *json, jsmntok_t *tok) {
    if (tok->type != JSMN_PRIMITIVE) {
        return QCLOUD_ERR_JSON_PARSE;
    }

    if (json[tok->start] == '-' || sscanf(json + tok->start, "%" SCNu8, i) != 1) {
        return QCLOUD_ERR_JSON_PARSE;
    }

    return QCLOUD_ERR_SUCCESS;
}

int get_int32(int32_t *i, const char *json, jsmntok_t *tok) {
    if (tok->type != JSMN_PRIMITIVE) {
        return QCLOUD_ERR_JSON_PARSE;
    }

    if (sscanf(json + tok->start, "%" SCNi32, i) != 1) {
        return QCLOUD_ERR_JSON_PARSE;
    }

    return QCLOUD_ERR_SUCCESS;
}

int get_int16(int16_t *i, const char *json, jsmntok_t *tok) {
    if (tok->type != JSMN_PRIMITIVE) {
        return QCLOUD_ERR_JSON_PARSE;
    }

    if (sscanf(json + tok->start, "%" SCNi16, i) != 1) {
        return QCLOUD_ERR_JSON_PARSE;
    }

    return QCLOUD_ERR_SUCCESS;
}

int get_int8(int8_t *i, const char *json, jsmntok_t *tok) {
    if (tok->type != JSMN_PRIMITIVE) {
        return QCLOUD_ERR_JSON_PARSE;
    }

    if (sscanf(json + tok->start, "%" SCNi8, i) != 1) {
        return QCLOUD_ERR_JSON_PARSE;
    }

    return QCLOUD_ERR_SUCCESS;
}

int get_float(float *f, const char *json, jsmntok_t *tok) {
    if (tok->type != JSMN_PRIMITIVE) {
        return QCLOUD_ERR_JSON_PARSE;
    }

    if (sscanf(json + tok->start, "%f", f) != 1) {
        return QCLOUD_ERR_JSON_PARSE;
    }

    return QCLOUD_ERR_SUCCESS;
}

int get_double(double *d, const char *json, jsmntok_t *tok) {
    if (tok->type != JSMN_PRIMITIVE) {
        return QCLOUD_ERR_JSON_PARSE;
    }

    if (sscanf(json + tok->start, "%lf", d) != 1) {
        return QCLOUD_ERR_JSON_PARSE;
    }

    return QCLOUD_ERR_SUCCESS;
}

int get_boolean(bool *b, const char *json, jsmntok_t *tok) {
    if (tok->type != JSMN_PRIMITIVE) {
        return QCLOUD_ERR_JSON_PARSE;
    }

    if (json[tok->start] == 't' && json[tok->start + 1] == 'r' && json[tok->start + 2] == 'u'
        && json[tok->start + 3] == 'e') {
        *b = true;
    } else if (json[tok->start] == 'f' && json[tok->start + 1] == 'a'
               && json[tok->start + 2] == 'l' && json[tok->start + 3] == 's'
               && json[tok->start + 4] == 'e') {
        *b = false;
    } else {
        return QCLOUD_ERR_JSON_PARSE;
    }

    return QCLOUD_ERR_SUCCESS;
}

int get_string(char *buf, const char *json, jsmntok_t *tok) {
    int size = 0;
    if (tok->type != JSMN_STRING) {
        return QCLOUD_ERR_JSON_PARSE;
    }
    size = tok->end - tok->start;
    memcpy(buf, json + tok->start, size);
    buf[size] = '\0';
    return QCLOUD_ERR_SUCCESS;
}

static jsmn_parser parser;
jsmntok_t tokens[MAX_JSON_TOKEN_EXPECTED];

bool check_json_valid(const char *pJsonDoc) {
    if (pJsonDoc == NULL) {
        return false;
    }

    int32_t tokenCount;

    jsmn_init(&parser);

    tokenCount = jsmn_parse(&parser, pJsonDoc, strlen(pJsonDoc), tokens,
                            sizeof(tokens) / sizeof(tokens[0]));

    if (tokenCount < 0) {
        Log_w("Failed to parse JSON: %d\n", tokenCount);
        return false;
    }

    /* Assume the top-level element is an object */
    if (tokenCount < 1 || tokens[0].type != JSMN_OBJECT) {
        return false;
    }

    return true;
}

bool check_and_parse_json(const char *pJsonDoc, int32_t *pTokenCount, void **pJsonTokens) {
    int32_t tokenCount = 0;
    *pTokenCount = 0;

    if (pJsonDoc == NULL) {
        return false;
    }

    jsmn_init(&parser);

    tokenCount = jsmn_parse(&parser, pJsonDoc, strlen(pJsonDoc), tokens, sizeof(tokens) / sizeof(tokens[0]));

    if (tokenCount < 0) {
        Log_w("jsmn_parse failed returned: %d\n", tokenCount);
        return false;
    }

    /* Assume the top-level element is an object */
    if (tokenCount < 1 || tokens[0].type != JSMN_OBJECT) {
        return false;
    }

    if (pJsonTokens != NULL) {
        *pJsonTokens = (void *) tokens;
    }

    *pTokenCount = tokenCount;

    return true;
}

#ifdef __cplusplus
}
#endif
