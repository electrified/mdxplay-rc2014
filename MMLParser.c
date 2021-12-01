#include	"Common.h"
#include	"MMLParser.h"
#include	"MDXParser.h"

// ※クロックの１単位は全音符／１９２
// typedef	void (MMLParser_ *CommandFunc)();
typedef void (*CommandFunc)(struct MMLParser *mmlParser);

static const CommandFunc CommandFuncTable[]={
	&MMLParser_C_xx_Unknown,
	&MMLParser_C_xx_Unknown,
	&MMLParser_C_xx_Unknown,
	&MMLParser_C_xx_Unknown,
	&MMLParser_C_xx_Unknown,
	&MMLParser_C_xx_Unknown,
	&MMLParser_C_xx_Unknown,
	&MMLParser_C_e7_Fadeout,
	&MMLParser_C_e8_PCM8Ext,
	&MMLParser_C_e9_LFODelay,
	&MMLParser_C_ea_LFOCtrl,
	&MMLParser_C_eb_LFOVolumeCtrl,
	&MMLParser_C_ec_LFOPitchCtrl,
	&MMLParser_C_ed_NoisePitch,
	&MMLParser_C_ee_SyncWait,
	&MMLParser_C_ef_SyncSend,
	&MMLParser_C_f0_KeyOnDelay,
	&MMLParser_C_f1_EndOfData,
	&MMLParser_C_f2_Portamento,
	&MMLParser_C_f3_Detune,
	&MMLParser_C_f4_ExitRepeat,
	&MMLParser_C_f5_BottomRepeat,
	&MMLParser_C_f6_StartRepeat,
	&MMLParser_C_f7_DisableKeyOn,
	&MMLParser_C_f8_KeyOnTime,
	&MMLParser_C_f9_VolumeUp,
	&MMLParser_C_fa_VolumeDown,
	&MMLParser_C_fb_Volume,
	&MMLParser_C_fc_Panpot,
	&MMLParser_C_fd_Timbre,
	&MMLParser_C_fe_Registar,
	&MMLParser_C_ff_Tempo,
};

void	MMLParser_Init(struct MMLParser *mmlParser, uint8_t ch,uint16_t base,uint16_t mmloffset){
	mmlParser->StatusF = 0;
	mmlParser->FunctionF = 0;
	mmlParser->CurrentAddr = base + mmloffset;
	mmlParser->Channel = ch;
	mmlParser->RegPAN = 0x3;
	mmlParser->KeyLength = 64;
	mmlParser->Volume = 0xf;
	mmlParser->VLFO.Offset = 0;
	mmlParser->PLFO.Offset = 0;
	mmlParser->Portamento = 0;
	for(int i=0;i<MMLParser_RepeatCnt;i++){
		mmlParser->RepeatList[i].Addr = 0;
	}
}
void	MMLParser_Elapse(struct MMLParser *mmlParser){
	uint8_t		command = MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr++);
	if(command < 0x80){ // 休符
		// [$00 ～ $7F] 長さはデータ値+1クロック
		mmlParser->Clock = command+1;
		mmlParser->KeyOffClock = 0;
		YM2151_write(0x08,0x00 + mmlParser->Channel);
		return;
	}
	if(command < 0xe0){ // 音符
		mmlParser->Note = command - 0x80;
		uint8_t		keytime = MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr++);
		// [$80 ～ $DF] + [クロック - 1]  音程は$80がo0d+ $DFがo8d  PCMﾊﾟｰﾄではﾃﾞｰﾀ番号
		mmlParser->Clock = keytime + 1;
		mmlParser->KeyOffClock = (uint8_t)(mmlParser->KeyOnDelay + (((uint16_t)mmlParser->Clock * (uint16_t)mmlParser->KeyLength) >> 6));	// 
		MMLParser_KeyOff(mmlParser);	//
		if(mmlParser->StatusF & FLG_NEXTNKEYOFF){
			mmlParser->StatusF |= FLG_NKEYOFF;
			mmlParser->StatusF &= ~FLG_NEXTNKEYOFF;
		} else {
			mmlParser->StatusF &= ~FLG_NKEYOFF;
		}
		if(mmlParser->KeyOnDelay==0){
			MMLParser_KeyOn(mmlParser);
		} else {
			mmlParser->KeyOnDelayClock = mmlParser->KeyOnDelay;
		}
		return;
	}
	CommandFuncTable[command-0xe0](mmlParser);
}

