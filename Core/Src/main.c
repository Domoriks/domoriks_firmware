/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdint.h>

#include "timer.h"

#include "Actions/actions.h"
#include "Flash/flash.h"

#include "IO/inputs.h"
#include "IO/input_handler.h"

#include "IO/outputs.h"

#include "Modbus/modbus.h"
#include "Modbus/modbusm.h"
#include "Modbus/modbus_ascii.h"
#include "Modbus/modbus_rtu.h"
#include "Modbus/modbusm_handler.h"

#include "Actions/actions.h"
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
IWDG_HandleTypeDef hiwdg;

TIM_HandleTypeDef htim14;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM14_Init(void);
static void MX_IWDG_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t MODBUS_ID;
//#define UPLOAD_NEW_MODBUS_ID
#ifdef UPLOAD_NEW_MODBUS_ID
  __attribute__((section(".modbus_id"))) const uint8_t new_id = 4;  // Logic ID, e.g., 1
#endif
#define NEWACTIONS

uint8_t uart_rxBuffer[UART_BUFFER_SIZE] = { 0 };
uint8_t new_rxdata = 0;
uint16_t rxDataLen = 0;

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
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_TIM14_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim14);

  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart_rxBuffer, UART_BUFFER_SIZE);
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
  __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);

  MODBUS_ID = *(uint8_t*)MODBUS_ID_ADDRESS;

  HAL_GPIO_WritePin(L6_GPIO_Port, L6_Pin, 0);
  HAL_GPIO_WritePin(L5_GPIO_Port, L5_Pin, 0);
  HAL_GPIO_WritePin(L4_GPIO_Port, L4_Pin, 0);
  HAL_GPIO_WritePin(L3_GPIO_Port, L3_Pin, 0);
  HAL_GPIO_WritePin(L2_GPIO_Port, L2_Pin, !0);
  HAL_GPIO_WritePin(L1_GPIO_Port, L1_Pin, !0);
  HAL_IWDG_Refresh(&hiwdg);
  HAL_Delay(500);

  for (int i = 0; i < MODBUS_ID / 10; i++)
  {
	  HAL_IWDG_Refresh(&hiwdg);
	  HAL_GPIO_WritePin(L1_GPIO_Port, L1_Pin, !1);
	  HAL_Delay(250);
	  HAL_GPIO_WritePin(L1_GPIO_Port, L1_Pin, !0);
	  HAL_Delay(150);
  }

  HAL_IWDG_Refresh(&hiwdg);
  HAL_Delay(500);

  for (int i = 0; i < MODBUS_ID % 10; i++)
  {
	  HAL_IWDG_Refresh(&hiwdg);
	  HAL_GPIO_WritePin(L1_GPIO_Port, L1_Pin, !1);
	  HAL_Delay(250);
	  HAL_GPIO_WritePin(L1_GPIO_Port, L1_Pin, !0);
	  HAL_Delay(150);
  }

  HAL_IWDG_Refresh(&hiwdg);
  uint32_t timer_blink = TIMER_SET();
  HAL_GPIO_WritePin(W_RS485_GPIO_Port, W_RS485_Pin, 0); //Set RS485 in read mode

