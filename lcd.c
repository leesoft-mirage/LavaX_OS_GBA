#include "define.h"

extern word graph_mode,bgcolor,fgcolor;

byte GRAY=0;

static byte palette[256*3];

const static byte syspalette[]={
	0x00,0x00,0x00,0xbf,0xbf,0xbf, //KEY调色
	0x00,0x00,0x00,0x00,0xff,0x00, //MESS调色
	0x00,0xc0,0x00,0x00,0x00,0x00, //黑绿2色
	0xff,0xff,0xff,0x00,0x00,0x00  //黑白2色
};

const byte lv9[]={0,0x20,0x40,0x60,0x80,0xa0,0xc0,0xe0,0xff};
const byte lv5[]={0,0x40,0x80,0xc0,0xff};

void calcPalette()
{
	int i;

	for (i=0;i<256;i++) {
		*(BGPAL_P+i)=((palette[i*3]>>3)<<10)|((palette[i*3+1]>>3)<<5)|(palette[i*3+2]>>3);
	}
}

void Palette2()
{
	if (GRAY) //黑白变换
		memcpy(palette,syspalette+18,6);
	else
		memcpy(palette,syspalette+12,6);
	calcPalette();
}

void Palette16()
{
	int i;

	for (i=0;i<16;i++) {
		palette[i*3]=palette[i*3+1]=palette[i*3+2]=(15-i)*0x11;
	}
	calcPalette();
}

void Palette256()
{
	int i;

	palette[0]=palette[1]=palette[2]=0;
	palette[255*3]=palette[255*3+1]=palette[255*3+2]=255;
	for (i=0;i<225;i++) {
		palette[i*3+48]=lv5[(i/5)%5];
		palette[i*3+49]=lv9[i/25];
		palette[i*3+50]=lv5[i%5];
	}
	calcPalette();
}

void SetPalette()
{
	palette[255*3]=palette[255*3+1]=palette[255*3+2]=0;
	if (graph_mode==4) Palette16();
	else if (graph_mode==8) Palette256();
	else Palette2();
}

void lav_setpalette(byte from,int num,byte *addr)
{
	int i;
	byte R,G,B;

	for (i=from;i<from+num;i++) {
		R=*addr++;
		G=*addr++;
		B=*addr++;
		addr++;
		*(BGPAL_P+i)=((B>>3)<<10)|((G>>3)<<5)|(R>>3);
	}
}

void Color256Init()
{
	REG_DISPCNT=0x404;
	graph_mode=8;
	bgcolor=0;
	fgcolor=255;
	SetPalette();
}

void Save_Palette()
{
	memcpy(task[task_lev].palette,palette,256*3);
}

void Load_Palette()
{
	memcpy(palette,task[task_lev].palette,256*3);
	calcPalette();
}
