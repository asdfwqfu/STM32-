#ifndef __IR_H__
#define __IR_H__


#include "main.h"
#include "stm32f1xx_hal.h"

extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart1;



void TIM_callback(void);
void IR_init(void);
void EXIT_recall(void);
void IR_sent(uint8_t * hand,int lenth);
void rece_init(uint8_t * hand,int lenth);
#endif
