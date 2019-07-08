/*
 * hcsr04.c
 *
 *  Created on: 4 gru 2017
 *		Author: Mateusz Salamon
 *		www.msalamon.pl
 *		mateusz@msalamon.pl
 *
 *	https://msalamon.pl/tani-dalmierz-ultradzwiekowy-hc-sr04/
 *	https://github.com/lamik/HCSR04_blocking_STM32
 */

#include "main.h"
#include "stm32l4xx_hal.h"
#include "tim.h"
#include "gpio.h"

#include "hcsr04.h"

#define _HCSR04_TIMEOUT			1000000
#define _HCSR04_CONST			((float)0.0171821)

TIM_HandleTypeDef *htim_hcsr04;

void HCSR04_Delay(uint16_t time_us)
{
	htim_hcsr04->Instance->CNT = 0;
	while(htim_hcsr04->Instance->CNT <= time_us);
}

HCSR04_STATUS HCSR04_Init(TIM_HandleTypeDef *htim)
{
	 htim_hcsr04 = htim;

	HAL_TIM_Base_Start(htim_hcsr04);
	// Pins are preconfigured in CubeMX
	HAL_GPIO_WritePin(HCSR04_0_Trig_GPIO_Port, HCSR04_0_Trig_Pin, GPIO_PIN_RESET);

	return HCSR04_OK;
}

HCSR04_STATUS HCSR04_CaptureEcho(uint16_t *Counter)
{
	uint32_t timeout;

	HAL_GPIO_WritePin(HCSR04_0_Trig_GPIO_Port, HCSR04_0_Trig_Pin, GPIO_PIN_SET); // Trigger to high - triggered
	HCSR04_Delay(10); // Keep high per 10 us
	HAL_GPIO_WritePin(HCSR04_0_Trig_GPIO_Port, HCSR04_0_Trig_Pin, GPIO_PIN_RESET); // Trigger to low again

	timeout = _HCSR04_TIMEOUT; // Define timeout

	while(!HAL_GPIO_ReadPin(HCSR04_0_Echo_GPIO_Port, HCSR04_0_Echo_Pin)) // Wait for high state on Echo
	{
		if(timeout-- == 0x00)
			return HCSR04_ERROR; // Error if timeout is reached
	}

	htim_hcsr04->Instance->CNT = 0;
	while(HAL_GPIO_ReadPin(HCSR04_0_Echo_GPIO_Port, HCSR04_0_Echo_Pin)){} // Wait while Echo is high
	*Counter =  htim_hcsr04->Instance->CNT; // Read us from timer

	return HCSR04_OK;
}

#ifdef HCSR04_HIGH_PRECISION
HCSR04_STATUS HCSR04_Read(float *Result)
{
	uint16_t time;

	HCSR04_CaptureEcho(&time);

	*Result = (float)time / 2.0 * 0.0343;

	return HCSR04_OK;
}
#else
HCSR04_STATUS HCSR04_Read(uint16_t *Result)
{
	uint16_t time;

	HCSR04_CaptureEcho(&time);

	*Result =  time / 58;

	return HCSR04_OK;
}
#endif
