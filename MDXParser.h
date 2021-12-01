#ifndef _MDXParser_H
#define _MDXParser_H

#include	"MMLParser.h"

#define		MDXParser_ChNum 8

void		MDXParser_Setup(struct MDXParser *mdxParser, uint16_t);
uint16_t	MDXParser_Elapse(struct MDXParser *mdxParser, uint16_t);
uint16_t	MDXParser_Forward(struct MDXParser *mdxParser, uint16_t);
void		MDXParser_SetTempo(struct MDXParser *mdxParser, uint8_t);
uint16_t	MDXParser_GetTimbreAddr(struct MDXParser *mdxParser, uint8_t);
void		MDXParser_SendSyncRelease(struct MDXParser *mdxParser, uint8_t);
uint32_t	MDXParser_ClockToMilliSec(struct MDXParser *mdxParser, uint8_t clock);
uint32_t	MDXParser_ClockToMicroSec(struct MDXParser *mdxParser, uint8_t clock);

struct	MDXParser{
	uint16_t	DataBP;
	uint16_t	BaseOffset;
	uint16_t	TimbreOffset;
	uint8_t		Tempo;
	uint32_t	ClockMicro;
	struct MMLParser OPMChannel[MDXParser_ChNum];
};

uint8_t	MDXParser_ReadData8(struct MDXParser *mdxParser, uint16_t addr){
	return (uint8_t)0;//(pgm_read_byte_near(mdxParser->DataBP + (addr)));
}

uint16_t MDXParser_ReadData16(struct MDXParser *mdxParser, uint16_t addr){
	// uint16_t rdata = pgm_read_word_near(mdxParser->DataBP + (addr));
	return	 0;//(rdata << 8) | (rdata >> 8);
}

extern struct	MDXParser	mdx;
#endif