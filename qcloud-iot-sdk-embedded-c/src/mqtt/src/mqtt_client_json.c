/*
 * mqtt_client_json.c
 *
 *  Created on: 2017年11月6日
 *      Author: shockcao
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "mqtt_client_json.h"

#include "qcloud_iot_export.h"
#include "qcloud_iot_json_utils.h"

bool parse_action(const char *pJsonDoc, int32_t tokenCount, char *pAction) {
	int32_t i;

	if (tokenCount <= 0) {
		if (check_and_parse_json(pJsonDoc, &tokenCount, NULL) == false) {
			Log_e("Received JSON is not valid");
			return false;
		}
	}

	for (i = 1; i < tokenCount; i++) {
		if (jsoneq(pJsonDoc, &tokens[i], ACTION_FIELD) == 0) {
			jsmntok_t token = tokens[i + 1];
			uint8_t length = token.end - token.start;
			strncpy(pAction, pJsonDoc + token.start, length);
			pAction[length] = '\0';
			return true;
		}
	}

	return false;
}

#ifdef __cplusplus
}
#endif


