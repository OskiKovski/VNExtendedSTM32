/*
 * hcsr04.h
 *
 *  Created on: 4 gru 2017
 *		Author: Mateusz Salamon
 *		www.msalamon.pl
 *		mateusz@msalamon.pl
 *
 *	https://msalamon.pl/tani-dalmierz-ultradzwiekowy-hc-sr04/
 *	https://github.com/lamik/HCSR04_blocking_STM32
 */

#ifndef HCSR04_H_
#define HCSR04_H_

#define HCSR04_HIGH_PRECISION

typedef enum{
	HCSR04_OK 	 = 0,
	HCSR04_ERROR = 1
} HCSR04_STATUS;

HCSR04_STATUS HCSR04_Init(TIM_HandleTypeDef *htim, GPIO_TypeDef *GPIOx_trig, uint16_t GPIO_Pin_trig);
#ifdef HCSR04_HIGH_PRECISION
HCSR04_STATUS HCSR04_Read(float *Result,
                          GPIO_TypeDef *GPIOx_trig,
                          uint16_t GPIO_Pin_trig,
                          GPIO_TypeDef *GPIOx_echo,
                          uint16_t GPIO_Pin_echo);
#else
HCSR04_STATUS HCSR04_Read(uint16_t *Result,
                          GPIO_TypeDef *GPIOx_trig,
                          uint16_t GPIO_Pin_trig,
                          GPIO_TypeDef *GPIOx_echo,
                          uint16_t GPIO_Pin_echo);
#endif

#endif /* HCSR04_H_ */
