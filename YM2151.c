/*
	YM2151 library	v0.13
	author:ISH
*/
#include "Common.h"
#include "YM2151.h"
#include "compat.h"

#define DIRECT_IO

#define REG_SEL_PORT 0xFEu
#define REG_DATA_PORT 0xFFu

// #ifdef __SDCC
__sfr __at REG_SEL_PORT REG_SEL;
__sfr __at REG_DATA_PORT REG_DATA;
// #else

// unsigned char REG_SEL;
// unsigned char REG_DATA;
// #endif

struct YM2151 ym2151;

#ifdef DIRECT_IO

static uint8_t last_write_addr = 0x00;

/*! 指定アドレスのレジスタに書き込みを行う
	\param addr		アドレス
	\param data		データ
 */
void YM2151_write(uint8_t addr, uint8_t data)
{
    uint8_t i;
    // addr 0x20へのアクセスの後busyフラグが落ちなくなる病 '86の石だと発生
    // 他のレジスタに書くまで落ちないので、強引だが0x20アクセス後ならチェックしない
    if (last_write_addr != 0x20)
    {
        for(i=0;i<32;i++){

			if(YM2151_read() & 0x80 == 0){	// Read Status
				break;
			}
			if(i>16){
				YM2151_wait(1);
			}
		}
    }
    printf("w: %02X %02X\n", addr, data); 
    REG_SEL = addr;
    REG_DATA = data;
    last_write_addr = addr;
}

/*! ステータスを読み込む、bit0のみ有効
 */
uint8_t YM2151_read()
{
    return REG_DATA;
}

/*! 約300nSec x loop分だけ待つ、あまり正確でない。
	\param loop		ループ数
 */
void YM2151_wait(uint8_t loop)
{
    uint8_t wi;
    for (wi = 0; wi < loop; wi++)
    {
        // 16MHz  nop = @60nSec
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
    }
}

/*! LFOを初期化する
 */
void YM2151_initLFO()
{
    YM2151_write(0x1, 0x1);
}

#endif

/*! MDX形式の音色データを読み込みレジスタにセットする、音色データセット後は
	ボリューム、パンポットの設定を必ず行い関連レジスタの整合性を取る。
	\param ch				設定するチャンネル
	\param prog_addr		プログラムアドレス
 */
void YM2151_loadTimbre(uint8_t ch, uint16_t prog_addr)
{
    static uint8_t carrier_slot_tbl[] = {
        0x08,
        0x08,
        0x08,
        0x08,
        0x0c,
        0x0e,
        0x0e,
        0x0f,
    };
    uint16_t taddr = prog_addr;
    uint8_t no = pgm_read_byte_near(taddr++);
    ym2151.RegFLCON[ch] = pgm_read_byte_near(taddr++);
    ym2151.CarrierSlot[ch] = carrier_slot_tbl[ym2151.RegFLCON[ch] & 0x7];
    ym2151.RegSLOTMASK[ch] = pgm_read_byte_near(taddr++);

    for (int i = 0; i < 32; i += 8)
    {
        uint8_t dt1_mul = pgm_read_byte_near(taddr++);
        YM2151_write(0x40 + ch + i, dt1_mul);
    }
    for (int i = 0; i < 4; i++)
    {
        ym2151.RegTL[ch][i] = pgm_read_byte_near(taddr++);
    }
    for (int i = 0; i < 32; i += 8)
    {
        uint8_t ks_ar = pgm_read_byte_near(taddr++);
        YM2151_write(0x80 + ch + i, ks_ar);
    }
    for (int i = 0; i < 32; i += 8)
    {
        uint8_t ame_d1r = pgm_read_byte_near(taddr++);
        YM2151_write(0xa0 + ch + i, ame_d1r);
    }
    for (int i = 0; i < 32; i += 8)
    {
        uint8_t dt2_d2r = pgm_read_byte_near(taddr++);
        YM2151_write(0xc0 + ch + i, dt2_d2r);
    }
    for (int i = 0; i < 32; i += 8)
    {
        uint8_t d1l_rr = pgm_read_byte_near(taddr++);
        YM2151_write(0xe0 + ch + i, d1l_rr);
    }
}

/*! 一般的なFM音色配列を読み込みレジスタにセットする、音色データセット後は
	ボリューム、パンポットの設定を必ず行い関連レジスタの整合性を取る。
	\param ch				設定するチャンネル
	\param prog_addr		プログラムアドレス
 */
