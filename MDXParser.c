#include "Common.h"
#include "compat.h"
#include "MDXParser.h"

extern struct MDXParser mdxParser;


void MDXParser_Setup(uint16_t bp)
{
    uint16_t tableaddr = 0;
    mdxParser.DataBP = bp;
    MDXParser_SetTempo(100);
    while (MDXParser_ReadData8(tableaddr++) != 0)
        ;
    mdxParser.BaseOffset = tableaddr;
    mdxParser.TimbreOffset = MDXParser_ReadData16(tableaddr);
    tableaddr += 2;
    for (int i = 0; i < MDXParser_ChNum; i++)
    {
        MMLParser_Init(&mdxParser.OPMChannel[i], i, mdxParser.BaseOffset, MDXParser_ReadData16(tableaddr));
        tableaddr += 2;
    }
    YM2151_write(0x0f, 0);
}
uint16_t MDXParser_Elapse(uint16_t c)
{
    uint8_t minclock = 0xff;
    uint8_t cmask = 0xff;

    for (int i = 0; i < MDXParser_ChNum; i++)
    {
        if (((1 << i) & cmask) == 0)
            continue;
        if (mdxParser.OPMChannel[i].StatusF & (FLG_HALT | FLG_SYNCWAIT | FLG_SYNCWAITRUN))
            continue;
        if (mdxParser.OPMChannel[i].Clock < c)
            ASSERT("Illegal clock");
        mdxParser.OPMChannel[i].Clock -= c;
        if (mdxParser.OPMChannel[i].KeyOnDelayClock > 0)
        {
            mdxParser.OPMChannel[i].KeyOnDelayClock -= c;
            if (mdxParser.OPMChannel[i].KeyOnDelayClock == 0)
            {
                MMLParser_KeyOn(&mdxParser.OPMChannel[i]);
            }
        }
        MMLParser_Calc(&mdxParser.OPMChannel[i]);
        if (mdxParser.OPMChannel[i].KeyOffClock > 0)
        {
            mdxParser.OPMChannel[i].KeyOffClock -= c;
            if (mdxParser.OPMChannel[i].KeyOffClock == 0)
            {
                MMLParser_KeyOff(&mdxParser.OPMChannel[i]);
            }
        }
        while (mdxParser.OPMChannel[i].Clock == 0)
        {
            MMLParser_Elapse(&mdxParser.OPMChannel[i]);
            if (mdxParser.OPMChannel[i].StatusF & (FLG_HALT | FLG_SYNCWAIT | FLG_SYNCWAITRUN))
                break;
        }
    }
    for (int i = 0; i < MDXParser_ChNum; i++)
    {
        if (((1 << i) & cmask) == 0)
            continue;
        mdxParser.OPMChannel[i].StatusF &= ~FLG_SYNCWAITRUN;
        if (mdxParser.OPMChannel[i].Clock < minclock)
            minclock = mdxParser.OPMChannel[i].Clock;
        if (mdxParser.OPMChannel[i].KeyOffClock > 0 && mdxParser.OPMChannel[i].KeyOffClock < minclock)
            minclock = mdxParser.OPMChannel[i].KeyOffClock;
    }
    return minclock;
}
void MDXParser_SetTempo(uint8_t tempo)
{
    mdxParser.Tempo = tempo;
    YM2151_write(0x12, tempo);
    // mdxParser.ClockMicro = (1024 * (256 - mdxParser.Tempo)) / 4;
}
void MDXParser_SendSyncRelease(uint8_t ch)
{
    if (ch < MDXParser_ChNum)
    {
        mdxParser.OPMChannel[ch].StatusF &= ~FLG_SYNCWAIT;
        while (mdxParser.OPMChannel[ch].Clock == 0)
        {
            MMLParser_Elapse(&mdxParser.OPMChannel[ch]);
        }
        mdxParser.OPMChannel[ch].StatusF |= FLG_SYNCWAITRUN;
    }
}
uint16_t MDXParser_GetTimbreAddr(uint8_t timbleno)
{

    for (int i = 0; i < 256; i++)
    {
        uint16_t addr = mdxParser.BaseOffset + mdxParser.TimbreOffset + 27 * (uint16_t)i;
        uint8_t tno = MDXParser_ReadData8(addr);
        if (tno == timbleno)
            return addr;
    }
    ASSERT("can not find timbre");
    return 0;
}
// uint32_t MDXParser_ClockToMilliSec(uint8_t clock)
// {
//     return ((uint32_t)clock * mdxParser.ClockMicro) / (uint32_t)1000;
// }
// uint32_t MDXParser_ClockToMicroSec(uint8_t clock)
// {
//     return ((uint32_t)clock * mdxParser.ClockMicro);
// }

uint8_t MDXParser_ReadData8(uint16_t addr)
{
    return (uint8_t)(pgm_read_byte_near(mdxParser.DataBP + (addr)));
}

uint16_t MDXParser_ReadData16(uint16_t addr)
{
    // printf("addr %X\n", addr);
    uint16_t rdata = pgm_read_word_near(mdxParser.DataBP + (addr));
    // printf("odata %X\n", rdata);
    rdata = (rdata << 8) | (rdata >> 8);
    // printf("rdata %X\n", rdata);
    return rdata; 
}