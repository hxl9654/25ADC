//*****************************************************/ 
#include<REG52.h>
#include<intrins.h>  
#include<uart.h>
#include <stc89c52_eeprom.h>
#define uchar unsigned char 
#define uint unsigned int 

int Time = 1000;
bit SystemStatu = 1;
int Timer1s = 0;
unsigned char ChannelStatu[25] = {0};

//单片机引脚配置
sbit SW1=P3^7;
sbit CS1=P2^6;
sbit CS2=P2^5;
sbit CS3=P2^4;
sbit AD_eoc=P2^0; 
sbit AD_clk=P2^1; 
sbit AD_in=P2^2; 
sbit AD_out=P2^3;
sbit LED1=P1^0; 
sbit LED2=P1^1; 
sbit M485_CON=P1^2;   
/******************************************************* 
函数名：延时函数
返回值：无
功能描述：延时几毫秒
  
*******************************************************/  
void delay(uint z) //延时程序ms级别
{
	uint k; 
	for(z;z>0;z--)  
	for(k=10;k>0;k--); 
}  

/******************************************************* 
函数名：
AD采样函数 void ADC(uchar n) 
参数：n  
返回值：ADC_Result  
功能描述：将AD采样的电压值送出 
*******************************************************/ 
uint Run_Adc(uint n)
{
	uchar i,ADC_Value_L=0,ADC_Value_H=0,fg; 
	uint ADC_Result; 
	AD_clk=0;
	if(n<11) 
	{
		CS1=0;
		CS2=1;
		CS3=1;
		fg=0;   
	} 
	else if(n>=11&&n<22)
	{
		CS1=1;
		CS2=0;
		CS3=1;
		n-=11;
		fg=1;   
	}
	else if(n>=22&&n<25)
	{
		CS1=1;
		CS2=1;
		CS3=0;
		n-=22;
		fg=2;   
	}    
	n<<=4; 
	for(i=0;i<4;i++) 
	{ 
		AD_in=n&0x80;  
		AD_clk=1; 
		_nop_(); 
		AD_clk=0; 
		_nop_(); 
		n<<=1; 
	} 

	AD_in=0; 

	for(i=0;i<8;i++) 
	{ 
		AD_clk=1;
		_nop_(); 
		AD_clk=0;
		_nop_();  
	}
	if(fg==0) 
	{
		CS1=1; 
		delay(5); 
		CS1=0;
	}
	else
	if(fg==1) 
	{
		CS2=1; 
		delay(5); 
		CS2=0;
	}
	if(fg==2) 
	{
		CS3=1; 
		delay(5); 
		CS3=0;
	}
   
	for(i=0;i<4;i++) 
	{ 
		AD_clk=1;  
		ADC_Value_H<<=1;  
		if(AD_out) 
			ADC_Value_H|=0x01; 
		AD_clk=0;
		_nop_(); 
		_nop_(); 
	 } 

	for(i=0;i<8;i++) 
	{   
		AD_clk=1; 
		ADC_Value_L<<=1;  
		if(AD_out)
			ADC_Value_L|=0x01;  
		AD_clk=0; 
		_nop_(); 
		_nop_();  
	} 
	CS1=1;
	CS2=1;
	CS3=1;  
	ADC_Result=(uint)ADC_Value_H;  
	ADC_Result<<=8;  
	ADC_Result|=ADC_Value_L; 
	return(ADC_Result); 
}

/******************************************************/

void Send_voltage(unsigned int value, unsigned char channel)
{		 
	unsigned char temp[20] = "CH00:0.000V\n";
	value *= 1.221;//5V/4096=0.001221,为计算方便,取1.221	
	temp[2] = channel / 10 + '0';
	temp[3] = channel % 10 + '0';
	temp[5] = value / 1000 + '0';

	temp[7] = value % 1000 / 100 + '0';
	temp[8] = value % 100 / 10 + '0';
	temp[9] = value % 10 + '0';
	
	UART_SendString(temp, 12);
	
	delay(1000);
	LED2=0;
	delay(1000);
	LED2=1;
}

/******************************************************* 
函数名：
void main() 
参数：无
返回值：无 
功能描述：主函数  
*******************************************************/ 
void main()  
{  
	unsigned char i;
	LED1=0;

	if(SW1 == 0)	 
	{
		delay(2000);
		LED2 = 0;
		iapEraseSector(0x2000);
		iapProgramByte(0x2000, 96);
		delay(2000); 
		LED2 = 1;
	}

	UART_Conf(iapReadByte(0x2000) * 100);
	M485_CON = 1;
	
	while(1) 
	{  
		UART_Driver();
		if(SystemStatu && Timer1s >= Time)
		{ 
			Timer1s = 0;
			for(i = 0; i < 25; i++)
			{
				if(ChannelStatu[i])
				{
					Send_voltage(Run_Adc(i), i);
				}
			}	
		}	   	 	   
	}   
} 