#ifndef TIME_H
#define TIME_H

#include "STC15F2K60S2.H"
#define uchar unsigned char
#define uint unsigned int
	
void Ds1302_write(uchar temp);
uchar Ds1302_read();
void WriteDS1302(uchar Addr, uchar Data);
uchar ReadDS1302(uchar cmd);
typedef struct
{
  uchar Hours; 

  uchar Minutes;       

  uchar Seconds;          

} RTC_TimeTypeDef;

RTC_TimeTypeDef DS1302_GetTime();
void Key_OFFON();
void Initial_DS1302(void);

#endif // TIME_H