void	MMLParser_Calc(struct MMLParser *mmlParser){
	if(mmlParser->FunctionF & FLG_FOUT){
	}
	if(mmlParser->FunctionF & FLG_VLFO){
		uint16_t	Delta;
		switch(mmlParser->VLFO.Type){
			case	0:	// non op
				break;
			case	1:	// L001120
				mmlParser->VLFO.Offset += mmlParser->VLFO.Delta;
				if(--mmlParser->VLFO.LengthCounter == 0){
					mmlParser->VLFO.LengthCounter = mmlParser->VLFO.Length;
					mmlParser->VLFO.Offset = mmlParser->VLFO.DeltaFixd;
				}
				break;
			case	2:	// L001138
				if(--mmlParser->VLFO.LengthCounter == 0){
					mmlParser->VLFO.LengthCounter = mmlParser->VLFO.Length;
					mmlParser->VLFO.Offset += mmlParser->VLFO.Delta;
					mmlParser->VLFO.Delta = -mmlParser->VLFO.Delta;
				}
				break;
			case	3:	// L00114e
				mmlParser->VLFO.Offset += mmlParser->VLFO.Delta;
				if(--mmlParser->VLFO.LengthCounter == 0){
					mmlParser->VLFO.LengthCounter = mmlParser->VLFO.Length;
					mmlParser->VLFO.Delta = -mmlParser->VLFO.Delta;
				}
				break;
			case	4:	// L001164
				if(--mmlParser->VLFO.LengthCounter == 0){
					mmlParser->VLFO.LengthCounter = mmlParser->VLFO.Length;
					mmlParser->VLFO.Offset = mmlParser->VLFO.Delta * rand(255);
				}
				break;
			default:
				ASSERT("Unknown VLFO type!");
				break;
		}
		MMLParser_UpdateVolume(mmlParser);
	}
	if(mmlParser->FunctionF & FLG_PLFO){
		uint16_t	Delta;
		switch(mmlParser->PLFO.Type){
			case	0:	// non op
				break;
			case	1:	// L0010be
				mmlParser->PLFO.Offset += mmlParser->PLFO.Delta;
				if(--mmlParser->PLFO.LengthCounter == 0){
					mmlParser->PLFO.LengthCounter = mmlParser->PLFO.Length;
					mmlParser->PLFO.Offset = -mmlParser->PLFO.Offset;
				}
				break;
			case	2:	// L0010d4
				mmlParser->PLFO.Offset = mmlParser->PLFO.Delta;
				if(--mmlParser->PLFO.LengthCounter == 0){
					mmlParser->PLFO.LengthCounter = mmlParser->PLFO.Length;
					mmlParser->PLFO.Delta = -mmlParser->PLFO.Delta;
				}
				break;
			case	3:	// L0010ea
				mmlParser->PLFO.Offset += mmlParser->PLFO.Delta;
				if(--mmlParser->PLFO.LengthCounter == 0){
					mmlParser->PLFO.LengthCounter = mmlParser->PLFO.Length;
					mmlParser->PLFO.Delta = -mmlParser->PLFO.Delta;
				}
				break;
			case	4:	// L001100
				if(--mmlParser->PLFO.LengthCounter == 0){
					mmlParser->PLFO.LengthCounter = mmlParser->PLFO.Length;
					mmlParser->PLFO.Offset = mmlParser->PLFO.Delta * random(255);
				}
				break;
			default:
				ASSERT("Unknown PLFO type!");
				break;
			
		}
	}
	if((mmlParser->FunctionF & FLG_MPT) || (mmlParser->FunctionF & FLG_PLFO)){
		if(mmlParser->FunctionF & FLG_MPT){
			mmlParser->Portamento += mmlParser->PortamentoDelta;
		}
		MMLParser_SetTone(mmlParser);
	}

}

