#include <stdio.h>
#include <string.h>
#include <gtest/gtest.h>
#include <limits.h>

#include "qcloud_iot_utils_httpc.h"
#include "qcloud_iot_export_log.h"
#include "qcloud_iot_import.h"
#include "ca.h"


class HTTPClientTests : public testing::Test
{
protected:
    virtual void SetUp()
    {
        IOT_Log_Set_Level(DEBUG);
        std::cout << "HTTPClientTests Test Begin \n";
    }
    virtual void TearDown()
    {
        std::cout << "HTTPClientTests Test End \n";
    } 
	
};

TEST_F(HTTPClientTests, HTTPDownload) {
    HTTPClient *http_client = (HTTPClient *)HAL_Malloc(sizeof(HTTPClient));            
    HTTPClientData *http_data = (HTTPClientData *)HAL_Malloc(sizeof(HTTPClientData));
    memset(http_client, 0, sizeof(HTTPClient));
    memset(http_data, 0, sizeof(HTTPClientData));

    char *buf = (char *)HAL_Malloc(2046);
    memset(buf, 0, 2046);
    http_data->response_buf = buf;
    http_data->response_buf_len = 2046;
    http_client->header = (char *)"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n" \
                         "Accept-Encoding: gzip, deflate\r\n";

    FILE *fp;
    char *file_path = (char*)"htts_down.txt";
    if (NULL == (fp = fopen(file_path, "wb+"))) {
        ASSERT_TRUE(NULL != fp);
    }

    char *url = (char *)"https://ota-1251132654.cos.ap-guangzhou.myqcloud.com/3.txt?sign=2iYBFIhf9qG3YLVzehkFSs25M7phPTEyNTExMzI2NTQmaz1BS0lEdnp2bjhDbGM3Q2swTDB1Ujh5SVUzQ3NqbG5mbnJ4anMmZT0xNTIwMzkwMzkxJnQ9MTUxNzc5ODM5MSZyPTEzMTQ2ODYxNDMmZj0vMy50eHQmYj1vdGE=";
    const char *ca_crt = iot_https_ca_get();
    // char *url = (char*)"https://cloud.tencent.com/";
    uint32_t timeout_ms = 5000;
    int rc;
    bool read_finish = false;

    qcloud_http_client_common(http_client, url, 443, ca_crt, HTTP_GET, http_data);

    while (!read_finish) {
        int diff = http_data->response_content_len - http_data->retrieve_len;

        rc = qcloud_http_recv_data(http_client, timeout_ms, http_data);
        
        int32_t len = http_data->response_content_len - http_data->retrieve_len - diff;
        read_finish = len == 0;
        if (len > 0) {
            rc = fwrite(http_data->response_buf, len, 1, fp);
        }
        ASSERT_TRUE(rc == 1);
    }
    
    fclose(fp);
    HAL_Free(buf);
    HAL_Free(http_client);
}