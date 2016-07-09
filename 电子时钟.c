#include <reg52.h>
#include <intrins.h>
#include "CH452.h"

#define uchar unsigned char
#define uint unsigned int

volatile bit flag;
volatile uchar N;
volatile uint Sec;
volatile uint Min;
volatile uint Hour;

void Delay1000ms()		//@12.018MHz
{
	unsigned char i, j, k;
	i = 46;
	j = 171;
	k = 119;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

void CH452_inter() interrupt 0 using 1
{
	EX0=0;
	IE0=0;
	flag=1;
	CH452_Read();
}

void main()
{
	uint setTime;
	EA=1;
	EX0=1;
	flag=0;
	CH452_I2c_Start();
	CH452_Write(0x403);
	CH452_Write(0x580);
	CH452_Write(CH452_DIG5|(0x12));
	CH452_Write(CH452_DIG2|(0x12));
	while(1)
	{
		CH452_Write(CH452_DIG0|(Sec%10));
		CH452_Write(CH452_DIG1|(Sec/10));
		CH452_Write(CH452_DIG3|(Min%10));
		CH452_Write(CH452_DIG4|(Min/10));
		CH452_Write(CH452_DIG6|(Hour%10));
		CH452_Write(CH452_DIG7|(Hour/10));
		Delay1000ms();
		Sec++;
		if(flag)
		{
			CH452_bcd(keycode);
			if(M == 0x0E)
			{
				switch(N%5)
				{
					case 0 : CH452_Write(CH452_TWINKLE|128),setTime=1;break;
					case 1 : CH452_Write(CH452_TWINKLE|64),setTime=2;break;
					case 2 : CH452_Write(CH452_TWINKLE|16),setTime=3;break;
					case 3 : CH452_Write(CH452_TWINKLE|8),setTime=4;break;
					case 4 : CH452_Write(CH452_TWINKLE|3),setTime=5;break;
					default : break;
				}
				N++;
			}
			if(M!=0x0A&&M!=0x0B&&M!=0x0C&&M!=0x0D&&M!=0x0E&&M!=0x0F)
			{
				switch(setTime)
				{
					case 1 : Hour=M*10+Hour%10;break;
					case 2 : Hour=(Hour/10)*10+M;break;
					case 3 : Min=Min%10+M*10;break;
					case 4 : Min=(Min/10)*10+M;break;
					default : break;
				}
			}
			if(M==0x0B&&setTime==5)
			{
				Sec = 0;
			}
			if(M==0x0F)
			{
				CH452_Write(CH452_TWINKLE|0);
				setTime = 0;
			}
		} 
		if(Sec>=60)
		{
			Min++,Sec=0;
		}
		if(Min>=60)
		{
			Hour++,Min=0;
		}
		if(Hour>=24)
		{
			Hour=0;
		}
		if(Sec == 0 && Min == 0)
		{
			P1M0 = 0x20;
			Delay1000ms();
			P1M0 = 0x00;
		}
		flag = 0;
	}
}