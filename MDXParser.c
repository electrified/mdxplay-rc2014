#include	"Common.h"
#include	"MDXParser.h"

void		MDXParser_Setup(struct MDXParser *mdxParser,uint16_t bp)
{
	uint16_t	tableaddr=0;
	mdxParser->DataBP = bp;
	MDXParser_SetTempo(mdxParser, 100);
	while(MDXParser_ReadData8(mdxParser, tableaddr++)!=0);
	mdxParser->BaseOffset = tableaddr;
	mdxParser->TimbreOffset = MDXParser_ReadData16(mdxParser, tableaddr);
	tableaddr+=2;
	for(int i=0;i<MDXParser_ChNum;i++){
		MMLParser_Init(&mdxParser->OPMChannel[i], i,mdxParser->BaseOffset,MDXParser_ReadData16(mdxParser, tableaddr));
		tableaddr+=2;
	}
	YM2151_write(0x0f,0);


}
uint16_t	MDXParser_Elapse(struct MDXParser *mdxParser,uint16_t c){
	uint8_t		minclock=0xff;
	uint8_t		cmask=0xff;
	
	for(int i=0;i<MDXParser_ChNum;i++){
		if(((1<<i) & cmask)==0)continue;
		if(mdxParser->OPMChannel[i].StatusF & (FLG_HALT | FLG_SYNCWAIT | FLG_SYNCWAITRUN))continue;
		if(mdxParser->OPMChannel[i].Clock < c)ASSERT("Illegal clock");
		mdxParser->OPMChannel[i].Clock -= c;
		if(mdxParser->OPMChannel[i].KeyOnDelayClock > 0){
			mdxParser->OPMChannel[i].KeyOnDelayClock -= c;
			if(mdxParser->OPMChannel[i].KeyOnDelayClock==0){
				MMLParser_KeyOn(&mdxParser->OPMChannel[i]);
			}
		}
		MMLParser_Calc(&mdxParser->OPMChannel[i]);
		if(mdxParser->OPMChannel[i].KeyOffClock > 0){
			mdxParser->OPMChannel[i].KeyOffClock -= c;
			if(mdxParser->OPMChannel[i].KeyOffClock==0){
				MMLParser_KeyOff(&mdxParser->OPMChannel[i]);
			}
		}
		while(mdxParser->OPMChannel[i].Clock==0){
			MMLParser_Elapse(&mdxParser->OPMChannel[i]);
			if(mdxParser->OPMChannel[i].StatusF & (FLG_HALT | FLG_SYNCWAIT | FLG_SYNCWAITRUN))break;
		}
	}
	for(int i=0;i<MDXParser_ChNum;i++){
		if(((1<<i) & cmask)==0)continue;
		mdxParser->OPMChannel[i].StatusF &= ~FLG_SYNCWAITRUN;
		if(mdxParser->OPMChannel[i].Clock < minclock)minclock = mdxParser->OPMChannel[i].Clock;
		if(mdxParser->OPMChannel[i].KeyOffClock > 0 &&  mdxParser->OPMChannel[i].KeyOffClock < minclock)minclock = mdxParser->OPMChannel[i].KeyOffClock;
	}
	return minclock;
}
void		MDXParser_SetTempo(struct MDXParser *mdxParser,uint8_t tempo){
	mdxParser->Tempo = tempo;
	mdxParser->ClockMicro = (1024*(256-mdxParser->Tempo))/4;
}
void		MDXParser_SendSyncRelease(struct MDXParser *mdxParser,uint8_t ch){
	if(ch < MDXParser_ChNum){
		mdxParser->OPMChannel[ch].StatusF &= ~FLG_SYNCWAIT;
		while(mdxParser->OPMChannel[ch].Clock==0){
			MMLParser_Elapse(&mdxParser->OPMChannel[ch]);
		}
		mdxParser->OPMChannel[ch].StatusF |= FLG_SYNCWAITRUN;
	}
}
uint16_t	MDXParser_GetTimbreAddr(struct MDXParser *mdxParser,uint8_t timbleno){

	for(int i=0;i<256;i++){
		uint16_t	addr =  mdxParser->BaseOffset + mdxParser->TimbreOffset + 27 * (uint16_t)i;
		uint8_t		tno = MDXParser_ReadData8(mdxParser, addr);
		if(tno == timbleno) return addr;
	}
	ASSERT("can not find timbre");
	return 0;
}
uint32_t	MDXParser_ClockToMilliSec(struct MDXParser *mdxParser,uint8_t clock){
	return ((uint32_t)clock * mdxParser->ClockMicro)/(uint32_t)1000;
}
uint32_t	MDXParser_ClockToMicroSec(struct MDXParser *mdxParser,uint8_t clock){
	return ((uint32_t)clock * mdxParser->ClockMicro);
}

