#include "STC15F2K60S2.H" 
#include "intrins.H" 
#include "time.H" 
#define uchar unsigned char
#define uint unsigned int


#define DS1302_SECOND_WRITE 0x80			
#define DS1302_MINUTE_WRITE 0x82
#define DS1302_HOUR_WRITE   0x84 
#define DS1302_WEEK_WRITE   0x8A
#define DS1302_DAY_WRITE    0x86
#define DS1302_MONTH_WRITE  0x88
#define DS1302_YEAR_WRITE   0x8C

#define DS1302_SECOND_READ  0x81
#define DS1302_MINUTE_READ  0x83
#define DS1302_HOUR_READ    0x85 
#define DS1302_WEEK_READ    0x8B
#define DS1302_DAY_READ     0x87
#define DS1302_MONTH_READ   0x89
#define DS1302_YEAR_READ    0x8D



sbit Rtc_sclk = P1^5;		
sbit Rtc_rst  = P1^6;		
sbit Rtc_io   = P5^4;		

uchar temp;				

RTC_TimeTypeDef sTime;			

void Ds1302_write(uchar temp) 	
{
	uchar i;
	for(i=0;i<8;i++)			
	{
		Rtc_sclk=0;			
		Rtc_io=(bit)(temp&0x01);
		temp>>=1;				
		Rtc_sclk=1;				
	}
}


uchar Ds1302_read()	 
{
	 uchar i, dat;
	 for(i=0;i<8;i++)			
	 {
	 	Rtc_sclk=0;			
	 	dat>>=1;				 
		if(Rtc_io==1)		
		dat|=0x80;			
		Rtc_sclk=1;			
	 }
	 return dat;				
}

void WriteDS1302(uchar Addr, uchar Data) 	  
{
    Rtc_rst = 0;					
    Rtc_sclk = 0;				
    Rtc_rst = 1;						
		Ds1302_write(Addr);				
		Ds1302_write(Data);			  
		Rtc_rst = 0;   				
    Rtc_sclk = 1;						 
}


uchar ReadDS1302(uchar cmd)
{
    uchar Data;
    Rtc_rst = 0;		
    Rtc_sclk = 0;		
    Rtc_rst = 1;		
    Ds1302_write(cmd);    
    Data =Ds1302_read();   
		Rtc_rst = 0;			
    Rtc_sclk = 1; 		
    return Data;		
}



RTC_TimeTypeDef DS1302_GetTime()
{
	RTC_TimeTypeDef Time;	
 	uchar ReadValue;
 	ReadValue = ReadDS1302(DS1302_SECOND_READ);
 	Time.Seconds=((ReadValue&0x70)>>4)*10 + (ReadValue&0x0F);		 
	ReadValue=ReadDS1302(DS1302_MINUTE_READ);
 	Time.Minutes = ((ReadValue&0x70)>>4)*10 + (ReadValue&0x0F);		  
 	ReadValue = ReadDS1302(DS1302_HOUR_READ);
 	Time.Hours = (((ReadValue&0x70)>>4)*10 + (ReadValue&0x0F))%24;		  
	
	return Time;
}


void Initial_DS1302(void)	
{
  WriteDS1302(0x8E,0x00);			
	temp=ReadDS1302(DS1302_SECOND_READ)&0x7f ;
	WriteDS1302(0x80,temp);		   
	WriteDS1302(0x8E,0x80);								 
}	