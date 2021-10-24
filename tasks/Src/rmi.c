/*
 * rmi.c
 *
 *  Created on: 4Aug.,2019
 *      Author: betocool
 */

/*
 * Includes
 */
#include <controller.h>
#include "rmi.h"
#include "fw_config.h"
#include "log.h"

#include <stdbool.h>
#include "uart.h"
#include "cmsis_os.h"
#include "api.h"
/*
 * Global variables
 */

/*
 * Local variables
 */
TaskHandle_t process_task_h;
TaskHandle_t send_task_h;

/* nanoPB Message variables */
static Service service_msg_in= Service_init_zero;
static Service service_msg_out = Service_init_zero;

/* nanoPB Stream variables */
pb_istream_t input_stream = {NULL, NULL, SIZE_MAX};
pb_ostream_t output_stream = {NULL, NULL, SIZE_MAX, 0};

static uint16_t message_length = 0;
static pb_byte_t pb_input_msg_buffer[NANOPB_BUFFER_LEN];
static pb_byte_t pb_output_msg_buffer[NANOPB_BUFFER_LEN];

/* Network variables */
static struct netconn *rmi_conn;
static struct netbuf *in_buf, *out_buf;
static ip_addr_t addr = {0};
static uint16_t port = 0;

/* Auxiliary variables */
static uint32_t aux_uint32;
static bool aux_bool;
static char *temp_str;
static uint32_t strLen;

/*
 * Function declaration
 */
static void rmi_task_rcv(void *pvParameters);
static void rmi_task_process(void *pvParameters);
static void rmi_task_send(void *pvParameters);

//void rmi_process_data_request(void);
//void rmi_process_dac_pulse_request(bool dac_pulse);
//void rmi_process_delay_request(void);
/*
 * Function definition
 */

/**
 * init the Remote Machine Interface (RMI)
 * The RMI receives PB messages over UDP and
 * is used to set or check variables, set or get commands
 * or set or get operation mode
 *
 */
void rmi_init(void)
{
	aux_bool=false;
	strLen = 0;
	temp_str=NULL;
	aux_uint32 = 0;

	message_length = 0;

	xTaskCreate(rmi_task_rcv, "RMI Rx Task", 256, NULL, RMI_TASK_PRIO, NULL);
	xTaskCreate(rmi_task_process, "RMI Process Task", 256, NULL, RMI_TASK_PRIO, &process_task_h);
	xTaskCreate(rmi_task_send, "RMI Tx Task", 256, NULL, RMI_TASK_PRIO, &send_task_h);
}

static void rmi_task_rcv(void *pvParameters)
{
	err_t err;
	void *data;

	rmi_conn = netconn_new(NETCONN_UDP);
	if (NULL == rmi_conn)
	{
		log_msg(LOG_ERR, "New Conn NOK: %s, line: %d\n\r", __FUNCTION__, __LINE__);
	}
	else
	{
		log_msg(LOG_INFO, "New Conn OK: %s, line: %d\n\r", __FUNCTION__, __LINE__);
		err = netconn_bind(rmi_conn, IP_ADDR_ANY, 5003);
		if(ERR_OK == err)
		{
			while(1)
			{
				err = netconn_recv(rmi_conn, &in_buf);
				if(ERR_OK == err)
				{
					err = netbuf_data(in_buf, &data, &message_length);
					addr = in_buf->addr;
					port = in_buf->port;
					if(ERR_OK == err)
					{
						/* Notify the processing task */
						memcpy(pb_input_msg_buffer, data, message_length);
						xTaskNotifyGive(process_task_h);
					}
				}
				else{
					log_msg(LOG_ERR, "Rx Error: %d, %s, line: %dn\r", err, __FUNCTION__, __LINE__);
				}
				netbuf_delete(in_buf);
			}
		}
		else
		{
			log_msg(LOG_ERR, "Bind NOK: %s, line: %d\n\r", __FUNCTION__, __LINE__);
		}
	}

	vTaskDelete(NULL);
}

