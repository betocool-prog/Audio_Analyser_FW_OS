/*
 * dioa.c
 *
 *  Created on: 29Jan.,2021
 *      Author: alberto.fahrenkrog
 */

#include "dioa.h"
#include "uart.h"
#include "fw_config.h"

/* Private variables */

/* Private functions */
void red_led_task(void *pvParameters);
void blue_led_task(void *pvParameters);

void dioa_init(void)
{
	/* Set the priority just higher than the idle task */
	xTaskCreate(red_led_task, "Red Task", 32, NULL, DIOA_TASK_PRIO, NULL);
	xTaskCreate(blue_led_task, "Blue Task", 32, NULL, DIOA_TASK_PRIO, NULL);
}

void red_led_task(void *pvParameters)
{
	const TickType_t xDelay = pdMS_TO_TICKS(250);

	while(1)
	{
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
		osDelay(xDelay);
	}
}

void blue_led_task(void *pvParameters)
{
	const TickType_t xDelay = pdMS_TO_TICKS(170);

	while(1)
	{
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
		osDelay(xDelay);
	}
}
