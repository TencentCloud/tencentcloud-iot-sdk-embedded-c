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

#include "mqtt_client_json.h"

#include "qcloud_iot_export.h"
#include "qcloud_iot_utils_json.h"

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


