#include "IR.h"
#include <string.h>
#include <stdio.h>

int rece_flag,sent_flag;//1 正在运行
int sent_arr[20];
int receive_curr;
int potical=575;
extern char led;
int rece_lenth;
uint8_t  buffer[50];
uint8_t * target;
int receive_potical=566;


int curr_buf=0;
char buff;
int state=0;
int receive_position;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (TIM2 == htim->Instance)
	{
		
			
		if (sent_flag)
		{

			HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_3);
			TIM2->ARR=(int)(potical*sent_arr[sent_flag]);
			sent_flag++;
			
			if(sent_flag==20)
			{
				
				HAL_TIM_Base_Stop_IT(&htim2);
			}
		}
		else if (rece_flag)
		{
			
			
//			char p=HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_11);
//			HAL_UART_Transmit_DMA(&huart1,(unsigned char *)&p , 1);
			
			if(receive_position==0)
			{
				if(state<16)
				{
					if(!HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_11))
					{
						state++;
					}else
					{
						state=-1;
					}
				}
				else
				{
					if(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_11))
					{
						state++;
					}else
					{
						state=-1;
					}
				}
				
				if(state==24)
				{
					receive_position++;
				}
			}
			else
			{
				
				if(state==24&&HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_11))
				{
					state=-1;
				}
				else if(state==24&&!HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_11))
				{
					state=0;
				}
				
				if(!HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_11))
				{
					
					if(state==0&&receive_position==1)
					{
						state=0;
					}
					else if(state==1)
					{
						state=0;
						receive_position++;
					}
					else if(state==3)
					{
						buff=buff|(0x01<<(8-receive_position));
						state=0;
						receive_position++;
					}else 
					{
						state=-1;
					}
				}
				else if(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_11))
				{
					state++;
					
					if(receive_position==11)
					{
						//sucess
						rece_flag=0;
						HAL_TIM_Base_Stop_IT(&htim2);
						__HAL_TIM_SET_COUNTER(&htim2,0);
						printf("Received data: 0x%02X\n", buff);
						led=255;
						HAL_UART_Transmit_DMA(&huart1,(unsigned char *)&buff , 1);
						
//						buffer[curr_buf]=buff;
//						curr_buf++;
//						if(curr_buf==rece_lenth)
//						{
//							for(int g=0;g<rece_lenth;g++)
//							{
//								target[g]=buffer[g];
//							}
//						}
					}
				}

			}
			

			if (state==-1)
			{
				HAL_TIM_Base_Stop_IT(&htim2);
				__HAL_TIM_SET_COUNTER(&htim2,0);
				state=receive_position=rece_flag=0;
			}
			
			
		}
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

	if (GPIO_Pin==GPIO_PIN_11)
	{
		
		if (rece_flag==0)
		{
			
			rece_flag=1;
			__HAL_TIM_SET_COUNTER(&htim2,0);
			TIM2->ARR=receive_potical;
			uint32_t Delay = 200;
			do
			{
					__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
			__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
			__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
			__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
			__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
			}
    while (Delay --);
			HAL_TIM_Base_Start_IT(&htim2);
		}
		
	}
}

void IR_init(void)
{
	
	__HAL_TIM_SET_COUNTER(&htim2,0);
	htim2.Init.Period = (int)(potical*16);
	HAL_TIM_Base_Init(&htim2);
	HAL_TIM_Base_Stop_IT(&htim2);
	sent_arr[0]=16;
	sent_arr[1]=8;
	sent_arr[18]=1;
	sent_arr[19]=1;
  HAL_NVIC_SetPriority(TIM2_IRQn,0,0);    
  HAL_NVIC_EnableIRQ(TIM2_IRQn);         

}



//HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_3)




void IR_sent(uint8_t * hand,int lenth)
{
	printf("发送数据");
	for(int c=0;c<lenth;c++)
	{
		
		for(int i=7;i>=0;i--)
		{

			if((hand[c]>>i)&0x1)
			{
				sent_arr[16-2*i]=1;
				sent_arr[17-2*i]=3;
			}
			else
			{
				sent_arr[16-2*i]=1;
				sent_arr[17-2*i]=1;
			}
		}
		sent_flag=1;
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_3,1);
		__HAL_TIM_SET_COUNTER(&htim2,0);
		TIM2->ARR= (int)(potical*16);

		HAL_TIM_Base_Start_IT(&htim2);
		 
//		char p=(int)(sent_arr[w]);
//HAL_UART_Transmit_DMA(&huart1, (unsigned char *)&p, 1);

		for(int j=0;j<37356;j++){
			__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
			__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
			__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
			__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
			__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
			
			if(sent_flag==20)
				{
					break;
				}
		}
				
	
		sent_flag=0;
	}
	
}


void rece_init(uint8_t * hand,int lenth)
{
	target=hand;
	rece_lenth=lenth;
}

