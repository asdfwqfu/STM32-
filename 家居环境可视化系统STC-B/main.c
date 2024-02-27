#include "STC15F2K60S2.H"        //必须。
#include "sys.H"                 //必须。
#include "EXT.h" 
#include "key.h"
#include "displayer.h"
#include "adc.h"
#include "beep.H" 
#include "ir.h"
#include "uart1.h"
#include "music.h"
#include "uart2.h"
#include "time.h"
#include "FM_Radio.h"
#include "M24C02.h"

code unsigned long SysClock=11059200;         //必须。定义系统工作时钟频率(Hz)，用户必须修改成与实际工作频率（下载时选择的）一致
#ifdef _displayer_H_                          //显示模块选用时必须。（数码管显示译码表，用戶可修改、增加等） 
code char decode_table[]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x00,0x08,0x40,0x01,0x41,0x48,0x76,0x38,0x40,0x00,	
	              /* 序号:   0   	1    2	   3    4	    5    6	  7   8	   	9	 		10	11	 12   13   14   15    16   17   18   19	 */
                /* 显示:   0   	1    2     3    4     5    6    7   8    9  (无)   下-  中-  上-  上中-  中下-  H    L    - 	（无）*/  
	                       0x3f|0x80,0x06|0x80,0x5b|0x80,0x4f|0x80,0x66|0x80,0x6d|0x80,0x7d|0x80,0x07|0x80,0x7f|0x80,0x6f|0x80 };  
             /* 带小数点     20         21         22         23      24        25        26        27        28        29        */
#endif
#define DS1302_SECOND_WRITE 0x80			
#define DS1302_MINUTE_WRITE 0x82
#define DS1302_HOUR_WRITE   0x84
#define DS1302_SECOND_READ  0x81
int code tempdata[]={239,197,175,160,150,142,135,129,124,120,116,113,109,107,104,101, 
										  99, 97, 95, 93, 91, 90, 88, 86, 85, 84, 82, 81, 80, 78, 77, 76, 
										  75, 74, 73, 72, 71, 70, 69, 68, 67, 67, 66, 65, 64, 63, 63, 62, 
										  61, 61, 60, 59, 58, 58, 57, 57, 56, 55, 55, 54, 54, 53, 52, 52, 
										  51, 51, 50, 50, 49, 49, 48, 48, 47, 47, 46, 46, 45, 45, 44, 44, 
										  43, 43, 42, 42, 41, 41, 41, 40, 40, 39, 39, 38, 38, 38, 37, 37, 
										  36, 36, 36, 35, 35, 34, 34, 34, 33, 33, 32, 32, 32, 31, 31, 31, 
										  30, 30, 29, 29, 29, 28, 28, 28, 27, 27, 27, 26, 26, 26, 25, 25,
										  24, 24, 24, 23, 23, 23, 22, 22, 22, 21, 21, 21, 20, 20, 20, 19, 
										  19, 19, 18, 18, 18, 17, 17, 16, 16, 16, 15, 15, 15, 14, 14, 14, 
										  13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10, 9, 9, 9, 8, 8, 8, 7, 
										  7, 7, 6, 6,5, 5, 5,4,4, 3, 3,3, 2, 2, 1, 1, 1, 0, 0, -1, -1, -1, 
										  -2, -2, -3, -3, -4, -4, -5, -5, -6, -6, -7, -7, -8, -8, -9, -9, 
										  -10, -10, -11, -11, -12, -13, -13, -14, -14, -15, -16, -16, -17, 
										  -18, -19, -19, -20, -21, -22, -23, -24, -25, -26, -27, -28, -29, 
										  -30, -32, -33, -35, -36, -38, -40, -43, -46, -50, -55, -63, 361};
