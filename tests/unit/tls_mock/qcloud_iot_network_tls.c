#include <stdint.h>
#include <string.h>
#include "qcloud_iot_platform_timer.h"
#include <qcloud_iot_mqtt_timer.h>
#include <stdio.h>
#include "HAL_TLS_mbedtls.h"
#include "unit_mock_tls_params.h"

unsigned char RxBuf[TLSMaxBufferSize];
unsigned char TxBuf[TLSMaxBufferSize];
char LastSubscribeMessage[TLSMaxBufferSize];
size_t lastSubscribeMsgLen;
char SecondLastSubscribeMessage[TLSMaxBufferSize];
size_t secondLastSubscribeMsgLen;

TlsBuffer RxBuffer = {.pBuffer = RxBuf, .len = 512, .NoMsgFlag=1, .expiry_time = {0, 0}, .BufMaxSize = TLSMaxBufferSize};
TlsBuffer TxBuffer = {.pBuffer = TxBuf, .len = 512, .NoMsgFlag=1, .expiry_time = {0, 0}, .BufMaxSize = TLSMaxBufferSize};

size_t RxIndex = 0;

char *invalidEndpointFilter;
char *invalidRootCAPathFilter;
char *invalidCertPathFilter;
char *invalidPrivKeyPathFilter;
uint16_t invalidPortFilter;

static unsigned char isTimerExpired(struct timeval target_time) {
    unsigned char ret_val = 0;
    struct timeval now, result;

    if (target_time.tv_sec != 0 || target_time.tv_usec != 0) {
        gettimeofday(&now, NULL);
        timersub(&(target_time), &now, &result);
        if (result.tv_sec < 0 || (result.tv_sec == 0 && result.tv_usec <= 0)) {
            ret_val = 1;
        }
    } else {
        ret_val = 1;
    }
    return ret_val;
}

int HAL_TLS_Connect(TLSConnectParams *pConnectParams, TLSDataParams *pDataParams) {

    if (invalidEndpointFilter != NULL && strcmp(invalidEndpointFilter, pConnectParams->host) == 0) {
        return QCLOUD_ERR_TCP_UNKNOWN_HOST;
    }

    if (invalidPortFilter == pConnectParams->port) {
        return QCLOUD_ERR_TCP_CONNECT;
    }

    if (invalidRootCAPathFilter != NULL && strcmp(invalidRootCAPathFilter, pConnectParams->ca_file) == 0) {
        return QCLOUD_ERR_SSL_CERT;
    }

    if (invalidCertPathFilter != NULL && strcmp(invalidCertPathFilter, pConnectParams->cert_file) == 0) {
        return QCLOUD_ERR_SSL_CERT;
    }

    if (invalidPrivKeyPathFilter != NULL && strcmp(invalidPrivKeyPathFilter, pConnectParams->key_file) == 0) {
        return QCLOUD_ERR_SSL_CERT;
    }

    return QCLOUD_ERR_SUCCESS;
}

void HAL_TLS_Disconnect(TLSDataParams *pParams) {

}

int HAL_TLS_Write(TLSDataParams *pParams, unsigned char *msg, size_t totalLen, int timeout_ms, size_t *written_len) {
    size_t i = 0;
    uint8_t firstByte, secondByte;
    uint16_t topicNameLen;
    Timer timer;
    InitTimer(&timer);
    countdown_ms(&timer, (unsigned int) timeout_ms);

    for (i = 0; (i < totalLen) && left_ms(&timer) > 0; i++) {
        TxBuffer.pBuffer[i] = msg[i];
    }
    TxBuffer.len = (size_t) totalLen;

    /* Save last two subscribed topics */
    if ((TxBuffer.pBuffer[0] == 0x82 ? true : false)) {
        snprintf(SecondLastSubscribeMessage, lastSubscribeMsgLen, "%s", LastSubscribeMessage);
        secondLastSubscribeMsgLen = lastSubscribeMsgLen;

        firstByte = (uint8_t) (TxBuffer.pBuffer[4]);
        secondByte = (uint8_t) (TxBuffer.pBuffer[5]);
        topicNameLen = (uint16_t) (secondByte + (256 * firstByte));

        snprintf(LastSubscribeMessage, topicNameLen + 1u, "%s", &(TxBuffer.pBuffer[6])); // Added one for null character
        lastSubscribeMsgLen = topicNameLen + 1u;
    }

    *written_len = totalLen;
    return QCLOUD_ERR_SUCCESS;
}

int HAL_TLS_Read(TLSDataParams *pParams, unsigned char *msg, size_t totalLen, int timeout_ms, size_t *read_len) {

    if (RxIndex > TLSMaxBufferSize - 1) {
        RxIndex = TLSMaxBufferSize - 1;
    }

    if (RxBuffer.len <= RxIndex || !isTimerExpired(RxBuffer.expiry_time)) {
        return QCLOUD_ERR_SSL_NOTHING_TO_READ; // no thing to read
    }

    if (RxBuffer.NoMsgFlag == false && RxIndex < RxBuffer.len) {
        memcpy(msg, &(RxBuffer.pBuffer[RxIndex]), totalLen);
        RxIndex += totalLen;
    }

    *read_len = totalLen;

    return QCLOUD_ERR_SUCCESS;
}
