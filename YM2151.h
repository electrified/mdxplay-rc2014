#ifndef YM2151_H_INCLUDED
#define YM2151_H_INCLUDED

#include <stdint.h>

void	YM2151_initLFO();
uint8_t	YM2151_read();
void	YM2151_write(uint8_t addr,uint8_t data);

void	YM2151_setTone(uint8_t ch,uint8_t keycode,int16_t kf);
void	YM2151_setVolume(uint8_t ch,uint8_t volume,uint16_t offset);
void	YM2151_noteOn(uint8_t ch) __z88dk_fastcall;
void	YM2151_noteOff(uint8_t ch) __z88dk_fastcall;
void	YM2151_loadTimbre(uint8_t ch,uint16_t prog_addr);
void	YM2151_loadSeparationTimbre(uint8_t ch,uint16_t prog_addr);
void	YM2151_dumpTimbre(uint16_t prog_addr) __z88dk_fastcall;
void	YM2151_setPanpot(uint8_t ch,uint8_t pan);
void	YM2151_wait(uint8_t loop) __z88dk_fastcall;

struct	YM2151{
	uint8_t		RegFLCON[8];
	uint8_t		RegSLOTMASK[8];
	uint8_t		CarrierSlot[8];
	uint8_t		RegTL[8][4];
};

#endif  //YM2151H_INCLUDED
