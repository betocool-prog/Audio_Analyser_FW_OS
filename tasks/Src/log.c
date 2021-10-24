/*
 * debug.c
 *
 *  Created on: 29Apr.,2021
 *      Author: alberto.fahrenkrog
 */

#include <log.h>
#include "uart.h"
#include "stdio.h"
#include "stdarg.h"

/* Private functions */
static void log_task(void* pvParameters);
static uint32_t log_send_to_stream(uint8_t* msg, uint32_t len);

/* Public variables */
TaskHandle_t log_task_handle;
StreamBufferHandle_t log_msg_handle;

/* Private Variables */
static uint8_t log_msg_in_buf[LOG_MSG_IN_LEN] = {0};
static uint8_t log_msg_out_buf[LOG_MSG_CHUNK] = {0};
static uint8_t log_msg_buf[LOG_MSG_BUF_LEN + 1] = {0};
static uint8_t temp_buf[128];
static StaticStreamBuffer_t log_stream_buffer_t = {0};

static va_list args;

void log_init(void)
{
	log_stream_buffer_t = (StaticStreamBuffer_t){0};
	log_msg_handle = xStreamBufferCreateStatic(LOG_MSG_BUF_LEN, LOG_MSG_CHUNK, log_msg_buf, &log_stream_buffer_t);
	xTaskCreate(log_task, "Logging Task", 384, NULL, LOGGING_TASK_PRIO, &log_task_handle);
}

void log_deinit(void)
{
	vTaskDelete(log_task_handle);
}

uint32_t log_msg(eLogType type, const char *format, ...)
{
	uint32_t bytes_written = 0;
	uint32_t bytes_sent = 0;

	vPortEnterCritical();

	va_start(args, format);

	vsnprintf((char*)temp_buf, 128, format, args);

	va_end(args);

	if(LOG_INFO == type)
	{
		bytes_written = snprintf((char*)log_msg_in_buf, LOG_MSG_IN_LEN,
				"[INFO] %s\n\r", temp_buf);
		bytes_sent = log_send_to_stream(log_msg_in_buf, bytes_written);
	}
	else if(LOG_WARN == type)
	{
		bytes_written = snprintf((char*)log_msg_in_buf, LOG_MSG_IN_LEN,
				"[WARN] %s\n\r", temp_buf);
		bytes_sent = log_send_to_stream(log_msg_in_buf, bytes_written);
	}
	else if(LOG_ERR == type)
	{
		bytes_written = snprintf((char*)log_msg_in_buf, LOG_MSG_IN_LEN,
				"[ERR] %s\n\r", temp_buf);
		bytes_sent = log_send_to_stream(log_msg_in_buf, bytes_written);
	}

	vPortExitCritical();

	return bytes_sent;
}

static uint32_t log_send_to_stream(uint8_t* msg, uint32_t len)
{
	uint32_t bytes_sent = 0;
	bytes_sent = xStreamBufferSend(log_msg_handle, msg, len, 0);
	return bytes_sent;
}

static void log_task(void* pvParameters)
{
	uint32_t bytes_read = 0;

	while(1)
	{
		bytes_read = xStreamBufferReceive(log_msg_handle, log_msg_out_buf, LOG_MSG_CHUNK, portMAX_DELAY);
		HAL_UART_Transmit(&huart3, log_msg_out_buf, bytes_read, 10);
	}
}
