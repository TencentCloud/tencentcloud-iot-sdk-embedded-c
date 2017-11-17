#ifndef MQTT_CLIENT_C_UNIT_MOCK_TLS_PARAMS_H
#define MQTT_CLIENT_C_UNIT_MOCK_TLS_PARAMS_H

#include <sys/time.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define TLSMaxBufferSize 1024
#define NO_MSG_LENGTH_MENTIONED -1

typedef struct {
    unsigned char *pBuffer;
    size_t len;
    bool NoMsgFlag;
    struct timeval expiry_time;
    size_t BufMaxSize;
} TlsBuffer;


extern TlsBuffer RxBuffer;
extern TlsBuffer TxBuffer;

extern size_t RxIndex;
extern unsigned char RxBuf[TLSMaxBufferSize];
extern unsigned char TxBuf[TLSMaxBufferSize];
extern char LastSubscribeMessage[TLSMaxBufferSize];
extern size_t lastSubscribeMsgLen;
extern char SecondLastSubscribeMessage[TLSMaxBufferSize];
extern size_t secondLastSubscribeMsgLen;

extern char hostAddress[512];
extern uint16_t port;
extern uint32_t handshakeTimeout_ms;

extern char *invalidEndpointFilter;
extern char *invalidRootCAPathFilter;
extern char *invalidCertPathFilter;
extern char *invalidPrivKeyPathFilter;
extern uint16_t invalidPortFilter;

#endif //MQTT_CLIENT_C_UNIT_MOCK_TLS_PARAMS_H
