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
					}

					if(service_msg_in.signalconfig.has_amplitude)
					{
						controller_set_amp(service_msg_in.signalconfig.amplitude);
					}
				}
			}
			else if(MESSAGE_TYPE_GET_MESSAGE == service_msg_in.message_type)
			{

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
/**
 * Receive the UDP packet with the PB message and process it
 */
//void rmi_decode_nanoPB_msg(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
//{
//	struct pbuf *tx_p;
//	tController_pulsed * pulsed_ptr;
//	Pulsed * pulsed_msg_ptr;
//
//	/* Init the outgoing message with the default values */
//	control_msg_out = (ControlMessage)ControlMessage_init_zero;
//
//	if(p != NULL){
//
//		message_length = p->len;
//		memcpy(pb_input_msg_buffer, p->payload, p->len);
//
//		pbuf_free(p);
//
//		input_stream = pb_istream_from_buffer(pb_input_msg_buffer, message_length);
//
//		if (!pb_decode(&input_stream, ControlMessage_fields, &control_msg_in)){
//			USART_PRINTF("Message NOK decoded\n\r");
//		}
//		else
//		{
//			control_msg_out.has_xfer_id = true;
//			control_msg_out.xfer_id = control_msg_in.xfer_id;
//
//			if(control_msg_in.has_message_type)
//			{
//				if(control_msg_in.message_type == MESSAGE_TYPE_ACTION_MESSAGE)
//				{
//
//					if(control_msg_in.get_adc_data)
//					{
//						rmi_process_data_request();
//					}
//
//					if(control_msg_in.has_reset)
//					{
//						if(control_msg_in.reset)
//						{
//							NVIC_SystemReset();
//						}
//					}
//
//				}
//				else if(control_msg_in.message_type == MESSAGE_TYPE_SET_MESSAGE)
//				{
//					if(control_msg_in.has_mode)
//					{
//						controller_set_mode(control_msg_in.mode);
//					}
//
//					if(control_msg_in.has_fft_size)
//					{
//						controller_set_fft_size(control_msg_in.fft_size);
//					}
//
//					if(control_msg_in.has_pulsed)
//					{
//						pulsed_msg_ptr = &control_msg_in.pulsed;
//						pulsed_ptr = controller_get_pulsed_ptr();
//
//						if(control_msg_in.pulsed.has_start)
//						{
//							pulsed_ptr->start = pulsed_msg_ptr->start;
//						}
//
//						if(control_msg_in.pulsed.has_stop)
//						{
//							pulsed_ptr->stop = pulsed_msg_ptr->stop;
//						}
//
//						if(control_msg_in.pulsed.has_total_len)
//						{
//							pulsed_ptr->total_len = pulsed_msg_ptr->total_len;
//						}
//
//						if(control_msg_in.pulsed.has_frequency)
//						{
//							controller_set_freq(pulsed_msg_ptr->frequency);
//						}
//
//						if(control_msg_in.pulsed.has_amplitude)
//						{
//							controller_set_amp(pulsed_msg_ptr->amplitude);
//						}
//
//						if(control_msg_in.pulsed.has_function_type)
//						{
//							controller_set_function_type(control_msg_in.pulsed.function_type);
//						}
//					}
//				}
//				else if(control_msg_in.message_type == MESSAGE_TYPE_GET_MESSAGE)
//				{
//					control_msg_out.has_mode = true;
//					control_msg_out.mode = controller_get_mode();
//
//					control_msg_out.has_fft_size = true;
//					control_msg_out.fft_size = controller_get_fft_size();
//
//					control_msg_out.has_pulsed = true;
//					control_msg_out.pulsed.has_function_type = true;
//					control_msg_out.pulsed.has_start = true;
//					control_msg_out.pulsed.has_stop = true;
//					control_msg_out.pulsed.has_total_len = true;
//					control_msg_out.pulsed.has_frequency = true;
//					control_msg_out.pulsed.has_amplitude = true;
//					control_msg_out.pulsed.has_function_type = true;
//					control_msg_out.pulsed.function_type = controller_get_pulsed_ptr()->function_type;
//					control_msg_out.pulsed.start = controller_get_pulsed_ptr()->start;
//					control_msg_out.pulsed.stop = controller_get_pulsed_ptr()->stop;
//					control_msg_out.pulsed.total_len = controller_get_pulsed_ptr()->total_len;
//					control_msg_out.pulsed.frequency = controller_get_freq();
//					control_msg_out.pulsed.amplitude = controller_get_amp();
//					control_msg_out.pulsed.function_type = controller_get_function_type();
//
//					control_msg_out.has_delay = true;
//					control_msg_out.delay = controller_dac_delay;
//				}
//			}
//
//			output_stream =  pb_ostream_from_buffer(pb_output_msg_buffer, NANOPB_BUFFER_LEN);
//
//			if (!pb_encode(&output_stream, ControlMessage_fields, &control_msg_out))
//			{
//				USART_PRINTF("Delay message NOT encoded!\n\r");
//			}
//			else
//			{
//				tx_p = pbuf_alloc(PBUF_TRANSPORT, output_stream.bytes_written, PBUF_RAM);
//				memcpy(tx_p->payload, (void*)pb_output_msg_buffer, tx_p->len);
//				if(udp_sendto(pcb, tx_p, addr, port) != ERR_OK)
//				{
//					USART_PRINTF("udp_send fail\n\r");
//				}
//
//				pbuf_free(tx_p);
//			}
//		}
//	}
//}
//
//
//void rmi_process_data_request(void)
//{
//
////	controller_get_data();
//	callbacks_add(controller_get_data, (void*)NULL);
//
//}

