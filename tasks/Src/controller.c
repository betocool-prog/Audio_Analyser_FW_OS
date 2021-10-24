/*
 * controller.c
 *
 *  Created on: 16Sep.,2019
 *      Author: betocool
 */

#include <api.h>
#include <controller.h>
#include <err.h>
#include <fw_config.h>
#include <FreeRTOS.h>
#include <log.h>
#include <portmacro.h>
#include <sine_lut.h>
#include <stm32h743xx.h>
#include <string.h>
#include <sys/_stdint.h>

/*
 * Function declaration
 */
static void controller_dac_task(void *pvParameters);
static void controller_adc_task(void *pvParameters);
static void controller_tcp_task(void *pvParameters);
static void controller_write_dac(void);

/* Variable declaration */
tController_Sample controller_output_buffer[OUTPUT_BUF_SIZE_STEREO_SAMPLES] __attribute__((section(".PCM1793_Output"))); /* Audio output section */
tController_Sample controller_input_buffer[INPUT_BUF_SIZE_STEREO_SAMPLES] __attribute__((section(".PCM1802_Input"))); /* Audio input section */

/* TCP Data out Stream buffer */
static uint8_t tcp_stream_buffer[TCP_STREAM_BUF_SIZE + 1] = {0};
static uint8_t tcp_out_buffer[TCP_OUT_BUF_SIZE] = {0};
static StaticStreamBuffer_t tcp_out_stream_buffer_struct = {0};
static StreamBufferHandle_t	tcp_out_stream_buffer_h = {0};

static TaskHandle_t tcp_task_handle;

static struct netconn *controller_conn, *newconn;

/* Signal variables */
static uint32_t frequency = 1000;
static float amplitude = 1.0f;
static uint32_t dac_idx = 0;
static uint32_t sine_lut_idx = 0;
static uint32_t signal_idx = 0;
static tController_Sample* buffer_ptr;
static OP_MODE op_mode = OP_MODE_FREE_RUNNING;
static FUNCTION_TYPE func_type = FUNCTION_TYPE_SINE;
static uint32_t signal_preamble = 0;
static uint32_t signal_len = 0;
static uint32_t signal_end = 0;

/* Sync variables */
bool conn_accepted = false;
uint16_t start_ticks;
uint16_t stop_ticks;
uint16_t diff_ticks;
bool got_start_ticks;
bool got_stop_ticks;

/* Global variables */
TaskHandle_t controller_adc_task_h;
TaskHandle_t controller_dac_task_h;

/* Function definitions */

/* Init function */
void controller_init(void)
{
	memset((void*)controller_output_buffer, 0, sizeof(tController_Sample) * OUTPUT_BUF_SIZE_STEREO_SAMPLES);
	xTaskCreate(controller_tcp_task, "Controller TCP Task", 512, NULL, CONTROLLER_TCP_TASK_PRIO, &tcp_task_handle);
	xTaskCreate(controller_dac_task, "Controller DAC Task", 512, NULL, CONTROLLER_DAC_TASK_PRIO, &controller_dac_task_h);
	xTaskCreate(controller_adc_task, "Controller ADC Task", 512, NULL, CONTROLLER_ADC_TASK_PRIO, &controller_adc_task_h);

	/* TCP Out stream buffer creation */
	tcp_out_stream_buffer_h = xStreamBufferCreateStatic(
			TCP_STREAM_BUF_SIZE + 1,
			TCP_OUT_BUF_SIZE,
			tcp_stream_buffer,
			&tcp_out_stream_buffer_struct
			);

	if(NULL == tcp_out_stream_buffer_h)
	{
		log_msg(LOG_ERR, "controller.c: Stream Buffer Error: %s, line: %d", __FUNCTION__, __LINE__);
	}

	start_ticks = 0;
	stop_ticks = 0;
	diff_ticks = 0;
	got_start_ticks = false;
	got_stop_ticks = false;
}

void controller_dac_task(void *pvParameters)
{
	uint32_t notification = 0;

	while(1)
	{
		notification = 0;
		xTaskNotifyWait(0x0, 0xFFFFFFFF, &notification, portMAX_DELAY);

		if(notification & DAC_HT_NOTIF)
		{
			buffer_ptr = &(controller_output_buffer[0]);
			controller_write_dac();
		}

		if(notification & DAC_TC_NOTIF)
		{
			buffer_ptr = &(controller_output_buffer[OUTPUT_BUF_SIZE_STEREO_SAMPLES/2]);
			controller_write_dac();
		}
	}
}

