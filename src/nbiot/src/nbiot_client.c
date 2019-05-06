#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "utils_base64.h"
#include "utils_hmac.h"
#include "qcloud_iot_sdk_impl_internal.h"

#define MAX_SIZE_DATA     1024
#define MAX_SIZE_TOKEN    1024
#define DECODE_PSK_LENGTH 48	/*控制台允许的最大长度为64，对应到原文最大长度64/4*3 = 48*/

#define ADDRESS_SIZE   1
#define LENGTH_SIZE    2
#define VERSION_SIZE   1
#define SIGNATURE_SIZE 1
#define EXPIRY_SIZE    4
#define TLV_CNT_SIZE   2
#define T_SIZE         1
#define L_SIZE         2
#define V4_SIZE        4
#define V5_SIZE        1

static uint32_t seq = 0;

typedef enum tlvType {
    TLV_TYPE_TOPIC = 1,
    TLV_TYPE_TOKEN,
    TLV_TYPE_PAYLOAD,
    TLV_TYPE_SEQ,
    TLV_TYPE_QOS,
} TlvType;

int toBigEnd32(unsigned char* buf, uint32_t x)
{
    POINTER_SANITY_CHECK(buf, QCLOUD_ERR_INVAL);

    buf[0] = (x >> 24) & 0xFF;
    buf[1] = (x >> 16) & 0xFF;
    buf[2] = (x >> 8) & 0xFF;
    buf[3] = x & 0xFF;
    //Log_d("%x, %x, %x, %x" , buf[0], buf[1], buf[2], buf[3]);
    return QCLOUD_ERR_SUCCESS;
}

int toBigEnd16(unsigned char* buf, uint16_t x)
{
    POINTER_SANITY_CHECK(buf, QCLOUD_ERR_INVAL);

    *buf = ((x) & 0xFF00) >> 8 ;
    *(buf+1) = (x) & 0xFF ;

    //Log_d("buf[0]: %x buf[1]: %x", *buf, *(buf+1));
    return QCLOUD_ERR_SUCCESS;
}

