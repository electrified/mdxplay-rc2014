#ifndef YM2151_H_INCLUDED
#define YM2151_H_INCLUDED

#include <stdint.h>

void	YM2151_begin();
void	YM2151_initLFO();
uint8_t	YM2151_read();
void	YM2151_write(uint8_t addr,uint8_t data);

void	YM2151_setTone(uint8_t ch,uint8_t keycode,int16_t kf);
void	YM2151_setVolume(uint8_t ch,uint8_t volume,uint16_t offset);
void	YM2151_noteOn(uint8_t ch);
void	YM2151_noteOff(uint8_t ch);
void	YM2151_loadTimbre(uint8_t ch,uint16_t prog_addr);
void	YM2151_loadSeparationTimbre(uint8_t ch,uint16_t prog_addr);
void	YM2151_dumpTimbre(uint16_t prog_addr);
void	YM2151_setPanpot(uint8_t ch,uint8_t pan);
void	YM2151_wait(uint8_t loop);

const	uint8_t		YM2151_PIN_D0=2;
const	uint8_t		YM2151_PIN_D1=3;
const	uint8_t		YM2151_PIN_D2=4;
const	uint8_t		YM2151_PIN_D3=5;
const	uint8_t		YM2151_PIN_D4=6;
const	uint8_t		YM2151_PIN_D5=7;
const	uint8_t		YM2151_PIN_D6=8;
const	uint8_t		YM2151_PIN_D7=9;
const	uint8_t		YM2151_PIN_RD=10;
const	uint8_t		YM2151_PIN_WR=11;
const	uint8_t		YM2151_PIN_ADDR0=12;
const	uint8_t		YM2151_PIN_IC=13;

struct	YM2151{
	uint8_t		RegFLCON[8];
	uint8_t		RegSLOTMASK[8];
	uint8_t		CarrierSlot[8];
	uint8_t		RegTL[8][4];
};

// extern YM2151_Class YM2151;
#endif  //YM2151H_INCLUDED
