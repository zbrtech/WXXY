#include <reg52.h>
#include <absacc.h>
#include <intrins.h>

#define CH452_I2C_ADDR1 0x60
#define CH452_I2C_MASK 0x3E
#define CH452_GET_KEY 0x0700

#define uchar unsigned char
#define uint unsigned int
sbit P15 = P1^5;
sfr P1M1 = 0x91;
sfr P1M0 = 0x92;
#define CH452_DIG0 0x0800
#define CH452_DIG1 0x0900
#define CH452_DIG2 0x0a00
#define CH452_DIG3 0x0b00
#define CH452_DIG4 0x0c00
#define CH452_DIG5 0x0d00
#define CH452_DIG6 0x0e00
#define CH452_DIG7 0x0f00
#define CH452_TWINKLE 0x0600
sbit CH452_SDA=P1^6;
sbit CH452_SCL=P1^7;
sbit CH452_INT=P3^2;
volatile bit flag;
volatile uchar keycode;
volatile uchar M;
volatile uchar N;

void DELAY_1US()
{
	_nop_();
}

void DELAYms(uint ms)
{
	uchar i;
	while (ms--)
	{
		for(i=0;i<124;i++)
		{}
	}
}

void CH452_I2c_Start(void)
{
	EX0=0;
	CH452_SDA=1;
	CH452_SCL=1;
	DELAY_1US();
	CH452_SDA=0;
	DELAY_1US();
	CH452_SCL=0;
	DELAY_1US();
}

void CH452_I2c_Stop(void)
{
	CH452_SDA=0;
	DELAY_1US();
	CH452_SCL=1;
	DELAY_1US();
	CH452_SDA=1;
	DELAY_1US();
	DELAY_1US();
	EX0=1;
}

void CH452_I2c_WrByte(unsigned char dat)
{
	uchar i;
	for(i=0;i!=8;i++)
	{
		if(dat&0x80)
		{
			CH452_SDA=1;
		}
		else
		{
			CH452_SDA=0;
		}
		DELAY_1US();
		CH452_SCL=1;
		dat<<=1;
		DELAY_1US();
		DELAY_1US();
		CH452_SCL=0;
		DELAY_1US();
	}
	CH452_SDA=1;
	DELAY_1US();
	CH452_SCL=1;
	DELAY_1US();
	DELAY_1US();
	CH452_SCL=0;
	DELAY_1US();
}

unsigned char CH452_I2c_RdByte(void)//¼üÅÌÉ¨Ãè
{
	uchar dat,i;
	CH452_SDA=1;
	dat=0;
	for(i=0;i!=8;i++)
	{
		CH452_SCL=1;
		DELAY_1US();
		DELAY_1US();
		dat<<=1;
		if(CH452_SDA)
			dat++;
		CH452_SCL=0;
		DELAY_1US();
		DELAY_1US();
	}
	CH452_SDA=1;
	DELAY_1US();
	CH452_SCL=1;
	DELAY_1US();
	DELAY_1US();
	CH452_SCL=0;
	DELAY_1US();
	return(dat);
}

void CH452_Write(unsigned short cmd)
{
	CH452_I2c_Start();
	CH452_I2c_WrByte((uchar)(cmd>>7)&CH452_I2C_MASK|CH452_I2C_ADDR1);
	CH452_I2c_WrByte((uchar)cmd);
	CH452_I2c_Stop();
}

unsigned char CH452_Read(void)
{
	CH452_I2c_Start();
	CH452_I2c_WrByte((uchar)(CH452_GET_KEY>>7)&CH452_I2C_MASK|0x01|CH452_I2C_ADDR1);
	keycode=CH452_I2c_RdByte();
	CH452_I2c_Stop();
	return(keycode);   //·µ»Ø¼üÂë
}

void CH452_bcd(uchar ds_bcd)
{
	switch(ds_bcd)
	{
		case 0x40: M=0x00; break;
		case 0x41: M=0x01; break;
		case 0x42: M=0x02; break;
		case 0x43: M=0x03; break;
		case 0x48: M=0x04; break;
		case 0x49: M=0x05; break;
		case 0x4A: M=0x06; break;
		case 0x4B: M=0x07; break;
		case 0x50: M=0x08; break;
		case 0x51: M=0x09; break;
		case 0x52: M=0x0A; break;
		case 0x53: M=0x0B; break;
		case 0x58: M=0x0C; break;
		case 0x59: M=0x0D; break;
		case 0x5A: M=0x0E; break;
		case 0x5B: M=0x0F; break;
		default: return;
	}
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
	EA=1;
	EX0=1;
	flag=0;
	CH452_I2c_Start();
	CH452_Write(0x403);
	CH452_Write(0x580);
	CH452_Write(CH452_DIG7|1);
	CH452_Write(CH452_DIG6|2);
	CH452_Write(CH452_DIG5|3);
	CH452_Write(CH452_DIG4|4);
	CH452_Write(CH452_DIG3|5);
	CH452_Write(CH452_DIG2|6);
	CH452_Write(CH452_DIG1|7);
	CH452_Write(CH452_DIG0|8);
	while(1)
	{
		if(flag)
		{
			CH452_bcd(keycode);
			if(M==0x0A)
			{
				CH452_Write(0x201);
				while(1)
				{
					CH452_Read();
					CH452_bcd(keycode);
					CH452_Write(0x403);
					CH452_Write(0x580);
					CH452_Write(CH452_DIG0|M);
					CH452_Write(CH452_DIG1|N);
					DELAYms(50);
				}
			}
			else if(M==0x0B)
			{
				CH452_Write(0x0301);
				DELAYms(500);
			}
			else if(M==0x0F)
			{
				TMOD=0x01;
				TH0=(65536-500)/256;
				TL0=(65536-500)%256;
				TR0=1;
				ET0=1;
				EA=1;
				P1M1=0x00;
				P1M0=0x20;
				while(1);
			}
		}
	}
}

void t0(void) interrupt 1 using 0
{
	uint t02s;
	TH0=(65536-500)/256;
	TL0=(65536-500)%256;
	t02s++;
	if(t02s==500)
	{
		t02s=0;
		P15=~P15;
	}
}