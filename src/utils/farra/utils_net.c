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

#ifdef __cplusplus
extern "C" {
#endif

#include "utils_net.h"

#include "qcloud_iot_export.h"
#include "qcloud_iot_sdk_impl_internal.h"

static int _utils_net_connected(Network *pNetwork) {
    return pNetwork->handle;
}

#ifndef AUTH_WITH_NOTLS
/*** SSL connection ***/
static int _read_tls(Network *pNetwork, unsigned char *data, size_t datalen, uint32_t timeout_ms, size_t *read_len)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

	return HAL_TLS_Read(pNetwork->handle, data, datalen, timeout_ms, read_len);
}

static int _write_tls(Network *pNetwork, unsigned char *data, size_t datalen, uint32_t timeout_ms, size_t *written_len)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

	return HAL_TLS_Write(pNetwork->handle, data, datalen, timeout_ms, written_len);
}

static int _disconnect_tls(Network *pNetwork)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

	HAL_TLS_Disconnect(pNetwork->handle);
	pNetwork->handle = 0;

	return 0;
}

static int _connect_tls(Network *pNetwork)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

	int ret = QCLOUD_ERR_FAILURE;

	pNetwork->handle = (uintptr_t)HAL_TLS_Connect(&(pNetwork->ssl_connect_params), pNetwork->host, pNetwork->port);
	if (pNetwork->handle != 0) {
		ret = QCLOUD_ERR_SUCCESS;
	}

	return ret;
}

#else
/*** TCP connection ***/
static int _read_tcp(Network *pNetwork, unsigned char *data, size_t datalen, uint32_t timeout_ms, size_t *read_len)
{
	return HAL_TCP_Read(pNetwork->handle, data, (uint32_t)datalen, timeout_ms, read_len);
}

static int _write_tcp(Network *pNetwork, unsigned char *data, size_t datalen, uint32_t timeout_ms, size_t *written_len)
{
	return HAL_TCP_Write(pNetwork->handle, data, datalen, timeout_ms, written_len);
}

static int _disconnect_tcp(Network *pNetwork)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

	if (0 == pNetwork->handle) {
		return -1;
	}

	HAL_TCP_Disconnect(pNetwork->handle);
	pNetwork->handle = 0;
	return 0;
}

static int _connect_tcp(Network *pNetwork)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

	pNetwork->handle = HAL_TCP_Connect(pNetwork->host, pNetwork->port);
	if (0 == pNetwork->handle) {
		return -1;
	}
	return 0;
}
#endif

int utils_net_read(Network *pNetwork, unsigned char *data, size_t datalen, uint32_t timeout_ms, size_t *read_len)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

    int rc = 0;

#ifndef AUTH_WITH_NOTLS
	rc = _read_tls(pNetwork, data, datalen, timeout_ms, read_len);
#else
	rc = _read_tcp(pNetwork, data, datalen, timeout_ms, read_len);
#endif

    return rc;
}

int utils_net_write(Network *pNetwork, unsigned char *data, size_t datalen, uint32_t timeout_ms, size_t *written_len)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

    int rc = 0;

#ifndef AUTH_WITH_NOTLS
	rc = _write_tls(pNetwork, data, datalen, timeout_ms, written_len);
#else
    rc = _write_tcp(pNetwork, data, datalen, timeout_ms, written_len);
#endif

    return rc;
}

void utils_net_disconnect(Network *pNetwork)
{
	POINTER_SANITY_CHECK_RTN(pNetwork);

#ifndef AUTH_WITH_NOTLS
	_disconnect_tls(pNetwork);
#else
    _disconnect_tcp(pNetwork);
#endif
}

int utils_net_connect(Network *pNetwork)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

    int rc = 0;

#ifndef AUTH_WITH_NOTLS
	rc = _connect_tls(pNetwork);
#else
    rc = _connect_tcp(pNetwork);
#endif

    return rc;
}

int utils_net_init(Network *pNetwork)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

	int rc = QCLOUD_ERR_SUCCESS;

    pNetwork->connect = utils_net_connect;
    pNetwork->read = utils_net_read;
    pNetwork->write = utils_net_write;
    pNetwork->disconnect = utils_net_disconnect;
    pNetwork->is_connected = _utils_net_connected;
    pNetwork->handle = 0;

    return rc;
}

