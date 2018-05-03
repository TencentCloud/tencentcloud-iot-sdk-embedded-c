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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/time.h>

#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"

/* 产品名称, 与云端同步设备状态时需要  */
#define QCLOUD_IOT_MY_PRODUCT_ID        	"PRODUCT_ID"
/* 设备名称, 与云端同步设备状态时需要 */
#define QCLOUD_IOT_MY_DEVICE_NAME         	"ThreadTestDev0"


#ifndef AUTH_WITH_NOTLS

/* 客户端证书文件名  非对称加密使用*/
#define QCLOUD_IOT_CERT_FILENAME          "YOUR_DEVICE_NAME_cert.crt"
/* 客户端私钥文件名 非对称加密使用*/
#define QCLOUD_IOT_KEY_FILENAME           "YOUR_DEVICE_NAME_private.key"

static char sg_cert_file[PATH_MAX + 1];		//客户端证书全路径
static char sg_key_file[PATH_MAX + 1];		//客户端密钥全路径

#endif

#define MAX_PUB_THREAD_COUNT 3
#define PUBLISH_COUNT 10
#define THREAD_SLEEP_INTERVAL_USEC 500000
#define CONNECT_MAX_ATTEMPT_COUNT 3
#define RX_RECEIVE_PERCENTAGE 99.0f
#define INTEGRATION_TEST_TOPIC ""QCLOUD_IOT_MY_PRODUCT_ID"/"QCLOUD_IOT_MY_DEVICE_NAME"/Thread"	// 需要创建设备的时候配置权限

static void *sg_pClient;

static bool sg_terminate_yield_thread;
static bool sg_terminate_subUnsub_thread;

static unsigned int sg_countArray[MAX_PUB_THREAD_COUNT][PUBLISH_COUNT];	// 订阅回调函数中，每次成功收订阅topic返回的数据的时候，记录每个订阅的成功次数
static unsigned int sg_rxMsgBufferTooBigCounter;                        // 记录收到消息长度过长的次数
static unsigned int sg_rxUnexpectedNumberCounter;                       // 记录收到错误消息的次数
static unsigned int sg_rePublishCount;									// 记录重新发布的次数
static unsigned int sg_wrongYieldCount;									// 记录yield失败的次数
static unsigned int sg_threadStatus[MAX_PUB_THREAD_COUNT];				// 记录所有线程的状态

typedef struct ThreadData {
	int threadId;
	void *client;
} ThreadData;



void event_handler(void *pcontext, void *pclient, MQTTEventMsg *msg) {
	MQTTMessage* mqtt_messge = (MQTTMessage*)msg->msg;

	switch(msg->event_type) {
		case MQTT_EVENT_UNDEF:
			Log_i("undefined event occur.");
			break;

		case MQTT_EVENT_DISCONNECT:
			Log_i("MQTT disconnect.");
			break;

		case MQTT_EVENT_RECONNECT:
			Log_i("MQTT reconnect.");
			break;

		case MQTT_EVENT_PUBLISH_RECVEIVED:
			Log_i("topic message arrived but without any related handle: topic=%.*s, topic_msg=%.*s",
					  mqtt_messge->topic_len,
					  mqtt_messge->ptopic,
					  mqtt_messge->payload_len,
					  mqtt_messge->payload);
			break;
		default:
			break;
	}
}

static int _setup_connect_init_params(MQTTInitParams* initParams)
{
	initParams->device_name = QCLOUD_IOT_MY_DEVICE_NAME;
	initParams->product_id = QCLOUD_IOT_MY_PRODUCT_ID;

#ifndef AUTH_WITH_NOTLS
	char certs_dir[PATH_MAX + 1] = "certs";
	char current_path[PATH_MAX + 1];
	char *cwd = getcwd(current_path, sizeof(current_path));
	if (cwd == NULL)
	{
		Log_e("getcwd return NULL");
		return QCLOUD_ERR_FAILURE;
	}
	sprintf(sg_cert_file, "%s/%s/%s", current_path, certs_dir, QCLOUD_IOT_CERT_FILENAME);
	sprintf(sg_key_file, "%s/%s/%s", current_path, certs_dir, QCLOUD_IOT_KEY_FILENAME);

#ifdef AUTH_MODE_CERT
	initParams->cert_file = sg_cert_file;
	initParams->key_file = sg_key_file;
#else
#endif
#endif

	initParams->command_timeout = QCLOUD_IOT_MQTT_COMMAND_TIMEOUT;
	initParams->keep_alive_interval_ms = QCLOUD_IOT_MQTT_KEEP_ALIVE_INTERNAL;

	initParams->auto_connect_enable = 1;
	initParams->event_handle.h_fp = event_handler;

    return QCLOUD_ERR_SUCCESS;
}

/**
 * 订阅topic回调函数
 */
