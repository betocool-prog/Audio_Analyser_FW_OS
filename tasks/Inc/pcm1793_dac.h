/*
 * pcm1793_dac.h
 *
 *  Created on: 28Jul.,2019
 *      Author: betocool
 */

#ifndef PCM1793_DAC_H_
#define PCM1793_DAC_H_

#include <stdbool.h>
#include "stm32h7xx_hal.h"
#include "uart.h"

#define tPCM1793_DAC_Sample_Init	{0, 0}

void pcm1793_dac_init(void);

#endif /* PCM1793_DAC_H_ */
