#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"

#include <string.h>
#include <ctype.h>

static void StrToHex(char *pbDest, char *pbSrc, int nLen)
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

int main(int argc, char** argv)
{
    IOT_Log_Set_Level(DEBUG);
    NBIoTSetMessage nbiotSetMsg = {0};
    unsigned char msgBuf[MAX_SIZE_MESSAGE] = {0};
    unsigned int length = 0;
    int i = 0;

    nbiotSetMsg.address = 0;
    nbiotSetMsg.version = 0;
    nbiotSetMsg.sigType = 1;
    nbiotSetMsg.expiry  = 0xffffffff;
    nbiotSetMsg.qos     = 1;
    nbiotSetMsg.topic   = "8W0BMHHEJC/shanghuirecv/event";
    nbiotSetMsg.payload = "{\"data\":\"123\",\"test\":1}";
    nbiotSetMsg.key     = "qF2SNXIyEOH50g+tNNLO1w==";

    IOT_NB_setMessage(msgBuf, &length, &nbiotSetMsg);

    for(i = 0; i < length; i++)
    {
        printf("%02x", msgBuf[i]);
    }
    printf("\n");
    printf("length: %d\n", length);

    NBIoTGetMessage nbiotGetMsg = {0};
    char topic[MAX_SIZE_TOPIC] = {0};
    char payload[MAX_SIZE_PAYLOAD] = {0};

    nbiotGetMsg.topic   = topic;
    nbiotGetMsg.payload = payload;

    char* pbSrc = "0100704151466250516A5541415542414268534F44684F4E544930554446464C3342316332686B5A5859765A585A6C626E514341425344554A416E2F6553346B534D667A2B5535533245325941493978774D414333736961325635496A6F784D6A4E39424141454141412B346755414151413D";
    unsigned char bDest[1024] = {0};
    StrToHex((char*)bDest, pbSrc, strlen(pbSrc)/2);

    IOT_NB_getMessage(&nbiotGetMsg, bDest);

    printf("address: 0x%x\n", nbiotGetMsg.address);
    printf("version: 0x%x\n", nbiotGetMsg.version);
    printf("signature: 0x%x\n", nbiotGetMsg.sigType);
    printf("expiry: 0x%x\n", nbiotGetMsg.expiry);
    printf("qos: 0x%x\n", nbiotGetMsg.qos);
    printf("topic: %s\n", nbiotGetMsg.topic);
    printf("payload: %s\n", nbiotGetMsg.payload);

    return QCLOUD_ERR_SUCCESS;
}