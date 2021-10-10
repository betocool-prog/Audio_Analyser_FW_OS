/*
 * controller.c
 *
 *  Created on: 16Sep.,2019
 *      Author: betocool
 */

#include <controller.h>
#include "fw_config.h"

#include <stdbool.h>
#include "log.h"
#include "cmsis_os.h"
#include "api.h"
#include "sine_lut.h"

/*
 * Function declaration
 */
static void controller_dac_task(void *pvParameters);
static void controller_adc_task(void *pvParameters);
static void controller_tcp_task(void *pvParameters);

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
static bool conn_accepted = false;

/* Signal variables */
static uint32_t frequency = 1000;
static float amplitude = 1.0f;

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
}

void controller_dac_task(void *pvParameters)
{
	uint32_t notification = 0;
	uint32_t idx = 0;
	uint32_t k = 0;

	tController_Sample* buffer_ptr;

	while(1)
	{
		notification = 0;
		xTaskNotifyWait(0x0, 0xFFFFFFFF, &notification, portMAX_DELAY);

		if(notification & DAC_HT_NOTIF)
		{

			buffer_ptr = &(controller_output_buffer[0]);
			for(idx = 0; idx < OUTPUT_BUF_SIZE_STEREO_SAMPLES / 2; idx++)
			{
				buffer_ptr[idx].left = (int32_t)((float)sine_lut[k] * amplitude);
				buffer_ptr[idx].right = buffer_ptr[idx].left;

				k += frequency;

				if(k >= FS)
				{
					k = k - FS;
				}
			}
		}

		if(notification & DAC_TC_NOTIF)
		{
			buffer_ptr = &(controller_output_buffer[OUTPUT_BUF_SIZE_STEREO_SAMPLES/2]);
			for(idx = 0; idx < OUTPUT_BUF_SIZE_STEREO_SAMPLES / 2; idx++)
			{
				buffer_ptr[idx].left = (int32_t)((float)sine_lut[k] * amplitude);
				buffer_ptr[idx].right = buffer_ptr[idx].left;

				k += frequency;

				if(k >= FS)
				{
					k = k - FS;
				}
			}
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
			if(conn_accepted)
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
		}

		if(notification & ADC_TC_NOTIF)
		{
			if(conn_accepted)
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
		}

		if(notification & ADC_ERR_NOTIF)
		{
			log_msg(LOG_ERR, "DMA RX Error");
		}

		if(conn_accepted)
		{
			if(prev_notif == notification)
			{
				log_msg(LOG_ERR, "controller.c: Notification equals! Prev: %lu, Curr: %lu", prev_notif, notification);
			}

			prev_notif = notification;
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

void controller_set_freq(uint32_t freq)
{
	frequency = freq;
}

void controller_set_amp(float amp)
{
	amplitude = amp;
}
