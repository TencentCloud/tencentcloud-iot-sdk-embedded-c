#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "shadow_client.h"

#include "qcloud_iot_export_shadow_json.h"
#include "qcloud_iot_sdk_impl_internal.h"
#include "qcloud_iot_json_utils.h"
#include "qcloud_iot_import.h"

#define MAX_TOPICS_AT_ANY_GIVEN_TIME 2 * MAX_DEVICE_HANDLED_AT_ANY_GIVEN_TIME
#define SUBSCRIBE_WAITING_TIME 2

/**
 * @brief 代表一个文档请求
 */
typedef struct {
    char                   client_token[MAX_SIZE_OF_CLIENT_TOKEN];    // 标识该请求的clientToken字段
    Method                 method;                                    // 文档操作方式

    void                   *user_data;                                // 用户数据
    bool                   is_free;                                   // 标识是否为空闲状态
    Timer                  timer;                                     // 请求超时定时器

    OnRequestCallback      callback;                                  // 文档操作请求返回处理函数
} Request;

/**
 * @brief 客户端订阅主题的结构体定义
 */
typedef struct {
    char       topic[MAX_SIZE_OF_CLOUD_TOPIC]; // 订阅主题名
    uint8_t    count;                          // 该订阅主题的订阅次数
    bool       is_free;                        // 该结构体是否处于空闲状态, 用遍历数组的时候判断元素是否被赋值
} Subscription;

/**
 * @brief 用于生成不同的主题
 */
typedef enum {
    ACCEPTED, REJECTED, METHOD
} RequestType;

//Qcloud_IoT_Client *pMqttClient;

/**
 * @brief 记录订阅的主题
 */
static Subscription sg_subscriptions[MAX_TOPICS_AT_ANY_GIVEN_TIME];

/**
 * @brief 保存所有设备注册属性的回调方法
 */
static Request sg_pending_requests[MAX_APPENDING_REQUEST_AT_ANY_GIVEN_TIME];

static char cloud_rcv_buf[CLOUD_RX_BUF_LEN];
   
/**
 * @brief 最近收到的文档的版本号
 */
uint32_t json_document_version = 0;
    
bool discard_old_delta_flag = true;
uint32_t client_token_num = 0;
    
/**
 * @brief 根据参数生成不同的topic
 *
 * 生成的topic如下:
 *     /get
 *     /get/accepted
 *     /get/rejected
 *     /update
 *     /update/accepted
 *     /update/rejected
 *     /delete
 *     /delete/accepted
 *     /delete/rejected
 *
 * @param pTopic         生成的topic
 * @param pDeviceName    操作设备的名称
 * @param method         文档操作方式
 * @param requestType    topic类型
 */
static void _get_topic(char *pTopic, int topicSize, Method method, RequestType requestType) {

    char methodStr[10];
    char reqTypeStr[10];

    if (method == GET) {
        strcpy(methodStr, "get");
    } else if (method == UPDATE) {
        strcpy(methodStr, "update");
    } else if (method == DELETE) {
        strcpy(methodStr, "delete");
    }

    if (requestType == ACCEPTED) {
        strcpy(reqTypeStr, "accepted");
    } else if (requestType == REJECTED) {
        strcpy(reqTypeStr, "rejected");
    }

    if (requestType == METHOD) {
        stiching_shadow_topic(pTopic, topicSize, methodStr);
    } else {
        char str[MAX_SIZE_OF_CLOUD_TOPIC_WITHOUT_DEVICE_NAME];
        sprintf(str, "%s/%s",methodStr, reqTypeStr);
        stiching_shadow_topic(pTopic, topicSize, str);
    }
}

/**
 * 获取某个主题在已订阅主题列表中的索引位置
 *
 * @param pTopic 查询的主题名
 * @return 索引位置
 */
static int16_t _get_index_in_subscription_list(const char *pTopic) {

    uint8_t i;

    for (i = 0; i < MAX_TOPICS_AT_ANY_GIVEN_TIME; i++) {
        if (!sg_subscriptions[i].is_free) {
            if ((strcmp(pTopic, sg_subscriptions[i].topic) == 0)) {
                return i;
            }
        }
    }

    return -1;
}

/**
 * 是否需要更新 本设备 的文档版本号
 *
 * @param pTopicName
 * @return
 */
