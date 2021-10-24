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
#include "rmi.h"

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
void controller_set_op_mode(OP_MODE mode);
void controller_set_func(FUNCTION_TYPE func);
void controller_set_signal_preamble(uint32_t val);
void controller_set_signal_len(uint32_t val);
void controller_set_signal_end(uint32_t val);
uint32_t controller_get_freq(void);
float controller_get_amp(void);
OP_MODE controller_get_op_mode(void);
FUNCTION_TYPE controller_get_func(void);
uint32_t controller_get_delay(void);

/* Extern variables */
extern tController_Sample controller_output_buffer[OUTPUT_BUF_SIZE_STEREO_SAMPLES];
extern tController_Sample controller_input_buffer[INPUT_BUF_SIZE_STEREO_SAMPLES];
extern TaskHandle_t controller_adc_task_h;
extern TaskHandle_t controller_dac_task_h;

extern bool conn_accepted;
extern uint16_t start_ticks;
extern uint16_t stop_ticks;
extern bool got_start_ticks;
extern uint16_t diff_ticks;
extern bool got_stop_ticks;

#endif /* CONTROLLER_H_ */
