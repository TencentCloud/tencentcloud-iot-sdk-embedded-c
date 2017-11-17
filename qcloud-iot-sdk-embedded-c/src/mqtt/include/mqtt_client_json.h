/*
 * mqtt_client_json.h
 *
 *  Created on: 2017年11月6日
 *      Author: shockcao
 */

#ifndef MQTT_CLIENT_JSON_H_
#define MQTT_CLIENT_JSON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#define ACTION_FIELD           "action"

bool parse_action(const char *pJsonDoc, int32_t tokenCount, char *pAction);

#ifdef __cplusplus
}
#endif


#endif /* MQTT_CLIENT_JSON_H_ */
