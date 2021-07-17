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

extern UART_HandleTypeDef huart3;

void UART_init(void);


#endif /* UART_H_ */
