#include "Common.h"
#include "compat.h"
#include "MDXParser.h"

uint16_t	MDXDataBP;
uint16_t	MDXBaseOffset;
uint16_t	MDXTimbreOffset;
uint8_t		MDXTempo;
// uint32_t	MDXClockMicro;
struct MMLParser MDXOPMChannel[MDXParser_ChNum];

void MDXParser_Setup(uint16_t bp)
{
    uint16_t tableaddr = 0;
    MDXDataBP = bp;
    MDXParser_SetTempo(100);
    while (MDXParser_ReadData8(tableaddr++) != 0)
        ;
    MDXBaseOffset = tableaddr;
    MDXTimbreOffset = MDXParser_ReadData16(tableaddr);
    tableaddr += 2;
    for (int i = 0; i < MDXParser_ChNum; i++)
    {
        MMLParser_Init(&MDXOPMChannel[i], i, MDXBaseOffset, MDXParser_ReadData16(tableaddr));
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
        if (MDXOPMChannel[i].StatusF & (FLG_HALT | FLG_SYNCWAIT | FLG_SYNCWAITRUN))
            continue;
        if (MDXOPMChannel[i].Clock < c)
            ASSERT("Illegal clock");
        MDXOPMChannel[i].Clock -= c;
        if (MDXOPMChannel[i].KeyOnDelayClock > 0)
        {
            MDXOPMChannel[i].KeyOnDelayClock -= c;
            if (MDXOPMChannel[i].KeyOnDelayClock == 0)
            {
                MMLParser_KeyOn(&MDXOPMChannel[i]);
            }
        }
        MMLParser_Calc(&MDXOPMChannel[i]);
        if (MDXOPMChannel[i].KeyOffClock > 0)
        {
            MDXOPMChannel[i].KeyOffClock -= c;
            if (MDXOPMChannel[i].KeyOffClock == 0)
            {
                MMLParser_KeyOff(&MDXOPMChannel[i]);
            }
        }
        while (MDXOPMChannel[i].Clock == 0)
        {
            MMLParser_Elapse(&MDXOPMChannel[i]);
            if (MDXOPMChannel[i].StatusF & (FLG_HALT | FLG_SYNCWAIT | FLG_SYNCWAITRUN))
                break;
        }
    }
    for (int i = 0; i < MDXParser_ChNum; i++)
    {
        if (((1 << i) & cmask) == 0)
            continue;
        MDXOPMChannel[i].StatusF &= ~FLG_SYNCWAITRUN;
        if (MDXOPMChannel[i].Clock < minclock)
            minclock = MDXOPMChannel[i].Clock;
        if (MDXOPMChannel[i].KeyOffClock > 0 && MDXOPMChannel[i].KeyOffClock < minclock)
            minclock = MDXOPMChannel[i].KeyOffClock;
    }
    return minclock;
}

void MDXParser_SetTempo(uint8_t tempo)
{
    MDXTempo = tempo;
    YM2151_write(0x12, tempo);
    // MDXClockMicro = (1024 * (256 - MDXTempo)) / 4;
}
void MDXParser_SendSyncRelease(uint8_t ch)
{
    if (ch < MDXParser_ChNum)
    {
        MDXOPMChannel[ch].StatusF &= ~FLG_SYNCWAIT;
        while (MDXOPMChannel[ch].Clock == 0)
        {
            MMLParser_Elapse(&MDXOPMChannel[ch]);
        }
        MDXOPMChannel[ch].StatusF |= FLG_SYNCWAITRUN;
    }
}
uint16_t MDXParser_GetTimbreAddr(uint8_t timbleno)
{

    for (int i = 0; i < 256; i++)
    {
        uint16_t addr = MDXBaseOffset + MDXTimbreOffset + 27 * (uint16_t)i;
        uint8_t tno = MDXParser_ReadData8(addr);
        if (tno == timbleno)
            return addr;
    }
    ASSERT("can not find timbre");
    return 0;
}
// uint32_t MDXParser_ClockToMilliSec(uint8_t clock)
// {
//     return ((uint32_t)clock * MDXClockMicro) / (uint32_t)1000;
// }
// uint32_t MDXParser_ClockToMicroSec(uint8_t clock)
// {
//     return ((uint32_t)clock * MDXClockMicro);
// }

uint8_t MDXParser_ReadData8(uint16_t addr)
{
    return pgm_read_byte_near(MDXDataBP + (addr));
}

uint16_t MDXParser_ReadData16(uint16_t addr)
{
    // printf("addr %X\n", addr);
    uint16_t rdata = pgm_read_word_near(MDXDataBP + (addr));
    // printf("odata %X\n", rdata);
    rdata = (rdata << 8) | (rdata >> 8);
    // printf("rdata %X\n", rdata);
    return rdata; 
}