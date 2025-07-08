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

#ifdef __cplusplus
extern "C" {
#endif

#include "qcloud_iot_ca.h"

#include <stdlib.h>
#include <string.h>

#include "qcloud_iot_import.h"
#include "qcloud_iot_common.h"

typedef struct _RegionDomain_ {
    const char *region;
    const char *domain;
} RegionDomain;

/*mqtt domain*/
static RegionDomain sg_iot_mqtt_coap_domain[] = {
    {.region = "china", .domain = QCLOUD_IOT_MQTT_COAP_DIRECT_DOMAIN},          /* China */
    {.region = "us-east", .domain = QCLOUD_IOT_MQTT_COAP_US_EAST_DOMAIN},       /* Eastern US */
    {.region = "europe", .domain = QCLOUD_IOT_MQTT_COAP_EUROPE_DOMAIN},         /* Europe */
    {.region = "ap-bangkok", .domain = QCLOUD_IOT_MQTT_COAP_AP_BANGKOK_DOMAIN}, /* Bangkok */
};

#ifdef WEBSOCKET_MQTT
/*websocket mqtt domain*/
static RegionDomain sg_iot_websocket_mqtt_domain[] = {
    {.region = "china", .domain = QCLOUD_IOT_WEBSOCKET_MQTT_DIRECT_DOMAIN},          /* China */
    {.region = "us-east", .domain = QCLOUD_IOT_WEBSOCKET_MQTT_US_EAST_DOMAIN},       /* Eastern US */
    {.region = "europe", .domain = QCLOUD_IOT_WEBSOCKET_MQTT_EUROPE_DOMAIN},         /* Europe */
    {.region = "ap-bangkok", .domain = QCLOUD_IOT_WEBSOCKET_MQTT_AP_BANGKOK_DOMAIN}, /* Bangkok */
};
#endif

/*dynreg domain*/
static RegionDomain sg_iot_dynreg_log_domain[] = {
    {.region = "china", .domain = DYNREG_LOG_SERVER_URL},                 /* China */
    {.region = "us-east", .domain = DYNREG_LOG_SERVER_US_EAST_URL},       /* Eastern US */
    {.region = "europe", .domain = DYNREG_LOG_SERVER_EUROPE_URL},         /* Europe */
    {.region = "ap-bangkok", .domain = DYNREG_LOG_SERVER_AP_BANGKOK_URL}, /* Bangkok */
};

#ifdef REMOTE_LOGIN_WEBSOCKET_SSH
/*websocket remote login ssh domain*/
static RegionDomain sg_iot_websocket_ssh_domain[] = {
    {.region = "china", .domain = REMOTE_LOGIN_WEBSOCKET_SSH_URL},                 /* China */
    {.region = "us-east", .domain = REMOTE_LOGIN_WEBSOCKET_SSH_US_EAST_URL},       /* Eastern US */
    {.region = "europe", .domain = REMOTE_LOGIN_WEBSOCKET_SSH_EUROPE_URL},         /* Europe */
    {.region = "ap-bangkok", .domain = REMOTE_LOGIN_WEBSOCKET_SSH_AP_BANGKOK_URL}, /* Bangkok */
};
#endif

#ifndef AUTH_WITH_NOTLS
#if defined(DEV_DYN_REG_ENABLED)
static const char *iot_dynreg_https_ca_crt = {
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\r\n"
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\r\n"
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\r\n"
    "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\r\n"
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\r\n"
    "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\r\n"
    "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\r\n"
    "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\r\n"
    "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\r\n"
    "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\r\n"
    "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\r\n"
    "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\r\n"
    "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\r\n"
    "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\r\n"
    "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\r\n"
    "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\r\n"
    "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\r\n"
    "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\r\n"
    "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\r\n"
    "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\r\n"
    "-----END CERTIFICATE-----\r\n"
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIFGjCCBAKgAwIBAgIQCgRw0Ja8ihLIkKbfgm7sSzANBgkqhkiG9w0BAQsFADBh\r\n"
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\r\n"
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\r\n"
    "QTAeFw0xOTA2MjAxMjI3NThaFw0yOTA2MjAxMjI3NThaMF8xCzAJBgNVBAYTAlVT\r\n"
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\r\n"
    "b20xHjAcBgNVBAMTFUdlb1RydXN0IENOIFJTQSBDQSBHMTCCASIwDQYJKoZIhvcN\r\n"
    "AQEBBQADggEPADCCAQoCggEBALFJ+j1KeZVG4jzgQob23lQ8PJUNhY31ufZihuUx\r\n"
    "hYc6HSU4Lw0fxfA43a9DpJl74M3E6F1ZRBOfJ+dWnaiyYD0PxRIQd4wJisti4Uad\r\n"
    "vz61IYY/oQ/Elxk/X7GFDquYuxCSyBdHtTVMXCxFSvQ2C/7jWZFDfGGKKNoQSiJy\r\n"
    "wDe8iiHbUOakLMmXmOTZyWJnFdR/TH5YNTiMKCNUPHAleG4IigGxDyL/gbwrdDNi\r\n"
    "bDA4lUNhD0xNvPjQ8BNKqm5HWDvirUuHdC+4hpi0GJO34O3iiRV16YmWTuVFNboU\r\n"
    "LDZ0+PQtctJnatpuZKPGyKX6jCpPvzzPw/EhNDlpEdrYHZMCAwEAAaOCAc4wggHK\r\n"
    "MB0GA1UdDgQWBBSRn14xFa4Qn61gwffBzKpINC8MJjAfBgNVHSMEGDAWgBQD3lA1\r\n"
    "VtFMu2bwo+IbG8OXsj3RVTAOBgNVHQ8BAf8EBAMCAYYwHQYDVR0lBBYwFAYIKwYB\r\n"
    "BQUHAwEGCCsGAQUFBwMCMA8GA1UdEwEB/wQFMAMBAf8wMQYIKwYBBQUHAQEEJTAj\r\n"
    "MCEGCCsGAQUFBzABhhVodHRwOi8vb2NzcC5kY29jc3AuY24wRAYDVR0fBD0wOzA5\r\n"
    "oDegNYYzaHR0cDovL2NybC5kaWdpY2VydC1jbi5jb20vRGlnaUNlcnRHbG9iYWxS\r\n"
    "b290Q0EuY3JsMIHOBgNVHSAEgcYwgcMwgcAGBFUdIAAwgbcwKAYIKwYBBQUHAgEW\r\n"
    "HGh0dHBzOi8vd3d3LmRpZ2ljZXJ0LmNvbS9DUFMwgYoGCCsGAQUFBwICMH4MfEFu\r\n"
    "eSB1c2Ugb2YgdGhpcyBDZXJ0aWZpY2F0ZSBjb25zdGl0dXRlcyBhY2NlcHRhbmNl\r\n"
    "IG9mIHRoZSBSZWx5aW5nIFBhcnR5IEFncmVlbWVudCBsb2NhdGVkIGF0IGh0dHBz\r\n"
    "Oi8vd3d3LmRpZ2ljZXJ0LmNvbS9ycGEtdWEwDQYJKoZIhvcNAQELBQADggEBABfg\r\n"
    "eXrxIrtlixBv+KMDeqKxtNJbZiLDzJBkGCd4HI63X5eS6BElJBn6mI9eYVrr7qOL\r\n"
    "Tp7WiO02Sf1Yrpaz/ePSjZ684o89UAGpxOfbgVSMvo/a07n/220jUWLxzaJhQNLu\r\n"
    "lACXwwWsxYf8twP8glkoIHnUUNTlhsyyl1ZzvVC4bDpI4hC6QkJGync1MNqYSMj8\r\n"
    "tZbhQNw3HdSmcTO0Nc/J/pK2VZc6fFbKBgspmzdHc6jMKG2t4lisXEysS3wPcg0a\r\n"
    "Nfr1Odl5+myh3MnMK08f6pTXvduLz+QZiIh8IYL+Z6QWgTZ9e2jnV8juumX1I8Ge\r\n"
    "7cZdtNnTCB8hFfwGLUA=\r\n"
    "-----END CERTIFICATE-----"};
#endif
#endif

#ifndef AUTH_WITH_NOTLS
#if defined(AUTH_MODE_CERT)
static const char *iot_ca_crt = {
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDxTCCAq2gAwIBAgIJALM1winYO2xzMA0GCSqGSIb3DQEBCwUAMHkxCzAJBgNV\r\n"
    "BAYTAkNOMRIwEAYDVQQIDAlHdWFuZ0RvbmcxETAPBgNVBAcMCFNoZW5aaGVuMRAw\r\n"
    "DgYDVQQKDAdUZW5jZW50MRcwFQYDVQQLDA5UZW5jZW50IElvdGh1YjEYMBYGA1UE\r\n"
    "AwwPd3d3LnRlbmNlbnQuY29tMB4XDTE3MTEyNzA0MjA1OVoXDTMyMTEyMzA0MjA1\r\n"
    "OVoweTELMAkGA1UEBhMCQ04xEjAQBgNVBAgMCUd1YW5nRG9uZzERMA8GA1UEBwwI\r\n"
    "U2hlblpoZW4xEDAOBgNVBAoMB1RlbmNlbnQxFzAVBgNVBAsMDlRlbmNlbnQgSW90\r\n"
    "aHViMRgwFgYDVQQDDA93d3cudGVuY2VudC5jb20wggEiMA0GCSqGSIb3DQEBAQUA\r\n"
    "A4IBDwAwggEKAoIBAQDVxwDZRVkU5WexneBEkdaKs4ehgQbzpbufrWo5Lb5gJ3i0\r\n"
    "eukbOB81yAaavb23oiNta4gmMTq2F6/hAFsRv4J2bdTs5SxwEYbiYU1teGHuUQHO\r\n"
    "iQsZCdNTJgcikga9JYKWcBjFEnAxKycNsmqsq4AJ0CEyZbo//IYX3czEQtYWHjp7\r\n"
    "FJOlPPd1idKtFMVNG6LGXEwS/TPElE+grYOxwB7Anx3iC5ZpE5lo5tTioFTHzqbT\r\n"
    "qTN7rbFZRytAPk/JXMTLgO55fldm4JZTP3GQsPzwIh4wNNKhi4yWG1o2u3hAnZDv\r\n"
    "UVFV7al2zFdOfuu0KMzuLzrWrK16SPadRDd9eT17AgMBAAGjUDBOMB0GA1UdDgQW\r\n"
    "BBQrr48jv4FxdKs3r0BkmJO7zH4ALzAfBgNVHSMEGDAWgBQrr48jv4FxdKs3r0Bk\r\n"
    "mJO7zH4ALzAMBgNVHRMEBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQDRSjXnBc3T\r\n"
    "d9VmtTCuALXrQELY8KtM+cXYYNgtodHsxmrRMpJofsPGiqPfb82klvswpXxPK8Xx\r\n"
    "SuUUo74Fo+AEyJxMrRKlbJvlEtnpSilKmG6rO9+bFq3nbeOAfat4lPl0DIscWUx3\r\n"
    "ajXtvMCcSwTlF8rPgXbOaSXZidRYNqSyUjC2Q4m93Cv+KlyB+FgOke8x4aKAkf5p\r\n"
    "XR8i1BN1OiMTIRYhGSfeZbVRq5kTdvtahiWFZu9DGO+hxDZObYGIxGHWPftrhBKz\r\n"
    "RT16Amn780rQLWojr70q7o7QP5tO0wDPfCdFSc6CQFq/ngOzYag0kJ2F+O5U6+kS\r\n"
    "QVrcRBDxzx/G\r\n"
    "-----END CERTIFICATE-----"};
#endif

#ifdef OTA_USE_HTTPS
static const char *iot_https_ca_crt = {
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\r\n"
    "A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\r\n"
    "b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\r\n"
    "MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\r\n"
    "YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\r\n"
    "aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\r\n"
    "jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\r\n"
    "xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\r\n"
    "1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\r\n"
    "snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\r\n"
    "U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\r\n"
    "9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\r\n"
    "BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\r\n"
    "AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\r\n"
    "yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\r\n"
    "38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\r\n"
    "AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\r\n"
    "DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\r\n"
    "HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\r\n"
    "-----END CERTIFICATE-----\r\n"
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIEaTCCA1GgAwIBAgILBAAAAAABRE7wQkcwDQYJKoZIhvcNAQELBQAwVzELMAkG\r\n"
    "A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\r\n"
    "b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw0xNDAyMjAxMDAw\r\n"
    "MDBaFw0yNDAyMjAxMDAwMDBaMGYxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\r\n"
    "YWxTaWduIG52LXNhMTwwOgYDVQQDEzNHbG9iYWxTaWduIE9yZ2FuaXphdGlvbiBW\r\n"
    "YWxpZGF0aW9uIENBIC0gU0hBMjU2IC0gRzIwggEiMA0GCSqGSIb3DQEBAQUAA4IB\r\n"
    "DwAwggEKAoIBAQDHDmw/I5N/zHClnSDDDlM/fsBOwphJykfVI+8DNIV0yKMCLkZc\r\n"
    "C33JiJ1Pi/D4nGyMVTXbv/Kz6vvjVudKRtkTIso21ZvBqOOWQ5PyDLzm+ebomchj\r\n"
    "SHh/VzZpGhkdWtHUfcKc1H/hgBKueuqI6lfYygoKOhJJomIZeg0k9zfrtHOSewUj\r\n"
    "mxK1zusp36QUArkBpdSmnENkiN74fv7j9R7l/tyjqORmMdlMJekYuYlZCa7pnRxt\r\n"
    "Nw9KHjUgKOKv1CGLAcRFrW4rY6uSa2EKTSDtc7p8zv4WtdufgPDWi2zZCHlKT3hl\r\n"
    "2pK8vjX5s8T5J4BO/5ZS5gIg4Qdz6V0rvbLxAgMBAAGjggElMIIBITAOBgNVHQ8B\r\n"
    "Af8EBAMCAQYwEgYDVR0TAQH/BAgwBgEB/wIBADAdBgNVHQ4EFgQUlt5h8b0cFilT\r\n"
    "HMDMfTuDAEDmGnwwRwYDVR0gBEAwPjA8BgRVHSAAMDQwMgYIKwYBBQUHAgEWJmh0\r\n"
    "dHBzOi8vd3d3Lmdsb2JhbHNpZ24uY29tL3JlcG9zaXRvcnkvMDMGA1UdHwQsMCow\r\n"
    "KKAmoCSGImh0dHA6Ly9jcmwuZ2xvYmFsc2lnbi5uZXQvcm9vdC5jcmwwPQYIKwYB\r\n"
    "BQUHAQEEMTAvMC0GCCsGAQUFBzABhiFodHRwOi8vb2NzcC5nbG9iYWxzaWduLmNv\r\n"
    "bS9yb290cjEwHwYDVR0jBBgwFoAUYHtmGkUNl8qJUC99BM00qP/8/UswDQYJKoZI\r\n"
    "hvcNAQELBQADggEBAEYq7l69rgFgNzERhnF0tkZJyBAW/i9iIxerH4f4gu3K3w4s\r\n"
    "32R1juUYcqeMOovJrKV3UPfvnqTgoI8UV6MqX+x+bRDmuo2wCId2Dkyy2VG7EQLy\r\n"
    "XN0cvfNVlg/UBsD84iOKJHDTu/B5GqdhcIOKrwbFINihY9Bsrk8y1658GEV1BSl3\r\n"
    "30JAZGSGvip2CTFvHST0mdCF/vIhCPnG9vHQWe3WVjwIKANnuvD58ZAWR65n5ryA\r\n"
    "SOlCdjSXVWkkDoPWoC209fN5ikkodBpBocLTJIg1MGCUF7ThBCIxPTsvFwayuJ2G\r\n"
    "K1pp74P1S8SqtCr4fKGxhZSM9AyHDPSsQPhZSZg=\r\n"
    "-----END CERTIFICATE-----"};
#endif

#ifdef REMOTE_LOGIN_WEBSOCKET_SSH
static const char *iot_wss_ssh_ca_crt = {
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIErjCCA5agAwIBAgIQBYAmfwbylVM0jhwYWl7uLjANBgkqhkiG9w0BAQsFADBh\r\n"
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\r\n"
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\r\n"
    "QTAeFw0xNzEyMDgxMjI4MjZaFw0yNzEyMDgxMjI4MjZaMHIxCzAJBgNVBAYTAkNO\r\n"
    "MSUwIwYDVQQKExxUcnVzdEFzaWEgVGVjaG5vbG9naWVzLCBJbmMuMR0wGwYDVQQL\r\n"
    "ExREb21haW4gVmFsaWRhdGVkIFNTTDEdMBsGA1UEAxMUVHJ1c3RBc2lhIFRMUyBS\r\n"
    "U0EgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCgWa9X+ph+wAm8\r\n"
    "Yh1Fk1MjKbQ5QwBOOKVaZR/OfCh+F6f93u7vZHGcUU/lvVGgUQnbzJhR1UV2epJa\r\n"
    "e+m7cxnXIKdD0/VS9btAgwJszGFvwoqXeaCqFoP71wPmXjjUwLT70+qvX4hdyYfO\r\n"
    "JcjeTz5QKtg8zQwxaK9x4JT9CoOmoVdVhEBAiD3DwR5fFgOHDwwGxdJWVBvktnoA\r\n"
    "zjdTLXDdbSVC5jZ0u8oq9BiTDv7jAlsB5F8aZgvSZDOQeFrwaOTbKWSEInEhnchK\r\n"
    "ZTD1dz6aBlk1xGEI5PZWAnVAba/ofH33ktymaTDsE6xRDnW97pDkimCRak6CEbfe\r\n"
    "3dXw6OV5AgMBAAGjggFPMIIBSzAdBgNVHQ4EFgQUf9OZ86BHDjEAVlYijrfMnt3K\r\n"
    "AYowHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUwDgYDVR0PAQH/BAQD\r\n"
    "AgGGMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjASBgNVHRMBAf8ECDAG\r\n"
    "AQH/AgEAMDQGCCsGAQUFBwEBBCgwJjAkBggrBgEFBQcwAYYYaHR0cDovL29jc3Au\r\n"
    "ZGlnaWNlcnQuY29tMEIGA1UdHwQ7MDkwN6A1oDOGMWh0dHA6Ly9jcmwzLmRpZ2lj\r\n"
    "ZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RDQS5jcmwwTAYDVR0gBEUwQzA3Bglg\r\n"
    "hkgBhv1sAQIwKjAoBggrBgEFBQcCARYcaHR0cHM6Ly93d3cuZGlnaWNlcnQuY29t\r\n"
    "L0NQUzAIBgZngQwBAgEwDQYJKoZIhvcNAQELBQADggEBAK3dVOj5dlv4MzK2i233\r\n"
    "lDYvyJ3slFY2X2HKTYGte8nbK6i5/fsDImMYihAkp6VaNY/en8WZ5qcrQPVLuJrJ\r\n"
    "DSXT04NnMeZOQDUoj/NHAmdfCBB/h1bZ5OGK6Sf1h5Yx/5wR4f3TUoPgGlnU7EuP\r\n"
    "ISLNdMRiDrXntcImDAiRvkh5GJuH4YCVE6XEntqaNIgGkRwxKSgnU3Id3iuFbW9F\r\n"
    "UQ9Qqtb1GX91AJ7i4153TikGgYCdwYkBURD8gSVe8OAco6IfZOYt/TEwii1Ivi1C\r\n"
    "qnuUlWpsF1LdQNIdfbW3TSe0BhQa7ifbVIfvPWHYOu3rkg1ZeMo6XRU9B4n5VyJY\r\n"
    "RmE=\r\n"
    "-----END CERTIFICATE-----\r\n"
    "-----BEGIN CERTIFICATE-----\r\n"
    "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\r\n"
    "XDDejuLz3BQMFyy/AzO3rGPF\r\n"
    "-----END CERTIFICATE-----"};
#endif

#ifdef WEBSOCKET_MQTT
static const char *iot_wss_mqtt_ca_crt = {
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIG2zCCBcOgAwIBAgIQB1P3ab1jMWo1LRNkq5S8lTANBgkqhkiG9w0BAQsFADBy\r\n"
    "MQswCQYDVQQGEwJDTjElMCMGA1UEChMcVHJ1c3RBc2lhIFRlY2hub2xvZ2llcywg\r\n"
    "SW5jLjEdMBsGA1UECxMURG9tYWluIFZhbGlkYXRlZCBTU0wxHTAbBgNVBAMTFFRy\r\n"
    "dXN0QXNpYSBUTFMgUlNBIENBMB4XDTIxMDkwNjAwMDAwMFoXDTIyMDkwNTIzNTk1\r\n"
    "OVowMzExMC8GA1UEAwwoKi5hcC1ndWFuZ3pob3UuaW90aHViLnRlbmNlbnRkZXZp\r\n"
    "Y2VzLmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKVb9w80wUTt\r\n"
    "tQbspCCJGgaL3AGsKnfXi0iXWb60N+UP/Gz4xW5DXNPVSKHpOS7eKs142lPyRX/P\r\n"
    "Cf7d3tV/OVz8Hur2OW7HPRb4fSjXC7hQTJznq7e5GC1Qa80MhsqO9SmDmqVa5iZ6\r\n"
    "6gCdlvz31vVbG31F/xDpVmE6bCnZQ0TTs9V8ZZs/UbkOWtc/gq8pKAhD9/6eaqJ7\r\n"
    "oS3DkCyYRRBCUbcM0jUx8TTmWGNGYea5IY24T/FTKbeVZliZmS7iEPP4FBOAys1h\r\n"
    "VVhBUecZnD3k2nMoV6SKIQt9WfeV86q0LYivRkgpVNLzm4Ew1Soz9+2cvC7Z77aR\r\n"
    "Hc/vrwcJLQUCAwEAAaOCA6owggOmMB8GA1UdIwQYMBaAFH/TmfOgRw4xAFZWIo63\r\n"
    "zJ7dygGKMB0GA1UdDgQWBBRoKH8kwb5UFaBQqttlWXFYGdwk2DCB0AYDVR0RBIHI\r\n"
    "MIHFgigqLmFwLWd1YW5nemhvdS5pb3RodWIudGVuY2VudGRldmljZXMuY29tgiUq\r\n"
    "LnVzLWVhc3QuaW90Y2xvdWQudGVuY2VudGRldmljZXMuY29tgiIqLmV1cm9wZS5p\r\n"
    "b3RodWIudGVuY2VudGRldmljZXMuY29tgiYqLmFwLWJhbmdrb2suaW90aHViLnRl\r\n"
    "bmNlbnRkZXZpY2VzLmNvbYImYXAtZ3Vhbmd6aG91LmlvdGh1Yi50ZW5jZW50ZGV2\r\n"
    "aWNlcy5jb20wDgYDVR0PAQH/BAQDAgWgMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggr\r\n"
    "BgEFBQcDAjA+BgNVHSAENzA1MDMGBmeBDAECATApMCcGCCsGAQUFBwIBFhtodHRw\r\n"
    "Oi8vd3d3LmRpZ2ljZXJ0LmNvbS9DUFMwgZIGCCsGAQUFBwEBBIGFMIGCMDQGCCsG\r\n"
    "AQUFBzABhihodHRwOi8vc3RhdHVzZS5kaWdpdGFsY2VydHZhbGlkYXRpb24uY29t\r\n"
    "MEoGCCsGAQUFBzAChj5odHRwOi8vY2FjZXJ0cy5kaWdpdGFsY2VydHZhbGlkYXRp\r\n"
    "b24uY29tL1RydXN0QXNpYVRMU1JTQUNBLmNydDAJBgNVHRMEAjAAMIIBgAYKKwYB\r\n"
    "BAHWeQIEAgSCAXAEggFsAWoAdgApeb7wnjk5IfBWc59jpXflvld9nGAK+PlNXSZc\r\n"
    "JV3HhAAAAXu68FUsAAAEAwBHMEUCIBMWhGfG35EWzQpepoOnxMU3wAhHHgR6tZOu\r\n"
    "8RAm5CSnAiEAsH00TzJxSK21iBs/Zbl5X9Gn35vqsVuPO+gMhz/pSo4AdwBRo7D1\r\n"
    "/QF5nFZtuDd4jwykeswbJ8v3nohCmg3+1IsF5QAAAXu68FUUAAAEAwBIMEYCIQCH\r\n"
    "NeLukMhB0RNd7Zm16YlsnAoLb5CAcJ68e5W9EJzZhAIhANOqk486D8etHA4p3nS9\r\n"
    "+VZkZD3y9M4fi1Kv8yjeo7dlAHcAQcjKsd8iRkoQxqE6CUKHXk4xixsD6+tLx2jw\r\n"
    "kGKWBvYAAAF7uvBUjwAABAMASDBGAiEAkif4MdoQ4E6DAIT0unnxRFyTwFP/myo/\r\n"
    "ofYYXfPMPqQCIQDU/jXaShkd1XVxAZpwW332c1MpG5xCsCJbDnjjPa0OCzANBgkq\r\n"
    "hkiG9w0BAQsFAAOCAQEASERgtLHdDfYDRfWpDHEX/Rbbx8Bv7agbk6+YWVF5YZ/z\r\n"
    "75YsbD8btojrjgksKQaBe1aHAq8cOai8wFkcQsnmmDN7cEJplj3JzJpwFPXU2J0B\r\n"
    "p8E59sA/DHzR1Z8DjGzyd70NKPa0Nf3w2EUYEpH0B5kGEhI5G0D3ybs66sXgNfcb\r\n"
    "usW1QOjEhPQOJ3X5g5NItqZkjbZEQ41EOQV5qUa8/eHe7GayyE+EZNdY5aj5BVR0\r\n"
    "YFLvrA5DVRa+DNbeqLWoQeVNb0kEjq0xnaaD3ohm9xmMYleRx0l8mhEEZ/5Hha1P\r\n"
    "Zcrqjyw+6baShrOfotoFDlFE/wqf6FjhgeRkOb5QlA==\r\n"
    "-----END CERTIFICATE-----\r\n"
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIErjCCA5agAwIBAgIQBYAmfwbylVM0jhwYWl7uLjANBgkqhkiG9w0BAQsFADBh\r\n"
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\r\n"
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\r\n"
    "QTAeFw0xNzEyMDgxMjI4MjZaFw0yNzEyMDgxMjI4MjZaMHIxCzAJBgNVBAYTAkNO\r\n"
    "MSUwIwYDVQQKExxUcnVzdEFzaWEgVGVjaG5vbG9naWVzLCBJbmMuMR0wGwYDVQQL\r\n"
    "ExREb21haW4gVmFsaWRhdGVkIFNTTDEdMBsGA1UEAxMUVHJ1c3RBc2lhIFRMUyBS\r\n"
    "U0EgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCgWa9X+ph+wAm8\r\n"
    "Yh1Fk1MjKbQ5QwBOOKVaZR/OfCh+F6f93u7vZHGcUU/lvVGgUQnbzJhR1UV2epJa\r\n"
    "e+m7cxnXIKdD0/VS9btAgwJszGFvwoqXeaCqFoP71wPmXjjUwLT70+qvX4hdyYfO\r\n"
    "JcjeTz5QKtg8zQwxaK9x4JT9CoOmoVdVhEBAiD3DwR5fFgOHDwwGxdJWVBvktnoA\r\n"
    "zjdTLXDdbSVC5jZ0u8oq9BiTDv7jAlsB5F8aZgvSZDOQeFrwaOTbKWSEInEhnchK\r\n"
    "ZTD1dz6aBlk1xGEI5PZWAnVAba/ofH33ktymaTDsE6xRDnW97pDkimCRak6CEbfe\r\n"
    "3dXw6OV5AgMBAAGjggFPMIIBSzAdBgNVHQ4EFgQUf9OZ86BHDjEAVlYijrfMnt3K\r\n"
    "AYowHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUwDgYDVR0PAQH/BAQD\r\n"
    "AgGGMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjASBgNVHRMBAf8ECDAG\r\n"
    "AQH/AgEAMDQGCCsGAQUFBwEBBCgwJjAkBggrBgEFBQcwAYYYaHR0cDovL29jc3Au\r\n"
    "ZGlnaWNlcnQuY29tMEIGA1UdHwQ7MDkwN6A1oDOGMWh0dHA6Ly9jcmwzLmRpZ2lj\r\n"
    "ZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RDQS5jcmwwTAYDVR0gBEUwQzA3Bglg\r\n"
    "hkgBhv1sAQIwKjAoBggrBgEFBQcCARYcaHR0cHM6Ly93d3cuZGlnaWNlcnQuY29t\r\n"
    "L0NQUzAIBgZngQwBAgEwDQYJKoZIhvcNAQELBQADggEBAK3dVOj5dlv4MzK2i233\r\n"
    "lDYvyJ3slFY2X2HKTYGte8nbK6i5/fsDImMYihAkp6VaNY/en8WZ5qcrQPVLuJrJ\r\n"
    "DSXT04NnMeZOQDUoj/NHAmdfCBB/h1bZ5OGK6Sf1h5Yx/5wR4f3TUoPgGlnU7EuP\r\n"
    "ISLNdMRiDrXntcImDAiRvkh5GJuH4YCVE6XEntqaNIgGkRwxKSgnU3Id3iuFbW9F\r\n"
    "UQ9Qqtb1GX91AJ7i4153TikGgYCdwYkBURD8gSVe8OAco6IfZOYt/TEwii1Ivi1C\r\n"
    "qnuUlWpsF1LdQNIdfbW3TSe0BhQa7ifbVIfvPWHYOu3rkg1ZeMo6XRU9B4n5VyJY\r\n"
    "RmE=\r\n"
    "-----END CERTIFICATE-----\r\n"};

#endif
#endif

const char *iot_ca_get()
{
#ifndef AUTH_WITH_NOTLS
#if defined(AUTH_MODE_CERT)
    return iot_ca_crt;
#else
    return NULL;
#endif
#else
    return NULL;
#endif
}

const char *iot_dynreg_https_ca_get()
{
#ifndef AUTH_WITH_NOTLS
#if defined(DEV_DYN_REG_ENABLED)
    return iot_dynreg_https_ca_crt;
#else
    return NULL;
#endif
#else
    return NULL;
#endif
}

const char *iot_https_ca_get()
{
#ifndef AUTH_WITH_NOTLS

#ifdef OTA_USE_HTTPS
    return iot_https_ca_crt;
#else
    return NULL;
#endif

#else
    return NULL;
#endif
}

const char *iot_wss_ssh_ca_get()
{
#ifndef AUTH_WITH_NOTLS

#ifdef REMOTE_LOGIN_WEBSOCKET_SSH
    return iot_wss_ssh_ca_crt;
#else
    return NULL;
#endif

#else
    return NULL;
#endif
}

const char *iot_wss_mqtt_ca_get()
{
#ifndef AUTH_WITH_NOTLS

#ifdef WEBSOCKET_MQTT
    return iot_wss_mqtt_ca_crt;
#else
    return NULL;
#endif

#else
    return NULL;
#endif
}

static const char *iot_get_domain(const char *region, const RegionDomain *region_domain_array, const int domain_num)
{
    const char *pDomain = region_domain_array[0].domain;

    int i;

    if (region) {
        for (i = 0; i < domain_num; i++) {
            if (0 == strcmp(region, region_domain_array[i].region)) {
                pDomain = region_domain_array[i].domain;
                break;
            }
        }
    }

    return pDomain;
}

const char *iot_get_mqtt_domain(const char *region)
{
#ifdef WEBSOCKET_MQTT
    return iot_get_domain(region, sg_iot_websocket_mqtt_domain,
                          sizeof(sg_iot_websocket_mqtt_domain) / sizeof(sg_iot_websocket_mqtt_domain[0]));
#else
    return iot_get_domain(region, sg_iot_mqtt_coap_domain,
                          sizeof(sg_iot_mqtt_coap_domain) / sizeof(sg_iot_mqtt_coap_domain[0]));
#endif
}

int iot_get_mqtt_port()
{
    int port = MQTT_SERVER_PORT_NOTLS;

#ifndef AUTH_WITH_NOTLS
    port = MQTT_SERVER_PORT_TLS;
#endif

#ifdef WEBSOCKET_MQTT
    port = QCLOUD_IOT_WEBSOCKET_MQTT_SERVER_PORT_NOTLS;
#ifndef AUTH_WITH_NOTLS
    port = QCLOUD_IOT_WEBSOCKET_MQTT_SERVER_PORT_TLS;
#endif
#endif
    return port;
}

const char *iot_get_coap_domain(const char *region)
{
    return iot_get_domain(region, sg_iot_mqtt_coap_domain,
                          sizeof(sg_iot_mqtt_coap_domain) / sizeof(sg_iot_mqtt_coap_domain[0]));
}

const char *iot_get_dyn_reg_domain(const char *region)
{
    return iot_get_domain(region, sg_iot_dynreg_log_domain,
                          sizeof(sg_iot_dynreg_log_domain) / sizeof(sg_iot_dynreg_log_domain[0]));
}

const char *iot_get_log_domain(const char *region)
{
    return iot_get_domain(region, sg_iot_dynreg_log_domain,
                          sizeof(sg_iot_dynreg_log_domain) / sizeof(sg_iot_dynreg_log_domain[0]));
}

const char *iot_get_ws_ssh_domain(const char *region)
{
#ifdef REMOTE_LOGIN_WEBSOCKET_SSH
    return iot_get_domain(region, sg_iot_websocket_ssh_domain,
                          sizeof(sg_iot_websocket_ssh_domain) / sizeof(sg_iot_websocket_ssh_domain[0]));
#else
    return NULL;
#endif
}

#ifdef __cplusplus
}
#endif