static bool _is_need_update_version_for_self(const char *pTopicName) {

    if (strstr(pTopicName, iot_device_info_get()->device_name) != NULL) {
        if (strstr(pTopicName, "get/accepted") != NULL || strstr(pTopicName, "delta") != NULL) {
            return true;
        }
    }

    return false;
}

/**
 * 若某个文档请求之前有请求过, 则增加相应topic的订阅次数
 *
 * @param method      文档操作方式
 */
static void _increment_subscription_cnt(Method method) {

    uint8_t i;
    char accepted_topic[MAX_SIZE_OF_CLOUD_TOPIC] = {0};
    char rejected_topic[MAX_SIZE_OF_CLOUD_TOPIC] = {0};

    _get_topic(accepted_topic, sizeof(accepted_topic), method, ACCEPTED);
    _get_topic(rejected_topic, sizeof(rejected_topic), method, REJECTED);

    for (i = 0; i < MAX_TOPICS_AT_ANY_GIVEN_TIME; i++) {
        if (!sg_subscriptions[i].is_free) {
            if ((strcmp(accepted_topic, sg_subscriptions[i].topic) == 0)) {
                Log_d("increment accepted_topic count=%d", ++(sg_subscriptions[i].count));
            }
            else if ((strcmp(rejected_topic, sg_subscriptions[i].topic) == 0)) {
            	Log_d("increment rejected_topic count=%d", ++(sg_subscriptions[i].count));
            }
        }
    }
}

/**
 * 文档请求之前有请求过, 减少相应topic的订阅次数
 *
 * @param index      pending request index，使用该index取出request，再构建topic找到subscription
 */
static void _decrement_subscription_cnt(uint8_t index) {

    char accepted_topic[MAX_SIZE_OF_CLOUD_TOPIC];
    char rejected_topic[MAX_SIZE_OF_CLOUD_TOPIC];

    Request request = sg_pending_requests[index];
    _get_topic(accepted_topic, sizeof(accepted_topic), request.method, ACCEPTED);
    _get_topic(rejected_topic, sizeof(rejected_topic), request.method, REJECTED);

    int16_t i = _get_index_in_subscription_list(accepted_topic);
    if (i >= 0) {
		sg_subscriptions[i].count--;
		Log_d("decrement accepted_topic count=%d", sg_subscriptions[i].count);
    }

    i = _get_index_in_subscription_list(rejected_topic);
    if (i >= 0) {
		sg_subscriptions[i].count--;
		Log_d("decrement rejected_topic count=%d", sg_subscriptions[i].count);
    }
}


/**
 * 文档操作请求响应回调函数
 * 成功订阅accepted和rejected主题之后，如果收到对应的accept/rejected消息则调用此方法 
 *
 * @param params
 */
static void _on_accepted_rejected_message_handler(char *topicName, uint16_t topicNameLen, MQTTMessage *params, void *pUserdata) {
    int32_t tokenCount;
    uint8_t i;
    char client_token[MAX_SIZE_OF_CLIENT_TOKEN];

    STRING_PTR_SANITY_CHECK_RTN(topicName);
	POINTER_SANITY_CHECK_RTN(params);
	NUMBERIC_SANITY_CHECK_RTN(topicNameLen);
    if (params->payload_len > CLOUD_RX_BUF_LEN) {
        return;
    }

    memcpy(cloud_rcv_buf, params->payload, params->payload_len);
    cloud_rcv_buf[params->payload_len] = '\0';    // jsmn_parse relies on a string

    if (!check_and_parse_json(cloud_rcv_buf, &tokenCount, NULL)) {
        Log_w("Received JSON is not valid");
        return;
    }
    
    if (_is_need_update_version_for_self(topicName)) {
        uint32_t version_num = 0;
        if (parse_version_num(cloud_rcv_buf, tokenCount, &version_num)) {
            if (version_num > json_document_version) {
                json_document_version = version_num;
            }
        }
    }

    if (!parse_client_token(cloud_rcv_buf, tokenCount, client_token)) return;

	for (i = 0; i < MAX_APPENDING_REQUEST_AT_ANY_GIVEN_TIME; i++) {

		if (!sg_pending_requests[i].is_free && (strcmp(sg_pending_requests[i].client_token, client_token) == 0)) {
			RequestAck status = ACK_TIMEOUT;
			if (strstr(topicName, "accepted") != NULL) {
				status = ACK_ACCEPTED;
			} else if (strstr(topicName, "rejected") != NULL) {
				status = ACK_REJECTED;
			}
			if (status == ACK_ACCEPTED || status == ACK_REJECTED) {
				if (sg_pending_requests[i].callback != NULL) {
					sg_pending_requests[i].callback(sg_pending_requests[i].method, status, cloud_rcv_buf, sg_pending_requests[i].user_data);
				}
				sg_pending_requests[i].is_free = true;

				_decrement_subscription_cnt(i);
			}
		}
	}
}

