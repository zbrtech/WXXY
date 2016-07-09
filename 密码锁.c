#include <reg52.h>
#include <absacc.h>
#include <intrins.h>
#include <string.h>

#define CH452_I2C_ADDR1 0x60
#define CH452_I2C_MASK 0x3E
#define CH452_GET_KEY 0x0700
#define uchar unsigned char
#define uint unsigned int

#define CH452_DIG0 0x0800     //数码管号码
#define CH452_DIG1 0x0900
#define CH452_DIG2 0x0a00
#define CH452_DIG3 0x0b00
#define CH452_DIG4 0x0c00
#define CH452_DIG5 0x0d00
#define CH452_DIG6 0x0e00
#define CH452_DIG7 0x0f00
sfr P1M0 = 0x92;
sbit CH452_SDA=P1^6;
sbit CH452_SCL=P1^7;
sbit CH452_INT=P3^2;
volatile bit flag;
volatile uchar keycode;
volatile uchar M;
volatile uchar False;

volatile uchar password_OK[8]={0,1,2,3,4,5,6,7};            //正确密码
volatile uchar password[8];                  //用户输入密码

void DELAY_1US()
{
	_nop_();
}

void Delay500ms()		//@11.977MHz,延时0.5s
{
	unsigned char i, j, k;

	i = 23;
	j = 194;
	k = 72;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

void CH452_I2c_Start(void)     //ch452驱动程序
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

unsigned char CH452_I2c_RdByte(void)//键盘扫描
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
	return(keycode);   //返回键码
}

void Panduan(uchar password_OK[],uchar password[])   //判断密码正确
{
	uint i = 0,j;
	while(i<8)
	{
		if(password[i]!=password_OK[i])
		{
			CH452_Write(CH452_DIG0|(0x0F));
			CH452_Write(CH452_DIG1|(0x0F));
			CH452_Write(CH452_DIG2|(0x10));
			CH452_Write(CH452_DIG3|(0x10));
			CH452_Write(CH452_DIG4|(0x10));
			CH452_Write(CH452_DIG5|(0x10));
			CH452_Write(CH452_DIG6|(0x10));
			CH452_Write(CH452_DIG7|(0x10));
			False++;
			if(False>=6)
			{
				for(;;)
				{
					P1M0 = 0x20;
					Delay500ms();
					P1M0 = 0x00;
					Delay500ms();
				}
			}
			else
			{
				for(j=0;j<3;j++)
				{
					P1M0 = 0x20;
					Delay500ms();
					P1M0 = 0x00;
					Delay500ms();
				}
			}
			break;
		}
		if(i == 7)
		{
			CH452_Write(CH452_DIG0|0);
			CH452_Write(CH452_DIG1|0);
			CH452_Write(CH452_DIG2|(0x10));
			CH452_Write(CH452_DIG3|(0x10));
			CH452_Write(CH452_DIG4|(0x10));
			CH452_Write(CH452_DIG5|(0x10));
			CH452_Write(CH452_DIG6|(0x10));
			CH452_Write(CH452_DIG7|(0x10));
			P1M0 = 0x20;
			Delay500ms();
			P1M0 = 0x00;
		}
		i++;
	}
}

void inputMode(void)               //输入模式
{
	CH452_Write(CH452_DIG0|(0x12));
	CH452_Write(CH452_DIG1|(0x12));
	CH452_Write(CH452_DIG2|(0x12));
	CH452_Write(CH452_DIG3|(0x12));
	CH452_Write(CH452_DIG4|(0x12));
	CH452_Write(CH452_DIG5|(0x12));
	CH452_Write(CH452_DIG6|(0x12));
	CH452_Write(CH452_DIG7|(0x12));
}

void CH452_bcd(uchar ds_bcd)             //bcd译码，输入键码确定M值
{
	switch(ds_bcd)
	{
		case 0x40: M=0; break;           
		case 0x41: M=1; break;
		case 0x42: M=2; break;
		case 0x43: M=3; break;
		case 0x48: M=4; break;
		case 0x49: M=5; break;
		case 0x4A: M=6; break;
		case 0x4B: M=7; break;
		case 0x50: M=8; break;
		case 0x51: M=9; break;
		case 0x52: M=0x0A; break;
		case 0x53: M=0x0B; break;
		case 0x58: M=0x0C; break;
		case 0x59: M=0x0D; break;
		case 0x5A: M=0x0E; break;
		case 0x5B: M=0x0F; break;
		default: return;
	}
}

void CH452_inter() interrupt 0 using 1      //flag中断
{
	EX0=0;
	IE0=0;
	flag=1;
	CH452_Read();
}

void main()
{
	uchar n,select,sel;
	uint i,j;
	select = 0,sel = 0;
	EA=1;
	EX0=1;
	flag=0;
	CH452_I2c_Start();
	CH452_Write(0x403);
	CH452_Write(0x580);
	inputMode();
	for(;;)
	{
		if(flag)
		{
			CH452_bcd(keycode);
			if(M==0x0B)                   //设置密码
			{
				for(j=0;j<8;j++)
				{
					password_OK[j] = 0;
				}
				inputMode();
				select = 1;
			}
			if(M!=0x0A&&M!=0x0B&&M!=0x0C&&M!=0x0D&&M!=0x0E&&M!=0x0F)           //键盘输入
			{
				CH452_Write(0x0300);
				CH452_Write(CH452_DIG0|M);
				while(n<8)
				{
					if(select == 0)
						password[n] = M;
					else if(select == 1)
					{
						sel = 1;
						password_OK[n] = M;
					}
					n++;
					break;
				}
			}
			if(M==0x0F)                   //确定键
			{
				if(sel == 0)
				{
					Panduan(password_OK,password);
					n = 0;
				}
				else if(sel == 1)
				{
					for(i=0;i<8;i++)
					{
						password[i] = 0;
					}
					inputMode();
					P1M0 = 0x20;
					Delay500ms();
					P1M0 = 0x00;
					select = 0;
					sel = 0;
					n = 0;
				}
			}
			if(M==0x0E)                  //输入模式键
			{
				for(i=0;i<8;i++)
				{
					password[i] = 0;
				}
				inputMode();
				n = 0;
			}
			flag=0;
		}
	}
}