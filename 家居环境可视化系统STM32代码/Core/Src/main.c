/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ir.h"
#include <string.h>
#include <stdio.h>
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xffff);
  return ch;
}
uint32_t temp = 0;//用于接收数据
#define ADDR_24LCxx_Write 0xA0
#define ADDR_24LCxx_Read 0xA1
#define BufferSize 256
uint8_t WriteBuffer[BufferSize],ReadBuffer[BufferSize];
uint16_t i;
#define MAX_Frequency 1080			  //最大电台频率
#define MIN_Frequency 875				  //最小电台频率
unsigned char WriteBufferFM[7]={
	0x11,          //02H:音频输出，静音禁用，12MHZ，启用状态
	0x1a,0x50,          //03H:97500KHZ,频率使能87-108M(US/Europe)，步进100KHZ
	0x40,0x02,		    //04H:1-0为GPIO1(10为低，灯亮；11为高，灯灭)，...
	0x88,0xa5 };	    //a5中的5为初始音量

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
RTC_DateTypeDef GetData; 

RTC_TimeTypeDef GetTime; 

RTC_AlarmTypeDef Alarm;

HAL_StatusTypeDef i2cStatus;
struct 
{ unsigned char head;
	unsigned char function;
	short AD_Data[3];
} DAM_BUF={0xaa,0x55,1,2,3};
struct
{
    unsigned char head;
    unsigned char function;
		unsigned char temperature;
    unsigned char lightData;
		unsigned char led;
		unsigned char hour;
		unsigned char minute;
	  unsigned char second;
} sendData1 = {0xaa, 0x55, 1, 2, 3, 4, 5, 6};
struct
{
    unsigned char head;
    unsigned char function;
		unsigned char freqh;
    unsigned char freql;
		unsigned char volume;
		unsigned char alarmh;
		unsigned char alarmm;
	  unsigned char alarms;
} sendData2 = {0xaa, 0x55, 1, 2, 3, 4, 5, 6};
char ans[5]={0xaa,0x55,0x30,0x20,0x10};
uint8_t receivedData[8];
uint8_t Rs485data[8];
unsigned char dataIndex;
unsigned char display[8]={1,2,3,4,5,6,7,8};
char segTable[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x00,0x3f+0x80,0x06+0x80,0x5b+0x80,0x4f+0x80,0x66+0x80,0x6d+0x80,0x7d+0x80,0x07+0x80,0x7f+0x80,0x6f+0x80};
char clockModificationMode = 0;
unsigned char nowhourlow;
unsigned char nowminutelow;
unsigned char nowsecondlow;
unsigned char nowhourhigh;
unsigned char nowminutehigh;
unsigned char nowsecondhigh;
unsigned long frqe=975;
unsigned char volume=5;
unsigned int chan=0;
uint8_t temperature;
uint8_t lightADCValue;
static uint32_t adcSum[3] = {0};
static uint8_t adcCount = 0;
char led=0;
char oldled=0;
char Alarmflag=0;
char backcount=0;
char sendflag=0;
char dma_state=0;
char wei=7;
char model=1;
int key1_falg=0;
int key2_falg=0;
int navikey_falg=0;
static uint8_t key1_state = 0;
static uint16_t key1_count = 0;
static uint8_t key2_state = 0;
static uint16_t key2_count = 0;
static uint8_t navikey_state = 0;
static uint16_t navikey_count = 0;
char Flag_100uS;    //100uS
char Flag_1mS;      //1mS
char Flag_10mS;      //1mS
char Flag_20mS;     //10mS
char Flag_100mS;     //100mS
char Flag_1S;       //1S
char Flag_1Min;     //1Min
char Flag_1Hour;    //1Hour
unsigned char count_100uS;    //100uS
unsigned char count_1mS;      //1mS
unsigned char count_20mS;     //20mS
unsigned char count_100mS;			//100ms
unsigned char count_1S;       //1S
unsigned char count_1Min;     //1Min


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void HAL_SYSTICK_Callback(void);
void Func_100uS(void);
void Func_1mS(void);
void Func_10mS(void);
void Func_20mS(void);
void Func_100mS(void);
void Func_1S(void);
void Func_1Min(void);
void Func_1Hour(void);
void debounceKey(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t *key_state, uint16_t *key_count);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/10000);   //SysTick设置成 100uS
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_RTC_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_I2C2_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
IR_init();
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&DAM_BUF.AD_Data, 3);
	HAL_UART_Transmit_DMA(&huart1, (unsigned char *)&sendData1, sizeof(sendData1));
	HAL_UART_Receive_DMA(&huart1, receivedData, sizeof(receivedData));

	HAL_I2C_Mem_Read(&hi2c1, ADDR_24LCxx_Read, 0, I2C_MEMADD_SIZE_8BIT,ReadBuffer,BufferSize, 0xff);
	Alarm.AlarmTime.Hours=ReadBuffer[0];
	Alarm.AlarmTime.Minutes=ReadBuffer[1];
	Alarm.AlarmTime.Seconds=ReadBuffer[2];
	if(HAL_I2C_Mem_Write(&hi2c2, 0x20, 0xc0, I2C_MEMADD_SIZE_8BIT,WriteBufferFM,7, 1000) == HAL_OK)//收音机
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
			if (Flag_100uS!=0) { Flag_100uS = 0;  Func_100uS();} 
			if (Flag_1mS!=0)   { Flag_1mS = 0;    Func_1mS();} 
			if (Flag_20mS!=0)  { Flag_20mS = 0;   Func_20mS(); } 
			if (Flag_100mS!=0)  { Flag_100mS = 0;   Func_100mS(); } 
			if (Flag_1S!=0)    { Flag_1S = 0;     Func_1S();} 
			if (Flag_1Min!=0)  { Flag_1Min = 0;   Func_1Min();}  		
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV2;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV4;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_SYSTICK_Callback(void)
{ Flag_100uS=1;
	if (++count_100uS>=10) 
	 { count_100uS=0;  Flag_1mS=1;
		 if (++count_1mS>=20) 
			 { count_1mS=0;		 Flag_20mS=1;
				 if (++count_20mS>=5) 
					 { count_20mS=0;			 Flag_100mS=1;
						if (++count_100mS>=10) 
					 { count_100mS=0;			 Flag_1S=1;
							 if(++count_1S>=60) 
								{ count_1S=0;						Flag_1Min=1;
									if(++count_1Min>=60) 
										{ count_1Min=0;							Flag_1Hour=1;
										}
								} 
					 }
				 }
			 }
	 }
}
void SegValue(char value)
{
	GPIOE->ODR &= ~(0xff<<8);
	GPIOE->ODR |= value<<8;
}
void LedValue(char value)
{
	GPIOE->ODR &= ~(0xff<<8);
	GPIOE->ODR |= value<<8;
	led=value;
}
void Setdisplay(char w, char value)
{
	if(w<=7)
	{
		GPIOB->ODR &= ~(0x0f);//低四位
		GPIOB->ODR |= (w & 0x07);//位选
		SegValue(segTable[value]);
	}
}
void Ledplay(char value)
{

	GPIOB->ODR |= (0x08);//PE3为1
	LedValue(value);
}
void all_seg_print(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, uint8_t d8)
{
	if(clockModificationMode==0 )
	{
    display[0] = d1;
    display[1] = d2;
    display[2] = d3;
    display[3] = d4;
    display[4] = d5;
    display[5] = d6;
    display[6] = d7;
    display[7] = d8;
	}
	else if(clockModificationMode==1 )
	{
    if(wei==0)display[0] = d1+11;
		else display[0] = d1;
    if(wei==1)display[1] = d2+11;
		else display[1] = d2;
    if(wei==2)display[2] = d3+11;
		else display[2] = d3;
    if(wei==3)display[3] = d4+11;
		else display[3] = d4;
    if(wei==4)display[4] = d5+11;
		else display[4] = d5;
    if(wei==5)display[5] = d6+11;
		else display[5] = d6;
    if(wei==6)display[6] = d7+11;
		else display[6] = d7;
    if(wei==7)display[7] = d8+11;
		else display[7] = d8;
	}
		if(model==4 )
	{
    display[0] = d1;
    display[1] = d2;
    display[2] = d3;
    display[3] = d4;
    display[4] = d5;
    display[5] = d6;
    display[6] = d7+11;
    display[7] = d8;
	}
}
void Func_1mS(void)
{ 
	static char count;
	if (count<=7)
	{
		Setdisplay(count, display[count]);
		count++;
	}
	else
	{
		count=0;
		Ledplay(led);
	}
	if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2) == GPIO_PIN_RESET)
    {
        if (key1_count < 10)
        {
            key1_count++;
        }
        else
        {
            if (key1_state == GPIO_PIN_SET)
            {
               key1_falg=1-key1_falg;
							 if (++model > 4) model= 1;
								if(model==3)
									{
										nowhourlow=Alarm.AlarmTime.Hours%10;
										nowminutelow=Alarm.AlarmTime.Minutes%10;
										nowsecondlow=Alarm.AlarmTime.Seconds%10;
										nowhourhigh=Alarm.AlarmTime.Hours/10;
										nowminutehigh=Alarm.AlarmTime.Minutes/10;
										nowsecondhigh=Alarm.AlarmTime.Seconds/10;
									}
            }
            key1_state = GPIO_PIN_RESET;
        }
    }
    else
    {
        if (key1_count > 0)
        {
            key1_count--;
        }
        else
        {
            if (key1_state == GPIO_PIN_RESET)
            {
            }
            key1_state = GPIO_PIN_SET;
        }
    }
	if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == GPIO_PIN_RESET)
    {
        if (key2_count < 10)
        {
            key2_count++;
        }
        else
        {
            if (key2_state == GPIO_PIN_SET)
            {
								key2_falg=1-key2_falg;
								if(key2_falg==1)led=led|0x0f;
								else led=led&0xf0;
            }
            key2_state = GPIO_PIN_RESET;
        }
    }
    else
    {
        if (key2_count > 0)
        {
            key2_count--;
        }
        else
        {
            if (key2_state == GPIO_PIN_RESET)
            {
            }
            key2_state = GPIO_PIN_SET;
        }
    }
		if (DAM_BUF.AD_Data[0] / 100 != 40)
    {

			if(volume==0)HAL_I2C_Mem_Write(&hi2c2, 0x20, 0x80, I2C_MEMADD_SIZE_8BIT,WriteBufferFM,7, 1000);
        if (navikey_count < 20)
        {
            navikey_count++;
        }
        else
        {
						 if (navikey_state == GPIO_PIN_SET)
            {
								if (DAM_BUF.AD_Data[0] / 100 == 28) //导航上建
                {
									if(model==1||model==3)
									{
										switch(wei)
										{
											case 0:
												if(nowhourhigh<2)nowhourhigh++;
												else nowhourhigh=0;
												break;
											case 1:
												if(nowhourhigh<2 && nowhourlow<10)
												{
													nowhourlow++;
													if(nowhourlow==10)
													{
														nowhourlow=0;
													}
												}
												else if(nowhourhigh==2 && nowhourlow<4)
												{
													nowhourlow++;
													if(nowhourlow==4)
													{
														nowhourlow=0;
													}
												}
												else nowhourlow=0;
												break;
											 case 3:
                        if (nowminutehigh < 5)
                            nowminutehigh++;
                        else
                            nowminutehigh = 0;
                        break;
                    case 4:
                        if (nowminutelow < 9)
                            nowminutelow++;
                        else
                            nowminutelow = 0;
                        break;
                    case 6:
                        if (nowsecondhigh < 5)
                            nowsecondhigh++;
                        else
                            nowsecondhigh = 0;
                        break;
                    case 7:
                        if (nowsecondlow < 9)
                            nowsecondlow++;
                        else
                            nowsecondlow = 0;
                        break;
										}
									}
									else if(model==4)
									{
										if(++volume>15)volume=0;
										WriteBufferFM[6] = (WriteBufferFM[6]&0xf0) | (volume&0x0f);
										if(volume==0)HAL_I2C_Mem_Write(&hi2c2, 0x20, 0x80, I2C_MEMADD_SIZE_8BIT,WriteBufferFM,7, 1000);
										else HAL_I2C_Mem_Write(&hi2c2, 0x20, 0xc0, I2C_MEMADD_SIZE_8BIT,WriteBufferFM,7, 1000);
									}
                }
								if(DAM_BUF.AD_Data[0] / 100 == 11)//导航下键
								{
									if(model==1||model==3)
									{
										switch(wei)
										{
											case 0:
												if(nowhourhigh>0)nowhourhigh--;
												else nowhourhigh=2;
												break;
											case 1:
												if(nowhourhigh<2 && nowhourlow>0)nowhourlow--;
												else if(nowhourhigh<2 && nowhourlow==0)nowhourlow=9;
												else if(nowhourhigh==2 && nowhourlow>0)nowhourlow--;
												else if(nowhourhigh==2 && nowhourlow==0)nowhourlow=3;
												break;
											 case 3:
                        if (nowminutehigh > 0)
                            nowminutehigh--;
                        else
                            nowminutehigh = 5;
                        break;
                    case 4:
                        if (nowminutelow > 0)
                            nowminutelow--;
                        else
                            nowminutelow = 9;
                        break;
                    case 6:
                        if (nowsecondhigh > 0)
                            nowsecondhigh--;
                        else
                            nowsecondhigh = 5;
                        break;
                    case 7:
                        if (nowsecondlow > 0)
                            nowsecondlow--;
                        else
                            nowsecondlow = 9;
                        break;
										}
									}
									else if(model==4)
									{
										if(volume>0)volume--;
										else volume=15;
										if(volume==0)
										WriteBufferFM[6] = (WriteBufferFM[6]&0xf0) | (volume&0x0f);
										if(volume==0)HAL_I2C_Mem_Write(&hi2c2, 0x20, 0x80, I2C_MEMADD_SIZE_8BIT,WriteBufferFM,7, 1000);
										else HAL_I2C_Mem_Write(&hi2c2, 0x20, 0xc0, I2C_MEMADD_SIZE_8BIT,WriteBufferFM,7, 1000);
									}
								}
								if(DAM_BUF.AD_Data[0] / 100 == 05)//导航右键
								{
									if(model==1||model==3)
									{
										if(wei!=1&&wei!=4&&wei<7)wei++;
										else if(wei==1||wei==4)wei+=2;
										else if(wei==7)wei=0;
									}
									else if(model==4)
									{
											if( frqe>MAX_Frequency )frqe = MIN_Frequency;
											else frqe++;
											chan = (frqe-870)/1;
											WriteBufferFM[1] = chan/4;
											WriteBufferFM[2] = ((chan%4)<<6)|0x10;
											HAL_I2C_Mem_Write(&hi2c2, 0x20, 0xc0, I2C_MEMADD_SIZE_8BIT,WriteBufferFM,7, 1000);
									}
								}
								if(DAM_BUF.AD_Data[0] / 100 == 23)//导航左键
								{
									if(model==1||model==3)
									{
									if(wei!=3&&wei!=6&&wei>0)wei--;
									else if(wei==3||wei==6)wei-=2;
									else if(wei==0)wei=7;
									}
									else if(model==4)
									{
											if( frqe<MIN_Frequency )frqe = MAX_Frequency;
											else frqe--;
											chan = (frqe-870)/1;
											WriteBufferFM[1] = chan/4;
											WriteBufferFM[2] = ((chan%4)<<6)|0x10;
											HAL_I2C_Mem_Write(&hi2c2, 0x20, 0xc0, I2C_MEMADD_SIZE_8BIT,WriteBufferFM,7, 1000);
									}
								}
								if(DAM_BUF.AD_Data[0] / 100 == 17)//导航中建
								{
									clockModificationMode=1-clockModificationMode;
									if(clockModificationMode==1 && model==1)
									{
										nowhourlow=GetTime.Hours%10;
										nowminutelow=GetTime.Minutes%10;
										nowsecondlow=GetTime.Seconds%10;
										nowhourhigh=GetTime.Hours/10;
										nowminutehigh=GetTime.Minutes/10;
										nowsecondhigh=GetTime.Seconds/10;
									}

									else if(clockModificationMode==0 && model==1)
									{
										GetTime.Hours=nowhourhigh*10+nowhourlow;
										GetTime.Minutes=nowminutehigh*10+nowminutelow;
										GetTime.Seconds=nowsecondhigh*10+nowsecondlow;
										HAL_RTC_SetTime(&hrtc, &GetTime, RTC_FORMAT_BIN);
									}
									else if(clockModificationMode==0 && model==3)
									{
										Alarm.AlarmTime.Hours=nowhourhigh*10+nowhourlow;
										Alarm.AlarmTime.Minutes=nowminutehigh*10+nowminutelow;
										Alarm.AlarmTime.Seconds=nowsecondhigh*10+nowsecondlow;
										WriteBuffer[0]=Alarm.AlarmTime.Hours;
										WriteBuffer[1]=Alarm.AlarmTime.Minutes;
										WriteBuffer[2]=Alarm.AlarmTime.Seconds;
                if(HAL_I2C_Mem_Write(&hi2c1, ADDR_24LCxx_Write, 0, I2C_MEMADD_SIZE_8BIT,WriteBuffer,8, 1000) == HAL_OK)
                printf("\r\n EEPROM 24C02 Write Test OK \r\n");
								HAL_Delay(20);
										HAL_RTC_SetAlarm_IT(&hrtc,&Alarm,RTC_FORMAT_BIN);
									}
								}
								if(DAM_BUF.AD_Data[0] / 100 == 00)
								{
								navikey_falg=1-navikey_falg;
								if(navikey_falg==1)led=led|0xf0;
								else led=led&0x0f;
								}
            }
						
            navikey_state = GPIO_PIN_RESET;
					HAL_GPIO_WritePin(GPIOE,GPIO_PIN_6,GPIO_PIN_SET);
					HAL_UART_Transmit(&huart2, (unsigned char *)&sendData2, sizeof(sendData2),0xFFFF);
					HAL_GPIO_WritePin(GPIOE,GPIO_PIN_6,GPIO_PIN_RESET);
        }
    }
    else
    {
        if (navikey_count > 0)
        {
            navikey_count--;
        }
        else
        {
            if (navikey_state == GPIO_PIN_RESET)
            {
            }
            navikey_state = GPIO_PIN_SET;
        }
    }
		if(GetTime.Hours==Alarm.AlarmTime.Hours && GetTime.Minutes==Alarm.AlarmTime.Minutes && ((GetTime.Seconds-Alarm.AlarmTime.Seconds)<=3) && ((GetTime.Seconds-Alarm.AlarmTime.Seconds)>=0))//闹钟
		{
			HAL_GPIO_TogglePin(GPIOE,GPIO_PIN_5);
		}

}
void Func_100uS(void)
{}
void Func_20mS(void)
{
				HAL_RTC_GetTime(&hrtc, &GetTime, RTC_FORMAT_BIN);
    adcSum[0] += DAM_BUF.AD_Data[0];
    adcSum[1] += DAM_BUF.AD_Data[1];
		adcSum[2] += DAM_BUF.AD_Data[2];
    adcCount++;
    if (adcCount >= 20)
    {
        adcSum[0] = adcSum[0] / 20;
        adcSum[1] = adcSum[1] / 20;
				adcSum[2] = adcSum[2] / 20;
        /*double VSense = (double)adcSum[2] * (3.3 / 4096.0);
        temperature = ((1.43 - VSense))*1000 / 43 + 25.0;
				lightADCValue = adcSum[1];
        uint8_t lightPercentage = (lightADCValue * 255) / 4096;*/

        if(model==1 && clockModificationMode==0)all_seg_print(GetTime.Hours/10,GetTime.Hours%10,10,(GetTime.Minutes/10),(GetTime.Minutes)%10,10,GetTime.Seconds/10,GetTime.Seconds%10);
				else if(model==1 && clockModificationMode==1)all_seg_print(nowhourhigh,nowhourlow,10,nowminutehigh,nowminutelow,10,nowsecondhigh,nowsecondlow);
				else if(model==3)all_seg_print(nowhourhigh,nowhourlow,10,nowminutehigh,nowminutelow,10,nowsecondhigh,nowsecondlow);
				else if(model==2)all_seg_print((lightADCValue/100)/10,(lightADCValue/10)%10,(lightADCValue%10),10,10,10,temperature/10,temperature%10);
				else if(model==4 && volume<10 && frqe<1000)all_seg_print(volume,10,10,10,10,frqe%1000/100,frqe%1000%100/10,frqe%1000%100%10);
				else if(model==4 && volume<10 && frqe>=1000)all_seg_print(volume,10,10,10,frqe/1000,frqe%1000/100,frqe%1000%100/10,frqe%1000%100%10);
				else if(model==4 && volume>=10 && frqe<1000)all_seg_print(volume/10,volume%10,10,10,10,frqe%1000/100,frqe%1000%100/10,frqe%1000%100%10);
				else if(model==4 && volume>=10 && frqe>=1000)all_seg_print(volume/10,volume%10,10,10,frqe/1000,frqe%1000/100,frqe%1000%100/10,frqe%1000%100%10);


        adcSum[0] = adcSum[1] = adcSum[2] = 0;
        adcCount = 0;
    }
}
void Func_100mS(void)
{
		if((&huart1)->gState == HAL_UART_STATE_READY)
	{ 
        sendflag=1-sendflag;
        dma_state=1;
        if( dma_state==1)
				{
						if(sendflag==1)
						{
							sendData1.temperature=temperature;
							sendData1.lightData=lightADCValue;
							sendData1.led=led;
							sendData1.hour=GetTime.Hours;
							sendData1.minute=GetTime.Minutes;
							sendData1.second=GetTime.Seconds;
							sendData1.function=0x55;
							
						}
						else if(sendflag==0)
						{
							sendData1.temperature=frqe/10;
							sendData1.lightData=frqe%10;
							sendData1.led=volume;
							sendData1.hour=Alarm.AlarmTime.Hours;
							sendData1.minute=Alarm.AlarmTime.Minutes;
							sendData1.second=Alarm.AlarmTime.Seconds;
							sendData1.function=0x56;
							
							sendData2.freqh=frqe/10;
							sendData2.freql=frqe%10;
							sendData2.volume=volume;
							sendData2.alarmh=led;
							sendData2.alarmm=Alarm.AlarmTime.Minutes;
							sendData2.alarms=Alarm.AlarmTime.Seconds;
						}
						HAL_UART_Transmit_DMA(&huart1, (unsigned char *)&sendData1, sizeof(sendData1));
						dma_state=0;
				}					
	}
	

}
void Func_1S(void)
{  
					HAL_GPIO_WritePin(GPIOE,GPIO_PIN_6,GPIO_PIN_SET);
					HAL_UART_Transmit(&huart2, (unsigned char *)&sendData2, sizeof(sendData2),0xFFFF);
					HAL_GPIO_WritePin(GPIOE,GPIO_PIN_6,GPIO_PIN_RESET);
	//if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2) == GPIO_PIN_RESET) if (++ADC_channel>2) ADC_channel=0;
}

void Func_1Min(void)
{  

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

	if(huart->Instance==USART1)
  {
		 if (receivedData[0] == 0xAA && receivedData[1] == 0x55)
    {
        led = receivedData[4];  // The 6th byte is the LED data
				if(receivedData[5]==1)
				{
					if(frqe<1080)frqe+=1;
					else frqe=875;
				}
				else if(receivedData[5]==2)
				{
					if(frqe>875)frqe-=1;
					else frqe=1080;
				}
				if(receivedData[6]==1)
				{
					if(volume<15)volume+=1;
					else volume=0;
				}
				else if(receivedData[6]==2)
				{
					if(volume>0)volume-=1;
					else volume=15;
				}
        HAL_UART_Receive_DMA(&huart1, receivedData, sizeof(receivedData));
    }
  }
	
}
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *nhrtc)
{
	oldled=led;
	led=255;
  HAL_RTC_SetAlarm_IT(&hrtc,&Alarm,RTC_FORMAT_BIN);
//--------------------------------------------------------
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
  __disable_irq();
  while (1)
  {
  }
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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