static void _iot_mqtt_tests_message_aggregator(void *pClient, MQTTMessage *message, void *userData) {
	// int rc;
	char *temp = NULL;

	if (message == NULL) {
		return;
	}

	if(30 >= message->payload_len) {
		/* 解析 payload */
		char tempBuf[30];
		unsigned int tempRow = 0, tempCol = 0;
		
		HAL_Snprintf(tempBuf, message->payload_len, "%s", (char *)message->payload);
		printf("\nMessage received : %s\n", tempBuf);
		temp = strtok(tempBuf, " ,:");
		if(NULL == temp) {
			return;
		}
		temp = strtok(NULL, " ,:");
		if(NULL == temp) {
			return;
		}
		tempRow = atoi(temp);
		temp = strtok(NULL, " ,:");
		if(NULL == temp) {
			return;
		}
		temp = strtok(NULL, " ,:");
		if(NULL == temp) {
			return;
		}
		tempCol = atoi(temp);

		if(((tempRow - 1) < MAX_PUB_THREAD_COUNT) && (tempCol < PUBLISH_COUNT)) {
			sg_countArray[tempRow - 1][tempCol]++;
		} else {
			Log_e(" Unexpected Thread : %d, Message : %d ", tempRow, tempCol);
			sg_rxUnexpectedNumberCounter++;
		}
	} else {
		sg_rxMsgBufferTooBigCounter++;
	}
}

/**
 * yield 测试线程函数
 */
static void *_iot_mqtt_tests_yield_thread_runner(void *ptr) {
	int rc = QCLOUD_ERR_SUCCESS;
	void *pClient = ptr;
	while(QCLOUD_ERR_SUCCESS == rc && false == sg_terminate_yield_thread) {
		do {
			usleep(THREAD_SLEEP_INTERVAL_USEC);
			//DEBUG("\n Yielding \n");
			rc = IOT_MQTT_Yield(pClient, 200);
		} while(rc != QCLOUD_ERR_SUCCESS && rc != QCLOUD_ERR_MQTT_RECONNECTED);

		if(QCLOUD_ERR_SUCCESS != rc) {
			Log_e("\nYield Returned : %d ", rc);
		}
	}
	return NULL;
}

/**
 * subscribe/unsubscribe 测试线程函数
 * 函数中会在一个单独的线程中先订阅相关主题，然后再取消订阅
 */
static void *_iot_mqtt_tests_sub_unsub_thread_runner(void *ptr) {
	int rc = QCLOUD_ERR_SUCCESS;
	void *pClient = ptr;
	char testTopic[128];
	HAL_Snprintf(testTopic, 128, "%s_temp", INTEGRATION_TEST_TOPIC);

	while(QCLOUD_ERR_SUCCESS == rc && false == sg_terminate_subUnsub_thread) {
		do {
			usleep(THREAD_SLEEP_INTERVAL_USEC);
			SubscribeParams sub_params = DEFAULT_SUB_PARAMS;
			sub_params.qos = QOS1;
			sub_params.on_message_handler = _iot_mqtt_tests_message_aggregator;
			rc = IOT_MQTT_Subscribe(pClient, testTopic, &sub_params);

		} while(QCLOUD_ERR_MQTT_NO_CONN == rc || QCLOUD_ERR_MQTT_REQUEST_TIMEOUT == rc);

		if(rc < 0) {
			Log_e("Subscribe Returned : %d ", rc);
		}

		do {
			usleep(THREAD_SLEEP_INTERVAL_USEC);
			rc = IOT_MQTT_Unsubscribe(pClient, testTopic);
		} while(QCLOUD_ERR_MQTT_NO_CONN == rc|| QCLOUD_ERR_MQTT_REQUEST_TIMEOUT == rc);

		if(QCLOUD_ERR_SUCCESS != rc) {
			Log_e("Unsubscribe Returned : %d ", rc);
		}
	}
	return NULL;
}

/**
 * 在子线程上进行subscribe操作
 * 这里会记录发布前/后的时间，并保存到pSubscribeTime中
 */
static int _iot_mqtt_tests_subscribe_to_test_topic(void *pClient, QoS qos, struct timeval *pSubscribeTime) {
	int rc;
	struct timeval start, end;

	gettimeofday(&start, NULL);

	SubscribeParams sub_params = DEFAULT_SUB_PARAMS;
    sub_params.on_message_handler = _iot_mqtt_tests_message_aggregator;
	rc = IOT_MQTT_Subscribe(pClient, INTEGRATION_TEST_TOPIC, &sub_params);

	printf("\n## Sub response rc : %d|topic : %s\n", rc, INTEGRATION_TEST_TOPIC);
	gettimeofday(&end, NULL);

	timersub(&end, &start, pSubscribeTime);

	return rc;
}