void YM2151_loadSeparationTimbre(uint8_t ch, uint16_t prog_addr)
{
    // D2R(SR)が2151は5bit、DT2(追加デチューン)が無視されているので微妙に使いきれない
    // D2Rは<<1するとどうも変なのでそのまま
    static uint8_t carrier_slot_tbl[] = {
        0x08,
        0x08,
        0x08,
        0x08,
        0x0c,
        0x0e,
        0x0e,
        0x0f,
    };
    uint16_t taddr = prog_addr;
    uint8_t con = pgm_read_byte_near(taddr++);
    uint8_t fl = pgm_read_byte_near(taddr++);
    ym2151.RegFLCON[ch] = (fl << 3) | con;
    ym2151.CarrierSlot[ch] = carrier_slot_tbl[con];
    ym2151.RegSLOTMASK[ch] = 0xf;
    for (int i = 0; i < 4; i++)
    {
        uint8_t slotindex = i * 8 + ch;
        uint8_t ar = pgm_read_byte_near(taddr++);
        uint8_t dr = pgm_read_byte_near(taddr++);
        uint8_t sr = pgm_read_byte_near(taddr++);
        uint8_t rr = pgm_read_byte_near(taddr++);
        uint8_t sl = pgm_read_byte_near(taddr++);
        uint8_t ol = pgm_read_byte_near(taddr++);
        uint8_t ks = pgm_read_byte_near(taddr++);
        uint8_t ml = pgm_read_byte_near(taddr++);
        uint8_t dt1 = pgm_read_byte_near(taddr++);
        uint8_t ams = pgm_read_byte_near(taddr++);

        ym2151.RegTL[ch][i] = ol;
        YM2151_write(0x40 + slotindex, (dt1 << 4) | ml); // DT1 MUL
        YM2151_write(0x80 + slotindex, (ks << 6) | ar);  // KS AR
        YM2151_write(0xa0 + slotindex, (ams << 7) | dr); // AMS D1R
        YM2151_write(0xc0 + slotindex, 0 | (sr));        // DT2 D2R
        YM2151_write(0xe0 + slotindex, (sl << 4) | rr);  // D1L RR
    }
}

/*! MDX形式の音色データを読み込みコンソールにダンプする
	\param prog_addr		プログラムアドレス
 */
void YM2151_dumpTimbre(uint16_t prog_addr)
{
    uint16_t taddr = prog_addr;
    uint8_t no = pgm_read_byte_near(taddr++);
    uint8_t flcon = pgm_read_byte_near(taddr++);
    uint8_t slotmask = pgm_read_byte_near(taddr++);

    PRINTH("No:", no);
    PRINTH("RegFLCON:", flcon);
    PRINTH("RegSLOTMASK:", slotmask);

    for (int i = 0; i < 32; i += 8)
    {
        uint8_t dt1_mul = pgm_read_byte_near(taddr++);
        PRINTH("DT1_MUL:", dt1_mul);
    }
    for (int i = 0; i < 4; i++)
    {
        uint8_t tl = pgm_read_byte_near(taddr++);
        PRINTH("TL:", tl);
    }
    for (int i = 0; i < 32; i += 8)
    {
        uint8_t ks_ar = pgm_read_byte_near(taddr++);
        PRINTH("KS_AR:", ks_ar);
    }
    for (int i = 0; i < 32; i += 8)
    {
        uint8_t ame_d1r = pgm_read_byte_near(taddr++);
        PRINTH("AME_D1R:", ame_d1r);
    }
    for (int i = 0; i < 32; i += 8)
    {
        uint8_t dt2_d2r = pgm_read_byte_near(taddr++);
        PRINTH("DT2_D2R:", dt2_d2r);
    }
    for (int i = 0; i < 32; i += 8)
    {
        uint8_t d1l_rr = pgm_read_byte_near(taddr++);
        PRINTH("D1L_RR:", d1l_rr);
    }
}

/*! 音量を設定する、あくまでオペレーターのレベルを操作するだけなので
	音色と音量の組み合わせによっては破綻することがあります。
	\param ch				設定するチャンネル
	\param volume			ボリューム、0(最小)～15(最大) 最上位ビット(0x80)Onの場合は0x80をマスク後TLの値にそのまま加算
	\param offset			上位8bitをTLの値に加算、MDX再生用
 */
