#include "define.h"

extern void IrqServe();

int ScreenWidth=240,ScreenHeight=160; //可变的屏幕宽高

byte *BmpData=(byte*)(0x2000000);

volatile byte lav_key;
byte cur_keyb[128],old_keyb[128];
const byte key_code[]={0,0x14,0x15,0x17,0x16,0xd,0x1b,0x1c,0x1d,0x13,0x0e};
const word kb[]={0,64,128,32,16,1,2,4,8,512,256};

volatile byte Hz128;
long TickCount;
int s_year,s_month,s_day,s_hour,s_min,s_sec;

byte *pLAVA,*pNextLAVA;

int c_keyid(byte k)
{
	const static byte direct_key[]={1,2,4,3};
	if (k==0xd) k=5;
	else if (k==0x1b) k=6;
	else if (k>=20 && k<=23) k=direct_key[k-20];
	else k=0;
	return k;
}

byte c_keyval(byte k)
{
	return key_code[k];
}

void SetWindow()
{
	;
}

double sin(double x)
{
	return x;
}

double cos(double x)
{
	return x;
}

double tan(double x)
{
	return x;
}

double asin(double x)
{
	return x;
}

double acos(double x)
{
	return x;
}

double atan(double x)
{
	return x;
}

double sqrt(double x)
{
	return x;
}

double exp(double x)
{
	return x;
}

double log(double x)
{
	return x;
}

void IrqVBlankEnable()
{
	REG_DISPSTAT|=8;
	REG_IE|=1;
}

void IrqTimerEnable()
{
	REG_TM3D=0x10000-64;
	REG_TM3CNT=0xc3;
	REG_IE|=0x40;
}

void update_sec()
{
	s_sec++;
	if (s_sec>=60) s_sec=0;
	else return;
	s_min++;
	if (s_min>=60) s_min=0;
	else return;
	s_hour++;
	if (s_hour>=24) s_hour=0;
}

void GetKeyboardState(byte *k)
{
	word i,key;
	
	key=REG_P1;
	for (i=1;i<11;i++) {
		if (key & kb[i]) k[i]=0;
		else k[i]=1;
	}
}

void IrqServe0()
{
	word i;
	static int fps;

	if (REG_IF&0x40) { //256Hz
		TickCount++;
		Hz128++;
		if (Hz128==0) update_sec();
		if ((Hz128&7)==0) {
			memcpy(old_keyb,cur_keyb,sizeof(cur_keyb));
			GetKeyboardState(cur_keyb);
			if (lav_key<128) {
				for (i=1;i<11;i++) {
					if (cur_keyb[i]) {
						if (!old_keyb[i]) {
							lav_key=key_code[i]|0x80;
						}
					}
				}
			}
		}
		REG_IF=0x40;
	}
	if (REG_IF&1) { //VBlank
		if (fps&1) {
			REG_DM0SAD=(u32)BmpData;
			REG_DM0DAD=VRAM_START;
			REG_DM0CNT_L=240*160/4;
			REG_DM0CNT_H=0x8400;
		}
		REG_IF=1;
		fps++;
	}
}

void InitIrq()
{
	IRQ_ADDR=(u32)IrqServe;
	REG_IE=0;
	REG_IME=1;
}
