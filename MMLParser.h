#ifndef YM_MMLP_H_INCLUDED
#define YM_MMLP_H_INCLUDED

#define		FLG_HALT		(1<<0)
#define		FLG_SYNCWAIT	(1<<1)
#define		FLG_SYNCWAITRUN	(1<<2)
#define		FLG_NKEYOFF		(1<<3)
#define		FLG_NEXTNKEYOFF	(1<<4)

#define		FLG_NEXTMPT			(1<<0)		// ポルタメント実行中
#define		FLG_NEXTFOUT		(1<<1)		// ボリュームフェード実行中
#define		FLG_MPT				(1<<4)		// ポルタメント実行中
#define		FLG_FOUT			(1<<5)		// ボリュームフェード実行中
#define		FLG_VLFO			(1<<6)		// ボリュームLFO実行中
#define		FLG_PLFO			(1<<7)		// ピッチLFO実行中

const	uint8_t	MMLParser_RepeatCnt=4;

struct RepeatFrame	{
	uint16_t	Addr;
	uint8_t		Count;
} ;

struct PLFOFrame	{
	uint32_t	OffsetStart;		//32->16
	uint32_t	DeltaStart;			//32->16
	uint32_t	Offset;				//32->16
	uint32_t	Delta;				//32->16
	uint16_t	LengthFixd;
	uint16_t	Length;
	uint16_t	LengthCounter;
	int8_t		Type;
};

struct VLFOFrame	{
	uint16_t	DeltaStart;
	uint16_t	DeltaFixd;
	uint16_t	Delta;
	uint16_t	Offset;
	uint16_t	Length;
	uint16_t	LengthCounter;
	int8_t		Type;
} ;
struct MMLParser{
	uint8_t		StatusF;
	uint8_t		FunctionF;
	uint8_t		Channel;
	uint16_t	MMLOffset;
	uint16_t	CurrentAddr;
	uint8_t		Clock;
	uint8_t		KeyOffClock;
	struct RepeatFrame	RepeatList[4];
	int8_t		KeyLength;
	
	uint32_t	Portamento;
	uint32_t	PortamentoDelta;
	uint16_t	Detune;
	uint8_t		Note;
	uint8_t		Volume;
	uint8_t		KeyOnDelay;
	uint8_t		KeyOnDelayClock;
	
	uint8_t		RegPAN;
	uint8_t		RegPMSAMS;
	struct PLFOFrame	PLFO;
	struct VLFOFrame	VLFO;
};

void		MMLParser_Init(struct MMLParser *mmlParser, uint8_t,uint16_t,uint16_t);
void		MMLParser_Elapse(struct MMLParser *mmlParser);
void		MMLParser_Calc(struct MMLParser *mmlParser);
void		MMLParser_SetTone(struct MMLParser *mmlParser);
void		MMLParser_KeyOn(struct MMLParser *mmlParser);
void		MMLParser_KeyOff(struct MMLParser *mmlParser);
void		MMLParser_UpdateVolume(struct MMLParser *mmlParser);

void		MMLParser_C_xx_Unknown(struct MMLParser *mmlParser);
void		MMLParser_C_e7_Fadeout(struct MMLParser *mmlParser);
void		MMLParser_C_e8_PCM8Ext(struct MMLParser *mmlParser);
void		MMLParser_C_e9_LFODelay(struct MMLParser *mmlParser);
void		MMLParser_C_ea_LFOCtrl(struct MMLParser *mmlParser);
void		MMLParser_C_eb_LFOVolumeCtrl(struct MMLParser *mmlParser);
void		MMLParser_C_ec_LFOPitchCtrl(struct MMLParser *mmlParser);
void		MMLParser_C_ed_NoisePitch(struct MMLParser *mmlParser);
void		MMLParser_C_ee_SyncWait(struct MMLParser *mmlParser);
void		MMLParser_C_ef_SyncSend(struct MMLParser *mmlParser);
void		MMLParser_C_f0_KeyOnDelay(struct MMLParser *mmlParser);
void		MMLParser_C_f1_EndOfData(struct MMLParser *mmlParser);
void		MMLParser_C_f2_Portamento(struct MMLParser *mmlParser);
void		MMLParser_C_f3_Detune(struct MMLParser *mmlParser);
void		MMLParser_C_f4_ExitRepeat(struct MMLParser *mmlParser);
void		MMLParser_C_f5_BottomRepeat(struct MMLParser *mmlParser);
void		MMLParser_C_f6_StartRepeat(struct MMLParser *mmlParser);
void		MMLParser_C_f7_DisableKeyOn(struct MMLParser *mmlParser);
void		MMLParser_C_f8_KeyOnTime(struct MMLParser *mmlParser);
void		MMLParser_C_f9_VolumeUp(struct MMLParser *mmlParser);
void		MMLParser_C_fa_VolumeDown(struct MMLParser *mmlParser);
void		MMLParser_C_fb_Volume(struct MMLParser *mmlParser);
void		MMLParser_C_fc_Panpot(struct MMLParser *mmlParser);
void		MMLParser_C_fd_Timbre(struct MMLParser *mmlParser);
void		MMLParser_C_fe_Registar(struct MMLParser *mmlParser);
void		MMLParser_C_ff_Tempo(struct MMLParser *mmlParser);

#endif  //YM_MMLP_H_INCLUDED
