/*
 * timers.c
 *
 *  Created on: 13Sep.,2019
 *      Author: betocool
 */

#include <counters.h>

/*
 * Defines
 */

/*
 * Function declaration
 */

/*
 * Global variables
 */

/*
 * Function definition
 */
void init_counters(void){

	RCC->APB1LENR |= RCC_APB1LENR_TIM7EN;

	TIM7->CNT = 0x0;
	TIM7->PSC = 199;
	TIM7->ARR = 0xFFFF;

	TIM7->CR1 = TIM_CR1_CEN;
}
