#ifndef _MDXParser_H
#define _MDXParser_H

#include	"MMLParser.h"

#define		MDXParser_ChNum 8

struct	MDXParser{
	uint16_t	DataBP;
	uint16_t	BaseOffset;
	uint16_t	TimbreOffset;
	uint8_t		Tempo;
	uint32_t	ClockMicro;
	struct MMLParser OPMChannel[MDXParser_ChNum];
};


void		MDXParser_Setup(struct MDXParser *mdxParser, uint16_t bp);
uint16_t	MDXParser_Elapse(struct MDXParser *mdxParser, uint16_t c);
// uint16_t	MDXParser_Forward(struct MDXParser *mdxParser, uint16_t);
void		MDXParser_SetTempo(struct MDXParser *mdxParser, uint8_t tempo);
uint16_t	MDXParser_GetTimbreAddr(struct MDXParser *mdxParser, uint8_t timbleno);
void		MDXParser_SendSyncRelease(struct MDXParser *mdxParser, uint8_t ch);
uint32_t	MDXParser_ClockToMilliSec(struct MDXParser *mdxParser, uint8_t clock);
uint32_t	MDXParser_ClockToMicroSec(struct MDXParser *mdxParser, uint8_t clock);
uint8_t	    MDXParser_ReadData8(struct MDXParser *mdxParser, uint16_t addr);
uint16_t    MDXParser_ReadData16(struct MDXParser *mdxParser, uint16_t addr);



extern struct	MDXParser	mdx;
#endif