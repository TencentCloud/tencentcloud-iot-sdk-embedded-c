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

#include "coap_client_net.h"

#include "network_interface.h"

int qcloud_iot_coap_network_init(Network *pNetwork)
{
    int rc;
    /* first choice: TLS */
    pNetwork->type = NETWORK_DTLS;

#ifdef AUTH_WITH_NOTLS
    pNetwork->type = NETWORK_UDP;
#endif

    rc = network_init(pNetwork);

    return rc;
}

#ifdef __cplusplus
}
#endif