void rmi_task_process(void *pvParameters)
{
	while(1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		service_msg_out = (Service)Service_init_zero;

		input_stream = pb_istream_from_buffer(pb_input_msg_buffer, message_length);

		if (!pb_decode(&input_stream, Service_fields, &service_msg_in)){
			log_msg(LOG_ERR, "Message NOK decoded: %s, line: %d\n\r", __FUNCTION__, __LINE__);
		}
		else
		{
			service_msg_out.xfer_id = service_msg_in.xfer_id;
			/* Start processing the message */
			if(MESSAGE_TYPE_SET_MESSAGE == service_msg_in.message_type)
			{
				if(service_msg_in.has_signalconfig)
				{
					if(service_msg_in.signalconfig.has_frequency)
					{
						controller_set_freq(service_msg_in.signalconfig.frequency);
//						log_msg(LOG_INFO, "Freq: %d", service_msg_in.signalconfig.frequency);
					}

					if(service_msg_in.signalconfig.has_amplitude)
					{
						controller_set_amp(service_msg_in.signalconfig.amplitude);
//						log_msg(LOG_INFO, "Amp: %d", (uint32_t)service_msg_in.signalconfig.amplitude);
					}

					if(service_msg_in.signalconfig.has_op_mode)
					{
						controller_set_op_mode(service_msg_in.signalconfig.op_mode);
//						log_msg(LOG_INFO, "Op Mode: %d", service_msg_in.signalconfig.op_mode);
					}

					if(service_msg_in.signalconfig.has_function)
					{
						controller_set_func(service_msg_in.signalconfig.function);
//						log_msg(LOG_INFO, "Function: %d", service_msg_in.signalconfig.function);
					}

					if(service_msg_in.signalconfig.has_signal_preamble)
					{
						controller_set_signal_preamble(service_msg_in.signalconfig.signal_preamble);
//						log_msg(LOG_INFO, "Pre: %d", service_msg_in.signalconfig.signal_preamble);
					}

					if(service_msg_in.signalconfig.has_signal_len)
					{
						controller_set_signal_len(service_msg_in.signalconfig.signal_len);
//						log_msg(LOG_INFO, "Len: %d", service_msg_in.signalconfig.signal_len);
					}

					if(service_msg_in.signalconfig.has_signal_end)
					{
						controller_set_signal_end(service_msg_in.signalconfig.signal_end);
//						log_msg(LOG_INFO, "End: %d", service_msg_in.signalconfig.signal_end);
					}
				}
			}
			else if(MESSAGE_TYPE_GET_MESSAGE == service_msg_in.message_type)
			{
				if(service_msg_in.has_signalconfig)
				{
					/* Signal config message */
					service_msg_out.has_signalconfig = true;
					service_msg_out.signalconfig.has_amplitude = true;
					service_msg_out.signalconfig.amplitude = controller_get_amp();
					service_msg_out.signalconfig.has_frequency = true;
					service_msg_out.signalconfig.frequency = controller_get_freq();
					service_msg_out.signalconfig.has_op_mode = true;
					service_msg_out.signalconfig.op_mode = controller_get_op_mode();
					service_msg_out.signalconfig.has_function = true;
					service_msg_out.signalconfig.function = controller_get_func();
					service_msg_out.signalconfig.has_delay = true;
					service_msg_out.signalconfig.delay = controller_get_delay();
				}
			}
			else if(MESSAGE_TYPE_ACTION_MESSAGE == service_msg_in.message_type)
			{
				if(service_msg_in.has_command)
				{
					if(service_msg_in.command.has_reset)
					{
						if(service_msg_in.command.reset)
						{
							NVIC_SystemReset();
						}
					}
				}
			}
			xTaskNotifyGive(send_task_h);
		}
	}
}

void rmi_task_send(void *pvParameters)
{
	void *data;

	while(1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		output_stream =  pb_ostream_from_buffer(pb_output_msg_buffer, NANOPB_BUFFER_LEN);

		if (!pb_encode(&output_stream, Service_fields, &service_msg_out))
		{
			log_msg(LOG_ERR, "Output message NOT encoded!: %s, line %d\n\r", __FUNCTION__, __LINE__);
		}
		else
		{
			out_buf = netbuf_new();
			if(NULL != out_buf)
			{
				data = netbuf_alloc(out_buf, output_stream.bytes_written);
				if(NULL != data){
					memcpy(data, pb_output_msg_buffer, output_stream.bytes_written);
					out_buf->addr = addr;
					out_buf->port = port;
					netconn_send(rmi_conn, out_buf);
					netbuf_delete(out_buf);
				}
				else
				{
					log_msg(LOG_ERR, "Could not allocate data: %s, line %d\n\r", __FUNCTION__, __LINE__);
				}
			}
		}
	}
}