#ifdef NEWACTIONS

  EventAction input1_singlePress = { toggle, nop, 0, 100, 64, 4, 0, 0 };
  EventAction input1_doublePress = { toggle, nop, 0, 100, 64, 7, 0, 0 };
  EventAction input1_longPress = { off, nop, 0, 100, 64, 4, 0, 5 };
  EventAction input2_singlePress = { toggle, nop, 0, 100, 64, 6, 0, 0 };
  EventAction input2_doublePress = { toggle, nop, 0, 100, 64, 4, 0, 1 };
  EventAction input2_longPress = { off, nop, 0, 100, 64, 0, 0, 8 };
  EventAction input3_singlePress = { toggle, nop, 0, 100, 64, 5, 0, 0 };
  EventAction input3_doublePress = { on, nop, 0, 100, 64, 4, 0, 3 };
  EventAction input3_longPress = { off, nop, 0, 100, 255, 255, 0, 14 };
  EventAction input4_singlePress = { toggle, nop, 0, 100, 64, 9, 0, 0 };
  EventAction input4_doublePress = { toggle, nop, 0, 100, 64, 7, 0, 4 };
  EventAction input4_longPress = { off, nop, 0, 100, 255, 255, 0, 0 };
  EventAction extraAction1 = { toggle, nop, 0, 100, 64, 6, 0, 0 };
  EventAction extraAction3 = { on, nop, 0, 100, 64, 5, 0, 4 };
  EventAction extraAction4 = { on, nop, 0, 100, 64, 6, 0, 0 };
  EventAction extraAction5 = { off, nop, 0, 100, 64, 5, 0, 6 };
  EventAction extraAction6 = { off, nop, 0, 100, 64, 6, 0, 0 };
  EventAction extraAction7 = { off, nop, 0, 100, 64, 0, 0, 8 };
  EventAction extraAction8 = { off, nop, 0, 100, 64, 1, 0, 9 };
  EventAction extraAction9 = { off, nop, 0, 100, 64, 2, 0, 10 };
  EventAction extraAction10 = { off, nop, 0, 100, 64, 3, 0, 11 };
  EventAction extraAction11 = { off, nop, 0, 100, 64, 4, 0, 12 };
  EventAction extraAction12 = { off, nop, 0, 100, 64, 7, 0, 13 };
  EventAction extraAction13 = { off, nop, 0, 100, 64, 8, 0, 5 };
  EventAction extraAction14 = { on, off, 180, 100, 64, 9, 0, 0 };


  memcpy((uint8_t*)&inputActions[0].singlePress, (uint8_t*)&input1_singlePress, sizeof(EventAction));
  memcpy((uint8_t*)&inputActions[1].singlePress, (uint8_t*)&input2_singlePress, sizeof(EventAction));
  memcpy((uint8_t*)&inputActions[2].singlePress, (uint8_t*)&input3_singlePress, sizeof(EventAction));
  memcpy((uint8_t*)&inputActions[3].singlePress, (uint8_t*)&input4_singlePress, sizeof(EventAction));

  memcpy((uint8_t*)&inputActions[0].doublePress, (uint8_t*)&input1_doublePress, sizeof(EventAction));
  memcpy((uint8_t*)&inputActions[1].doublePress, (uint8_t*)&input2_doublePress, sizeof(EventAction));
  memcpy((uint8_t*)&inputActions[2].doublePress, (uint8_t*)&input3_doublePress, sizeof(EventAction));
  memcpy((uint8_t*)&inputActions[3].doublePress, (uint8_t*)&input4_doublePress, sizeof(EventAction));

  memcpy((uint8_t*)&inputActions[0].longPress, (uint8_t*)&input1_longPress, sizeof(EventAction));
  memcpy((uint8_t*)&inputActions[1].longPress, (uint8_t*)&input2_longPress, sizeof(EventAction));
  memcpy((uint8_t*)&inputActions[2].longPress, (uint8_t*)&input3_longPress, sizeof(EventAction));
  memcpy((uint8_t*)&inputActions[3].longPress, (uint8_t*)&input4_longPress, sizeof(EventAction));

  memcpy((uint8_t*)&extraActions[0], (uint8_t*)&extraAction1, sizeof(EventAction));
  //memcpy((uint8_t*)&extraActions[1], (uint8_t*)&extraAction2, sizeof(EventAction));
  memcpy((uint8_t*)&extraActions[2], (uint8_t*)&extraAction3, sizeof(EventAction));
  memcpy((uint8_t*)&extraActions[3], (uint8_t*)&extraAction4, sizeof(EventAction));
  memcpy((uint8_t*)&extraActions[4], (uint8_t*)&extraAction5, sizeof(EventAction));
  memcpy((uint8_t*)&extraActions[5], (uint8_t*)&extraAction6, sizeof(EventAction));
  memcpy((uint8_t*)&extraActions[6], (uint8_t*)&extraAction7, sizeof(EventAction));
  memcpy((uint8_t*)&extraActions[7], (uint8_t*)&extraAction8, sizeof(EventAction));
  memcpy((uint8_t*)&extraActions[8], (uint8_t*)&extraAction9, sizeof(EventAction));
  memcpy((uint8_t*)&extraActions[9], (uint8_t*)&extraAction10, sizeof(EventAction));
  memcpy((uint8_t*)&extraActions[10], (uint8_t*)&extraAction11, sizeof(EventAction));
  memcpy((uint8_t*)&extraActions[11], (uint8_t*)&extraAction12, sizeof(EventAction));
  memcpy((uint8_t*)&extraActions[12], (uint8_t*)&extraAction13, sizeof(EventAction));
  memcpy((uint8_t*)&extraActions[13], (uint8_t*)&extraAction14, sizeof(EventAction));
//  memcpy((uint8_t*)&extraActions[14], (uint8_t*)&extraAction15, sizeof(EventAction));
//  memcpy((uint8_t*)&extraActions[15], (uint8_t*)&extraAction16, sizeof(EventAction));
//  memcpy((uint8_t*)&extraActions[16], (uint8_t*)&extraAction17, sizeof(EventAction));
//  memcpy((uint8_t*)&extraActions[17], (uint8_t*)&extraAction18, sizeof(EventAction));
//  memcpy((uint8_t*)&extraActions[18], (uint8_t*)&extraAction19, sizeof(EventAction));
//  memcpy((uint8_t*)&extraActions[19], (uint8_t*)&extraAction20, sizeof(EventAction));
//  memcpy((uint8_t*)&extraActions[20], (uint8_t*)&extraAction21, sizeof(EventAction));
//  memcpy((uint8_t*)&extraActions[21], (uint8_t*)&extraAction22, sizeof(EventAction));
//  memcpy((uint8_t*)&extraActions[22], (uint8_t*)&extraAction23, sizeof(EventAction));
//  memcpy((uint8_t*)&extraActions[23], (uint8_t*)&extraAction24, sizeof(EventAction));



  Flash_Erase(USERDATA_ORIGIN, USERDATA_LENGTH);
  Flash_WriteInputs(inputs);
  Flash_WriteInputActions(inputActions);
  Flash_WriteExtraActions(extraActions);
  Flash_WriteOutputs(outputs);
