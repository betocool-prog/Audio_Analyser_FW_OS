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

/*
 * Function declaration
 */
static void controller_dac_task(void *pvParameters);
static void controller_adc_task(void *pvParameters);
static void controller_tcp_task(void *pvParameters);

/* Variable declaration */
tController_Sample controller_output_buffer[OUTPUT_BUF_SIZE_STEREO_SAMPLES] __attribute__((section(".PCM1793_Output"))); /* Audio output section */
tController_Sample controller_input_buffer[INPUT_BUF_SIZE_STEREO_SAMPLES] __attribute__((section(".PCM1802_Input"))); /* Audio input section */

static TaskHandle_t tcp_task_handle;
static uint8_t tcp_out_buffer[INPUT_BUF_SIZE_STEREO_SAMPLES * 2 * 8] = {0};
static uint8_t *tcp_out_buffer_ptr = NULL;

static struct netconn *controller_conn, *newconn;

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
}

void controller_dac_task(void *pvParameters)
{
	uint32_t notification;
	static float frequency = 1000.0f;
	static float amplitude = 1.0f;
	static float k = 0.0;
	uint32_t idx = 0;

	tController_Sample* buffer_ptr;

	while(1)
	{
		xTaskNotifyWait(0x0, 0xFFFFFFFF, &notification, portMAX_DELAY);

		if(notification & DAC_HT_NOTIF)
		{

			buffer_ptr = &(controller_output_buffer[0]);
			for(idx = 0; idx < OUTPUT_BUF_SIZE_STEREO_SAMPLES / 2; idx++)
			{
				buffer_ptr[idx].left = (int32_t)(sin(2 * M_PI * frequency * k / FS) * (double)((int32_t)0x7FFFFFFF) * amplitude);
				buffer_ptr[idx].right = buffer_ptr[idx].left;
				k++;
			}
		}

		if(notification & DAC_TC_NOTIF)
		{
			buffer_ptr = &(controller_output_buffer[OUTPUT_BUF_SIZE_STEREO_SAMPLES/2]);
			for(idx = 0; idx < OUTPUT_BUF_SIZE_STEREO_SAMPLES / 2; idx++)
			{
				buffer_ptr[idx].left = (int32_t)(sin(2 * M_PI * frequency * k / FS) * (double)((int32_t)0x7FFFFFFF) * amplitude);
				buffer_ptr[idx].right = buffer_ptr[idx].left;
				k++;
			}
		}
	}
}

void controller_adc_task(void *pvParameters)
{
	uint8_t* from_ptr = NULL;
	uint8_t* to_ptr = NULL;
	uint32_t idx = 0;
	uint32_t notification;

	while(1)
	{
		/* Wait for ADC to signal arrival of data */
		xTaskNotifyWait(0x0, 0xFFFFFFFF, &notification, portMAX_DELAY);

		if(notification & ADC_HT_NOTIF)
		{
			from_ptr = (uint8_t*)controller_input_buffer;
		}

		if(notification & ADC_HT_NOTIF)
		{
			from_ptr = (uint8_t*)&controller_input_buffer[INPUT_BUF_SIZE_STEREO_SAMPLES / 2];
		}

		to_ptr = &tcp_out_buffer[idx];
		memcpy((void*)to_ptr, (void*)from_ptr, INPUT_BUF_SIZE_STEREO_SAMPLES * 4);

		idx += (INPUT_BUF_SIZE_STEREO_SAMPLES * 4);

		if (INPUT_BUF_SIZE_STEREO_SAMPLES * 8 == idx)
		{
			tcp_out_buffer_ptr = tcp_out_buffer;
			xTaskNotifyGive(tcp_task_handle);
		}

		else if(INPUT_BUF_SIZE_STEREO_SAMPLES * 8 * 2 == idx)
		{
			tcp_out_buffer_ptr = &tcp_out_buffer[INPUT_BUF_SIZE_STEREO_SAMPLES * 8];
			idx = 0;
			xTaskNotifyGive(tcp_task_handle);
		}
	}
}

static void controller_tcp_task(void *pvParameters)
{
	err_t err, accept_err;
	uint32_t bytes_written = 0;

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
			log_msg(LOG_INFO, "Listen OK: %s, line: %d \n\r", __FUNCTION__, __LINE__);
			while(1)
			{
				/* accept any incoming TCP connection */
				accept_err = netconn_accept(controller_conn, &newconn);
				if(accept_err == ERR_OK)
				{
					log_msg(LOG_INFO, "Accept OK: %s, line: %d \n\r", __FUNCTION__, __LINE__);
					/* Write TCP data */
//					while(8 * 1024 > bytes_written)
					err = ERR_OK;
					while(ERR_OK != err)
					{
						ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
						err = netconn_write(newconn, tcp_out_buffer_ptr, INPUT_BUF_SIZE_STEREO_SAMPLES * 8, NETCONN_NOCOPY);
						if(ERR_OK != err)
						{
							log_msg(LOG_ERR, "Write NOK: %s, line: %d \n\r", __FUNCTION__, __LINE__);
							bytes_written = 8 * 1024;
						}
						else
						{
							bytes_written += INPUT_BUF_SIZE_STEREO_SAMPLES * 8;
						}
					}

					bytes_written = 0;
					netconn_close(newconn);
					netconn_delete(newconn);
				}
				else
				{
					log_msg(LOG_ERR,"Accept NOK: %s, line: %d \n\r", __FUNCTION__, __LINE__);
				}
			}
		}
		else
		{
			log_msg(LOG_ERR, "Bind NOK: %s, line: %d \n\r", __FUNCTION__, __LINE__);
		}
	}
	vTaskDelete(NULL);
}