void controller_adc_task(void *pvParameters)
{
	uint8_t* from_ptr = NULL;
	uint32_t notification = 0;
	uint32_t data_size = 0;
	uint32_t prev_notif = 0;

	while(1)
	{
		notification = 0;
		/* Wait for ADC to signal arrival of data */
		xTaskNotifyWait(0x0, 0xFFFFFFFF, &notification, portMAX_DELAY);

		if(notification & ADC_HT_NOTIF)
		{
			if(conn_accepted && got_start_ticks)
			{
				from_ptr = (uint8_t*)controller_input_buffer;
				data_size = xStreamBufferSend(tcp_out_stream_buffer_h,
	                    (const void *)from_ptr,
						INPUT_BUF_SIZE_STEREO_SAMPLES * 4,
	                    0);

				if (data_size != (INPUT_BUF_SIZE_STEREO_SAMPLES * 4))
				{
					log_msg(LOG_ERR, "controller.c: Error writing to buffer: %s, line: %d, size: %lu", __FUNCTION__, __LINE__, data_size);
				}
			}

			if(prev_notif == notification)
			{
				log_msg(LOG_ERR, "controller.c: Notification equals! Prev: %lu, Curr: %lu", prev_notif, notification);
			}

			prev_notif = notification;
		}

		if(notification & ADC_TC_NOTIF)
		{
			if(conn_accepted && got_start_ticks)
			{
				from_ptr = (uint8_t*)&controller_input_buffer[INPUT_BUF_SIZE_STEREO_SAMPLES / 2];
				data_size = xStreamBufferSend(tcp_out_stream_buffer_h,
	                    (const void *)from_ptr,
						INPUT_BUF_SIZE_STEREO_SAMPLES * 4,
	                    0);

				if (data_size != (INPUT_BUF_SIZE_STEREO_SAMPLES * 4))
				{
					log_msg(LOG_ERR, "controller.c: Error writing to buffer: %s, line: %d, size: %lu", __FUNCTION__, __LINE__, data_size);
				}
			}

			if(prev_notif == notification)
			{
				log_msg(LOG_ERR, "controller.c: Notification equals! Prev: %lu, Curr: %lu", prev_notif, notification);
			}

			prev_notif = notification;
		}

		if(notification & ADC_ERR_NOTIF)
		{
			log_msg(LOG_ERR, "DMA RX Error");
		}
	}
}

static void controller_tcp_task(void *pvParameters)
{
	err_t err, accept_err;
	uint32_t bytes_written = 0;
	uint32_t bytes_read = 0;
	bool do_loop = true;

	/* Create a new TCP connection handle */
	controller_conn = netconn_new(NETCONN_TCP);

	if (controller_conn!= NULL)
	{
		/* Bind to port 80 (HTTP) with default IP address */
		err = netconn_bind(controller_conn, NULL, 8888);

		if (err == ERR_OK)
		{
			/* Put the connection into LISTEN state */
			netconn_listen(controller_conn);
			log_msg(LOG_INFO, "Listen OK: %s, line: %d", __FUNCTION__, __LINE__);
			while(1)
			{
				/* accept any incoming TCP connection */
				accept_err = netconn_accept(controller_conn, &newconn);
				if(accept_err == ERR_OK)
				{
					log_msg(LOG_INFO, "Accept OK: %s, line: %d", __FUNCTION__, __LINE__);
					start_ticks = 0;
					stop_ticks = 0;
					diff_ticks = 0;
					got_start_ticks = false;
					got_stop_ticks = false;
					conn_accepted = true;

					/* Write TCP data */
					do_loop = true;
					while(do_loop)
					{

						bytes_read = xStreamBufferReceive(
								tcp_out_stream_buffer_h,
								(void *)tcp_out_buffer,
								TCP_OUT_BUF_SIZE,
								1
								);

						if(bytes_read != 0)
						{
							err = netconn_write(newconn, tcp_out_buffer, bytes_read, NETCONN_COPY);
							if(ERR_OK == err)
							{
								bytes_written += bytes_read;
							}
							else
							{
								do_loop = false;
								log_msg(LOG_ERR, "Write NOK: %s, line: %d", __FUNCTION__, __LINE__);
							}
						}
					}

					conn_accepted = false;
					netconn_close(newconn);
					netconn_delete(newconn);
					xStreamBufferReset(tcp_out_stream_buffer_h);
					log_msg(LOG_INFO, "Bytes Written: %lu", bytes_written);
					bytes_written = 0;
				}
				else
				{
					log_msg(LOG_ERR,"Accept NOK: %s, line: %d", __FUNCTION__, __LINE__);
				}
			}
		}
		else
		{
			log_msg(LOG_ERR, "Bind NOK: %s, line: %d", __FUNCTION__, __LINE__);
		}
	}
	vTaskDelete(NULL);
}