/**
 * 在子线程上进行publish操作
 * 这里会循环PUBLISH_COUNT次，发布topic
 * 如果第一次publish失败，则进行第二次发布，并且记录失败的次数
 */
static void *_iot_mqtt_tests_publish_thread_runner(void *ptr) {
	int itr = 0;
	char cPayload[30];

	PublishParams params;
	int rc = QCLOUD_ERR_SUCCESS;
	ThreadData *threadData = (ThreadData *) ptr;
	void *pClient = threadData->client;
	int threadId = threadData->threadId;

	for(itr = 0; itr < PUBLISH_COUNT; itr++) {
		snprintf(cPayload, 30, "Thread : %d, Msg : %d", threadId, itr);
		printf("\nMsg being published: %s \n", cPayload);
		params.payload = (void *) cPayload;
		params.payload_len = strlen(cPayload) + 1;
		params.qos = QOS1;

		do {
			rc = IOT_MQTT_Publish(pClient, INTEGRATION_TEST_TOPIC, &params);
			usleep(THREAD_SLEEP_INTERVAL_USEC);
		} while(/*MUTEX_LOCK_ERROR == rc || */QCLOUD_ERR_MQTT_NO_CONN == rc || QCLOUD_ERR_MQTT_REQUEST_TIMEOUT == rc);
		
		// 发布失败的时候进行一次重新发布，并且记录重新发布的次数
		if(QCLOUD_ERR_SUCCESS != rc) {
			Log_e("Failed attempt 1 Publishing Thread : %d, Msg : %d, cs : %d ", threadId, itr, rc);
			do {
				rc = IOT_MQTT_Publish(pClient, INTEGRATION_TEST_TOPIC, &params);
				usleep(THREAD_SLEEP_INTERVAL_USEC);
			} while(QCLOUD_ERR_MQTT_NO_CONN == rc);
			sg_rePublishCount++;
			if(QCLOUD_ERR_SUCCESS != rc) {
				Log_e("Failed attempt 2 Publishing Thread : %d, Msg : %d, cs : %d Second Attempt ", threadId, itr, rc);
			}
		}
	}
	sg_threadStatus[threadId - 1] = 1;
	return 0;
}

/**
 * 线程安全测试函数
 */
