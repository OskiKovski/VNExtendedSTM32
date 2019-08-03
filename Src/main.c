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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "gps.h"
#include "HMC5883L.h"
#include "hcsr04.h"
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct strCoor {
  float latitude;
  float longitude;
} coordinate;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
const int numberOfPoints = 2;
coordinate routePoints[2] = {
    {5225.1079, 2099.6542},
    {5225.1312, 2099.5248}
};
float distance0, distance1, currentLatitude, currentLongitude, targetLatitude, targetLongitude, distanceForTarget;
int targetPointIndex = 0;

char buf[30];
uint8_t len;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void vibrateAllMotorsForGivenTime(int);

void vibrateTheProperDirectionMotorOnce(float);

void vibrateMotorsForFinish();

float getCurrentCourseAngle(float, float, float, float);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
volatile struct gps_state gps_handle;
volatile uint8_t recv_char;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *uart) {
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
  MX_TIM1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HCSR04_0_Init(&htim1);
  HCSR04_1_Init(&htim2);

  gps_handle = gps_init(&huart1);
  HAL_UART_Receive_IT(&huart1, &recv_char, 1);

  char output_buffer[100];
  for (uint8_t i = 0; i < 100; i++)
    output_buffer[i] = '\0';

  HMC5883L_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    sprintf(output_buffer, "\r\n");
    HAL_UART_Transmit(&huart2, output_buffer, strlen(output_buffer), 100);

    sprintf(output_buffer, "Data: %02d-%02d-20%02d\r\n", gps_handle.date_day, gps_handle.date_mounth, gps_handle.date_year);
    HAL_UART_Transmit(&huart2, output_buffer, strlen(output_buffer), 100);

    sprintf(output_buffer, "Czas: %02d:%02d:%02d\r\n", gps_handle.time_hour, gps_handle.time_min, gps_handle.time_sec);
    HAL_UART_Transmit(&huart2, output_buffer, strlen(output_buffer), 100);

    targetLatitude = routePoints[targetPointIndex].latitude;
    targetLongitude = routePoints[targetPointIndex].longitude;

    sprintf(output_buffer, "Szerokosc geograficzna celu: %f \r\n", targetLatitude);
    HAL_UART_Transmit(&huart2, output_buffer, strlen(output_buffer), 100);

    sprintf(output_buffer, "Dlugosc geograficzna celu: %f \r\n", targetLongitude);
    HAL_UART_Transmit(&huart2, output_buffer, strlen(output_buffer), 100);

    currentLatitude = gps_handle.latitude;
    currentLongitude = gps_handle.longitude;

    sprintf(output_buffer, "Obecna szerokosc geograficzna: %f %c\r\n", currentLatitude, gps_handle.latitude_direction);
    HAL_UART_Transmit(&huart2, output_buffer, strlen(output_buffer), 100);

    sprintf(output_buffer, "Obecna dlugosc geograficzna: %f %c\r\n", currentLongitude, gps_handle.longitude_direction);
    HAL_UART_Transmit(&huart2, output_buffer, strlen(output_buffer), 100);

    sprintf(output_buffer, "Angle is: %d\r\n", HMC5883L_GetAngle());
    HAL_UART_Transmit(&huart2, (uint8_t*)output_buffer, strlen(output_buffer), 100);

    HCSR04_0_Read(&distance0);
    len = sprintf(buf, "distance0: %.2f\n\r", distance0);
    HAL_UART_Transmit(&huart2, (uint8_t *) buf, len, 20);

    HCSR04_1_Read(&distance1);
    len = sprintf(buf, "distance1: %.2f\n\r", distance1);
    HAL_UART_Transmit(&huart2, (uint8_t *) buf, len, 20);

    distanceForTarget = distance_between(currentLatitude, currentLongitude, targetLatitude, targetLongitude);

    if (distanceForTarget < 5) {
      if (targetPointIndex >= numberOfPoints) {
        vibrateMotorsForFinish();
        break;
      }
      vibrateAllMotorsForGivenTime(2000);
      ++targetPointIndex;
    }

    if (distance0 < 100) {
      HAL_GPIO_WritePin(LED_White_GPIO_Port, LED_White_Pin, 1);
      HAL_Delay(1000);
      HAL_GPIO_WritePin(LED_White_GPIO_Port, LED_White_Pin, 0);
    } else if (distance1 < 100) {
      HAL_GPIO_WritePin(LED_Blue_GPIO_Port, LED_Blue_Pin, 1);
      HAL_Delay(1000);
      HAL_GPIO_WritePin(LED_Blue_GPIO_Port, LED_Blue_Pin, 0);
    } else {
      vibrateTheProperDirectionMotorOnce(
          getCurrentCourseAngle(currentLatitude, currentLongitude, targetLatitude, targetLongitude));
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    HAL_Delay(2000);
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

  /** Configure LSE Drive Capability 
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 24;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
  /** Enable MSI Auto calibration 
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/* USER CODE BEGIN 4 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
  HCSR04_TIM_IC_CaptureCallback(htim);
}

void vibrateAllMotorsForGivenTime(int time) {
  HAL_GPIO_WritePin(LED_White_GPIO_Port, LED_White_Pin, 1);
  HAL_GPIO_WritePin(LED_Green_GPIO_Port, LED_Green_Pin, 1);
  HAL_GPIO_WritePin(LED_Blue_GPIO_Port, LED_Blue_Pin, 1);
  HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, 1);
  HAL_Delay(time);
  HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, 0);
  HAL_GPIO_WritePin(LED_Blue_GPIO_Port, LED_Blue_Pin, 0);
  HAL_GPIO_WritePin(LED_Green_GPIO_Port, LED_Green_Pin, 0);
  HAL_GPIO_WritePin(LED_White_GPIO_Port, LED_White_Pin, 0);
}

void vibrateTheProperDirectionMotorOnce(float destinationAngle) {
  if ((0 <= destinationAngle && destinationAngle < 22.5) ||
      (337.5 <= destinationAngle && destinationAngle < 360)) { //front
    HAL_GPIO_WritePin(LED_Green_GPIO_Port, LED_Green_Pin, 1);
    HAL_Delay(1000);
    HAL_GPIO_WritePin(LED_Green_GPIO_Port, LED_Green_Pin, 0);
  } else if (destinationAngle >= 22.5 && destinationAngle < 67.5) {    //front + right
    HAL_GPIO_WritePin(LED_Green_GPIO_Port, LED_Green_Pin, 1);
    HAL_GPIO_WritePin(LED_Blue_GPIO_Port, LED_Blue_Pin, 1);
    HAL_Delay(1000);
    HAL_GPIO_WritePin(LED_Blue_GPIO_Port, LED_Blue_Pin, 0);
    HAL_GPIO_WritePin(LED_Green_GPIO_Port, LED_Green_Pin, 0);
  } else if (destinationAngle >= 67.5 && destinationAngle < 112.5) {    //right side
    HAL_GPIO_WritePin(LED_Blue_GPIO_Port, LED_Blue_Pin, 1);
    HAL_Delay(1000);
    HAL_GPIO_WritePin(LED_Blue_GPIO_Port, LED_Blue_Pin, 0);
  } else if (destinationAngle >= 112.5 && destinationAngle < 157.5) {   //back + right
    HAL_GPIO_WritePin(LED_Blue_GPIO_Port, LED_Blue_Pin, 1);
    HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, 1);
    HAL_Delay(1000);
    HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, 0);
    HAL_GPIO_WritePin(LED_Blue_GPIO_Port, LED_Blue_Pin, 0);
  } else if (destinationAngle >= 157.5 && destinationAngle < 202.5) {   //back
    HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, 1);
    HAL_Delay(1000);
    HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, 0);
  } else if (destinationAngle >= 202.5 && destinationAngle < 247.5) {   //back + left
    HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, 1);
    HAL_GPIO_WritePin(LED_White_GPIO_Port, LED_White_Pin, 1);
    HAL_Delay(1000);
    HAL_GPIO_WritePin(LED_Red_GPIO_Port, LED_Red_Pin, 0);
    HAL_GPIO_WritePin(LED_White_GPIO_Port, LED_White_Pin, 0);
  } else if (destinationAngle >= 247.5 && destinationAngle < 292.5) {   //left side
    HAL_GPIO_WritePin(LED_White_GPIO_Port, LED_White_Pin, 1);
    HAL_Delay(1000);
    HAL_GPIO_WritePin(LED_White_GPIO_Port, LED_White_Pin, 0);
  } else if (destinationAngle >= 292.5 && destinationAngle < 337.5) {   // front + left
    HAL_GPIO_WritePin(LED_White_GPIO_Port, LED_White_Pin, 1);
    HAL_GPIO_WritePin(LED_Green_GPIO_Port, LED_Green_Pin, 1);
    HAL_Delay(1000);
    HAL_GPIO_WritePin(LED_Green_GPIO_Port, LED_Green_Pin, 0);
    HAL_GPIO_WritePin(LED_White_GPIO_Port, LED_White_Pin, 0);
  } else {                                                              // all for 2 seconds
    vibrateAllMotorsForGivenTime(3000);
  }
}

void vibrateMotorsForFinish() {
  vibrateAllMotorsForGivenTime(500);
  HAL_Delay(1000);
  vibrateAllMotorsForGivenTime(500);
  HAL_Delay(1000);
  vibrateAllMotorsForGivenTime(1000);
}

float getCurrentCourseAngle(float lat1, float long1, float lat2, float long2) {
  int16_t currentHeading = HMC5883L_GetAngle();
  int16_t currentBearing = course_to(lat1, long1, lat2, long2);
  float currentCourseAngle = currentBearing - currentHeading;
  if (currentCourseAngle < 0) {
    currentCourseAngle += 360;
  }
  return currentCourseAngle;
}
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
