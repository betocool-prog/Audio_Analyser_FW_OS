/*
 * controller.h
 *
 *  Created on: 16Sep.,2019
 *      Author: betocool
 */

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

/* Includes */
#include <counters.h>
#include <stdbool.h>
#include <math.h>
#include "stm32h7xx_hal.h"
#include "analyser.pb.h"
#include "uart.h"

/* Defines */
#define FS									96000.0

#define OUTPUT_BUF_SIZE_STEREO_SAMPLES		256
#define INPUT_BUF_SIZE_STEREO_SAMPLES		64 //364
#define ADC_PACKET_SIZE						1024

#define controller_mode_INIT				MODE_PULSED
#define controller_pulsed_INIT				{FUNCTION_TYPE_SINE, 0, 4608, 4608}
#define controller_sine_params_INIT			{0, 984.375f, 1.0f}

typedef enum
{
	DMA_HALF_XFER = 0,
	DMA_XFER_CPLT = 1
} eDMAXfer_enum;

typedef struct
{
	int32_t	left;
	int32_t right;
}tController_Sample;

typedef struct
{
	double k;
	double frequency;
	double amplitude;
}tController_sine_params;

typedef struct
{
	FUNCTION_TYPE function_type;
	uint32_t start;
	uint32_t stop;
	uint32_t total_len;
}tController_pulsed;

/* Function declaration */

/* Init function */
void controller_init(void);

/* Extern variables */
extern tController_Sample controller_output_buffer[OUTPUT_BUF_SIZE_STEREO_SAMPLES];
extern tController_Sample controller_input_buffer[INPUT_BUF_SIZE_STEREO_SAMPLES];
extern TaskHandle_t controller_adc_task_h;
extern TaskHandle_t controller_dac_task_h;
extern eDMAXfer_enum adc_xfer_flag;
extern eDMAXfer_enum dac_xfer_flag;

#endif /* CONTROLLER_H_ */