void YM2151_setVolume(uint8_t ch, uint8_t volume, uint16_t offset)
{
    static uint8_t volume_tbl[] = {
        0x2a,
        0x28,
        0x25,
        0x22,
        0x20,
        0x1d,
        0x1a,
        0x18,
        0x15,
        0x12,
        0x10,
        0x0d,
        0x0a,
        0x08,
        0x05,
        0x02,
    };
    int16_t tl, att;
    if (volume & (0x80))
    {
        tl = volume & 0x7f;
    }
    else
    {
        if (volume > 15)
            ASSERT("Illegal volume.");
        tl = volume_tbl[volume];
    }
    tl += offset >> 8;
    for (int i = 0; i < 4; i++)
    {
        if (ym2151.CarrierSlot[ch] & (1 << i))
        {
            att = ym2151.RegTL[ch][i] + tl;
        }
        else
        {
            att = ym2151.RegTL[ch][i];
        }
        if (att > 0x7f || att < 0)
            att = 0x7f;
        YM2151_write(0x60 + i * 8 + ch, att);
    }
}

/*! ノートオンする
	\param ch				オンするチャンネル
 */
void YM2151_noteOn(uint8_t ch)
{
    YM2151_write(0x08, (ym2151.RegSLOTMASK[ch] << 3) + ch);
}

/*! ノートオフする
	\param ch				オフするチャンネル
 */
void YM2151_noteOff(uint8_t ch)
{
    YM2151_write(0x08, 0x00 + ch);
}

const char KeyCodeTable[] = {
    0x00,
    0x01,
    0x02,
    0x04,
    0x05,
    0x06,
    0x08,
    0x09,
    0x0a,
    0x0c,
    0x0d,
    0x0e,
    0x10,
    0x11,
    0x12,
    0x14,
    0x15,
    0x16,
    0x18,
    0x19,
    0x1a,
    0x1c,
    0x1d,
    0x1e,
    0x20,
    0x21,
    0x22,
    0x24,
    0x25,
    0x26,
    0x28,
    0x29,
    0x2a,
    0x2c,
    0x2d,
    0x2e,
    0x30,
    0x31,
    0x32,
    0x34,
    0x35,
    0x36,
    0x38,
    0x39,
    0x3a,
    0x3c,
    0x3d,
    0x3e,
    0x40,
    0x41,
    0x42,
    0x44,
    0x45,
    0x46,
    0x48,
    0x49,
    0x4a,
    0x4c,
    0x4d,
    0x4e,
    0x50,
    0x51,
    0x52,
    0x54,
    0x55,
    0x56,
    0x58,
    0x59,
    0x5a,
    0x5c,
    0x5d,
    0x5e,
    0x60,
    0x61,
    0x62,
    0x64,
    0x65,
    0x66,
    0x68,
    0x69,
    0x6a,
    0x6c,
    0x6d,
    0x6e,
    0x70,
    0x71,
    0x72,
    0x74,
    0x75,
    0x76,
    0x78,
    0x79,
    0x7a,
    0x7c,
    0x7d,
    0x7e,
};

/*! 音程を設定する
	\param ch				設定するチャンネル
	\param keycode			オクターブ0のD#を0とした音階、D# E F F# G G# A A# B (オクターブ1) C C# D....と並ぶ
	\param kf				音階微調整、64で1音分上がる。
 */
void YM2151_setTone(uint8_t ch, uint8_t keycode, int16_t kf)
{
    int16_t offset_kf = (kf & 0x3f);
    int16_t offset_note = keycode + (kf >> 6);
    if (offset_note < 0)
        offset_note = 0;
    if (offset_note > 0xbf)
        offset_note = 0xbf;

    YM2151_write(0x30 + ch, offset_kf << 2);
    YM2151_write(0x28 + ch, pgm_read_byte_near(*(KeyCodeTable + offset_note)));
}
/*! パンポットを設定する
	\param ch				設定するチャンネル
	\param pan				パン設定、0:出力なし 1:左 2:右 3:両出力
 */
void YM2151_setPanpot(uint8_t ch, uint8_t pan)
{
    YM2151_write(0x20 + ch,
                 (pan << 6) | (ym2151.RegFLCON[ch]));
}
