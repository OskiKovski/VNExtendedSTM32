/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "gps.h"
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
volatile struct gps_state gps_handle;
volatile uint8_t recv_char;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef * uart) {
  if (uart == &huart1) {
    gps_recv_char(&gps_handle, recv_char);
    HAL_UART_Receive_IT(&huart1, &recv_char, 1);
  }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();

  /* USER CODE BEGIN 2 */
  gps_handle = gps_init(&huart1);
  HAL_UART_Receive_IT(&huart1, &recv_char, 1);

  char output_buffer[100];
  for (uint8_t i = 0; i < 100; i++)
    output_buffer[i] = '\0';
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    sprintf(output_buffer, "\r\n");
    HAL_UART_Transmit(&huart2, output_buffer, strlen(output_buffer), 100);

    sprintf(output_buffer, "Data: %02d-%02d-20%02d\r\n", gps_handle.date_day, gps_handle.date_mounth, gps_handle.date_year);
    HAL_UART_Transmit(&huart2, output_buffer, strlen(output_buffer), 100);

    sprintf(output_buffer, "Czas: %02d:%02d:%02d\r\n", gps_handle.time_hour, gps_handle.time_min, gps_handle.time_sec);
    HAL_UART_Transmit(&huart2, output_buffer, strlen(output_buffer), 100);

    sprintf(output_buffer, "Szerokosc geograficzna: %f %c\r\n", gps_handle.latitude, gps_handle.latitude_direction);
    HAL_UART_Transmit(&huart2, output_buffer, strlen(output_buffer), 100);

    sprintf(output_buffer, "Dlugosc geograficzna: %f %c\r\n", gps_handle.longitude, gps_handle.longitude_direction);
    HAL_UART_Transmit(&huart2, output_buffer, strlen(output_buffer), 100);

    sprintf(output_buffer, "Wysokosc: %f m n.p.m.\r\n", gps_handle.altitude);
    HAL_UART_Transmit(&huart2, output_buffer, strlen(output_buffer), 100);

    sprintf(output_buffer, "Predkosc w wezlach: %f\r\n", gps_handle.speed_knots);
    HAL_UART_Transmit(&huart2, output_buffer, strlen(output_buffer), 100);

    sprintf(output_buffer, "Predkosc w km/h: %f\r\n", gps_handle.speed_kilometers);
    HAL_UART_Transmit(&huart2, output_buffer, strlen(output_buffer), 100);

    sprintf(output_buffer, "Liczba widocznych satelit: %d\r\n", gps_handle.satelites_number);
    HAL_UART_Transmit(&huart2, output_buffer, strlen(output_buffer), 100);

    sprintf(output_buffer, "Jakosc okreslonej pozycji: %d\r\n", gps_handle.quality);
    HAL_UART_Transmit(&huart2, output_buffer, strlen(output_buffer), 100);

    sprintf(output_buffer, "Precyzja wyznaczenia pozycji (DOP): %f\r\n", gps_handle.dop);
    HAL_UART_Transmit(&huart2, output_buffer, strlen(output_buffer), 100);

    sprintf(output_buffer, "Horyzontalna precyzja wyznaczenia pozycji (HDOP): %f\r\n", gps_handle.hdop);
    HAL_UART_Transmit(&huart2, output_buffer, strlen(output_buffer), 100);

    sprintf(output_buffer, "Wertykalna precyzja wyznaczenia pozycji (VDOP): %f\r\n", gps_handle.vdop);
    HAL_UART_Transmit(&huart2, output_buffer, strlen(output_buffer), 100);
    /* USER CODE END WHILE */
    HAL_Delay(1000);
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage 
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
