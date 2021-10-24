/*
 * pcm1802_adc.c
 *
 *  Created on: 8Aug.,2019
 *      Author: betocool
 */

#include <controller.h>
#include "pcm1802_adc.h"

extern TaskHandle_t controller_adc_task_h;

void pcm1802_adc_init(void)
{
	uint32_t aux_register = 0x0;

	/* Set up DMA RX */
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;   // DMA2 clock enable;

	DMA2_Stream2->CR &= ~DMA_SxCR_EN;
	while(DMA2_Stream2->CR & DMA_SxCR_EN)
	{

	}

	aux_register |= DMA_SxCR_MSIZE_1;
	aux_register |= DMA_SxCR_PSIZE_1;
	aux_register |= DMA_SxCR_MINC;
	aux_register |= DMA_SxCR_CIRC;
	aux_register |= DMA_SxCR_PL_1;
	aux_register |= DMA_SxCR_TCIE;
	aux_register |= DMA_SxCR_HTIE;
	aux_register |= DMA_SxCR_TEIE;
	aux_register |= DMA_SxCR_DMEIE;

	DMA2_Stream2->CR = aux_register;
	DMA2_Stream2->FCR &= ~(DMA_SxFCR_DMDIS);

    HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

	DMA2_Stream2->PAR = (uint32_t)&(SPI2->RXDR);

	DMAMUX1_Channel10->CCR = 0x27;

	/* Set up I2S2 RX */
	SPI2->CR1 &= ~SPI_CR1_SPE;
	while(SPI2->CR1 & ~SPI_CR1_SPE)
	{

	}

	aux_register = 0;
	aux_register |= SPI_CFG1_RXDMAEN;
	SPI2->CFG1 |= aux_register;

	/* Need to swap MISO and MOSI ! */
	SPI2->CFG2 |= SPI_CFG2_IOSWP;
	/* Channel len not set to 1 by HW */
	SPI2->I2SCFGR |= SPI_I2SCFGR_CHLEN;

	//Chip Enable pin
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET);

	DMA2_Stream2->M0AR = (uint32_t)&(controller_input_buffer[0].left);
	DMA2_Stream2->NDTR = INPUT_BUF_SIZE_STEREO_SAMPLES * 2;

	/* Clear interrupt flags */
	DMA2->LIFCR |= DMA_LIFCR_CTCIF2 | DMA_LIFCR_CHTIF2;
	DMA2_Stream2->CR |= DMA_SxCR_EN;
	SPI2->CR1 |= SPI_CR1_SPE;
	SPI2->CR1 |= SPI_CR1_CSTART;
}

/**
  * @brief This function handles SPI2 (ADC) DMA global interrupt.
  */
void DMA2_Stream2_IRQHandler(void)
{

	BaseType_t pxHigherPriorityTaskWoken;

	if(DMA2->LISR & DMA_LISR_TCIF2)
	{
		DMA2->LIFCR |= DMA_LIFCR_CTCIF2;
		xTaskNotifyFromISR(controller_adc_task_h, ADC_TC_NOTIF, eSetValueWithOverwrite, &pxHigherPriorityTaskWoken);
		if(conn_accepted && (!got_start_ticks))
		{
			start_ticks = TIM7->CNT;
			got_start_ticks = true;
		}
	}
	else if(DMA2->LISR & DMA_LISR_HTIF2)
	{
		DMA2->LIFCR |= DMA_LIFCR_CHTIF2;
		xTaskNotifyFromISR(controller_adc_task_h, ADC_HT_NOTIF, eSetValueWithOverwrite, &pxHigherPriorityTaskWoken);
		if(conn_accepted && (!got_start_ticks))
		{
			start_ticks = TIM7->CNT;
			got_start_ticks = true;
		}
	}

	if(DMA2->LISR & DMA_LISR_TEIF2)
	{
		DMA2->LIFCR |= DMA_LIFCR_CTEIF2;
		xTaskNotifyFromISR(controller_adc_task_h, ADC_ERR_NOTIF, eSetBits, &pxHigherPriorityTaskWoken);
	}

	if(DMA2->LISR & DMA_LISR_DMEIF2)
	{
		DMA2->LIFCR |= DMA_LIFCR_CDMEIF2;
		xTaskNotifyFromISR(controller_adc_task_h, ADC_ERR_NOTIF, eSetBits, &pxHigherPriorityTaskWoken);
	}
}
