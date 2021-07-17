/*
 * debug.h
 *
 *  Created on: 29Apr.,2021
 *      Author: alberto.fahrenkrog
 */

#ifndef INC_LOGS_H_
#define INC_LOGS_H_

#include "fw_config.h"
#include "cmsis_os.h"
#include "task.h"
#include "stream_buffer.h"


#define LOG_MSG_IN_LEN				256
#define LOG_MSG_CHUNK				1
#define LOG_MSG_BUF_LEN				2048

typedef enum {
	LOG_INFO,
	LOG_WARN,
	LOG_ERR
} eLogType;

/* Public functions */
void log_init(void);
void log_deinit(void);
//uint32_t log_msg(const char* text);
uint32_t log_msg(eLogType type, const char *format, ...);

/* Global variables */
extern TaskHandle_t log_task_handle;

#endif /* INC_LOGS_H_ */
