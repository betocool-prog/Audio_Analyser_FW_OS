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

#include "cmsis_os.h"
#include "task.h"
#include "stream_buffer.h"

/* Defines */
#define FS									96000.0

#define OUTPUT_BUF_SIZE_STEREO_SAMPLES		256

#define INPUT_BUF_SIZE_STEREO_SAMPLES		384										///<  Stereo Samples
#define INPUT_BUF_SIZE_BYTES				INPUT_BUF_SIZE_STEREO_SAMPLES * 8
//#define	TCP_OUT_BLOCKS						16
//#define	TCP_OUT_BLOCK_SIZE					INPUT_BUF_SIZE_BYTES / 2				///< 512 Bytes
#define TCP_OUT_BUF_SIZE					1024
#define TCP_STREAM_BUF_SIZE					32768

#define controller_mode_INIT				MODE_PULSED
#define controller_pulsed_INIT				{FUNCTION_TYPE_SINE, 0, 4608, 4608}
#define controller_sine_params_INIT			{0, 984.375f, 1.0f}

typedef enum
{
	NONE_NOTIF = 	0x0000,
	DAC_HT_NOTIF = 	0x0001,
	DAC_TC_NOTIF = 	0x0002,
	ADC_HT_NOTIF = 	0x0004,
	ADC_TC_NOTIF = 	0x0008,
	ADC_ERR_NOTIF =	0x0010
} eNotif_enum;

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

/* Extern function */
void controller_init(void);
void controller_set_freq(uint32_t freq);
void controller_set_amp(float amp);

/* Extern variables */
extern tController_Sample controller_output_buffer[OUTPUT_BUF_SIZE_STEREO_SAMPLES];
extern tController_Sample controller_input_buffer[INPUT_BUF_SIZE_STEREO_SAMPLES];
extern TaskHandle_t controller_adc_task_h;
extern TaskHandle_t controller_dac_task_h;

#endif /* CONTROLLER_H_ */
