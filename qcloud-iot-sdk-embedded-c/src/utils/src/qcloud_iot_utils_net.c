#ifdef __cplusplus
extern "C" {
#endif
    
#include "qcloud_iot_utils_net.h"
#include "qcloud_iot_export_log.h"
#include "qcloud_iot_export_error.h"
#include "qcloud_iot_sdk_impl_internal.h"
    
static int utils_net_connected(Network *pNetwork) {
    return pNetwork->handle;
}

/*** TCP connection ***/
static int _read_tcp(Network *pNetwork, unsigned char *pMsg, size_t len, int timeout_ms, size_t *read_len)
{
    // TODO: 后续完善
    return -1;
}

static int _write_tcp(Network *pNetwork, unsigned char *pMsg, size_t len, int timeout_ms, size_t *written_len)
{
    // TODO: 后续完善
    return -1;
}
    
static int _disconnect_tcp(Network *pNetwork)
{
    // TODO: 后续完善
    return -1;
}

static int _connect_tcp(Network *pNetwork)
{
    // TODO: 后续完善
    Log_d("use tcp to connect");
    return -1;
}
    
/*** TLS connection ***/
static int _read_tls(Network *pNetwork, unsigned char *pMsg, size_t len, int timeout_ms, size_t *read_len)
{
    POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);
    
    return HAL_TLS_Read(pNetwork->handle, pMsg, len, timeout_ms, read_len);
}
    
static int _write_tls(Network *pNetwork, unsigned char *pMsg, size_t len, int timeout_ms, size_t *written_len)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);
    
    return HAL_TLS_Write(pNetwork->handle, pMsg, len, timeout_ms, written_len);
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

    pNetwork->handle = (uintptr_t)HAL_TLS_Connect(&(pNetwork->tlsConnectParams));
    if (pNetwork->handle != 0) {
        ret = QCLOUD_ERR_SUCCESS;
    }

    return ret;
}

/****** network interface ******/
int utils_net_read(Network *pNetwork, unsigned char *pMsg, size_t len, int timeout_ms, size_t *read_len)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

    int rc = 0;

    if (NULL == pNetwork->tlsConnectParams.ca_file && NULL == pNetwork->tlsConnectParams.psk) {
        rc = _read_tcp(pNetwork, pMsg, len, timeout_ms, read_len);
    } else {
        rc = _read_tls(pNetwork, pMsg, len, timeout_ms, read_len);
    }
    return rc;
}

int utils_net_write(Network *pNetwork, unsigned char *pMsg, size_t len, int timeout_ms, size_t *written_len)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

    int rc = 0;

    if (NULL == pNetwork->tlsConnectParams.ca_file && NULL == pNetwork->tlsConnectParams.psk) {
        rc = _write_tcp(pNetwork, pMsg, len, timeout_ms, written_len);
    } else {
        rc = _write_tls(pNetwork, pMsg, len, timeout_ms, written_len);
    }
    
    return rc;
}

void utils_net_disconnect(Network *pNetwork)
{
	POINTER_SANITY_CHECK_RTN(pNetwork);

    if (NULL == pNetwork->tlsConnectParams.ca_file && NULL == pNetwork->tlsConnectParams.psk) {
        _disconnect_tcp(pNetwork);
    } else {
        _disconnect_tls(pNetwork);
    }    
}

int utils_net_connect(Network *pNetwork)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);

    int rc = 0;

    if (NULL == pNetwork->tlsConnectParams.ca_file && NULL == pNetwork->tlsConnectParams.psk) {
        rc = _connect_tcp(pNetwork);
    } else {
        rc = _connect_tls(pNetwork);
    }
    
    return rc;
}

int utils_net_init(Network *pNetwork)
{
	POINTER_SANITY_CHECK(pNetwork, QCLOUD_ERR_INVAL);
	POINTER_SANITY_CHECK(pNetwork->tlsConnectParams.host, QCLOUD_ERR_INVAL);

	int rc = QCLOUD_ERR_SUCCESS;

    pNetwork->connect = utils_net_connect;
    pNetwork->read = utils_net_read;
    pNetwork->write = utils_net_write;
    pNetwork->disconnect = utils_net_disconnect;
    pNetwork->is_connected = utils_net_connected;
    pNetwork->handle = 0;

    return rc;
}

#ifdef __cplusplus
}
#endif
