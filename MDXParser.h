#ifndef _MDXParser_H
#define _MDXParser_H

#include	"MMLParser.h"

#define		MDXParser_ChNum 8

void		MDXParser_Setup(uint16_t bp) __z88dk_fastcall;
uint16_t	MDXParser_Elapse(uint16_t c) __z88dk_fastcall;
// uint16_t	MDXParser_Forward(uint16_t) __z88dk_fastcall;
void		MDXParser_SetTempo(uint8_t tempo) __z88dk_fastcall;
uint16_t	MDXParser_GetTimbreAddr(uint8_t timbleno) __z88dk_fastcall;
void		MDXParser_SendSyncRelease(uint8_t ch) __z88dk_fastcall;
// uint32_t	MDXParser_ClockToMilliSec(uint8_t clock) __z88dk_fastcall;
// uint32_t	MDXParser_ClockToMicroSec(uint8_t clock) __z88dk_fastcall;
uint8_t	    MDXParser_ReadData8(uint16_t addr) __z88dk_fastcall;
uint16_t    MDXParser_ReadData16(uint16_t addr) __z88dk_fastcall;

#endif