/**
 * 获取已订阅主题列表中的空闲位置
 *
 * @return
 */
static int16_t _get_free_index_in_subscription_list(void) {
    uint8_t i;

    for (i = 0; i < MAX_TOPICS_AT_ANY_GIVEN_TIME; i++) {
        if (sg_subscriptions[i].is_free) {
            sg_subscriptions[i].is_free = false;
            return i;
        }
    }
    return -1;
}


/**
 * 判断相应文档请求对应的ACCEPT和REJECT主题是否已经被订阅
 *
 * @param Method            文档操作方式
 */
static bool _has_subscribe_accept_reject_to_cloud(Method method) {

    uint8_t i = 0;
    bool accepted = false;
    bool rejected = false;
    char accepted_topic[MAX_SIZE_OF_CLOUD_TOPIC] = {0};
    char rejected_topic[MAX_SIZE_OF_CLOUD_TOPIC] = {0};

    _get_topic(accepted_topic, sizeof(accepted_topic), method, ACCEPTED);
    _get_topic(rejected_topic, sizeof(rejected_topic), method, REJECTED);

    for (i = 0; i < MAX_TOPICS_AT_ANY_GIVEN_TIME; i++) {
        if (!sg_subscriptions[i].is_free) {
            if ((strcmp(accepted_topic, sg_subscriptions[i].topic) == 0)) {
                accepted = true;
            } else if ((strcmp(rejected_topic, sg_subscriptions[i].topic) == 0)) {
                rejected = true;
            }
        }
    }

    return (rejected && accepted);
}

/**
 * 订阅相应设备的文档accepted和rejected主题
 *
 * @param pClient	   Qcloud_IoT_Client对象
 * @param method       文档操作方式
 * @return             返回QCLOUD_ERR_SUCCESS表示订阅成功
 */
static int _subscribe_accepted_rejected_to_cloud(void* pClient, Method method) {

    IOT_FUNC_ENTRY;
    int rc = QCLOUD_ERR_SUCCESS;
    SubscribeParams subscribe_params = DEFAULT_SUB_PARAMS;
    subscribe_params.on_message_handler = _on_accepted_rejected_message_handler;
    subscribe_params.qos = QOS0;

    bool is_subscribe_success = false;
    int16_t index_accepted_topic = _get_free_index_in_subscription_list();

    if (index_accepted_topic >= 0) {
        _get_topic(sg_subscriptions[index_accepted_topic].topic, MAX_SIZE_OF_CLOUD_TOPIC, method, ACCEPTED);

        rc = IOT_MQTT_Subscribe(pClient, sg_subscriptions[index_accepted_topic].topic, &subscribe_params);
        if (rc == QCLOUD_ERR_SUCCESS) {
            sg_subscriptions[index_accepted_topic].count = 1;
        }
        else {
        	Log_e("subscribe topic: %s failed: %d.", sg_subscriptions[index_accepted_topic].topic, rc);
        }
    }

    int16_t index_rejected_topic = _get_free_index_in_subscription_list();

    if (index_rejected_topic >= 0 && rc == QCLOUD_ERR_SUCCESS) {
        _get_topic(sg_subscriptions[index_rejected_topic].topic, MAX_SIZE_OF_CLOUD_TOPIC, method, REJECTED);

		rc = IOT_MQTT_Subscribe(pClient, sg_subscriptions[index_rejected_topic].topic, &subscribe_params);
		if (rc == QCLOUD_ERR_SUCCESS) {
			sg_subscriptions[index_rejected_topic].count = 1;
			is_subscribe_success = true;

			// wait for SUBSCRIBE_WAITING_TIME seconds to let the subscription take effect
			Timer subWaitTimer;
			InitTimer(&subWaitTimer);
			countdown(&subWaitTimer, SUBSCRIBE_WAITING_TIME);
			while (!expired(&subWaitTimer));
		}
		else {
			Log_e("subscribe topic: %s failed: %d.", sg_subscriptions[index_rejected_topic].topic, rc);
		}
    }


    if (!is_subscribe_success) {

        if (index_accepted_topic >= 0) {
            sg_subscriptions[index_accepted_topic].is_free = true;
        }

        if (index_rejected_topic >= 0) {
            sg_subscriptions[index_rejected_topic].is_free = true;
        }

        if (sg_subscriptions[index_accepted_topic].count == 1) {
            IOT_MQTT_Unsubscribe(pClient, sg_subscriptions[index_accepted_topic].topic);
        }
    }

    IOT_FUNC_EXIT_RC(rc);
}

