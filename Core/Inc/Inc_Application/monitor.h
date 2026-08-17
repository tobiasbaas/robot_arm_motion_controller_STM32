/*
 * monitor.h
 *
 *  Created on: Mar 3, 2025
 *      Author: manue
 */

#ifndef INC_MONITOR_H_
#define INC_MONITOR_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "typedefs.h"
#include <stdbool.h>
#include <string.h>
#include <stdint.h>



#define RX_BUFFER_LENGTH 1
#define TX_BUFFER_LENGTH 200

extern uint8_t rx_buff[RX_BUFFER_LENGTH];
extern uint8_t tx_buff[TX_BUFFER_LENGTH];
extern char cmd_str[CMD_STR_LENGTH];
extern uint16_t rx_cmd_str_cnt;
extern bool new_cmd;
extern bool TX_DMA_buff_free;


extern UART_HandleTypeDef hlpuart1;
extern DMA_HandleTypeDef hdma_lpuart1_rx;
extern DMA_HandleTypeDef hdma_lpuart1_tx;




extern void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

extern void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);

extern void send_string(const char* str, bool new_line);

extern void test_monitor();
extern void run_monitor();

#ifdef __cplusplus
}
#endif

#endif /* INC_MONITOR_H_ */
