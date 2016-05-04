#include <intrins.h>
#define uchar unsigned char
#define uint unsigned int
/*declare SFR associated with the IAP*/
sfr IAP_DATA   =0XE2;
sfr IAP_ADDRH  =0XE3;
sfr IAP_ADDRL  =0XE4;
sfr IAP_CMD    =0XE5;
sfr IAP_TRIG   =0XE6;
sfr IAP_CONTR  =0XE7;

/*Define ISP/IAP/EEPROM command*/
#define CMD_IDLE     0
#define CMD_READ     1
#define CMD_PROGRAM  2
#define CMD_ERASE    3

/*Define ISP/IAP/EEPROM operation const for IAP_CONTR*/
//#define  ENABLE_IAP  0X80 //<40mhz
#define  ENABLE_IAP    0X81 //<20mhz
//#define  ENABLE_IAP  0X82 //<10mhz
//#define  ENABLE_IAP  0X83 //5mhz

#define IAP_ADDRESS  0X02000


void iapidle()
{
	IAP_CONTR=0;
	IAP_CMD=0;
	IAP_TRIG=0;
	IAP_ADDRH=0X02;
	IAP_ADDRL=0;
}

/////////////////////////////////////////////
uchar iapReadByte(uint addr)
{
	uint dat;
	IAP_CONTR=ENABLE_IAP;
	IAP_CMD=CMD_READ;
	IAP_ADDRL=addr;
	IAP_ADDRH=addr>>8;
	IAP_TRIG=0X46;
	IAP_TRIG=0XB9;
	_nop_();
	dat=IAP_DATA;
	iapidle();

	return dat;
}

//////////////////////////////////////////
void  iapProgramByte(uint addr,uchar dat)
{
	IAP_CONTR=ENABLE_IAP;
	IAP_CMD=CMD_PROGRAM;
	IAP_ADDRL=addr;
	IAP_ADDRH=addr>>8;
	IAP_DATA=dat;
	IAP_TRIG=0X46;
	IAP_TRIG=0XB9;
	_nop_();

	iapidle();
}
///////////////////////////////////
void  iapEraseSector(uint addr)
{   
	IAP_CONTR=ENABLE_IAP;
	IAP_CMD=CMD_ERASE;
	IAP_ADDRL=addr;
	IAP_ADDRH=addr>>8;
	IAP_TRIG=0X46;
	IAP_TRIG=0XB9;
	_nop_();

	iapidle();
}
/////////////////////////////////////