/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;

extern UART_HandleTypeDef huart2;

/* USER CODE BEGIN Private defines */
/* USER CODE BEGIN Private defines */
#define USART2_RXBUFFERSIZE 1 //每次接收1个数据进入一次中断
#define USART2_REC_LEN 200  	 //定义最大接收字节数 200

#define RS485DIR_TX HAL_GPIO_WritePin(GPIOE,GPIO_PIN_6,GPIO_PIN_SET);//定义我们的控制引脚为发送状态
#define RS485DIR_RX HAL_GPIO_WritePin(GPIOE,GPIO_PIN_6,GPIO_PIN_RESET);//定义我们的控制引脚为接收状态
/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