/**
 * @brief 取消订阅相应设备的文档accepted和rejected主题
 *
 * @param pClient Qcloud_IoT_Client对象
 * @param 订阅的主题的index
 *
 */
static void _unsubscribe_accepted_rejected_to_cloud(void *pClient, uint8_t index) {

    char accepted_topic[MAX_SIZE_OF_CLOUD_TOPIC] = {0};
    char rejected_topic[MAX_SIZE_OF_CLOUD_TOPIC] = {0};

    Request request = sg_pending_requests[index];
    _get_topic(accepted_topic, sizeof(accepted_topic), request.method, ACCEPTED);
    _get_topic(rejected_topic, sizeof(rejected_topic), request.method, REJECTED);

    int16_t i = _get_index_in_subscription_list(accepted_topic);
    if (i >= 0) {
		IOT_MQTT_Unsubscribe(pClient, accepted_topic);
    }

    i = _get_index_in_subscription_list(rejected_topic);
    if (i >= 0) {
		IOT_MQTT_Unsubscribe(pClient, rejected_topic);
    }
}

/**
 * @brief 发布文档请求到物联云
 *
 * @param pClient 				Qcloud_IoT_Client对象
 * @param method                文档操作方式
 * @param pJsonDocumentToBeSent 等待发送的文档
 * @return 返回QCLOUD_ERR_SUCCESS, 表示发布文档请求成功
 */
static int _shadow_publish_to_cloud(void *pClient, Method method, const char *pJsonDocumentToBeSent) {

    IOT_FUNC_ENTRY;
    int rc;
    char topic_name[MAX_SIZE_OF_CLOUD_TOPIC] = {0};
    _get_topic(topic_name, sizeof(topic_name), method, METHOD);
    PublishParams pubParams = DefaultPubParams;
    pubParams.qos = QOS0;
    pubParams.payload_len = strlen(pJsonDocumentToBeSent);
    pubParams.payload = (char *) pJsonDocumentToBeSent;
    rc = IOT_MQTT_Publish(pClient, topic_name, &pubParams);
    IOT_FUNC_EXIT_RC(rc);
}

/**
 * 返回请求列表中的第一个空闲位置索引
 *
 * @return -1 表示没有空闲位置
 */
static int16_t _get_free_index_in_request_list() {

    IOT_FUNC_ENTRY;
    uint8_t i;

    for (i = 0; i < MAX_APPENDING_REQUEST_AT_ANY_GIVEN_TIME; i++) {
        if (sg_pending_requests[i].is_free) {
            IOT_FUNC_EXIT_RC(i);
        }
    }

    IOT_FUNC_EXIT_RC(-1);
}

/**
 * 添加操作请求到appending请求列表中
 *
 * @param index         插入的位置
 * @param pClientToken  请求的clientToken
 * @param pParams       请求参数
 */
static void _add_to_request_list(uint8_t index, const char *pClientToken, RequestParams *pParams) {
    sg_pending_requests[index].callback = pParams->request_callback;

    strncpy(sg_pending_requests[index].client_token, pClientToken, MAX_SIZE_OF_CLIENT_TOKEN);

    sg_pending_requests[index].user_data = pParams->user_data;
    sg_pending_requests[index].method = pParams->method;

    InitTimer(&(sg_pending_requests[index].timer));
    countdown(&(sg_pending_requests[index].timer), pParams->timeout_sec);
    sg_pending_requests[index].is_free = false;
}

