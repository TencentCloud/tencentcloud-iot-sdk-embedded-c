/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2018-2020 Tencent. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"
#include "utils_getopt.h"

#ifdef AUTH_MODE_CERT
/* NULL cert file */
#define QCLOUD_IOT_NULL_CERT_FILENAME "YOUR_DEVICE_CERT_FILE_NAME"
/* NULL key file */
#define QCLOUD_IOT_NULL_KEY_FILENAME "YOUR_DEVICE_PRIVATE_KEY_FILE_NAME"
#else
/* NULL device secret */
#define QCLOUD_IOT_NULL_DEVICE_SECRET "YOUR_IOT_PSK"
#endif

int main(int argc, char **argv)
{
    int c;
    while ((c = utils_getopt(argc, argv, "c:")) != EOF) switch (c) {
            case 'c':
                if (HAL_SetDevInfoFile(utils_optarg))
                    return -1;
                break;

            default:
                HAL_Printf(
                    "usage: %s [options]\n"
                    "  [-c <config file for DeviceInfo>] \n",
                    argv[0]);

                return -1;
        }

    // init log level
    IOT_Log_Set_Level(eLOG_DEBUG);

    int        ret;
    DeviceInfo sDevInfo;
    bool       infoNullFlag = false;
    bool       userCertFlag = false;

    memset((char *)&sDevInfo, 0, sizeof(DeviceInfo));

    ret = HAL_GetDevInfo(&sDevInfo);

#ifdef AUTH_MODE_CERT
    /* just demo the cert/key files are empty */
    if (!strcmp(sDevInfo.dev_cert_file_name, QCLOUD_IOT_NULL_CERT_FILENAME) ||
        !strcmp(sDevInfo.dev_key_file_name, QCLOUD_IOT_NULL_KEY_FILENAME)) {
        Log_d("dev Cert not exist!");
        infoNullFlag = true;
    } else if (0 != strcmp(sDevInfo.dev_cert_file_name, QCLOUD_IOT_NULL_CERT_FILENAME) &&
               0 != strcmp(sDevInfo.dev_key_file_name, QCLOUD_IOT_NULL_KEY_FILENAME)) {
        Log_d("dev Cert is exist!");
        userCertFlag = true;
    } else {
        Log_d("dev Cert exist");
    }
#else
    /* just demo the PSK is empty */
    if (!strcmp(sDevInfo.device_secret, QCLOUD_IOT_NULL_DEVICE_SECRET)) {
        Log_d("dev psk not exist!");
        infoNullFlag = true;
    } else {
        Log_d("dev psk exist");
    }
#endif

    /* device cert/key files or PSK is empty, do dynamic register to fetch */
    if (infoNullFlag || userCertFlag) {
        if (QCLOUD_RET_SUCCESS == IOT_DynReg_Device(&sDevInfo)) {
            ret = HAL_SetDevInfo(&sDevInfo);
            if (QCLOUD_RET_SUCCESS != ret) {
                Log_e("devices info save fail");
            } else {
#ifdef AUTH_MODE_CERT
                Log_d("dynamic register success, productID: %s, devName: %s, CertFile: %s, KeyFile: %s",
                      STRING_PTR_PRINT_SANITY_CHECK(sDevInfo.product_id),
                      STRING_PTR_PRINT_SANITY_CHECK(sDevInfo.device_name),
                      STRING_PTR_PRINT_SANITY_CHECK(sDevInfo.dev_cert_file_name),
                      STRING_PTR_PRINT_SANITY_CHECK(sDevInfo.dev_key_file_name));
#else
                Log_d("dynamic register success,productID: %s, devName: %s, device_secret: %s",
                      STRING_PTR_PRINT_SANITY_CHECK(sDevInfo.product_id),
                      STRING_PTR_PRINT_SANITY_CHECK(sDevInfo.device_name),
                      STRING_PTR_PRINT_SANITY_CHECK(sDevInfo.device_secret));
#endif
            }
        } else {
            Log_e("devName: %s dynamic register fail", STRING_PTR_PRINT_SANITY_CHECK(sDevInfo.device_name));
        }
    }

    return ret;
}