static void controller_write_dac(void)
{
	static bool started = false;
	uint32_t val = 0x7FFFFF;

	if(OP_MODE_FREE_RUNNING == op_mode)
	{
		for(dac_idx = 0; dac_idx < OUTPUT_BUF_SIZE_STEREO_SAMPLES / 2; dac_idx++)
		{
			buffer_ptr[dac_idx].left = (int32_t)((float)sine_lut[sine_lut_idx] * amplitude);
			buffer_ptr[dac_idx].right = buffer_ptr[dac_idx].left;

			sine_lut_idx += frequency;

			if(sine_lut_idx >= FS)
			{
				sine_lut_idx = sine_lut_idx - FS;
			}
		}
	}
	else if(OP_MODE_SYNC == op_mode)
	{

		if(FUNCTION_TYPE_SINE == func_type)
		{
			if(conn_accepted && got_stop_ticks)
			{
				for(dac_idx = 0; dac_idx < OUTPUT_BUF_SIZE_STEREO_SAMPLES / 2; dac_idx++)
				{
					if(signal_idx < signal_preamble)
					{
						buffer_ptr[dac_idx].left = 0;
						buffer_ptr[dac_idx].right = 0;
					}
					else if(signal_idx >= (signal_preamble + signal_len))
					{
						buffer_ptr[dac_idx].left = 0;
						buffer_ptr[dac_idx].right = 0;
					}
					else
					{
						buffer_ptr[dac_idx].left = (int32_t)((float)sine_lut[sine_lut_idx] * amplitude);
						buffer_ptr[dac_idx].right = buffer_ptr[dac_idx].left;
						sine_lut_idx += frequency;
					}

					if(sine_lut_idx >= FS)
					{
						sine_lut_idx = sine_lut_idx - FS;
					}
					signal_idx++;
				}
			}
			else
			{
				for(dac_idx = 0; dac_idx < OUTPUT_BUF_SIZE_STEREO_SAMPLES / 2; dac_idx++)
				{
					buffer_ptr[dac_idx].left = 0;
					buffer_ptr[dac_idx].right = 0;
				}

				sine_lut_idx = 0;
				signal_idx = 0;
			}
		}
		else if(FUNCTION_TYPE_IMPULSE == func_type)
		{
			if(conn_accepted && got_stop_ticks)
			{
				for(dac_idx = 0; dac_idx < OUTPUT_BUF_SIZE_STEREO_SAMPLES / 2; dac_idx++)
				{
					if(0 == sine_lut_idx)
					{
						if(!started)
						{
							started = true;
							val = 0x7FFFFFFF;
						}
						sine_lut_idx = 1;
					}
					else
					{
						val = 0;
					}

					buffer_ptr[dac_idx].left = val;
					buffer_ptr[dac_idx].right = val;
				}
			}
			else
			{
				for(dac_idx = 0; dac_idx < OUTPUT_BUF_SIZE_STEREO_SAMPLES / 2; dac_idx++)
				{
					buffer_ptr[dac_idx].left = 0;
					buffer_ptr[dac_idx].right = 0;
				}
				started = false;
				sine_lut_idx = 0;
			}

		}
	}
}

void controller_set_freq(uint32_t freq)
{
	frequency = freq;
}

void controller_set_amp(float amp)
{
	amplitude = amp;
}

void controller_set_op_mode(OP_MODE mode)
{
	op_mode = mode;
}

void controller_set_func(FUNCTION_TYPE func)
{
	func_type = func;
}

void controller_set_signal_preamble(uint32_t val)
{
	signal_preamble = val;
}

void controller_set_signal_len(uint32_t val)
{
	signal_len = val;
}

void controller_set_signal_end(uint32_t val)
{
	signal_end = val;
}


uint32_t controller_get_freq(void)
{
	return frequency;
}

float controller_get_amp(void)
{
	return amplitude;
}

OP_MODE controller_get_op_mode(void)
{
	return op_mode;
}

FUNCTION_TYPE controller_get_func(void)
{
	return func_type;
}

uint32_t controller_get_delay(void)
{
	return diff_ticks;
}
