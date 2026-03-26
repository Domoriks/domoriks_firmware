/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define W_RS485_Pin GPIO_PIN_14
#define W_RS485_GPIO_Port GPIOC
#define L6_Pin GPIO_PIN_15
#define L6_GPIO_Port GPIOC
#define B1_Pin GPIO_PIN_0
#define B1_GPIO_Port GPIOA
#define B2_Pin GPIO_PIN_1
#define B2_GPIO_Port GPIOA
#define B3_Pin GPIO_PIN_2
#define B3_GPIO_Port GPIOA
#define B4_Pin GPIO_PIN_3
#define B4_GPIO_Port GPIOA
#define B5_Pin GPIO_PIN_4
#define B5_GPIO_Port GPIOA
#define B6_Pin GPIO_PIN_5
#define B6_GPIO_Port GPIOA
#define L1_Pin GPIO_PIN_6
#define L1_GPIO_Port GPIOA
#define L2_Pin GPIO_PIN_7
#define L2_GPIO_Port GPIOA
#define L3_Pin GPIO_PIN_8
#define L3_GPIO_Port GPIOA
#define L4_Pin GPIO_PIN_11
#define L4_GPIO_Port GPIOA
#define L5_Pin GPIO_PIN_12
#define L5_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */
extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;

#define UART_BUFFER_SIZE  128

#define MODBUS_ID_ADDRESS  0x08007FFE  // The fixed address where the ID is stored
extern uint8_t MODBUS_ID;

extern uint8_t uart_rxBuffer[UART_BUFFER_SIZE];
extern uint8_t new_rxdata;
extern uint16_t rxDataLen;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
