/*
 * qcloud_iot_mqtt_json.c
 *
 *  Created on: 2017年10月25日
 *      Author: shockcao
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "qcloud_iot_export_mqtt_json.h"

#include "mqtt_client_json.h"

bool IOT_MQTT_JSON_GetAction(const char *pJsonDoc, int32_t tokenCount, char *pAction) {
	return parse_action(pJsonDoc, tokenCount, pAction);
}

#ifdef __cplusplus
}
#endif

