#include<reg51.h>
#include<UART.h>

#define XTAL 11.059200
sfr AUXR = 0x8E;
void UART_Action(unsigned char *dat, unsigned char len)
{
	
}

unsigned char pdata UART_Buff[64];     //串口接收缓冲区
unsigned char UART_BuffIndex = 0;           //串口接收缓冲区当前位置

bit UART_SendFlag;                          //串口发送完成标志
bit UART_ResiveFlag;                        //串口接收完成标志
bit UART_ResiveStringEndFlag;               //串口字符串接收全部完成标志
bit UART_ResiveStringFlag;                  //串口字符串正在接收标志

/*///////////////////////////////////////////////////////////////////////////////////
*函数名：UART_Conf
*函数功能：配置串口
*参数列表：
*   baud
*       参数类型：unsigned char型数据
*       参数描述：要设置的波特率
*////////////////////////////////////////////////////////////////////////////////////
void UART_Conf(unsigned int baud) //UART设置函数（buad：欲设置的波特率）
{
	AUXR &= 0xBF;		//定时器1时钟为Fosc/12,即12T
	AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
	TL1 = TH1 = 256 - XTAL * 1000000 / 12 / 32 / baud;    //计算定时器初值
	EA = 1;         //使能总中断
	ES = 1;         //使能串口中断
	TMOD &= 0X0F;   //配置定时器1为自动重装模式
	TMOD |= 0X20;
	SCON = 0X50;    //配置串口工作模式
	TR1 = 1;        //使能定时器1
}
/*///////////////////////////////////////////////////////////////////////////////////
*函数名：UART_SendString
*函数功能：向串口发送一个字符串
*参数列表：
*   *dat
*       参数类型：unsigned char型指针
*       参数描述：要发送的字符串的首地址
*   len
*       参数类型：unsigned char型数据
*       参数描述：要发送的字符串的长度
*////////////////////////////////////////////////////////////////////////////////////
void UART_SendString(unsigned char *dat, unsigned char len)
{
	while(len)
	{
		len --;                     //每发送一位，长度减1
		SBUF = *dat;                //发送一位数据
		dat ++;                     //数据指针移向下一位
		while(! UART_SendFlag);     //等待串口发送完成标志
		UART_SendFlag = 0;          //清空串口发送完成标志
	}
}
/*///////////////////////////////////////////////////////////////////////////////////
*函数名：UART_Read
*函数功能：将暂存数组中的数据读取出来。
*参数列表：
*   *to
*       参数类型：unsigned char型指针
*       参数描述：存储接收到的字符的位置
*   len
*       参数类型：unsigned char型数据
*       参数描述：要读取的字符串的长度
*返回值：unsigned char型数据，字符串的实际长度
*////////////////////////////////////////////////////////////////////////////////////
unsigned char UART_Read(unsigned char *to, unsigned char len)
{
	unsigned char i;
	if(UART_BuffIndex < len)len = UART_BuffIndex;   //获取当前接收数据的位数
	for(i = 0;i < len;i ++)                         //复制数据的目标数组
		{
			*to = UART_Buff[i];
			to ++;
		}
	UART_BuffIndex = 0;                             //清空串口接收缓冲区当前位置
	return len;
}
/*///////////////////////////////////////////////////////////////////////////////////
*函数名：UART_Driver
*函数功能：串口通信监控函数，在主循环中调用。
*           如果接收到字符串，会自动调用另行编写的UART_Action(unsigned char *dat,unsigned char len)
*////////////////////////////////////////////////////////////////////////////////////
void UART_Driver()
{
	unsigned char pdata dat[64];       //定义数据暂存数组
	unsigned char len;                      //数据的长度
	if(UART_ResiveStringEndFlag)            //如果串口接收到一个完整的字符串
		{
			UART_ResiveStringEndFlag = 0;   //清空接收完成标志
			len = UART_Read(dat, UART_BUFF_MAX);  //将数据从原数组读出，并得到数据的长度
			UART_Action(dat, len);          //调用用户编写的UART_Action函数，将接收到的数据及数据长度作为参数
		}
}
/*///////////////////////////////////////////////////////////////////////////////////
*函数名：UART_RxMonitor
*函数功能：串口字符串接收结束判断，在定时器0中断函数中调用
*参数列表：
*   ms
*       参数类型：unsigned char型数据
*       参数描述：定时器延时时长（单位：ms）
*////////////////////////////////////////////////////////////////////////////////////
void UART_RxMonitor(unsigned char ms)
{
	static unsigned char ms30 = 0;                  //30毫秒计时
	static unsigned char UART_BuffIndex_Backup;     //串口数据暂存数组位置备份
	if(! UART_ResiveStringFlag)return ;             //如果当前没有在接受数据，直接退出函数
    ms30 += ms;                                     //每一次定时器中断，表示时间过去了若干毫秒
	if(UART_BuffIndex_Backup != UART_BuffIndex)     //如果串口数据暂存数组位置备份不等于串口接收缓冲区当前位置（接收到了新数据位）
	{
		UART_BuffIndex_Backup = UART_BuffIndex;     //记录当前的串口接收缓冲区位置
		ms30 = 0;                                   //复位30毫秒计时
	}
	if(ms30 > 30)                                   //30毫秒到了
		{
			ms30 = 0;                               //复位30毫秒计时
			UART_ResiveStringEndFlag = 1;           //设置串口字符串接收全部完成标志
			UART_ResiveStringFlag = 0;              //清空串口字符串正在接收标志
		}
}
/*///////////////////////////////////////////////////////////////////////////////////
*函数名：interrupt_UART
*函数功能：串口中断函数
*////////////////////////////////////////////////////////////////////////////////////
void interrupt_UART() interrupt 4
{
	if(TI)                                  //如果串口发送完成
	{
		TI = 0;                             //清空系统标志位
		UART_SendFlag = 1;                  //设置串口发送完成标志
	}
	if(RI)                                  //如果串口接收完成
	{
		RI = 0;                             //清空系统标志位
		UART_ResiveFlag = 1;                //设置串口接收完成标志
		UART_Buff[UART_BuffIndex] = SBUF;   //将接收到的数据放到暂存数组
		UART_ResiveStringFlag = 1;          //设置串口字符串正在接收标志
		UART_BuffIndex ++;                  //串口接收缓冲区当前位置右移
	}
}
