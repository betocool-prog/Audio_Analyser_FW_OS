/*
 * fw_config.h
 *
 *  Created on: 13Apr.,2021
 *      Author: betocool
 */

#ifndef INC_FW_CONFIG_H_
#define INC_FW_CONFIG_H_

#include "cmsis_os.h"

/* Task priorities */
#define DIOA_TASK_PRIO			osPriorityNormal
#define UART_TASK_PRIO			osPriorityNormal
#define RMI_TASK_PRIO			osPriorityNormal

#endif /* INC_FW_CONFIG_H_ */