unsigned char model=1;//模式
unsigned char flag=0;//定时判断
unsigned char light=0;//光照
unsigned int suml=0;//光照和
unsigned char temperature=0;//光照和
RTC_TimeTypeDef time;
unsigned int sumt=0;//温度和
unsigned char freq=0;//adc次数
struct_ADC adc_data;//adc数据
unsigned char led=0;//灯泡光亮
struct_FMRadio FM = {975,5,0,1,1};
unsigned char alarmhour;	
unsigned char alarmminute;	
unsigned char alarmsecond;
unsigned char modifimodel;
unsigned char nowhourlow;
unsigned char nowminutelow;
unsigned char nowsecondlow;
unsigned char nowhourhigh;
unsigned char nowminutehigh;
unsigned char nowsecondhigh;
unsigned char wei;
unsigned char key2_falg;
unsigned char key3_falg;
uchar dtime;
code	uchar table_D_BCD[]={0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09};
code char matchhead[2]={0xaa,0x55};
unsigned char sentdata[8]={0,0,0,0,0,0,0,0};
unsigned char sendflag=0;
char save[8]={0,0,0,0,0,0,0,0};//串口接收数据

void keycallback()
{
		if(GetKeyAct(enumKey1)==enumKeyPress){
				if(++model>4)model=1;
			}
		else if(GetKeyAct(enumKey2)==enumKeyPress){
					key2_falg=1-key2_falg;
					if(key2_falg==1)led=led|0x0f;
					else led=led&0xf0;
			}
		else if(GetKeyAct(enumKey3)==enumKeyPress){

			}	
}
void displaycallback()
{
		LedPrint(led);
			time=DS1302_GetTime();
			adc_data = GetADC();
			suml+=adc_data.Rop;
			adc_data = GetADC();
			sumt+=adc_data.Rt/4;
			freq++;
			if(freq==250)
			{
				light=(suml+freq/2)/freq;
				temperature=(sumt+freq/2)/freq;
				temperature=tempdata[temperature-1];
				freq=0;
				suml=0;
				sumt=0;
			}
	
	if(model==1 && modifimodel==0)Seg7Print(time.Hours/10,time.Hours%10,10,time.Minutes/10,time.Minutes%10,10,time.Seconds/10,time.Seconds%10);
	else if((model==1||model==3 )&& modifimodel==1)
	{
		switch(wei)
		{
			case 0:
					Seg7Print(nowhourhigh+20,nowhourlow,10,nowminutehigh,nowminutelow,10,nowsecondhigh,nowsecondlow);
			break;
			case 1:
					Seg7Print(nowhourhigh,nowhourlow+20,10,nowminutehigh,nowminutelow,10,nowsecondhigh,nowsecondlow);
			break;
			case 3:
					Seg7Print(nowhourhigh,nowhourlow,10,nowminutehigh+20,nowminutelow,10,nowsecondhigh,nowsecondlow);
			break;
			case 4:
					Seg7Print(nowhourhigh,nowhourlow,10,nowminutehigh,nowminutelow+20,10,nowsecondhigh,nowsecondlow);
			break;
			case 6:
					Seg7Print(nowhourhigh,nowhourlow,10,nowminutehigh,nowminutelow,10,nowsecondhigh+20,nowsecondlow);
			break;
			case 7:
					Seg7Print(nowhourhigh,nowhourlow,10,nowminutehigh,nowminutelow,10,nowsecondhigh,nowsecondlow+20);
			break;
		}
	}
	else if(model==2)Seg7Print(light/100,(light/10)%10,light%10,10,10,10,temperature/10,temperature%10);
	else if(model==3)Seg7Print(alarmhour/10,alarmhour%10,10,alarmminute/10,alarmminute%10,10,alarmsecond/10,alarmsecond%10);
	else if(model==4 && FM.volume<10 && FM.frequency<1000)Seg7Print(FM.volume,10,10,10,10,FM.frequency%1000/100,FM.frequency%1000%100/10+20,FM.frequency%1000%100%10);
	else if(model==4 && FM.volume<10 && FM.frequency>=1000)Seg7Print(FM.volume,10,10,10,FM.frequency/1000,FM.frequency%1000/100,FM.frequency%1000%100/10+20,FM.frequency%1000%100%10);
	else if(model==4 && FM.volume>=10 && FM.frequency<1000)Seg7Print(FM.volume/10,FM.volume%10,10,10,10,FM.frequency%1000/100,FM.frequency%1000%100/10+20,FM.frequency%1000%100%10);
	else if(model==4 && FM.volume>=10 && FM.frequency>=1000)Seg7Print(FM.volume/10,FM.volume%10,10,10,FM.frequency/1000,FM.frequency%1000/100,FM.frequency%1000%100/10+20,FM.frequency%1000%100%10);
	if(time.Hours==alarmhour && time.Minutes==alarmminute && (0<=(time.Seconds-alarmsecond)<=3))SetBeep(100,40);
}	
void nav_callback()
{
	if(GetAdcNavAct(enumAdcNavKeyUp)==enumKeyPress)
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
		if(model==4)
		{
			if(FM.volume<15)FM.volume++;
			else FM.volume=0;
		}
	}
	else if(GetAdcNavAct(enumAdcNavKeyDown)==enumKeyPress)
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
		if(model==4)
		{
			if(FM.volume>0)FM.volume--;
			else FM.volume=15;
		}
	}
	else if(GetAdcNavAct(enumAdcNavKeyCenter)==enumKeyPress)
	{
		if(model==1||model==3)
		{
			modifimodel=1-modifimodel;
		}
		if(modifimodel==1 && model==1)
		{
			nowhourlow=time.Hours%10;
			nowminutelow=time.Minutes%10;
			nowsecondlow=time.Seconds%10;
			nowhourhigh=time.Hours/10;
			nowminutehigh=time.Minutes/10;
			nowsecondhigh=time.Seconds/10;
		}
		else if(modifimodel==1 && model==3)
		{
			nowhourlow=alarmhour%10;
			nowminutelow=alarmminute%10;
			nowsecondlow=alarmsecond%10;
			nowhourhigh=alarmhour/10;
			nowminutehigh=alarmminute/10;
			nowsecondhigh=alarmsecond/10;
		}
		else if(modifimodel==0 && model==1)
		{
		WriteDS1302(0x8E,0x00);	 //禁止写保护位
		dtime=ReadDS1302(DS1302_SECOND_READ)|0x80;
		WriteDS1302(0x80,dtime);//晶振停止工作
		/*写入时、分、秒值*/	 
		dtime=(table_D_BCD[nowsecondhigh]<<4)|table_D_BCD[nowsecondlow];	
		WriteDS1302(DS1302_SECOND_WRITE,dtime);
		dtime=(table_D_BCD[nowminutehigh]<<4)|table_D_BCD[nowminutelow];
		WriteDS1302(DS1302_MINUTE_WRITE,dtime);
		dtime=(table_D_BCD[nowhourhigh]<<4)|table_D_BCD[nowhourlow];	
		WriteDS1302(DS1302_HOUR_WRITE,dtime);
		WriteDS1302(0x8E,0x80); //写保护位置1
		}
		else if(modifimodel==0 && model==3)
		{
			alarmhour=nowhourhigh*10+nowhourlow;
			alarmminute=nowminutehigh*10+nowminutelow;
			alarmsecond=nowsecondhigh*10+nowsecondlow;
			M24C02_Write(0,alarmhour);
			M24C02_Write(1,alarmminute);
			M24C02_Write(2,alarmsecond);
		}

	}
	else if(GetAdcNavAct(enumAdcNavKeyRight)==enumKeyPress)
	{
		if(model==1||model==3)
		{
			if(wei!=1&&wei!=4&&wei<7)wei++;
			else if(wei==1||wei==4)wei+=2;
			else if(wei==7)wei=0;
		}
		if(model==4)
		{
			if(FM.frequency < 1080)FM.frequency++;
			else FM.frequency=887;
		}
	}
	else if(GetAdcNavAct(enumAdcNavKeyLeft)==enumKeyPress)
	{
		if(model==1||model==3)
		{
			if(wei!=3&&wei!=6&&wei>0)wei--;
			else if(wei==3||wei==6)wei-=2;
			else if(wei==0)wei=7;
		}
		if(model==4)
		{
			if(FM.frequency > 887)FM.frequency--;
			else FM.frequency=1080;
		}
	}
	else if(GetAdcNavAct(enumAdcNavKey3)==enumKeyPress)
	{
					key3_falg=1-key3_falg;
					if(key3_falg==1)led=led|0xf0;
					else led=led&0x0f;
	}
}