#ifdef COAP_COMM_ENABLED
static int _utils_udp_net_connected(Network *pNetwork) {
    return pNetwork->handle;
}

#ifndef AUTH_WITH_NOTLS
static int _read_dtls(Network *pNetwork, unsigned char *data, size_t datalen, uint32_t timeout_ms, size_t *read_len)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

	return HAL_DTLS_Read(pNetwork->handle, data, datalen, timeout_ms, read_len);
}

static int _write_dtls(Network *pNetwork, unsigned char *data, size_t datalen, uint32_t timeout_ms, size_t *written_len)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

	return HAL_DTLS_Write(pNetwork->handle, data, datalen, written_len);
}

static int _disconnect_dtls(Network *pNetwork)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

	HAL_DTLS_Disconnect(pNetwork->handle);
	pNetwork->handle = 0;

	return 0;
}

static int _connect_dtls(Network *pNetwork)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

	int ret = QCLOUD_ERR_FAILURE;

	pNetwork->handle = (uintptr_t)HAL_DTLS_Connect(&(pNetwork->ssl_connect_params), pNetwork->host, pNetwork->port);
	if (pNetwork->handle != 0) {
		ret = QCLOUD_ERR_SUCCESS;
	}

	return ret;
}
#else
static int _read_udp(Network *pNetwork, unsigned char *data, size_t datalen, uint32_t timeout_ms, size_t *read_len)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

	int ret = HAL_UDP_ReadTimeout(pNetwork->handle, data, datalen, timeout_ms);
	if (ret > 0) {
		*read_len = ret;
		ret = 0;
	}

	return ret;
}

static int _write_udp(Network *pNetwork, unsigned char *data, size_t datalen, uint32_t timeout_ms, size_t *written_len)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

	int ret = HAL_UDP_Write(pNetwork->handle, data, datalen);
	if (ret > 0) {
		*written_len = ret;
		ret = 0;
	}

	return ret;
}

static int _disconnect_udp(Network *pNetwork)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

	HAL_UDP_Disconnect(pNetwork->handle);
	pNetwork->handle = 0;

	return 0;
}

static int _connect_udp(Network *pNetwork)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

	pNetwork->handle = HAL_UDP_Connect(pNetwork->host, pNetwork->port);
	if (0 == pNetwork->handle) {
		return -1;
	}
	return 0;
}
#endif

int utils_udp_net_read(Network *pNetwork, unsigned char *data, size_t datalen, uint32_t timeout_ms, size_t *read_len)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

	int rc = 0;

#ifndef AUTH_WITH_NOTLS
	rc = _read_dtls(pNetwork, data, datalen, timeout_ms, read_len);
#else
    rc = _read_udp(pNetwork, data, datalen, timeout_ms, read_len);
#endif

	return rc;
}

int utils_udp_net_write(Network *pNetwork, unsigned char *data, size_t datalen, uint32_t timeout_ms, size_t *written_len)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

	int rc = 0;
#ifndef AUTH_WITH_NOTLS
	rc = _write_dtls(pNetwork, data, datalen, timeout_ms, written_len);
#else
    rc = _write_udp(pNetwork, data, datalen, timeout_ms, written_len);
#endif

	return rc;
}

void utils_udp_net_disconnect(Network *pNetwork)
{
	POINTER_SANITY_CHECK_RTN(pNetwork);
#ifndef AUTH_WITH_NOTLS
	_disconnect_dtls(pNetwork);
#else
    _disconnect_udp(pNetwork);
#endif
}

int utils_udp_net_connect(Network *pNetwork)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

	int rc = 0;
#ifndef AUTH_WITH_NOTLS
	rc = _connect_dtls(pNetwork);
#else
    rc = _connect_udp(pNetwork);
#endif

	return rc;
}

int utils_udp_net_init(Network *pNetwork)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

	int rc = QCLOUD_ERR_SUCCESS;

    pNetwork->connect = utils_udp_net_connect;
    pNetwork->read = utils_udp_net_read;
    pNetwork->write = utils_udp_net_write;
    pNetwork->disconnect = utils_udp_net_disconnect;
    pNetwork->is_connected = _utils_udp_net_connected;
    pNetwork->handle = 0;

    return rc;
}
#endif

#ifdef __cplusplus
}
#endif