static int _iot_mqtt_tests_multi_threading_validation(void)
{
	pthread_t publish_thread[MAX_PUB_THREAD_COUNT];	// 用来保存所有publish的线程
	pthread_t yield_thread;							// yield的线程
	pthread_t sub_unsub_thread;						// 订阅/取消订阅的线程

	MQTTInitParams initParams;
	_setup_connect_init_params(&initParams);

	int rc = QCLOUD_ERR_SUCCESS;
	int test_result = 0;
	
	unsigned int connectCounter = 0;				// 记录client连接的次数
	int threadId[MAX_PUB_THREAD_COUNT];				// 记录线程id
	int pubThreadReturn[MAX_PUB_THREAD_COUNT];		// 保存pub线程创建函数返回结果		
	int yieldThreadReturn = 0;						// 保存yield线程创建函数返回结果
	int subUnsubThreadReturn = 0;					// 保存sub线程创建函数返回结果
	float percentOfRxMsg = 0.0;						// 记录从pub到sub整个流程的成功率
	int finishedThreadCount = 0;					// 记录publish_thread线程数组中, 已经执行完pub操作的线程数

	int i, rxMsgCount = 0, j = 0;					// 记录订阅回调函数成功的次数
	struct timeval connectTime;						// 记录client连接的时间
	struct timeval subscribeTopic;					// 记录订阅topic的时间
	
	ThreadData threadData[MAX_PUB_THREAD_COUNT];	// 对client和threadId进行包装，在publish线程函数中进行传递

	void* client;
	sg_terminate_yield_thread = false;
	sg_rxMsgBufferTooBigCounter = 0;
	sg_rxUnexpectedNumberCounter = 0;
	sg_rePublishCount = 0;
	sg_wrongYieldCount = 0;
	
	for(j = 0; j < MAX_PUB_THREAD_COUNT; j++) {
		threadId[j] = j + 1;	// 设置线程id，区间：1 - MAX_PUB_THREAD_COUNT
		sg_threadStatus[j] = 0;	// 设置线程状态：0
		for(i = 0; i < PUBLISH_COUNT; i++) {
			sg_countArray[j][i] = 0;
		}
	}

	printf("\nConnecting Client ");
	do {
		client = IOT_MQTT_Construct(&initParams);
		sg_pClient = client;
		if(NULL == client) {
			rc = QCLOUD_ERR_FAILURE;
			Log_e("ERROR Construct!");
			return -1;
		}

		connectCounter++;
	} while(QCLOUD_ERR_SUCCESS != rc && connectCounter < CONNECT_MAX_ATTEMPT_COUNT);

	if(QCLOUD_ERR_SUCCESS == rc) {
		printf("\n## Connect Success. Time sec: %lu, usec: %lu\n", connectTime.tv_sec, connectTime.tv_usec);
	} else {
		Log_e("## Connect Failed. error code %d\n", rc);
		return -1;
	}

	_iot_mqtt_tests_subscribe_to_test_topic(sg_pClient, QOS1, &subscribeTopic);

	printf("\nRunning Test! ");

	yieldThreadReturn = pthread_create(&yield_thread, NULL, _iot_mqtt_tests_yield_thread_runner, client);
	subUnsubThreadReturn = pthread_create(&sub_unsub_thread, NULL, _iot_mqtt_tests_sub_unsub_thread_runner, client);

	printf("\nyieldThreadReturn=%d|subUnsubThreadReturn=%d ", yieldThreadReturn, subUnsubThreadReturn);

	/* 创建多个线程单独测试publish */
	for(i = 0; i < MAX_PUB_THREAD_COUNT; i++) {
		threadData[i].client = sg_pClient;
		threadData[i].threadId = threadId[i];
		pubThreadReturn[i] = pthread_create(&publish_thread[i], NULL, _iot_mqtt_tests_publish_thread_runner,
											&threadData[i]);
											
		printf("\ni=%d|pubThreadReturn=%d ", i, pubThreadReturn[i]);
	}

	/* 等待所有publish线程执行完 */
	do {
		finishedThreadCount = 0;
		for(i = 0; i < MAX_PUB_THREAD_COUNT; i++) { 
			finishedThreadCount += sg_threadStatus[i];
		}
		printf("\nFinished thread count : %d \n", finishedThreadCount);
		sleep(1);
	} while(finishedThreadCount < MAX_PUB_THREAD_COUNT);

	printf("\nFinished publishing!!");

	sg_terminate_yield_thread = true;
	sg_terminate_subUnsub_thread = true;

	/* Allow time for yield_thread and sub_sunsub thread to exit */
	sleep(1);

	/* Not using pthread_join because all threads should have terminated gracefully at this point. If they haven't,
	 * which should not be possible, something below will fail. */

	printf("\n\nCalculating Results!! \n\n");
	for(i = 0; i < PUBLISH_COUNT; i++) {
		for(j = 0; j < MAX_PUB_THREAD_COUNT; j++) {
			if(sg_countArray[j][i] > 0) {
				rxMsgCount++;
			}
		}
	}

	printf("\n\nResult : \n");
	percentOfRxMsg = (float) rxMsgCount * 100 / (PUBLISH_COUNT * MAX_PUB_THREAD_COUNT);

	if(RX_RECEIVE_PERCENTAGE <= percentOfRxMsg && 	// 成功率达标
		0 == sg_rxMsgBufferTooBigCounter && 		// 返回数据buffer没有越界
		0 == sg_rxUnexpectedNumberCounter &&		// 返回预期范围内的数据
	    0 == sg_wrongYieldCount) 					// yield过程中没有发生失败
	{	
		// 测试成功
		printf("\nSuccess: %f %%\n", percentOfRxMsg);
		printf("Published Messages: %d , Received Messages: %d \n", PUBLISH_COUNT * MAX_PUB_THREAD_COUNT, rxMsgCount);
		printf("QoS 1 re publish count %u\n", sg_rePublishCount);
		printf("Connection Attempts %u\n", connectCounter);
		printf("Yield count without error during callback %u\n", sg_wrongYieldCount);
		test_result = 0;
	} else {
		// 测试失败
		printf("\nFailure: %f\n", percentOfRxMsg);
		printf("\"Received message was too big than anything sent\" count: %u\n", sg_rxMsgBufferTooBigCounter);
		printf("\"The number received is out of the range\" count: %u\n", sg_rxUnexpectedNumberCounter);
		printf("Yield count without error during callback %u\n", sg_wrongYieldCount);
		test_result = -2;
	}
	IOT_MQTT_Destroy(&client);
	sg_pClient = NULL;
	return test_result;
}

static int _run_thread_test(void)
{
    printf("\n\n");
	printf("******************************************************************\n");
	printf("* Starting MQTT Version 3.1.1 Multithreading Validation Test     *\n");
	printf("******************************************************************\n");
	int rc = _iot_mqtt_tests_multi_threading_validation();
	if(0 != rc) {
		printf("\n*******************************************************************\n");
		printf("*MQTT Version 3.1.1 Multithreading Validation Test FAILED! RC : %d \n", rc);
		printf("*******************************************************************\n");
		return 1;
	}

	printf("******************************************************************\n");
	printf("* MQTT Version 3.1.1 Multithreading Validation Test SUCCESS!!    *\n");
	printf("******************************************************************\n");

    return 0;
}

int main(int argc, char **argv) {
	return _run_thread_test();
}