void init_request_manager()
{
    client_token_num = 0;
    uint32_t i;

    for (i = 0; i < MAX_APPENDING_REQUEST_AT_ANY_GIVEN_TIME; i++) {
        sg_pending_requests[i].is_free = true;
    }

    for (i = 0; i < MAX_TOPICS_AT_ANY_GIVEN_TIME; i++) {
        sg_subscriptions[i].is_free = true;
        sg_subscriptions[i].count = 0;
    }
}
    
void reset_requset_manager(void *pClient)
{
    client_token_num = 0;
    uint32_t i;
    
    // 取消订阅所有accept和reject主题
    for (i = 0; i < MAX_APPENDING_REQUEST_AT_ANY_GIVEN_TIME; i++) {
        _unsubscribe_accepted_rejected_to_cloud(pClient, i);
    }

    for (i = 0; i < MAX_APPENDING_REQUEST_AT_ANY_GIVEN_TIME; i++) {
        sg_pending_requests[i].is_free = true;
    }

    for (i = 0; i < MAX_TOPICS_AT_ANY_GIVEN_TIME; i++) {
        sg_subscriptions[i].is_free = true;
        sg_subscriptions[i].count = 0;
    }
}

void handle_expired_request() {
    IOT_FUNC_ENTRY;

    uint8_t i;
    for (i = 0; i < MAX_APPENDING_REQUEST_AT_ANY_GIVEN_TIME; i++) {
        if (!sg_pending_requests[i].is_free) {
            if (expired(&(sg_pending_requests[i].timer))) {
                if (sg_pending_requests[i].callback != NULL) {
                    sg_pending_requests[i].callback(sg_pending_requests[i].method, ACK_TIMEOUT, cloud_rcv_buf, sg_pending_requests[i].user_data);
                }
                sg_pending_requests[i].is_free = true;
                _decrement_subscription_cnt(i);
            }
        }
    }

    IOT_FUNC_EXIT;
}

int do_shadow_request(void *pClient, RequestParams *pParams, char *pJsonDoc) {

    IOT_FUNC_ENTRY;
    int rc = QCLOUD_ERR_SUCCESS;

    POINTER_SANITY_CHECK(pJsonDoc, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(pParams, QCLOUD_ERR_INVAL);

    char client_token[MAX_SIZE_OF_CLIENT_TOKEN];
    int32_t tokenCount = 0;
    int16_t index_for_insert;

    // 解析文档中的clientToken, 如果解析失败, 直接返回错误
    if (!parse_client_token(pJsonDoc, tokenCount, client_token)) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_INVAL);
    }
    
    // 获取请求列表中第一个空闲位置索引
    index_for_insert = _get_free_index_in_request_list();

    if (index_for_insert < 0) IOT_FUNC_EXIT_RC(QCLOUD_ERR_MAX_APPENDING_REQUEST);

	if (_has_subscribe_accept_reject_to_cloud(pParams->method)) {
		_increment_subscription_cnt(pParams->method);
	} else {
		rc = _subscribe_accepted_rejected_to_cloud(pClient, pParams->method);
	}

    // 相应的accepted/rejected Topic订阅成功或已经订阅
    if (rc == QCLOUD_ERR_SUCCESS) {
        rc = _shadow_publish_to_cloud(pClient, pParams->method, pJsonDoc);
    }

    if (rc == QCLOUD_ERR_SUCCESS) {
        _add_to_request_list((uint8_t) index_for_insert, client_token, pParams);
    }

    IOT_FUNC_EXIT_RC(rc);
}


int stiching_shadow_topic(char *topic, int buf_size, const char *action) 
{
    int size = HAL_Snprintf(topic, buf_size, "$shadow/%s/%s/%s", action, iot_device_info_get()->product_name, iot_device_info_get()->device_name);
    if (size < 0 || size > buf_size - 1)
    {
        Log_e("buf_size < topic length!");
        return QCLOUD_ERR_FAILURE;
    }
    return QCLOUD_ERR_SUCCESS;
}

#ifdef __cplusplus
}
#endif
