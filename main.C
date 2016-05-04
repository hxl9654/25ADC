//*****************************************************/ 
#include<REG52.h>
#include<intrins.h>  
#include<uart.h>
#define uchar unsigned char 
#define uint unsigned int 
#include <stc89c52_eeprom.h>
#define OSC 11059200
uint BAUD;
uint ADC_dat; 
uchar ser_data,ch,baud_bit;
bit flag;
sbit SW1=P3^7;

//单片机引脚配置
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
j,k 
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
   {CS1=0;
    CS2=1;
	CS3=1;
	fg=0;   
   } 
  else if(n>=11&&n<22)
   {CS1=1;
    CS2=0;
	CS3=1;
	n-=11;
	fg=1;   
   }
   else if(n>=22&&n<25)
   {CS1=1;
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
{ AD_clk=1;
 _nop_(); 
  AD_clk=0;
  _nop_();  
}
  if(fg==0) 
  {CS1=1; 
   delay(5); 
   CS1=0;
  }
  else
  if(fg==1) 
  {CS2=1; 
   delay(5); 
   CS2=0;
  }
  if(fg==2) 
  {CS3=1; 
   delay(5); 
   CS3=0;
  }
   
for(i=0;i<4;i++) 
{ AD_clk=1;  
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
void Init_serial()
{
   SCON=0X50;
   TMOD=0x20;
   switch(baud_bit)
   {
   	case 0:BAUD=1200;break;
   	case 1:BAUD=2400;break;
    case 2:BAUD=4800;break;
    case 3:BAUD=9600;break;
   }            
   TH1=TL1=256-(OSC/12/32/BAUD);
   ES=1;
   EA=1;
   REN=1;
   TR1=1;
   PCON=0X80;//波特率翻倍 
}

/******************************************************/
void Send_Data(uchar k)//
{
   ES=0;
   TI=0;
   SBUF=k;
   while(!TI);
   TI=0;
   ES=1;
}

void Send_voltage(uchar chx)
{		 
   ADC_dat*=1.221;//5V/4096=0.001221,为计算方便,取1.221	
   Send_Data('C');
   Send_Data('H'); 
   Send_Data(chx/10+0x30);
   Send_Data(chx%10+0x30);
   Send_Data(':');
   Send_Data(ADC_dat/1000+0x30);
   Send_Data('.');
   Send_Data(ADC_dat%1000/100+0x30);
   Send_Data(ADC_dat%100/10+0x30);
   Send_Data(ADC_dat%100%10+0x30);
   Send_Data('V');
   Send_Data(0x0d);
   Send_Data(0x0a);
   ADC_dat=0;
   delay(1000);
   LED2=0;
  delay(1000);
  LED2=1;
}

void Change_baud(uchar x)
{ 
 iapEraseSector(0x2000);
 iapProgramByte(0x2000,x);
 delay(200); 
}

/******************************************************* 
函数名：
void main() 
参数：无
返回值：无 
功能描述：主函数  
*******************************************************/ 
void main()  
{  	LED1=0;

    if(SW1==0)	 
     {
	   delay(2000);
	   LED2=0;
	   iapEraseSector(0x2000);
       iapProgramByte(0x2000,3);
       delay(2000); 
	   LED2=1;
	 }
	 	 	  
	baud_bit=iapReadByte(0x2000);
	Init_serial();
	M485_CON=1;
  while(1) 
   {  
    if(flag==1)
	{ 
	 flag=0;
	 switch(ser_data)
	 {
	   case 0xa0:ADC_dat=Run_Adc(0);Send_voltage(0);break;
	   case 0xa1:ADC_dat=Run_Adc(1);Send_voltage(1);break;
	   case 0xa2:ADC_dat=Run_Adc(2);Send_voltage(2);break;
	   case 0xa3:ADC_dat=Run_Adc(3);Send_voltage(3);break;
	   case 0xa4:ADC_dat=Run_Adc(4);Send_voltage(4);break;
	   case 0xa5:ADC_dat=Run_Adc(5);Send_voltage(5);break;
	   case 0xa6:ADC_dat=Run_Adc(6);Send_voltage(6);break;
	   case 0xa7:ADC_dat=Run_Adc(7);Send_voltage(7);break;
	   case 0xa8:ADC_dat=Run_Adc(8);Send_voltage(8);break;
	   case 0xa9:ADC_dat=Run_Adc(9);Send_voltage(9);break;
	   case 0xaa:ADC_dat=Run_Adc(10);Send_voltage(10);break;
	   case 0xab:ADC_dat=Run_Adc(11);Send_voltage(11);break;
	   case 0xac:ADC_dat=Run_Adc(12);Send_voltage(12);break;
	   case 0xad:ADC_dat=Run_Adc(13);Send_voltage(13);break;
	   case 0xae:ADC_dat=Run_Adc(14);Send_voltage(14);break;
	   case 0xaf:ADC_dat=Run_Adc(15);Send_voltage(15);break;
	   case 0xb0:ADC_dat=Run_Adc(16);Send_voltage(16);break;
	   case 0xb1:ADC_dat=Run_Adc(17);Send_voltage(17);break;
	   case 0xb2:ADC_dat=Run_Adc(18);Send_voltage(18);break;
	   case 0xb3:ADC_dat=Run_Adc(19);Send_voltage(19);break;
	   case 0xb4:ADC_dat=Run_Adc(20);Send_voltage(20);break;
	   case 0xb5:ADC_dat=Run_Adc(21);Send_voltage(21);break;
	   case 0xb6:ADC_dat=Run_Adc(22);Send_voltage(22);break;
	   case 0xb7:ADC_dat=Run_Adc(23);Send_voltage(23);break;
	   case 0xb8:ADC_dat=Run_Adc(24);Send_voltage(24);break;
	   case 0xe0:Change_baud(0);break;
	   case 0xe1:Change_baud(1);break;
	   case 0xe2:Change_baud(2);break;
	   case 0xe3:Change_baud(3);break;
	 }	
	}	   	 	   
 }   
} 

/******************************************************* 
函数名：
void Ser_int()  
参数：无
返回值：无 
功能描述：串口中断函数  
*******************************************************/ 
void Ser_int() interrupt 4 using 0
{ 
   if(RI)
   {
   	RI=0;
    ser_data=SBUF;
	flag=1; 
   }
}