void sentcallback()
{
	if(light<40 || temperature<15)IrPrint(sentdata,8);
	sendflag=1-sendflag;
	if(sendflag==1)
	{
	sentdata[0]=0xAA;
	sentdata[1]=0x55;
	sentdata[2]=temperature;
	sentdata[3]=light;
	sentdata[4]=led;
	sentdata[5]=time.Hours;
	sentdata[6]=time.Minutes;
	sentdata[7]=time.Seconds;
	Uart2Print(sentdata,8);
	}
	else if(sendflag==0)
	{
	sentdata[0]=0xAA;
	sentdata[1]=0x56;
	sentdata[2]=FM.frequency/10;
	sentdata[3]=FM.frequency%10;
	sentdata[4]=FM.volume;
	sentdata[5]=alarmhour;
	sentdata[6]=alarmminute;
	sentdata[7]=alarmsecond;

	}
	Uart1Print(sentdata,8);

}
void comcallback()
{
	led=save[4];
	if(save[5]==1)
	{
		if(FM.frequency<1080)FM.frequency+=1;
		else FM.frequency=875;
	}
	else if(save[5]==2)
	{
		if(FM.frequency>875)FM.frequency-=1;
		else FM.frequency=1080;
	}
	if(save[6]==1)
	{
		if(FM.volume<15)FM.volume+=1;
		else FM.volume=0;
	}
	else if(save[6]==2)
	{
		if(FM.volume>0)FM.volume-=1;
		else FM.volume=15;
	}

	return;
}
void uart485callback()
{
	led=save[5];
	FM.frequency=save[2]*10+save[3];
	FM.volume=save[4];
	alarmminute=save[6];
	alarmsecond=save[7];
	M24C02_Write(1,alarmminute);
	M24C02_Write(2,alarmsecond);
}
void main()
{
	FMRadioInit(FM);
	Initial_DS1302();
	KeyInit();
	DisplayerInit();
	IrInit(NEC_R05d);
	AdcInit(ADCexpEXT);
	BeepInit();	
	SetDisplayerArea(0,7);
	Seg7Print(10,10,10,10,10,10,10,10);
	LedPrint(0);
	Uart1Init(115200);
	Uart2Init(115200,Uart2Usedfor485);
	SetUart1Rxd(save, 8, matchhead, 2);
	SetUart2Rxd(save, 8, matchhead, 2);
	alarmhour=M24C02_Read(0);
	alarmminute=M24C02_Read(1);
	alarmsecond=M24C02_Read(2);
	SetEventCallBack(enumEventUart1Rxd,comcallback);
	SetEventCallBack(enumEventUart2Rxd,uart485callback);
	SetEventCallBack(enumEventKey,keycallback);//变速
	SetEventCallBack(enumEventSys1mS,displaycallback);//显示
	SetEventCallBack(enumEventNav,nav_callback);//导航键
	SetEventCallBack(enumEventSys100mS,sentcallback);//发送数据
	//SetEventCallBack(enumEventSys1S,ircallback);//发送数据
	MySTC_Init();	    
	while(1)             	
	{
		MySTC_OS();    
	}	
}               