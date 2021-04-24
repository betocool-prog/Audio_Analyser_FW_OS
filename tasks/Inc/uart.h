/*
 * usart.h
 *
 *  Created on: May 24, 2018
 *      Author: betocool
 */

#ifndef UART_H_
#define UART_H_

#include <string.h>
#include "stm32h7xx_hal.h"

#include "main.h"
#include "cmsis_os.h"
//#include "task.h"
#include "queue.h"

#define UART_BUF_SIZE		128

typedef struct{
	uint8_t *buf_ptr;
	uint32_t buf_size;
} sUartQueueElement;

extern char usart_buf[UART_BUF_SIZE];
extern QueueHandle_t uart_queue;

void UART_init(void);
void UART_MSG_Send(const char* string);

#define UART_PRINTF(...) snprintf(usart_buf, UART_BUF_SIZE, ##__VA_ARGS__); \
						UART_MSG_Send((const char*)usart_buf)

#endif /* UART_H_ */
