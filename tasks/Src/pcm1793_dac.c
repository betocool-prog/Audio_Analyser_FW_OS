/*
 * pcm1793_dac.c
 *
 *  Created on: 28Jul.,2019
 *      Author: betocool
 */

#include <controller.h>
#include "pcm1793_dac.h"
#include "pcm1802_adc.h"

void pcm1793_dac_init(void)
{
	uint32_t aux_register = 0x0;

	/* Set up DMA TX */
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;   // DMA2 clock enable;

	DMA2_Stream3->CR &= ~DMA_SxCR_EN;
	while(DMA2_Stream3->CR & DMA_SxCR_EN)
	{

	}

	aux_register |= DMA_SxCR_MSIZE_1;
	aux_register |= DMA_SxCR_PSIZE_1;
	aux_register |= DMA_SxCR_MINC;
	aux_register |= DMA_SxCR_CIRC;
	aux_register |= DMA_SxCR_PL_1;
	aux_register |= DMA_SxCR_DIR_0;
	aux_register |= DMA_SxCR_TCIE;
	aux_register |= DMA_SxCR_HTIE;

    HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);

	DMA2_Stream3->CR = aux_register;
	DMA2_Stream3->FCR &= ~(DMA_SxFCR_DMDIS);

	DMA2_Stream3->PAR = (uint32_t)&(SPI3->TXDR);

	DMAMUX1_Channel11->CCR = 0x3E;

	/* Set up I2S3 TX */
	SPI3->CR1 &= ~SPI_CR1_SPE;
	while(SPI3->CR1 & ~SPI_CR1_SPE)
	{

	}

	aux_register = 0;
	aux_register |= SPI_CFG1_TXDMAEN;
	SPI3->CFG1 |= aux_register;

	//Chip Enable pin
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);

	DMA2_Stream3->M0AR = (uint32_t)&(controller_output_buffer[0].left);
	DMA2_Stream3->NDTR = 2 * OUTPUT_BUF_SIZE_STEREO_SAMPLES;

	/* Clear interrupt flags */
	DMA2->LIFCR |= DMA_LIFCR_CTCIF3 | DMA_LIFCR_CHTIF3;
	DMA2_Stream3->CR |= DMA_SxCR_EN;
	SPI3->CR1 |= SPI_CR1_SPE;
	SPI3->CR1 |= SPI_CR1_CSTART;
}

/**
  * @brief This function handles SPI3 (DAC) DMA global interrupt.
  */
void DMA2_Stream3_IRQHandler(void)
{
	BaseType_t pxHigherPriorityTaskWoken;

	if(DMA2->LISR & DMA_LISR_TCIF3)
	{
		DMA2->LIFCR |= DMA_LIFCR_CTCIF3;
		xTaskNotifyFromISR(controller_dac_task_h, DAC_TC_NOTIF, eSetBits, &pxHigherPriorityTaskWoken);

		if(got_start_ticks && (!got_stop_ticks))
		{
			got_stop_ticks = true;
			stop_ticks = TIM7->CNT;

			if(start_ticks > stop_ticks)
			{
				diff_ticks = 0xFFFF - start_ticks + stop_ticks;
			}
			else
			{
				diff_ticks = stop_ticks - start_ticks;
			}
		}
	}
	else if(DMA2->LISR & DMA_LISR_HTIF3)
	{
		DMA2->LIFCR |= DMA_LIFCR_CHTIF3;
		xTaskNotifyFromISR(controller_dac_task_h, DAC_HT_NOTIF, eSetBits, &pxHigherPriorityTaskWoken);

		if(got_start_ticks && (!got_stop_ticks))
		{
			got_stop_ticks = true;
			stop_ticks = TIM7->CNT;

			if(start_ticks > stop_ticks)
			{
				diff_ticks = 0xFFFF - start_ticks + stop_ticks;
			}
			else
			{
				diff_ticks = stop_ticks - start_ticks;
			}
		}
	}
}