int ntohBig32(uint32_t* x, unsigned char* buf)
{
    POINTER_SANITY_CHECK(x, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(buf, QCLOUD_ERR_INVAL);

    *x = (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];

    return QCLOUD_ERR_SUCCESS;
}

int ntohBig16(uint16_t* x, unsigned char* buf)
{
    POINTER_SANITY_CHECK(x, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(buf, QCLOUD_ERR_INVAL);

    *x = (buf[0] << 8) + buf[1];

    return QCLOUD_ERR_SUCCESS;
}

void StrToHex(char *pbDest, char *pbSrc, int nLen)
{
    char h1,h2,s1,s2;
    int i;
    for (i=0; i < nLen; i++)
    {
        h1 = pbSrc[2*i];
        h2 = pbSrc[2*i + 1];
        s1 = toupper(h1) - 0x30;
        if (s1 > 9)
           s1 -= 7;

        s2 = toupper(h2) - 0x30;
        if (s2 > 9)
            s2 -= 7;

        pbDest[i] = s1*16 + s2;
    }
}

int calToken(char* token, int* tokenSize, const unsigned char* keyBase64, char* topic, uint32_t expiry, uint8_t qos, char* payload)
{
    POINTER_SANITY_CHECK(token, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(keyBase64, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(topic, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(payload, QCLOUD_ERR_INVAL);

    size_t len = 0;

    static unsigned char psk_str[DECODE_PSK_LENGTH] = {0};
    char buffer[256] = {0};
    char strTok[256] = {0};
    size_t keyBase64Len = strlen((const char*)keyBase64);
    char bufferTemp[11] = {0};
    const char* semicolon = ";";

    qcloud_iot_utils_base64decode(psk_str, sizeof( psk_str ), &len, keyBase64, keyBase64Len);
    Log_d("psk_str: %s, len: %ld, keyBase64: %s, len: %ld", psk_str, len, keyBase64, keyBase64Len);

    strncpy(buffer, topic, strlen(topic));
    strncat(buffer, semicolon, sizeof(char));
    //Log_d("buffer = %s, len = %ld", buffer, strlen(buffer));

    sprintf(bufferTemp, "%u", expiry);
    //Log_d("bufferTemp = %s", bufferTemp);
    strncat(buffer, bufferTemp, strlen(bufferTemp));
    strncat(buffer, semicolon, sizeof(char));
    //Log_d("buffer = %s, len = %ld", buffer, strlen(buffer));

    sprintf(bufferTemp, "%u", seq);
    strncat(buffer, bufferTemp, sizeof(char)*4);
    strncat(buffer, semicolon, sizeof(char));
    //Log_d("buffer = %s, len = %ld", buffer, strlen(buffer));

    sprintf(bufferTemp, "%u", qos);
    strncat(buffer, bufferTemp, strlen(bufferTemp));
    strncat(buffer, semicolon, sizeof(char));
    //Log_d("buffer = %s, len = %ld", buffer, strlen(buffer));

    strncat(buffer, payload, strlen(payload));
    //Log_d("buffer = %s, len = %ld", buffer, strlen(buffer));

    utils_hmac_sha1(buffer, strlen(buffer), strTok, (const char*)psk_str, strlen((char*)psk_str));

    //Log_d("token = %s, len = %ld", token, *tokenSize);
    StrToHex(token, strTok, 20);
    *tokenSize = 20;

    return QCLOUD_ERR_SUCCESS;
}

int addTLV(unsigned char* buf, uint8_t type, char* value)
{
    POINTER_SANITY_CHECK(buf, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(value, QCLOUD_ERR_INVAL);
    unsigned char* ptr = buf;
    uint16_t length = strlen(value);

    // write type
    *ptr = type;
    ptr += sizeof(uint8_t);

    // write length
    toBigEnd16(ptr, length);
    ptr += sizeof(uint16_t);

    // write value
    strncpy((char* )ptr, value, length);
    ptr += sizeof(uint16_t);
    //Log_d("buf[0]:%2x, buf[1]:%2x, buf[2]:%2x",*buf, *(buf+1), *(buf+2));

    return QCLOUD_ERR_SUCCESS;
}

int IOT_NB_setMessage(unsigned char* msg, unsigned int* length, NBIoTSetMessage* nbiotMsg)
{
    POINTER_SANITY_CHECK(msg, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(length, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(nbiotMsg, QCLOUD_ERR_INVAL);

    uint16_t v1Size = strlen(nbiotMsg->topic);
    if( v1Size > MAX_SIZE_TOPIC )
    {
        Log_e("Topic size is %d, oversize MAX_SIZE_TOPIC %d", v1Size, MAX_SIZE_TOPIC);
        return QCLOUD_ERR_FAILURE;
    }

    uint16_t v3Size = strlen(nbiotMsg->payload);
    if( v3Size > MAX_SIZE_PAYLOAD )
    {
        Log_e("Payload size is %d, oversize MAX_SIZE_PAYLOAD %d", v3Size, MAX_SIZE_PAYLOAD);
        return QCLOUD_ERR_FAILURE;
    }

    uint16_t dataLen = 0;

    char token[41] = {0};
    unsigned char base64[MAX_SIZE_MESSAGE] = {0};
    size_t olen = 0;
    int  v2Size = 0;
    static int seq = 0;

    unsigned char data[MAX_SIZE_MESSAGE] = {0};
    unsigned char* ptr = data;

    calToken(token, &v2Size, (const unsigned char*)nbiotMsg->key, nbiotMsg->topic, nbiotMsg->expiry, nbiotMsg->qos, nbiotMsg->payload);
    //Log_d("token = %s, len = %d", token, v2Size);

    uint16_t dataLenRaw = VERSION_SIZE + SIGNATURE_SIZE + EXPIRY_SIZE + TLV_CNT_SIZE + T_SIZE*5 + L_SIZE*5 + v1Size + v2Size + v3Size + V4_SIZE + V5_SIZE;

    *ptr = nbiotMsg->version;
    ptr += VERSION_SIZE;

    *ptr = nbiotMsg->sigType;
    ptr += SIGNATURE_SIZE;

    toBigEnd32(ptr, nbiotMsg->expiry);
    ptr += EXPIRY_SIZE;

    toBigEnd16(ptr, TLV_COUNT);
    ptr += TLV_CNT_SIZE;

    // write tlv topic
    addTLV(ptr, TLV_TYPE_TOPIC, nbiotMsg->topic);
    ptr += (T_SIZE + L_SIZE + v1Size);

    // write tlv token
    addTLV(ptr, TLV_TYPE_TOKEN, token);
    ptr += (T_SIZE + L_SIZE + v2Size);

    // write tlv payload
    addTLV(ptr, TLV_TYPE_PAYLOAD, nbiotMsg->payload);
    ptr += (T_SIZE + L_SIZE + v3Size);

    // write tlv seq
    *ptr = TLV_TYPE_SEQ;
    ptr += T_SIZE;
    toBigEnd16(ptr, V4_SIZE);
    ptr += L_SIZE;
    toBigEnd32(ptr, seq);
    ptr += V4_SIZE;
    seq++;

    // write tlv qos
    *ptr = TLV_TYPE_QOS;
    ptr += T_SIZE;
    toBigEnd16(ptr, V5_SIZE);
    ptr += L_SIZE;
    *ptr = nbiotMsg->qos;
    ptr += V5_SIZE;

    qcloud_iot_utils_base64encode( base64, MAX_SIZE_MESSAGE, &olen, (const unsigned char*)data, dataLenRaw );

    ptr = msg;

    *ptr = nbiotMsg->address;
    ptr += ADDRESS_SIZE;

    dataLen = strlen((const char*)base64);

    toBigEnd16(ptr, dataLen);
    ptr += LENGTH_SIZE;

    strncpy((char*)ptr, (const char*)base64, dataLen);

    *length = ADDRESS_SIZE + LENGTH_SIZE + dataLen;

    return QCLOUD_ERR_SUCCESS;
}

int IOT_NB_getMessage(NBIoTGetMessage* nbiotMsg, unsigned char* msg)
{
    POINTER_SANITY_CHECK(nbiotMsg, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(msg, QCLOUD_ERR_INVAL);

    size_t len = 0;
    //uint16_t u16Temp = 0;
    uint32_t u32Temp = 0;
    uint32_t u32Seq  = 0;
    uint8_t  u8Qos   = 0;

    uint16_t u16TlvCnt = 0;
    uint16_t v1Length  = 0;
    uint16_t v2Length  = 0;
    uint16_t v3Length  = 0;
    uint16_t v4Length  = 0;
    uint16_t v5Length  = 0;


    unsigned char* ptr = msg;
    unsigned int dataLen = 0;
    unsigned char data[MAX_SIZE_DATA] = {0};
    unsigned char dataRaw[MAX_SIZE_DATA] = {0};
    char value[MAX_SIZE_DATA] = {0};

    nbiotMsg->address = ptr[0];
    dataLen = (ptr[1] << 8 )| ptr[2];
    Log_d("data length: 0x%x", dataLen);

    strncpy((char*)data, (char*)(ptr + ADDRESS_SIZE + LENGTH_SIZE), dataLen);

    qcloud_iot_utils_base64decode(dataRaw, sizeof( dataRaw ), &len, data, dataLen);

    ptr = dataRaw;
    nbiotMsg->version = ptr[0];
    ptr += VERSION_SIZE;

    nbiotMsg->sigType = ptr[0];
    ptr += SIGNATURE_SIZE;

    ntohBig32(&u32Temp, ptr);
    nbiotMsg->expiry = u32Temp;
    ptr += EXPIRY_SIZE;

    ntohBig16(&u16TlvCnt, ptr);
    ptr += TLV_CNT_SIZE;
    Log_d("tlv count: %d", u16TlvCnt);

    if(ptr[0] == TLV_TYPE_TOPIC)
    {
        ptr += T_SIZE;
        ntohBig16(&v1Length, ptr);
        ptr += L_SIZE;
        strncpy((char*)(nbiotMsg->topic), (char*)ptr, v1Length);
        ptr += v1Length;
        Log_d("v1Length: %d, topic: %s", v1Length, nbiotMsg->topic);
    }
    else
    {
        Log_e("parse tvl1 topic failed");
        return QCLOUD_ERR_FAILURE;
    }

    if(ptr[0] == TLV_TYPE_TOKEN)
    {
        ptr += T_SIZE;
        ntohBig16(&v2Length, ptr);
        ptr += L_SIZE;

        strncpy(value, (char*)ptr, v2Length);
        ptr += v2Length;
        Log_d("v2Length: %d, value: %s", v2Length, value);
    }
    else
    {
        Log_e("parse tvl2 token failed");
        return QCLOUD_ERR_FAILURE;
    }

    if(ptr[0] == TLV_TYPE_PAYLOAD)
    {
        ptr += T_SIZE;
        ntohBig16(&v3Length, ptr);
        ptr += L_SIZE;

        strncpy((char*)(nbiotMsg->payload), (char*)ptr, v3Length);
        ptr += v3Length;
        Log_d("v3Length: %d, payload: %s", v3Length, nbiotMsg->payload);
    }
    else
    {
        Log_e("parse tvl3 payload failed");
        return QCLOUD_ERR_FAILURE;
    }

    if(ptr[0] == TLV_TYPE_SEQ)
    {
        ptr += T_SIZE;
        ntohBig16(&v4Length, ptr);
        ptr += L_SIZE;
        ntohBig32(&u32Seq, ptr);
        ptr += v4Length;
        Log_d("v4Length: %d, seq: %d", v4Length, u32Seq);
    }
    else
    {
        Log_e("parse tvl4 seq failed");
        return QCLOUD_ERR_FAILURE;
    }

    if(ptr[0] == TLV_TYPE_QOS)
    {
        ptr += T_SIZE;
        ntohBig16(&v5Length, ptr);
        ptr += L_SIZE;
        u8Qos = ptr[0];
        Log_d("v1Length: %d, qos: %d", v1Length, u8Qos);
        nbiotMsg->qos = u8Qos;
    }
    else
    {
        Log_e("parse tvl5 qos failed");
        return QCLOUD_ERR_FAILURE;
    }


    return QCLOUD_ERR_SUCCESS;
}