void	MMLParser_KeyOn(struct MMLParser *mmlParser){
	mmlParser->Portamento = 0;
	MMLParser_SetTone(mmlParser);
	YM2151_noteOn(mmlParser->Channel);
	//YM2151.write(0x08,(RegSLOTMASK << 3)  + Channel);
	mmlParser->FunctionF &= ~(FLG_MPT | FLG_FOUT);
	if(mmlParser->FunctionF & FLG_NEXTMPT){
		mmlParser->FunctionF |= FLG_MPT;
	}
	if(mmlParser->FunctionF & FLG_NEXTFOUT){
		mmlParser->FunctionF |= FLG_FOUT;
	}
	mmlParser->FunctionF &= ~(FLG_NEXTMPT | FLG_NEXTFOUT);
}

void	MMLParser_KeyOff(struct MMLParser *mmlParser){
	if((mmlParser->StatusF & FLG_NKEYOFF) == 0){
		YM2151_write(0x08,0x00 + mmlParser->Channel);
	} else {
		mmlParser->StatusF &= ~FLG_NKEYOFF;
	}
	mmlParser->KeyOffClock = 0;
}

void	MMLParser_SetTone(struct MMLParser *mmlParser){
	int16_t	offset;
	offset = mmlParser->Detune + 128;
	if(mmlParser->FunctionF & FLG_MPT){
		offset += mmlParser->Portamento>>16;	// 右シフトは問題なし
	}
	if(mmlParser->FunctionF & FLG_PLFO){
		offset += mmlParser->PLFO.Offset>>16;
	}
	YM2151_setTone(mmlParser->Channel,mmlParser->Note,offset);
	return;
}

void	MMLParser_UpdateVolume(struct MMLParser *mmlParser){
	YM2151_setVolume(mmlParser->Channel,mmlParser->Volume,mmlParser->VLFO.Offset);
}

