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
#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"

#ifdef AT_TCP_ENABLED

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "at_client.h"
#include "stm32l4xx_hal.h"
#include "utils_ringbuff.h"

#define HAL_AT_UART_IRQHandler USART1_IRQHandler
extern UART_HandleTypeDef  huart1;
static UART_HandleTypeDef *pAtUart = &huart1;

extern void AT_Uart_Init(void);
extern void at_client_uart_rx_isr_cb(uint8_t *pdata, uint8_t len);

#include "board.h"
/**
 * @brief This function handles AT UART global interrupt,push recv char to ringbuff.
 */
void HAL_AT_UART_IRQHandler(void)
{
    uint8_t ch;
    if (__HAL_UART_GET_FLAG(pAtUart, UART_FLAG_RXNE) == SET) {
        ch = (uint8_t)READ_REG(pAtUart->Instance->RDR) & 0xFF;
        /*this callback for at_client*/
        at_client_uart_rx_isr_cb(&ch, 1);
        HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    }
    __HAL_UART_CLEAR_PEFLAG(pAtUart);
}

/**
 *pdata: pointer of data for send
 *len:  len of data to be sent
 *return: the len of data send success
 * @brief hal api for at data send
 */
int HAL_AT_Uart_Send(void *data, uint32_t size)
{
    if (HAL_OK == HAL_UART_Transmit(pAtUart, data, size, 0xFFFF)) {
        return size;
    } else {
        return 0;
    }
}

int HAL_AT_Uart_Init(void)
{
    AT_Uart_Init();
    return QCLOUD_RET_SUCCESS;
}

int HAL_AT_Uart_Deinit(void)
{
    return QCLOUD_RET_SUCCESS;
}
#endif
