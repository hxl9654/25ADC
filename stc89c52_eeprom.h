#ifndef _STC89C52_EEPROM_H_
#define _STC89C52_EEPROM_H_
void  iapEraseSector(unsigned int addr);
void  iapProgramByte(unsigned int addr,unsigned char dat);
unsigned char iapReadByte(unsigned int addr);
void iapidle();
#endif