//・未定義コマンド
void	MMLParser_C_xx_Unknown(struct MMLParser *mmlParser){
	ASSERT("Unknown MML Command.");
	mmlParser->StatusF |= FLG_HALT;
}
//・フェードアウト
//    [$E7] + [$01] + [SPEED]       $FOコマンド対応
void	MMLParser_C_e7_Fadeout(struct MMLParser *mmlParser){
	MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr++);
	MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr++);
}
//・PCM8拡張モード移行
//    [$E8]                         Achの頭で有効
void	MMLParser_C_e8_PCM8Ext(struct MMLParser *mmlParser){
}
//・LFOディレイ設定
//    [$E9] + [???]                 MDコマンド対応
void	MMLParser_C_e9_LFODelay(struct MMLParser *mmlParser){
	MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr++);
}
//・OPMLFO制御
//    [$EA] + [$80]                 MHOF
//    [$EA] + [$81]                 MHON
//    [$EA] + [SYNC/WAVE] + [LFRQ] + [PMD] + [AMD] + [PMS/AMS]
void	MMLParser_C_ea_LFOCtrl(struct MMLParser *mmlParser){
	uint8_t		lfocom = MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr++);
	if(lfocom & 0x80){
		if(lfocom & 0x01){		// ??
			YM2151_write(0x38+mmlParser->Channel,mmlParser->RegPMSAMS);
		} else {
			YM2151_write(0x38+mmlParser->Channel,0);
		}
		return;
	}
	YM2151_write(0x1b,lfocom);

	uint8_t		lfrq = MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr++);
	YM2151_write(0x18,lfrq);
	uint8_t		pmd = MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr++);
	YM2151_write(0x19,pmd);
	uint8_t		amd = MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr++);
	YM2151_write(0x19,amd);
	uint8_t		RegPMSAMS = MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr++);
	YM2151_write(0x38+mmlParser->Channel,RegPMSAMS);
}
//・音量LFO制御
//    [$EB] + [$80]                 MAOF
//    [$EB] + [$81]                 MAON
//    [$EB] + [WAVE※1] + [周期※2].w + [変移※4].w
void	MMLParser_C_eb_LFOVolumeCtrl(struct MMLParser *mmlParser){
	uint8_t		lfocom = MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr++);
	if(lfocom & 0x80){
		if(lfocom & 0x01){
			mmlParser->FunctionF |= FLG_VLFO;
		} else {
			mmlParser->FunctionF &= ~FLG_VLFO;
			mmlParser->VLFO.Offset = 0;
		}
		return;
	}
	int16_t	DeltaFixd;
	mmlParser->VLFO.Type = lfocom+1;
	mmlParser->VLFO.Length = MDXParser_ReadData16(&mdx, mmlParser->CurrentAddr);
	mmlParser->CurrentAddr+=2;
	DeltaFixd = mmlParser->VLFO.DeltaStart = MDXParser_ReadData16(&mdx, mmlParser->CurrentAddr);
	mmlParser->CurrentAddr+=2;
	if((lfocom & 1)==0){
		DeltaFixd *= (int16_t)mmlParser->VLFO.Length;
	}
	DeltaFixd = -DeltaFixd;
	if(DeltaFixd < 0)DeltaFixd = 0;
	mmlParser->VLFO.DeltaFixd = DeltaFixd;
	
	mmlParser->VLFO.LengthCounter = mmlParser->VLFO.Length;
	mmlParser->VLFO.Delta = mmlParser->VLFO.DeltaStart;
	mmlParser->VLFO.Offset = mmlParser->VLFO.DeltaFixd;
	
	mmlParser->FunctionF |= FLG_VLFO;
}
//・音程LFO制御
//    [$EC] + [$80]                 MPOF
//    [$EC] + [$81]                 MPON
//    [$EC] + [WAVE※1] + [周期※2].w + [変移※3].w
void	MMLParser_C_ec_LFOPitchCtrl(struct MMLParser *mmlParser){
	uint8_t		lfocom_f = MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr++);
	if(lfocom_f & 0x80){
		if(lfocom_f & 0x01){
			mmlParser->FunctionF |= FLG_PLFO;
		} else {
			mmlParser->FunctionF &= ~FLG_PLFO;
			mmlParser->PLFO.Offset = 0;
		}
		return;
	}
	uint8_t		lfocom = lfocom_f;
	lfocom &= 0x3;
	mmlParser->PLFO.Type = lfocom + 1;
	lfocom += lfocom;
	uint16_t	length = mmlParser->PLFO.Length = MDXParser_ReadData16(&mdx, mmlParser->CurrentAddr);mmlParser->CurrentAddr+=2;
	if(lfocom != 2){
		length >>= 1;
		if(lfocom == 6){length = 1;}
	}
	mmlParser->PLFO.LengthFixd = length;
	int16_t	delta = MDXParser_ReadData16(&mdx, mmlParser->CurrentAddr);mmlParser->CurrentAddr+=2;
	int16_t	delta_l = delta;
	if(lfocom_f >= 0x4){
		lfocom_f &= 0x3;
	} else {
		delta_l >>= 8;
	}

	mmlParser->PLFO.DeltaStart = delta_l;
	if(lfocom_f != 0x2) delta_l = 0;
	mmlParser->PLFO.OffsetStart = delta_l;

	mmlParser->PLFO.LengthCounter = mmlParser->PLFO.LengthFixd;
	mmlParser->PLFO.Delta = mmlParser->PLFO.DeltaStart;
	mmlParser->PLFO.Offset = mmlParser->PLFO.OffsetStart;

	mmlParser->FunctionF |= FLG_PLFO;
}