#endif

  Flash_ReadInputs(inputs);
  Flash_ReadInputActions(inputActions);
  Flash_ReadExtraActions(extraActions);
  Flash_ReadOutputs(outputs);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_TIM_Base_Start_IT(&htim14);

  while (1)
  {
	//feed the dog
	HAL_IWDG_Refresh(&hiwdg); 		//375 * 8ms = 3sec
									//256ms 32 * (32000khz/256=8ms) OLD

    //call every 10ms
    //GLOBAL_TIMER_TICK(); -> interrupt (done in stm32g0xxit.c)

    if (TIMER_ELAPSED_MS(timer_blink, 1000)) {
      timer_blink = TIMER_SET();

    }
    //read inputs
    update_inputs();

    //input handler
    input_handler();

    //prep registers
    modbus_get_outputs();

    //modbus RS485
    modbus();

    //Parse single output
    modbus_set_outputs();

    modbus_parse_register(); // always after set. this write outputs directly
    modbus_parse_action_update();

    //write outputs
    update_outputs();
    /* USER CODE END WHILE */

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

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
  hiwdg.Init.Window = 375;
  hiwdg.Init.Reload = 375;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief TIM14 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM14_Init(void)
{

  /* USER CODE BEGIN TIM14_Init 0 */

  /* USER CODE END TIM14_Init 0 */

  /* USER CODE BEGIN TIM14_Init 1 */

  /* USER CODE END TIM14_Init 1 */
  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 15;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 39999;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM14_Init 2 */

  /* USER CODE END TIM14_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_DMADISABLEONERROR_INIT;
  huart1.AdvancedInit.DMADisableonRxError = UART_ADVFEATURE_DMA_DISABLEONRXERROR;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, W_RS485_Pin|L6_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, L1_Pin|L2_Pin|L3_Pin|L4_Pin
                          |L5_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : W_RS485_Pin L6_Pin */
  GPIO_InitStruct.Pin = W_RS485_Pin|L6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : B1_Pin B2_Pin B3_Pin B4_Pin
                           B5_Pin B6_Pin */
  GPIO_InitStruct.Pin = B1_Pin|B2_Pin|B3_Pin|B4_Pin
                          |B5_Pin|B6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : L1_Pin L2_Pin L3_Pin L4_Pin
                           L5_Pin */
  GPIO_InitStruct.Pin = L1_Pin|L2_Pin|L3_Pin|L4_Pin
                          |L5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
	if (huart->Instance == USART1)
	{
		__HAL_UART_CLEAR_IDLEFLAG(huart);
		uint8_t overflow = 0;
		if (size >= UART_BUFFER_SIZE) // Check if buffer might be full
		{
			// Wait for idle line to ensure complete reception
			while (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) == RESET) {}

			// Check for overflow condition
			if (__HAL_UART_GET_FLAG(huart, UART_FLAG_ORE) != RESET)
			{
				overflow = 1;
				__HAL_UART_CLEAR_OREFLAG(huart); // Clear overflow flag
				//HAL_UART_Transmit(huart, (uint8_t *)"\n\rError: overflow of the receive buffer\n\r", 41, 100);
			}
		}

		if (!overflow)
		{
			rxDataLen = size;
			new_rxdata = 1;
		} else {
			HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart_rxBuffer, UART_BUFFER_SIZE);
			__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT); // Disable half-transfer interrupt if not needed
			//clear buffer (needed after overflow)
			memset(uart_rxBuffer, 0, UART_BUFFER_SIZE);
			rxDataLen = 0;
		}
	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART1)
	{
		__HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_PEF | UART_CLEAR_FEF | UART_CLEAR_NEF | UART_CLEAR_OREF);
		HAL_UART_Abort_IT(huart);
		HAL_UART_DeInit(huart);
		HAL_UART_Init(huart);

		HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart_rxBuffer, UART_BUFFER_SIZE);
		__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT); // Disable half-transfer interrupt if not needed
	}
}

//void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
//{
//  timer_lastbyte = TIMER_SET();						     //reset timer (time passed since last recieved byte)
//  new_uartstream = true;								 //indicate new message
//  received_buffer[uart_index++] = uart_received_byte;    //copy received byte and shift index
//
//  if (uart_index >= RECEIVE_BUFFER_SIZE) {
//    new_uartstream = false;
//    uart_index = 0;
//    memset(received_buffer, 0, sizeof(received_buffer));
//  }
//  HAL_UART_Receive_IT(&huart1, &uart_received_byte, 1); //Set new interrupt
//}



/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