//・ADPCM/ノイズ周波数設定
//  チャンネルH
//    [$ED] + [???]                 ノイズ周波数設定。ビット7はノイズON/OFF
//  チャンネルP
//    [$ED] + [???]                 Fコマンド対応
void	MMLParser_C_ed_NoisePitch(struct MMLParser *mmlParser){
	uint8_t		noise = MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr++);
	YM2151_write(0x0f,noise);
}
//・同期信号待機
//    [$EE]
void	MMLParser_C_ee_SyncWait(struct MMLParser *mmlParser){
	mmlParser->StatusF |= FLG_SYNCWAIT;
}
//・同期信号送出
//    [$EF] + [チャネル番号(0～15)]
void	MMLParser_C_ef_SyncSend(struct MMLParser *mmlParser){
	uint8_t		sendch = MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr++);
	MDXParser_SendSyncRelease(&mdx, sendch);
}
//・キーオンディレイ
//    [$F0] + [???]                 kコマンド対応
void	MMLParser_C_f0_KeyOnDelay(struct MMLParser *mmlParser){
	mmlParser->KeyOnDelay = MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr++);
}
//・データエンド
//    [$F1] + [$00]                 演奏終了
//    [$F1] + [ループポインタ].w    ポインタ位置から再演奏
void	MMLParser_C_f1_EndOfData(struct MMLParser *mmlParser){
	uint8_t	off_u = (uint8_t)MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr++);
	if(off_u==0x0){
		mmlParser->StatusF |= FLG_HALT;
		MMLParser_KeyOff(mmlParser);
	} else {
		uint8_t	off_l = (uint8_t)MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr++);
		uint16_t	off=(uint16_t)off_u<< 8 | (uint16_t)off_l;
		off = (off ^ 0xffff)+1;
		mmlParser->CurrentAddr -= off;
	}
}
//・ポルタメント
//    [$F2] + [変移※3].w	          _コマンド対応   １単位は（半音／１６３８４）
// ※3  変移  1クロック毎の変化量。単位はデチューンの1/256
void	MMLParser_C_f2_Portamento(struct MMLParser *mmlParser){
	int16_t	port = MDXParser_ReadData16(&mdx, mmlParser->CurrentAddr);
	mmlParser->CurrentAddr+=2;
	mmlParser->PortamentoDelta = port;
	for(int i=0;i<8;i++){	// =<< 8 だけどArduinoの32bit演算libで挙動不審
		mmlParser->PortamentoDelta += mmlParser->PortamentoDelta;
	}
	mmlParser->Portamento = 0;
	mmlParser->FunctionF |= FLG_NEXTMPT;
}
//・デチューン
//    [$F3] + [???].w               Dコマンド対応   １単位は（半音／６４）
void	MMLParser_C_f3_Detune(struct MMLParser *mmlParser){
	int16_t	dt = MDXParser_ReadData16(&mdx, mmlParser->CurrentAddr);
	mmlParser->CurrentAddr+=2;
	mmlParser->Detune = dt;
}
//・リピート脱出
//    [$F4] + [終端コマンドへのオフセット+1].w
void	MMLParser_C_f4_ExitRepeat(struct MMLParser *mmlParser){
	int16_t	off = (int16_t)MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr);
	mmlParser->CurrentAddr += 2;
	int16_t	btmaddr = mmlParser->CurrentAddr+off;
	int16_t	btmoff = (int16_t)MDXParser_ReadData8(&mdx, btmaddr);
	btmaddr += 2;
	btmoff = (btmoff ^ 0xffff)+1;
	int16_t	tgtaddr = btmaddr-btmoff-1;
	uint8_t i;
	for(i=0;i<MMLParser_RepeatCnt;i++){
		if(mmlParser->RepeatList[i].Addr == tgtaddr)break;
	}
	if(i >= MMLParser_RepeatCnt) ASSERT("Address can not found.");
	if(mmlParser->RepeatList[i].Count==1){
		// break
		mmlParser->CurrentAddr = btmaddr;
		mmlParser->RepeatList[i].Addr = 0;
		mmlParser->RepeatList[i].Count = 0;
	}
}
//・リピート終端
//    [$F5] + [開始コマンドへのオフセット+2].w
void	MMLParser_C_f5_BottomRepeat(struct MMLParser *mmlParser){
	int16_t	off = (int16_t)MDXParser_ReadData16(&mdx, mmlParser->CurrentAddr);
	mmlParser->CurrentAddr += 2;
	off = (off ^ 0xffff)+1;
	int16_t	tgtaddr = mmlParser->CurrentAddr-off-1;
	uint8_t i;
	for(i=0;i<MMLParser_RepeatCnt;i++){
		if(mmlParser->RepeatList[i].Addr == tgtaddr)break;
	}
	if(i >= MMLParser_RepeatCnt) ASSERT("Address can not found.");
	
	mmlParser->RepeatList[i].Count--;
	if(mmlParser->RepeatList[i].Count>0){
		// repeat
		mmlParser->CurrentAddr -= off;
	} else {
		// exit repeat
		mmlParser->RepeatList[i].Addr = 0;
	}
}
//・リピート開始
//    [$F6] + [リピート回数] + [$00]
void	MMLParser_C_f6_StartRepeat(struct MMLParser *mmlParser){
	uint8_t	count = MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr);
	mmlParser->CurrentAddr+=2;
	uint8_t i;
	for(i=0;i<MMLParser_RepeatCnt;i++){
		if(mmlParser->RepeatList[i].Addr == 0)break;
	}
	if(i >= MMLParser_RepeatCnt) ASSERT("Repeat list overflow.");
	mmlParser->RepeatList[i].Addr = mmlParser->CurrentAddr-1;
	mmlParser->RepeatList[i].Count = count;
}
//・キーオフ無効
//    [$F7]                         次のNOTE発音後キーオフしない
// A6->S0016 |= 0x04;
void	MMLParser_C_f7_DisableKeyOn(struct MMLParser *mmlParser){
	mmlParser->StatusF |= FLG_NEXTNKEYOFF;
}
//・発音長指定
//    [$F8] + [$01～$08]            qコマンド対応
//    [$F8] + [$FF～$80]            @qコマンド対応（2の補数）
void	MMLParser_C_f8_KeyOnTime(struct MMLParser *mmlParser){
	// 全音長の<num>÷8にあたる時間が経過した時点でキーオフします。
	uint8_t		length = MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr++);
	if(length <= 8){
		mmlParser->KeyLength = length * 8;
	} else {
		mmlParser->KeyLength = length - 0x80;	// ??
	}
}
//・音量増大
//    [$F9]                         ※vコマンド後では、 v0  →   v15 へと変化
//                                  ※@vコマンド後では、@v0 → @v127 へと変化
void	MMLParser_C_f9_VolumeUp(struct MMLParser *mmlParser){
	if(mmlParser->Volume & 0x80){
		if(mmlParser->Volume > 0x80)mmlParser->Volume--;
	} else {
		if(mmlParser->Volume < 15)mmlParser->Volume++;
	}
	MMLParser_UpdateVolume(mmlParser);
}
//・音量減小
//    [$FA]                         ※vコマンド後では、 v15   →  v0 へと変化
//                                  ※@vコマンド後では、@v127 → @v0 へと変化
void	MMLParser_C_fa_VolumeDown(struct MMLParser *mmlParser){
	if(mmlParser->Volume & 0x80){
		if(mmlParser->Volume < 0xff)mmlParser->Volume++;
	} else {
		if(mmlParser->Volume > 0)mmlParser->Volume--;
	}
	MMLParser_UpdateVolume(mmlParser);
}
//・音量設定
//    [$FB] + [$00～$15]            vコマンド対応
//    [$FB] + [$80～$FF]            @vコマンド対応（ビット7無効）
void	MMLParser_C_fb_Volume(struct MMLParser *mmlParser){
	mmlParser->Volume = MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr++);
	MMLParser_UpdateVolume(mmlParser);
}
//・出力位相設定
//    [$FC] + [???]                 pコマンド対応
void	MMLParser_C_fc_Panpot(struct MMLParser *mmlParser){
	uint8_t		pan = MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr++);
	mmlParser->RegPAN = pan;
	YM2151_setPanpot(mmlParser->Channel,mmlParser->RegPAN);
}
//・音色設定
//    [$FD] + [???]                 @コマンド対応
#define	DUMP_TIMBRE
void	MMLParser_C_fd_Timbre(struct MMLParser *mmlParser){
	uint8_t		tno = MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr++);
	uint16_t	taddr = MDXParser_GetTimbreAddr(&mdx,tno);
	uint8_t	no = MDXParser_ReadData8(&mdx, taddr++);
	if(no!=tno)ASSERT("TimbreNo Unmuch");
	YM2151_loadTimbre(mmlParser->Channel,MDXParser_GetTimbreAddr(&mdx, tno) + mdx.DataBP);
	YM2151_setPanpot(mmlParser->Channel,mmlParser->RegPAN);
	MMLParser_UpdateVolume(mmlParser);
}

//・OPMレジスタ設定
//    [$FE] + [レジスタ番号] + [出力データ]
void	MMLParser_C_fe_Registar(struct MMLParser *mmlParser){
	uint8_t		reg = MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr++);
	uint8_t		data = MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr++);
	YM2151_write(reg,data);
}
//・テンポ設定
//    [$FF] + [???]                 @tコマンド対応
void	MMLParser_C_ff_Tempo(struct MMLParser *mmlParser){
	uint8_t		tempo = MDXParser_ReadData8(&mdx, mmlParser->CurrentAddr++);
	MDXParser_SetTempo(&mdx, tempo);
}
