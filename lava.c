#include "define.h"

#define LTRUE 0xffffffff
#define LFALSE 0
#define NULL 0

#define CMDLINE	0x700

//register a32 REG9 asm ("r9"); //用来保留寄存器

/*extern double sin(double x);
extern double cos(double x);
extern double tan(double x);
extern double asin(double x);
extern double acos(double x);
extern double atan(double x);
extern double sqrt(double x);
extern double exp(double x);
extern double log(double x);
extern char *strchr(const char *string,int c);
extern char *strstr(const char *string1,const char *string2);*/

extern a32 WorkHandle;

a32 TextBuffer;

byte patbuf[0x400];
byte *ram_base;

const word sin90[]={
	0,18,36,54,71,89,107,125,
	143,160,178,195,213,230,248,265,
	282,299,316,333,350,367,384,400,
	416,433,449,465,481,496,512,527,
	543,558,573,587,602,616,630,644,
	658,672,685,698,711,724,737,749,
	761,773,784,796,807,818,828,839,
	849,859,868,878,887,896,904,912,
	920,928,935,943,949,956,962,968,
	974,979,984,989,994,998,1002,1005,
	1008,1011,1014,1016,1018,1020,1022,1023,
	1023,1024,1024
};

const byte crc1[256]={
	0x0,0x21,0x42,0x63,0x84,0xa5,0xc6,0xe7,
	0x8,0x29,0x4a,0x6b,0x8c,0xad,0xce,0xef,
	0x31,0x10,0x73,0x52,0xb5,0x94,0xf7,0xd6,
	0x39,0x18,0x7b,0x5a,0xbd,0x9c,0xff,0xde,
	0x62,0x43,0x20,0x1,0xe6,0xc7,0xa4,0x85,
	0x6a,0x4b,0x28,0x9,0xee,0xcf,0xac,0x8d,
	0x53,0x72,0x11,0x30,0xd7,0xf6,0x95,0xb4,
	0x5b,0x7a,0x19,0x38,0xdf,0xfe,0x9d,0xbc,
	0xc4,0xe5,0x86,0xa7,0x40,0x61,0x2,0x23,
	0xcc,0xed,0x8e,0xaf,0x48,0x69,0xa,0x2b,
	0xf5,0xd4,0xb7,0x96,0x71,0x50,0x33,0x12,
	0xfd,0xdc,0xbf,0x9e,0x79,0x58,0x3b,0x1a,
	0xa6,0x87,0xe4,0xc5,0x22,0x3,0x60,0x41,
	0xae,0x8f,0xec,0xcd,0x2a,0xb,0x68,0x49,
	0x97,0xb6,0xd5,0xf4,0x13,0x32,0x51,0x70,
	0x9f,0xbe,0xdd,0xfc,0x1b,0x3a,0x59,0x78,
	0x88,0xa9,0xca,0xeb,0xc,0x2d,0x4e,0x6f,
	0x80,0xa1,0xc2,0xe3,0x4,0x25,0x46,0x67,
	0xb9,0x98,0xfb,0xda,0x3d,0x1c,0x7f,0x5e,
	0xb1,0x90,0xf3,0xd2,0x35,0x14,0x77,0x56,
	0xea,0xcb,0xa8,0x89,0x6e,0x4f,0x2c,0xd,
	0xe2,0xc3,0xa0,0x81,0x66,0x47,0x24,0x5,
	0xdb,0xfa,0x99,0xb8,0x5f,0x7e,0x1d,0x3c,
	0xd3,0xf2,0x91,0xb0,0x57,0x76,0x15,0x34,
	0x4c,0x6d,0xe,0x2f,0xc8,0xe9,0x8a,0xab,
	0x44,0x65,0x6,0x27,0xc0,0xe1,0x82,0xa3,
	0x7d,0x5c,0x3f,0x1e,0xf9,0xd8,0xbb,0x9a,
	0x75,0x54,0x37,0x16,0xf1,0xd0,0xb3,0x92,
	0x2e,0xf,0x6c,0x4d,0xaa,0x8b,0xe8,0xc9,
	0x26,0x7,0x64,0x45,0xa2,0x83,0xe0,0xc1,
	0x1f,0x3e,0x5d,0x7c,0x9b,0xba,0xd9,0xf8,
	0x17,0x36,0x55,0x74,0x93,0xb2,0xd1,0xf0
};
const byte crc2[256]={
	0x0,0x10,0x20,0x30,0x40,0x50,0x60,0x70,
	0x81,0x91,0xa1,0xb1,0xc1,0xd1,0xe1,0xf1,
	0x12,0x2,0x32,0x22,0x52,0x42,0x72,0x62,
	0x93,0x83,0xb3,0xa3,0xd3,0xc3,0xf3,0xe3,
	0x24,0x34,0x4,0x14,0x64,0x74,0x44,0x54,
	0xa5,0xb5,0x85,0x95,0xe5,0xf5,0xc5,0xd5,
	0x36,0x26,0x16,0x6,0x76,0x66,0x56,0x46,
	0xb7,0xa7,0x97,0x87,0xf7,0xe7,0xd7,0xc7,
	0x48,0x58,0x68,0x78,0x8,0x18,0x28,0x38,
	0xc9,0xd9,0xe9,0xf9,0x89,0x99,0xa9,0xb9,
	0x5a,0x4a,0x7a,0x6a,0x1a,0xa,0x3a,0x2a,
	0xdb,0xcb,0xfb,0xeb,0x9b,0x8b,0xbb,0xab,
	0x6c,0x7c,0x4c,0x5c,0x2c,0x3c,0xc,0x1c,
	0xed,0xfd,0xcd,0xdd,0xad,0xbd,0x8d,0x9d,
	0x7e,0x6e,0x5e,0x4e,0x3e,0x2e,0x1e,0xe,
	0xff,0xef,0xdf,0xcf,0xbf,0xaf,0x9f,0x8f,
	0x91,0x81,0xb1,0xa1,0xd1,0xc1,0xf1,0xe1,
	0x10,0x0,0x30,0x20,0x50,0x40,0x70,0x60,
	0x83,0x93,0xa3,0xb3,0xc3,0xd3,0xe3,0xf3,
	0x2,0x12,0x22,0x32,0x42,0x52,0x62,0x72,
	0xb5,0xa5,0x95,0x85,0xf5,0xe5,0xd5,0xc5,
	0x34,0x24,0x14,0x4,0x74,0x64,0x54,0x44,
	0xa7,0xb7,0x87,0x97,0xe7,0xf7,0xc7,0xd7,
	0x26,0x36,0x6,0x16,0x66,0x76,0x46,0x56,
	0xd9,0xc9,0xf9,0xe9,0x99,0x89,0xb9,0xa9,
	0x58,0x48,0x78,0x68,0x18,0x8,0x38,0x28,
	0xcb,0xdb,0xeb,0xfb,0x8b,0x9b,0xab,0xbb,
	0x4a,0x5a,0x6a,0x7a,0xa,0x1a,0x2a,0x3a,
	0xfd,0xed,0xdd,0xcd,0xbd,0xad,0x9d,0x8d,
	0x7c,0x6c,0x5c,0x4c,0x3c,0x2c,0x1c,0xc,
	0xef,0xff,0xcf,0xdf,0xaf,0xbf,0x8f,0x9f,
	0x6e,0x7e,0x4e,0x5e,0x2e,0x3e,0xe,0x1e
};

byte *VRam;//[0xc0000];//[65536];
struct TASK task[/*16*/8]; //任务栈
int task_lev; //任务级
byte *lRam;
long a1,a3;
long seed;
a32 *bp;

byte no_buf,lcmd,negative;
word xx,yy,x0,Y0,x1,Y1;
a32 m1l;
word graph_mode,bgcolor,fgcolor;
byte scr_x,scr_y,scr_mode,curr_RPS,curr_CPR;
word scr_off;
byte small_up,small_down,small_left;
unsigned long line_mode;
byte func_x;
byte negative_tbl[256];

void setscreen(int mode);
int get_val();
void put_val(int t);

void get_bp()
{
	bp=(a32 *)(*(a32 *)(lRam+0x20))+2;
}

float i2f(int i)
{
	return *(float*)(&i);
}

int f2i(float f)
{
	return *(int *)(&f);
}

void make_negative()
{
	int i;
	byte t;
	for (i=0;i<256;i++) {
		t=0;
		if (i&0x80) t|=1;
		if (i&0x40) t|=2;
		if (i&0x20) t|=4;
		if (i&0x10) t|=8;
		if (i&0x8) t|=0x10;
		if (i&0x4) t|=0x20;
		if (i&0x2) t|=0x40;
		if (i&0x1) t|=0x80;
		negative_tbl[i]=t;
	}
}

void set_sys_ram()
{
	TextBuffer=		0x1400; //故意这样，以检验是否地址相关
}

void wait_no_key()
{
	int i,t;
	
	for (;;) {
		t=1;
		for (i=0;i<sizeof(cur_keyb);i++) {
			if (cur_keyb[i]) {
				t=0;
				break;
			}
		}
		if (t) break;
	}
}

void Adjust_Window(int width,int height)
{
	if (width!=ScreenWidth || height!=ScreenHeight) {
		ScreenWidth=width;
		ScreenHeight=height;
		SetWindow();
	}
}

void lavReset()
{
	word t1,t2;

	memset(lRam+0x2000,0,(VRam+MY_DATA_SIZE)-(lRam+0x2000));
	set_sys_ram();
	bgcolor=0;
	graph_mode=8;
	fgcolor=255;
	SetPalette();
	memset(cur_keyb,0,sizeof(cur_keyb));
	lav_key=0;
	//osd_clear(); TEST!!!
	Adjust_Window(240,160);
	line_mode=0;
	setscreen(0);
	seed+=Hz128;
	seed|=1;
	workdir_init();
	make_negative();
}

a32 TaskExit()
{
	int i;
	int ret_val;

	c_closeall(); //关闭当前任务所有打开的文件
	if (task_lev) {
		task_lev--;
		lRam=task[task_lev].lRam;
		//local_sp=task[task_lev].local_sp;
		//local_bp=task[task_lev].local_bp;
		//curr_file=task[task_lev].curr_file;
		//first_file=task[task_lev].first_file;
		//list_set=task[task_lev].list_set;
		//RamBits=task[task_lev].RamBits;
		graph_mode=task[task_lev].graph_mode;
		bgcolor=task[task_lev].bgcolor;
		fgcolor=task[task_lev].fgcolor;
		Load_Palette();
		if (ScreenWidth!=task[task_lev].ScreenWidth || ScreenHeight!=task[task_lev].ScreenHeight)
			Adjust_Window(task[task_lev].ScreenWidth,task[task_lev].ScreenHeight);
		//strcpy(CD,task[task_lev].CD); TEST!!!
		WorkHandle=task[task_lev].WorkHandle;
		set_sys_ram();
		//pLAVA=task[task_lev].pLAVA;
		//pNextLAVA=task[task_lev+1].pLAVA;
	}
	wait_no_key();
	lav_key=0;
	return (a32)lRam;
}

//------------------------------------------------------------------------------

void ByteAddr()
{
	m1l=yy*LCD_WIDTH+xx;
	if (!no_buf) m1l+=SCROLL_CON;
}

void put_dot()
{
	if (xx>=ScreenWidth) return; //在pc端允许操作最左列，毕竟不是所有LCD都带ICON的
	if (yy>=ScreenHeight) return;
	ByteAddr();
	if (graph_mode==1) {
		switch (lcmd) {
		case 0:
			BmpData[m1l]=0;
			break;
		case 1:
			BmpData[m1l]=1;
			break;
		case 2:
			BmpData[m1l]^=1;
			break;
		}
	} else if (graph_mode==4) {
		switch (lcmd) {
		case 0:
			BmpData[m1l]=(byte)bgcolor;
			break;
		case 1:
			BmpData[m1l]=(byte)fgcolor;
			break;
		case 2:
			BmpData[m1l]^=15;
			break;
		}
	} else {
		switch (lcmd) {
		case 0:
			BmpData[m1l]=(byte)bgcolor;
			break;
		case 1:
			BmpData[m1l]=(byte)fgcolor;
			break;
		case 2:
			BmpData[m1l]^=255;
			break;
		}
	}
}

word get_dot()
{
	ByteAddr();
	return BmpData[m1l];
}

void write_comm(word x,word y,word width,word height,byte *data)
{			
	int i;
	register int j;
	byte t;
	word widths;
	int x_bak;
	const static byte msktbl[]={128,64,32,16,8,4,2,1}; 
	byte *td;
	register byte *disp,*tt;
	int temp;

	x_bak=x;
	if (graph_mode==1) {
		widths=(width+7)>>3;
		if ((width&7)==0 && negative) {
			td=ram_base+MY_LOCAL_RAM;
			memcpy(td,data,widths*height);
			for (i=0;i<height;i++)
				for (j=0;j<widths/2;j++) {
					t=td[i*widths+j];
					td[i*widths+j]=td[i*widths+widths-1-j];
					td[i*widths+widths-1-j]=t;
				}
			for (i=0;i<widths*height;i++)
				td[i]=negative_tbl[td[i]];
		} else
			td=data;

		if (y>=ScreenHeight) {
			temp=0x10000-y;
			if (height<=temp) return; //全在画面外，不需要画
			height-=temp;
			td+=temp*widths;
			y=0;
		}
		if (y+height>ScreenHeight) height=ScreenHeight-y; //剪去出底屏部分
		if (x_bak>=ScreenWidth) {
			temp=0x10000-x_bak;
			if (width<=temp) return; //全在画面外，不需要画
			width-=temp;
			td+=temp>>3;
			if (temp&7) {
				width++;
				x_bak=0-(temp&7);
				if (width>ScreenWidth-x_bak) width=ScreenWidth-x_bak;
			} else {
				x_bak=0;
				if (width>ScreenWidth) width=ScreenWidth;
			}
		} else {
			if (x_bak+width>ScreenWidth) width=ScreenWidth-x_bak;
		}

		for (i=0;i<height;i++,y++) {
			if (x_bak<0) {
				j=-x_bak;
				x=0;
			} else {
				j=0;
				x=x_bak;
			}
			xx=x;yy=y;
			ByteAddr();
			disp=BmpData+m1l;
			tt=td;
			t=*tt++;
			if ((lcmd&8) || (lcmd&7)==2) {
				switch (lcmd&7) {
				case 3:
					while (j<width) {		
						if (!(t&msktbl[j&7])) *disp=1;
						disp++;
						if ((j&7)==7) t=*tt++;
						j++;	
					}
					break;
				case 4:
					while (j<width) {		
						if (t&msktbl[j&7]) *disp=0;
						disp++;
						if ((j&7)==7) t=*tt++;
						j++;	
					}
					break;
				case 5:
					while (j<width) {		
						if (!(t&msktbl[j&7])) *disp^=1;
						disp++;
						if ((j&7)==7) t=*tt++;
						j++;	
					}
					break;
				default:
					while (j<width) {		
						if (t&msktbl[j&7]) *disp=0;
						else *disp=1;
						disp++;
						if ((j&7)==7) t=*tt++;
						j++;	
					}
					break;
				}
			} else {
				switch (lcmd&7) {
					case 3:
					while (j<width) {		
						if (t&msktbl[j&7]) *disp=1;
						disp++;
						if ((j&7)==7) t=*tt++;
						j++;	
					}
					break;
				case 4:
					while (j<width) {		
						if (!(t&msktbl[j&7])) *disp=0;
						disp++;
						if ((j&7)==7) t=*tt++;
						j++;	
					}
					break;
				case 5:
					while (j<width) {		
						if (t&msktbl[j&7]) *disp^=1;
						disp++;
						if ((j&7)==7) t=*tt++;
						j++;	
					}
					break;
				default:
					while (j<width) {		
						if (t&msktbl[j&7]) *disp=1;
						else *disp=0;
						disp++;
						if ((j&7)==7) t=*tt++;
						j++;	
					}
					break;
				}
			}
			td+=widths;
		}
	} else if (graph_mode==4) {
		widths=(width+1)>>1;	
		if ((width&1)==0 && negative) {
			td=ram_base+MY_LOCAL_RAM;
			memcpy(td,data,widths*height);
			for (i=0;i<height;i++)
				for (j=0;j<widths/2;j++) {
					t=td[i*widths+j];
					td[i*widths+j]=td[i*widths+widths-1-j];
					td[i*widths+widths-1-j]=t;
				}
			for (i=0;i<widths*height;i++) {
				t=td[i];
				td[i]=(t<<4)|(t>>4);
			}
		} else
			td=data;
			
		if (y>=ScreenHeight) {
			temp=0x10000-y;
			if (height<=temp) return; //全在画面外，不需要画
			height-=temp;
			td+=temp*widths;
			y=0;
		}
		if (y+height>ScreenHeight) height=ScreenHeight-y; //剪去出底屏部分
		if (x_bak>=ScreenWidth) {
			temp=0x10000-x_bak;
			if (width<=temp) return; //全在画面外，不需要画
			width-=temp;
			td+=temp>>1;
			if (temp&1) {
				width++;
				x_bak=-1;
				if (width>ScreenWidth+1) width=ScreenWidth+1;
			} else {
				x_bak=0;
				if (width>ScreenWidth) width=ScreenWidth;
			}
		} else {
			if (x_bak+width>ScreenWidth) width=ScreenWidth-x_bak;
		}

		for (i=0;i<height;i++,y++) {
			if (x_bak==-1) {
				j=1;
				x=0;
			} else {
				j=0;
				x=x_bak;
			}
			xx=x;yy=y;
			ByteAddr();
			disp=BmpData+m1l;
			tt=td;
			t=*tt++;
			if ((lcmd&8) || (lcmd&7)==2) {
				switch (lcmd&7) {
				case 3:
					while (j<width) {		
						if (j&1) {
							*disp|=(t&0xf)^0xf;
							t=*tt++;
						} else *disp|=(t>>4)^0xf;
						disp++;
						j++;
					}
					break;
				case 4:
					while (j<width) {		
						if (j&1) {
							*disp&=(t&0xf)^0xf;
							t=*tt++;
						} else *disp&=(t>>4)^0xf;
						disp++;
						j++;
					}
					break;
				case 5:
					while (j<width) {		
						if (j&1) {
							*disp^=(t&0xf)^0xf;
							t=*tt++;
						} else *disp^=(t>>4)^0xf;
						disp++;
						j++;
					}
					break;													
				default:
					while (j<width) {		
						if (j&1) {
							*disp++=(t&0xf)^0xf;
							t=*tt++;
						} else *disp++=(t>>4)^0xf;
						j++;
					}
					break;
				}
			} else {
				switch (lcmd&7) {
				case 3:
					while (j<width) {		
						if (j&1) {
							*disp|=t&0xf;
							t=*tt++;
						} else *disp|=t>>4;
						disp++;
						j++;	
					}
					break;
				case 4:
					while (j<width) {		
						if (j&1) {
							*disp&=t&0xf;
							t=*tt++;
						} else *disp&=t>>4;
						disp++;
						j++;	
					}
					break;
				case 5:
					while (j<width) {		
						if (j&1) {
							*disp^=t&0xf;
							t=*tt++;
						} else *disp^=t>>4;
						disp++;
						j++;	
					}
					break;				
				default:
					while (j<width) {		
						if (j&1) {
							*disp++=t&0xf;
							t=*tt++;
						} else *disp++=t>>4;
						j++;
					}
				}
			}
			td+=widths;
		}
	} else {
		widths=width;	
		if (negative) {
			td=ram_base+MY_LOCAL_RAM;
			memcpy(td,data,widths*height);
			for (i=0;i<height;i++)
				for (j=0;j<widths/2;j++) {
					t=td[i*widths+j];
					td[i*widths+j]=td[i*widths+widths-1-j];
					td[i*widths+widths-1-j]=t;
				}
		} else
			td=data;
			
		if (y>=ScreenHeight) {
			temp=0x10000-y;
			if (height<=temp) return; //全在画面外，不需要画
			height-=temp;
			td+=temp*widths;
			y=0;
		}
		if (y+height>ScreenHeight) height=ScreenHeight-y; //剪去出底屏部分
		if (x_bak>=ScreenWidth) {
			temp=0x10000-x_bak;
			if (width<=temp) return; //全在画面外，不需要画
			width-=temp;
			td+=temp;
			x_bak=0;
		}
		if (x_bak+width>ScreenWidth) width=ScreenWidth-x_bak;

		if (lcmd==1) {
			xx=x_bak;yy=y;
			ByteAddr();
			disp=BmpData+m1l;
			j=height;
			while (j--) {
				memcpy(disp,td,width);
				td+=widths;
				disp+=LCD_WIDTH;
			}
			return;
		}
		for (i=0;i<height;i++,y++) {
			j=width;
			xx=x_bak;yy=y;
			ByteAddr();
			disp=BmpData+m1l;
			tt=td;
			if ((lcmd&8) || (lcmd&7)==2) {
				switch (lcmd&7) {
				case 3:
					while (j--) {
						*disp|=(*tt++)^0xff;
						disp++;
					}
					break;
				case 4:
					while (j--) {
						*disp&=(*tt++)^0xff;
						disp++;
					}
					break;
				case 5:
					while (j--) {
						*disp^=(*tt++)^0xff;
						disp++;
					}
					break;													
				default:
					while (j--) {
						*disp++=(*tt++)^0xff;
					}
					break;
				}
			} else {
				switch (lcmd&7) {
				case 3:
					while (j--) {
						*disp|=*tt++;
						disp++;
					}
					break;
				case 4:
					while (j--) {
						*disp&=*tt++;
						disp++;
					}
					break;
				case 5:
					while (j--) {
						*disp^=*tt++;
						disp++;
					}
					break;
				case 6:
					while (j--) {
						t=*tt++;
						if (t) *disp++=t;
						else disp++;
					}
					break;					
				default:
					/*while (j--) {
						*disp++=*tt++;
					}*/
					memcpy(disp,tt,j);
				}
			}
			td+=widths;
		}
	}
}

void font_6x12(word x,word y,byte c)
{
	byte *addr;
	byte t,t2;
	int i;
	if (graph_mode==1) {
		if (c>=128) memset(patbuf,0,12);
		else memcpy(patbuf,ascii+c*12,12);
	} else if (graph_mode==4) {
		if (c>=128) memset(patbuf,0,36);
		else {
			addr=ascii+c*12;
			for (i=0;i<12;i++) {
				t=*addr++;
				if (t&0x80) t2=fgcolor<<4;
				else t2=bgcolor<<4;
				if (t&0x40) t2|=fgcolor;
				else t2|=bgcolor;
				patbuf[i*3]=t2;
				if (t&0x20) t2=fgcolor<<4;
				else t2=bgcolor<<4;
				if (t&0x10) t2|=fgcolor;
				else t2|=bgcolor;
				patbuf[i*3+1]=t2;
				if (t&8) t2=fgcolor<<4;
				else t2=bgcolor<<4;
				if (t&4) t2|=fgcolor;
				else t2|=bgcolor;
				patbuf[i*3+2]=t2;
			}
		}
	} else {
		if (c>=128) memset(patbuf,0,72);
		else {
			addr=ascii+c*12;
			for (i=0;i<12;i++) {
				t=*addr++;
				if (t&0x80) patbuf[i*6]=(byte)fgcolor;
				else patbuf[i*6]=(byte)bgcolor;
				if (t&0x40) patbuf[i*6+1]=(byte)fgcolor;
				else patbuf[i*6+1]=(byte)bgcolor;
				if (t&0x20) patbuf[i*6+2]=(byte)fgcolor;
				else patbuf[i*6+2]=(byte)bgcolor;
				if (t&0x10) patbuf[i*6+3]=(byte)fgcolor;
				else patbuf[i*6+3]=(byte)bgcolor;
				if (t&0x8) patbuf[i*6+4]=(byte)fgcolor;
				else patbuf[i*6+4]=(byte)bgcolor;
				if (t&0x4) patbuf[i*6+5]=(byte)fgcolor;
				else patbuf[i*6+5]=(byte)bgcolor;
			}
		}
	}
	write_comm(x,y,6,12,patbuf);
}

void font_8x16(word x,word y,byte c)
{
	byte *addr;
	byte t,t2;
	int i;
	if (graph_mode==1) {
		if (c>=128) memset(patbuf,0,16);
		else memcpy(patbuf,ascii8+c*16,16);
	} else if (graph_mode==4) {
		if (c>=128) memset(patbuf,0,64);
		else {
			addr=ascii8+c*16;
			for (i=0;i<16;i++) {
				t=*addr++;
				if (t&0x80) t2=fgcolor<<4;
				else t2=bgcolor<<4;
				if (t&0x40) t2|=fgcolor;
				else t2|=bgcolor;
				patbuf[i*4]=t2;
				if (t&0x20) t2=fgcolor<<4;
				else t2=bgcolor<<4;
				if (t&0x10) t2|=fgcolor;
				else t2|=bgcolor;
				patbuf[i*4+1]=t2;
				if (t&8) t2=fgcolor<<4;
				else t2=bgcolor<<4;
				if (t&4) t2|=fgcolor;
				else t2|=bgcolor;
				patbuf[i*4+2]=t2;
				if (t&2) t2=fgcolor<<4;
				else t2=bgcolor<<4;
				if (t&1) t2|=fgcolor;
				else t2|=bgcolor;
				patbuf[i*4+3]=t2;
			}
		}
	} else {
		if (c>=128) memset(patbuf,0,128);
		else {
			addr=ascii8+c*16;
			for (i=0;i<16;i++) {
				t=*addr++;
				if (t&0x80) patbuf[i*8]=(byte)fgcolor;
				else patbuf[i*8]=(byte)bgcolor;
				if (t&0x40) patbuf[i*8+1]=(byte)fgcolor;
				else patbuf[i*8+1]=(byte)bgcolor;
				if (t&0x20) patbuf[i*8+2]=(byte)fgcolor;
				else patbuf[i*8+2]=(byte)bgcolor;
				if (t&0x10) patbuf[i*8+3]=(byte)fgcolor;
				else patbuf[i*8+3]=(byte)bgcolor;
				if (t&0x8) patbuf[i*8+4]=(byte)fgcolor;
				else patbuf[i*8+4]=(byte)bgcolor;
				if (t&0x4) patbuf[i*8+5]=(byte)fgcolor;
				else patbuf[i*8+5]=(byte)bgcolor;
				if (t&0x2) patbuf[i*8+6]=(byte)fgcolor;
				else patbuf[i*8+6]=(byte)bgcolor;
				if (t&0x1) patbuf[i*8+7]=(byte)fgcolor;
				else patbuf[i*8+7]=(byte)bgcolor;
			}
		}
	}
	write_comm(x,y,8,16,patbuf);
}

void font_12x12(word x,word y,byte c1,byte c2)
{
	byte *addr;
	byte t,t2;
	int i,j;
	if (c1<0xb0) addr=gbfont+(((c1-0xa1)*94+(c2-0xa1))*24);
	else addr=gbfont+(((c1-0xa7)*94+(c2-0xa1))*24);
	if (graph_mode==1) memcpy(patbuf,addr,24);
	else if (graph_mode==4) {
		for (i=0,j=0;i<24;i++) {
			t=*addr++;
			if (t&0x80) t2=fgcolor<<4;
			else t2=bgcolor<<4;
			if (t&0x40) t2|=fgcolor;
			else t2|=bgcolor;
			patbuf[j++]=t2;
			if (t&0x20) t2=fgcolor<<4;
			else t2=bgcolor<<4;
			if (t&0x10) t2|=fgcolor;
			else t2|=bgcolor;
			patbuf[j++]=t2;
			if (i&1) continue;
			if (t&8) t2=fgcolor<<4;
			else t2=bgcolor<<4;
			if (t&4) t2|=fgcolor;
			else t2|=bgcolor;
			patbuf[j++]=t2;
			if (t&2) t2=fgcolor<<4;
			else t2=bgcolor<<4;
			if (t&1) t2|=fgcolor;
			else t2|=bgcolor;
			patbuf[j++]=t2;
		}
	} else {
		for (i=0,j=0;i<24;i++) {
			t=*addr++;
			if (t&0x80) patbuf[j++]=(byte)fgcolor;
			else patbuf[j++]=(byte)bgcolor;
			if (t&0x40) patbuf[j++]=(byte)fgcolor;
			else patbuf[j++]=(byte)bgcolor;
			if (t&0x20) patbuf[j++]=(byte)fgcolor;
			else patbuf[j++]=(byte)bgcolor;
			if (t&0x10) patbuf[j++]=(byte)fgcolor;
			else patbuf[j++]=(byte)bgcolor;
			if (i&1) continue;
			if (t&0x8) patbuf[j++]=(byte)fgcolor;
			else patbuf[j++]=(byte)bgcolor;
			if (t&0x4) patbuf[j++]=(byte)fgcolor;
			else patbuf[j++]=(byte)bgcolor;
			if (t&0x2) patbuf[j++]=(byte)fgcolor;
			else patbuf[j++]=(byte)bgcolor;
			if (t&0x1) patbuf[j++]=(byte)fgcolor;
			else patbuf[j++]=(byte)bgcolor;
		}
	}
	write_comm(x,y,12,12,patbuf);
}

void font_16x16(word x,word y,byte c1,byte c2)
{
	byte *addr;
	byte t,t2;
	int i;
	
	if (c1<0xb0) addr=gbfont16+(((c1-0xa1)*94+(c2-0xa1))*32);
	else addr=gbfont16+(((c1-0xa7)*94+(c2-0xa1))*32);
	if (graph_mode==1) memcpy(patbuf,addr,32);
	else  if (graph_mode==4) {
		for (i=0;i<32;i++) {
			t=*addr++;
			if (t&0x80) t2=fgcolor<<4;
			else t2=bgcolor<<4;
			if (t&0x40) t2|=fgcolor;
			else t2|=bgcolor;
			patbuf[i*4]=t2;
			if (t&0x20) t2=fgcolor<<4;
			else t2=bgcolor<<4;
			if (t&0x10) t2|=fgcolor;
			else t2|=bgcolor;
			patbuf[i*4+1]=t2;
			if (t&8) t2=fgcolor<<4;
			else t2=bgcolor<<4;
			if (t&4) t2|=fgcolor;
			else t2|=bgcolor;
			patbuf[i*4+2]=t2;
			if (t&2) t2=fgcolor<<4;
			else t2=bgcolor<<4;
			if (t&1) t2|=fgcolor;
			else t2|=bgcolor;
			patbuf[i*4+3]=t2;
		}
	} else {
		for (i=0;i<32;i++) {
			t=*addr++;
			if (t&0x80) patbuf[i*8]=(byte)fgcolor;
			else patbuf[i*8]=(byte)bgcolor;
			if (t&0x40) patbuf[i*8+1]=(byte)fgcolor;
			else patbuf[i*8+1]=(byte)bgcolor;
			if (t&0x20) patbuf[i*8+2]=(byte)fgcolor;
			else patbuf[i*8+2]=(byte)bgcolor;
			if (t&0x10) patbuf[i*8+3]=(byte)fgcolor;
			else patbuf[i*8+3]=(byte)bgcolor;
			if (t&0x8) patbuf[i*8+4]=(byte)fgcolor;
			else patbuf[i*8+4]=(byte)bgcolor;
			if (t&0x4) patbuf[i*8+5]=(byte)fgcolor;
			else patbuf[i*8+5]=(byte)bgcolor;
			if (t&0x2) patbuf[i*8+6]=(byte)fgcolor;
			else patbuf[i*8+6]=(byte)bgcolor;
			if (t&0x1) patbuf[i*8+7]=(byte)fgcolor;
			else patbuf[i*8+7]=(byte)bgcolor;
		}
	}
	write_comm(x,y,16,16,patbuf);
}

void block_comm()
{
	get_bp();
	a1=bp[4];
	no_buf=a1&0x40;
	lcmd=a1&3;
	Y1=(word)bp[3];
	x1=(word)bp[2];
	Y0=(word)bp[1];
	x0=(word)bp[0];
}

void vline()
{
	xx=x0;
	if (Y0<Y1)
		for (yy=Y0;yy<=Y1;yy++) put_dot();
	else
		for (yy=Y1;yy<=Y0;yy++) put_dot();
}

void hline()
//调用前必须调整Y0,x0,x1使其满足：
//Y0<ScreenHeight x0<ScreenWidth x1<ScreenWidth x0<=x1
{
	word width;
	byte *p;
	yy=Y0;
	xx=x0;
	ByteAddr();
	width=x1-x0+1;
	p=BmpData+m1l;
	//for (xx=x0;xx<=x1;xx++) put_dot();
	if (graph_mode==1) {
		switch (lcmd) {
		case 0:
			memset(p,0,width);
			break;
		case 1:
			memset(p,1,width);
			break;
		case 2:
			while (width--) {
				*p^=1;
				p++;
			}
			break;
		}
	} else if (graph_mode==4) {
		switch (lcmd) {
		case 0:
			memset(p,(byte)bgcolor,width);
			break;
		case 1:
			memset(p,(byte)fgcolor,width);
			break;
		case 2:
			while (width--) {
				*p^=15;
				p++;
			}
			break;
		}
	} else {
		switch (lcmd) {
		case 0:
			memset(p,(byte)bgcolor,width);
			break;
		case 1:
			memset(p,(byte)fgcolor,width);
			break;
		case 2:
			while (width--) {
				*p^=255;
				p++;
			}
			break;
		}
	}
}

void block_check()
{
	word t;
	if (Y0>Y1) {
		t=Y0;
		Y0=Y1;
		Y1=t;
	}
	if (x0>x1) {
		t=x0;
		x0=x1;
		x1=t;
	}
	if (x0>=ScreenWidth) x0=ScreenWidth-1;
	if (x1>=ScreenWidth) x1=ScreenWidth-1;
	if (Y0>=ScreenHeight) Y0=ScreenHeight-1;
	if (Y1>=ScreenHeight) Y1=ScreenHeight-1;
}

int hline_check()
{
	word t;
	if (Y0>=ScreenHeight) return 0; //线在屏幕外
	if (x0>x1) {
		t=x0;
		x0=x1;
		x1=t;
	}
	if (x0>=ScreenWidth) return 0; //线在屏幕外
	if (x1>=ScreenWidth) x1=ScreenWidth-1;
	return 1;
}

void block_draw()
{
	word t;
	block_check();
	t=Y1-Y0+1;
	while (t) {
		hline();
		Y0++;
		t--;
	}
}

void square_draw()
{
	word t;
	block_check();
	hline();
	vline();
	t=x0;
	x0=x1;
	vline();
	x0=t;
	Y0=Y1;
	hline();
}

void scr_up()
{
	memcpy(lRam+TextBuffer,lRam+TextBuffer+curr_CPR,curr_CPR*(curr_RPS-1));
	memset(lRam+TextBuffer+curr_CPR*(curr_RPS-1),0x20,curr_CPR);
	scr_off=curr_CPR*(curr_RPS-1);
	scr_x=0;
	scr_y=curr_RPS-1;
}

void cout(byte c)
{
	if (scr_y>=curr_RPS) scr_up();
	if (c==0x0d) return;
	if (c==0x0a) {
		scr_x=0;
		scr_y++;
		if (scr_y>=curr_RPS) scr_up();
		else scr_off=scr_y*curr_CPR;
		return;
	}
	if (c==9) c=0x20;
	lRam[TextBuffer+scr_off++]=c;
	if (++scr_x>=curr_CPR) {
		scr_x=0;
		scr_y++;
	}
}

void update_lcd_small() //Test 没有考虑反显
{
	byte c,c2;
	unsigned long mask;
	int i,j,color;
	if (curr_RPS<=8) mask=0x80;
	else if (curr_RPS<=16) mask=0x8000;
	else mask=0x800000;
	no_buf=0x40;
	lcmd=0;
	if (graph_mode==1) color=0;
	else color=bgcolor;
	for (i=0;i<small_up;i++)
		memset(BmpData+i*LCD_WIDTH,color,LCD_WIDTH);
	for (i=ScreenHeight-small_down;i<ScreenHeight;i++)
		memset(BmpData+i*LCD_WIDTH,color,LCD_WIDTH);
	for (i=1;i<curr_RPS;i++)
		memset(BmpData+(i*13+small_up-1)*LCD_WIDTH,color,LCD_WIDTH);
	Y0=0;Y1=ScreenHeight-1;
	for (i=0;i<small_left;i++) {
		x0=i;vline();
		x0=ScreenWidth-1-i;vline();
	}
	lcmd=1;
	for (i=0;i<curr_RPS;i++,mask>>=1) {
		if (mask&line_mode) continue;
		for (j=0;j<curr_CPR;j++) {
			c=lRam[TextBuffer+i*curr_CPR+j];
			if (c<128) font_6x12((word)(j*6+small_left),(word)(i*13+small_up),c);
			else {
				c2=lRam[TextBuffer+i*curr_CPR+j+1];
				font_12x12((word)(j*6+small_left),(word)(i*13+small_up),c,c2);
				j++;
			}
		}
	}
}

void update_lcd_large() //Test 没有考虑反显
{
	byte c,c2;
	unsigned long mask;
	int i,j;
	if (curr_RPS<=8) mask=0x80;
	else if (curr_RPS<=16) mask=0x8000;
	else mask=0x800000;
	no_buf=0x40;
	lcmd=1;
	for (i=0;i<curr_RPS;i++,mask>>=1) {
		if (mask&line_mode) continue;
		for (j=0;j<curr_CPR;j++) {
			c=lRam[TextBuffer+i*curr_CPR+j];
			if (c<128) font_8x16((word)(j<<3),(word)(i<<4),c);
			else {
				c2=lRam[TextBuffer+i*curr_CPR+j+1];
				font_16x16((word)(j<<3),(word)(i<<4),c,c2);
				j++;
			}
		}
	}
}

void update_lcd_0()
{
	negative=0;
	if (scr_mode==0) update_lcd_large();
	else update_lcd_small();
}

void next_arg()
{
	func_x++;
	a1=bp[func_x];
}

void printint(int digit,int flag)
{
	char num[20];
	int i,real_digit;

	next_arg();
	sprintf(num,"%ld",a1);
	real_digit=strlen(num);
	if (digit==0 || real_digit>=digit) {
		for (i=0;;i++) {
			if (num[i]) cout(num[i]);
			else break;
		}
	} else {
		if (flag==0x80) {
			for (i=0;;i++) {
				if (num[i]) cout(num[i]);
				else break;
			}
			for (i=0;i<digit-real_digit;i++) {
				cout(' ');
			}
		} else if (flag==0x40) {
			for (i=0;i<digit-real_digit;i++) {
				cout('0');
			}
			for (i=0;;i++) {
				if (num[i]) cout(num[i]);
				else break;
			}
		} else {
			for (i=0;i<digit-real_digit;i++) {
				cout(' ');
			}
			for (i=0;;i++) {
				if (num[i]) cout(num[i]);
				else break;
			}
		}
	}
}

void printfloat()
{
	char num[20];
	int i,flag;
	next_arg();
	if (((a1>>23)&0xff)==0xff) {
		strcpy(num,"error");
	} else
		sprintf(num,"%g",i2f(a1));
	flag=0;
	for (i=0;;i++) {
		if (num[i]=='e' && i) flag=1;
		if (flag) flag++;
		if (flag==4) continue; //转换x.xxxxxe+0yy为x.xxxxxe+yy
		if (num[i]) cout(num[i]);
		else break;
	}
}

void PutPixelx(int x,int y)
{
	if (x<0 || y<0 || x>=ScreenWidth || y>=ScreenHeight) return;
	xx=x;yy=y;
	put_dot();
}

int circle_buf[320];

void put_dot4(int x0,int y0,int x,int y,int mode)
{
	if (mode) {
		if (y0-y>=0 && y0-y<ScreenHeight)
			if (x>=circle_buf[y0-y]) circle_buf[y0-y]=x;
		if (y0+y>=0 && y0+y<ScreenHeight)
			if (x>=circle_buf[y0+y]) circle_buf[y0+y]=x;	
	} else {
		if (x==0) {
			PutPixelx(x0,y0+y);
			PutPixelx(x0,y0-y);
		} else if (y==0) {
			PutPixelx(x0+x,y0);
			PutPixelx(x0-x,y0);
		} else {
			PutPixelx(x0+x,y0+y);
			PutPixelx(x0-x,y0+y);
			PutPixelx(x0+x,y0-y);
			PutPixelx(x0-x,y0-y);
		}
	}
}

void Ellipsex(short x0,short y0,word r1,word r2,int mode)
{
	int i,j,fxy,fx,fy,incx,incy,temp_x,temp_y;
	word delta_x,delta_y,distant_a,distant_b,circle_r,dot_start;

	if (mode) for (i=0;i<ScreenHeight;i++) circle_buf[i]=-1;
	distant_a=r1;
	distant_b=r2;
	dot_start=0;
	if (distant_a==0 && distant_a==0) {
		PutPixelx(x0,y0);
		return;
	}
	circle_r=(distant_a>distant_b)?distant_a:distant_b;
	incx=-1;
	incy=1;
	fy=1;
	fx=1-2*circle_r;
	fxy=0;
	delta_x=0;
	delta_y=0;
	temp_x=distant_a;
	temp_y=0;
	put_dot4(x0,y0,temp_x,temp_y,mode);
	do {
	if (fxy>=0) {
		delta_x+=distant_a;
		if (delta_x>=circle_r) {
			temp_x+=incx;
			delta_x-=circle_r;
			if (temp_x+1!=distant_a)
				put_dot4(x0,y0,temp_x,temp_y,mode);
		}
		fxy-=abs(fx);
		fx+=2;
		if (fx<0 || fx>=3) continue;
		incy=-incy;
		fy=-fy+2;
		fxy=-fxy;
	} else {
		delta_y+=distant_b;
		if (delta_y>=circle_r) {
			delta_y-=circle_r;
			temp_y+=incy;
			if ((temp_y==1 || temp_y==2) && dot_start==0) {
				put_dot4(x0,y0,distant_a,temp_y,mode);
			} else {
				dot_start=1;
				put_dot4(x0,y0,temp_x,temp_y,mode);
			}
		}
		fxy=fxy+abs(fy);
		fy+=2;
		if (fy<0 || fy>2) continue;
		incx=-incx;
		fx=-fx+2;
		fxy=-fxy;
	}
	} while (temp_x);
	if (mode) {
		for (i=0;i<ScreenHeight;i++) {
			if (circle_buf[i]>=0) {
				for (j=0;j<circle_buf[i]*2+1;j++) PutPixelx(x0-circle_buf[i]+j,i);
			}
		}
	}
}

void c_putchar()
{
	get_bp();
	cout((byte)bp[0]);
	update_lcd_0();
}

a32 fenxi_fmt(a32 s,int *digit,int *flag)
{
	byte c;
	int have_0,have_fu,total_wei;

	have_0=0;
	have_fu=0;
	total_wei=0;
	for (;;) {
		c=lRam[s++];
		if (c=='0') have_0=1;
		else if (c=='-') have_fu=1;
		else if (c>'0' && c<='9') {
			total_wei=c-'0';
			for (;;) {
				c=lRam[s++];
				if (c>='0' && c<='9') {
					total_wei=total_wei*10+c-'0';
				} else {
					s--;
					break;
				}
			}
		} else {
			s--;
			break;
		}
	}
	*digit=total_wei;
	if (total_wei) {
		if (have_fu) *flag=0x80;
		else if (have_0) *flag=0x40;
	} else *flag=0;
	return s;
}

void c_printf()
{
	a32 fmt_str,str2;
	byte c;
	int digit,flag;
	
	get_bp();
	func_x=1;
	fmt_str=bp[1];
	for (;;) {
		c=lRam[fmt_str++];
		if (c==0) break;
		if (c=='%') {
			fmt_str=fenxi_fmt(fmt_str,&digit,&flag);
			c=lRam[fmt_str++];
			if (c==0) break;
			if (c=='d') printint(digit,flag);
			else if (c=='f') printfloat();
			else if (c=='%') cout(c);
			else if (c=='c') {
				next_arg();
				cout((byte)a1);
			} else if (c=='s') {
				next_arg();
				str2=a1;
				for (;;) {
					c=lRam[str2++];
					if (c==0) break;
					if (c<128) cout(c);
					else {
						if (scr_x+1>=curr_CPR) cout(0x20);
						cout(c);
						c=lRam[str2++];
						if (c==0) break;
						cout(c);
					}
				}
			} else cout(c);
		} else if (c<128) {
			cout(c);
		} else {
			if (scr_x+1>=curr_CPR) cout(0x20);
			cout(c);
			c=lRam[fmt_str++];
			if (c==0) break;
			cout(c);
		}
	}
	update_lcd_0();
}

void c_strcpy()
{
	get_bp();
	strcpy(lRam+bp[0],lRam+bp[1]);
}

a32 c_strlen()
{
	get_bp();
	return strlen(lRam+bp[0]);
}

void setscreen(int mode)
{
	if (mode) {
		scr_mode=1;
		curr_CPR=((ScreenWidth-2)/6)&0xfe;
		curr_RPS=(ScreenHeight-1)/13;
		small_left=(ScreenWidth-curr_CPR*6)/2;
		small_up=(ScreenHeight-(curr_RPS*13-1))/2;
		small_down=ScreenHeight-(curr_RPS*13-1)-small_up;
	} else {
		scr_mode=0;
		curr_CPR=ScreenWidth/8;
		curr_RPS=ScreenHeight/16;
	}
	scr_x=0;scr_y=0;scr_off=0;
	memset(lRam+TextBuffer,0,curr_CPR*curr_RPS);
}

void c_setscreen()
{
	get_bp();
	setscreen(bp[0]&0xff);
}

void c_updatelcd()
{
	;
}

void c_writeblock()
{
	a32 data;
	word x,y,width,height;

	get_bp();
	data=bp[5];
	a1=bp[4];
	no_buf=a1&0x40;
	negative=a1&0x20;
	lcmd=a1&0xf;
	height=(word)bp[3];
	width=(word)bp[2];
	y=(word)bp[1];
	x=(word)bp[0];
	write_comm(x,y,width,height,lRam+data);
}

void scroll_to_lcd()
{
	memcpy(BmpData,BmpData+SCROLL_CON,LCD_WIDTH*LCD_HEIGHT);
}

void c_textout()
{
	byte t,c;
	word font_x,font_y;
	a32 addr;

	get_bp();
	a1=bp[3];
	no_buf=a1&0x40;
	negative=a1&0x20;
	lcmd=a1&0xf;
	t=(byte)a1;
	addr=bp[2];
	font_y=(word)bp[1];
	font_x=(word)bp[0];
	if ((t&0x80)==0) {
		for (;;) {
			if (font_x>=ScreenWidth) break;
			c=lRam[addr++];
			if (c==0) break;
			if (c<128) {
				font_6x12(font_x,font_y,c);
				font_x+=6;
			} else {
				t=lRam[addr++];
				font_12x12(font_x,font_y,c,t);
				font_x+=12;
			}
		}
	} else {
		for (;;) {
			if (font_x>=ScreenWidth) break;
			c=lRam[addr++];
			if (c==0) break;
			if (c<128) {
				font_8x16(font_x,font_y,c);
				font_x+=8;
			} else {
				t=lRam[addr++];
				font_16x16(font_x,font_y,c,t);
				font_x+=16;
			}
		}
	}
}

void c_block()
{
	block_comm();
	block_draw();
}

void c_rectangle()
{
	block_comm();
	square_draw();
}

void c_clearscreen()
{	
	if (graph_mode==1)
		memset(BmpData+SCROLL_CON,0,LCD_WIDTH*LCD_HEIGHT);
	else {
		memset(BmpData+SCROLL_CON,bgcolor,LCD_WIDTH*LCD_HEIGHT);
	}
}

a32 c_abs()
{
	get_bp();
	a1=bp[0];
	if (a1<0) a1=0-a1;
	return a1;
}

a32 c_rand()
{
	a1=seed*0x15a4e35+1;
	seed=a1;
	a1=(a1>>16)&0x7fff;
	return a1;
}

void c_srand()
{
	get_bp();
	seed=bp[0];
}

void c_locate()
{
	byte t;

	get_bp();
	t=(byte)bp[1];
	if (t>=curr_CPR) t=curr_CPR-1;
	scr_x=t;
	t=(byte)bp[0];
	if (t>=curr_RPS) t=curr_RPS-1;
	scr_y=t;
	scr_off=scr_y*curr_CPR+scr_x;
}

void c_point()
{
	get_bp();
	a1=bp[2];
	no_buf=(a1&0x40)^0x40;
	lcmd=a1&3;
	yy=(word)bp[1];
	xx=(word)bp[0];
	put_dot();
}

a32 c_getpoint()
{
	get_bp();
	yy=(word)bp[1];
	xx=(word)bp[0];
	no_buf=0x40;
	return get_dot();
}

void c_line()
{
	word delta_x,delta_y,distance,tt,xerr,yerr;
	word t;
	int incy;

	block_comm();
	no_buf^=0x40;
	if (x0==x1) {
		vline();
		return;
	}
	if (Y0==Y1) {
		if (hline_check())
			hline();
		return;
	}
	if (x1<x0) {
		t=x0;
		x0=x1;
		x1=t;
		t=Y0;
		Y0=Y1;
		Y1=t;
	}
	delta_x=x1-x0;
	delta_y=abs(Y1-Y0);
	if (Y1>Y0) incy=1;
	else incy=-1;
	distance=(delta_x>delta_y)?delta_x:delta_y;
	tt=0;
	xerr=0;
	yerr=0;
	xx=x0;
	yy=Y0;
	for (;;) {
		put_dot();
		xerr+=delta_x;
		yerr+=delta_y;
		if (xerr>=distance) {
			xerr-=distance;
			xx++;
		}
		if (yerr>=distance) {
			yerr-=distance;
			yy+=incy;
		}
		tt++;
		if (distance<tt) break;
	}
}

void c_box()
{
	byte t;

	get_bp();
	a1=bp[5];
	lcmd=a1&3;
	no_buf=0x40;
	t=(byte)bp[4];
	Y1=(word)bp[3];
	x1=(word)bp[2];
	Y0=(word)bp[1];
	x0=(word)bp[0];
	if (t) block_draw();
	else square_draw();
}

void XorLine(int s,int e)
{
	x0=0;x1=ScreenWidth-1;
	Y0=s;Y1=e;
	lcmd=2;no_buf=0x40;
	block_draw();
}

void c_circle()
{
	word mode,r,x,y;

	get_bp();
	a1=bp[4];
	no_buf=(a1&0x40)^0x40;
	lcmd=a1&3;
	mode=(word)bp[3];
	r=(word)bp[2];
	y=(word)bp[1];
	x=(word)bp[0];
	Ellipsex(x,y,r,r,mode);
}

void c_ellipse()
{
	word mode,ra,rb,x,y;

	get_bp();
	a1=bp[5];
	no_buf=(a1&0x40)^0x40;
	lcmd=a1&3;
	mode=(word)bp[4];
	rb=(word)bp[3];
	ra=(word)bp[2];
	y=(word)bp[1];
	x=(word)bp[0];
	Ellipsex(x,y,ra,rb,mode);
}

void c_beep()
{
	;
}

a32 c_isalnum()
{
	get_bp();
	a1=isalnum(bp[0]);
	if (a1) a1=LTRUE;
	return a1;
}

a32 c_isalpha()
{
	get_bp();
	a1=isalpha(bp[0]);
	if (a1) a1=LTRUE;
	return a1;
}

a32 c_iscntrl()
{
	get_bp();
	a1=iscntrl(bp[0]);
	if (a1) a1=LTRUE;
	return a1;
}

a32 c_isdigit()
{
	get_bp();
	a1=isdigit(bp[0]);
	if (a1) a1=LTRUE;
	return a1;
}

a32 c_isgraph()
{
	get_bp();
	a1=isgraph(bp[0]);
	if (a1) a1=LTRUE;
	return a1;
}

a32 c_islower()
{
	get_bp();
	a1=islower(bp[0]);
	if (a1) a1=LTRUE;
	return a1;
}

a32 c_isprint()
{
	get_bp();
	a1=isprint(bp[0]);
	if (a1) a1=LTRUE;
	return a1;
}

a32 c_ispunct()
{
	get_bp();
	a1=ispunct(bp[0]);
	if (a1) a1=LTRUE;
	return a1;
}

a32 c_isspace()
{
	get_bp();
	a1=isspace(bp[0]);
	if (a1) a1=LTRUE;
	return a1;
}

a32 c_isupper()
{
	get_bp();
	a1=isupper(bp[0]);
	if (a1) a1=LTRUE;
	return a1;
}

a32 c_isxdigit()
{
	get_bp();
	a1=isxdigit(bp[0]);
	if (a1) a1=LTRUE;
	return a1;
}

void c_strcat()
{
	get_bp();
	strcat(lRam+bp[0],lRam+bp[1]);
}

a32 c_strchr()
{
	byte *p;

	get_bp();
	p=(byte *)strchr((char *)lRam+bp[0],(byte)bp[1]);
	if (p==NULL) a1=0;
	else a1=p-lRam;
	return a1;
}

a32 c_strcmp()
{
	int t;

	get_bp();
	t=strcmp(lRam+bp[0],lRam+bp[1]);
	if (t==0) a1=0;
	else if (t>0) a1=1;
	else a1=-1;
	return a1;
}

a32 c_strstr()
{
	byte *p;

	get_bp();
	p=(byte *)strstr((char *)lRam+bp[0],(char *)lRam+bp[1]);
	if (p==NULL) a1=0;
	else a1=p-lRam;
	return a1;
}

a32 c_tolower()
{
	byte c;

	get_bp();
	c=(byte)bp[0];
	a1=tolower(c);
	return a1;
}

a32 c_toupper()
{
	byte c;

	get_bp();
	c=(byte)bp[0];
	a1=toupper(c);
	return a1;
}

void c_memset()
{
	a32 len;

	get_bp();
	len=bp[2];
	if (len==0) return;
	memset(lRam+bp[0],(word)bp[1],len);
}

void c_memcpy()
{
	a32 len;

	get_bp();
	len=bp[2];
	if (len==0) return;
	memcpy(lRam+bp[0],lRam+bp[1],len);
}

void c_sprintf()
{
	a32 str1,fmt_str,str2;
	byte c;
	char num[20];
	int i,digit,flag,real_digit;
	
	get_bp();
	func_x=1;
	str1=bp[1];
	next_arg();
	fmt_str=a1;
	for (;;) {
		c=lRam[fmt_str++];
		if (c==0) break;
		if (c=='%') {
			fmt_str=fenxi_fmt(fmt_str,&digit,&flag);
			c=lRam[fmt_str++];
			if (c==0) break;
			if (c=='d') {
				next_arg();
				sprintf(num,"%ld",a1);
				real_digit=strlen(num);
				if (digit==0 || real_digit>=digit) {
					for (i=0;;i++) {
						if (num[i]) lRam[str1++]=num[i];
						else break;
					}
				} else {
					if (flag==0x80) {
						for (i=0;;i++) {
							if (num[i]) lRam[str1++]=num[i];
							else break;
						}
						for (i=0;i<digit-real_digit;i++) {
							lRam[str1++]=' ';
						}
					} else if (flag==0x40) {
						for (i=0;i<digit-real_digit;i++) {
							lRam[str1++]='0';
						}
						for (i=0;;i++) {
							if (num[i]) lRam[str1++]=num[i];
							else break;
						}
					} else {
						for (i=0;i<digit-real_digit;i++) {
							lRam[str1++]=' ';
						}
						for (i=0;;i++) {
							if (num[i]) lRam[str1++]=num[i];
							else break;
						}
					}
				}
			} else if (c=='f') {
				int flag;
				next_arg();
				if (((a1>>23)&0xff)==0xff) {
					strcpy(num,"error");
				} else
					sprintf(num,"%g",i2f(a1));
				flag=0;
				for (i=0;;i++) {
					if (num[i]=='e' && i) flag=1;
					if (flag) flag++;
					if (flag==4) continue; //转换x.xxxxxe+0yy为x.xxxxxe+yy
					if (num[i]) lRam[str1++]=num[i];
					else break;
				}
			} else if (c=='%') lRam[str1++]=c;
			else if (c=='c') {
				next_arg();
				lRam[str1++]=(byte)a1;
			} else if (c=='s') {
				next_arg();
				str2=a1;
				for (;;) {
					c=lRam[str2++];
					if (c==0) break;
					lRam[str1++]=c;
				}
			} else lRam[str1++]=c;
		} else {
			lRam[str1++]=c;
		}
	}
	lRam[str1]=0;
}

void c_memmove()
{
	a32 len;

	get_bp();
	len=bp[2];
	if (len==0) return;
	memmove(lRam+bp[0],lRam+bp[1],len);
}

a32 c_crc16()
{
	a32 t1,t2;
	byte c1,c2,x;

	get_bp();
	t1=bp[0];
	t2=bp[1];
	c1=0;c2=0;
	while (t2--) {
		x=c2^lRam[t1++];
		c2=crc2[x]^c1;
		c1=crc1[x];
	}
	a1=c1+(c2<<8);
	return a1;
}

void c_jiami()
{
	a32 str1,str2,len;
	int i;
	byte c;

	get_bp();
	str2=bp[2];
	str1=bp[0];
	len=bp[1];
	i=0;
	while (len--) {
		c=lRam[str2+i++];
		if (c==0) {
			c=lRam[str2];
			i=1;
		}
		lRam[str1]^=c;
		str1++;
	}
}

void c_xdraw()
{
	byte t;
	byte tb[320];
	int i,j;

	get_bp();
	t=(byte)bp[0];
	if (graph_mode==1) {
		if (t==0) {
			j=0;
			for (i=0;i<ScreenHeight;i++) {
				memcpy(BmpData+SCROLL_CON+j,BmpData+SCROLL_CON+j+1,ScreenWidth-1);
				BmpData[SCROLL_CON+j+ScreenWidth-1]=0;
				j+=LCD_WIDTH;
			}
		} else if (t==1) {
			j=0;
			for (i=0;i<ScreenHeight;i++) {
				memmove(BmpData+SCROLL_CON+j+1,BmpData+SCROLL_CON+j,ScreenWidth-1);
				BmpData[SCROLL_CON+j]=0;
				j+=LCD_WIDTH;
			}
		} else if (t==2) {
			memcpy(BmpData+SCROLL_CON,BmpData+SCROLL_CON+LCD_WIDTH,LCD_WIDTH*(ScreenHeight-1));
			memset(BmpData+SCROLL_CON+LCD_WIDTH*(ScreenHeight-1),0,LCD_WIDTH);
		} else if (t==3) {
			memmove(BmpData+SCROLL_CON+LCD_WIDTH,BmpData+SCROLL_CON,LCD_WIDTH*(ScreenHeight-1));
			memset(BmpData+SCROLL_CON,0,LCD_WIDTH);
		} else if (t==4) {
			for (i=0;i<ScreenHeight;i++)
				for (j=0;j<ScreenWidth/2;j++) {
					t=BmpData[SCROLL_CON+i*LCD_WIDTH+j];
					BmpData[SCROLL_CON+i*LCD_WIDTH+j]=BmpData[SCROLL_CON+i*LCD_WIDTH+ScreenWidth-1-j];
					BmpData[SCROLL_CON+i*LCD_WIDTH+ScreenWidth-1-j]=t;
				}
		} else if (t==5) {
			for (i=0;i<ScreenHeight/2;i++) {
				memcpy(tb,BmpData+SCROLL_CON+i*LCD_WIDTH,ScreenWidth);
				memcpy(BmpData+SCROLL_CON+i*LCD_WIDTH,BmpData+SCROLL_CON+(ScreenHeight-1-i)*LCD_WIDTH,ScreenWidth);
				memcpy(BmpData+SCROLL_CON+(ScreenHeight-1-i)*LCD_WIDTH,tb,ScreenWidth);
			}
		} else if (t==6) {
			memcpy(BmpData+SCROLL_CON,BmpData,LCD_WIDTH*LCD_HEIGHT);
		}
	} else {
		if (t==0) {
			j=0;
			for (i=0;i<ScreenHeight;i++) {
				memcpy(BmpData+SCROLL_CON+j,BmpData+SCROLL_CON+j+1,ScreenWidth-1);
				BmpData[SCROLL_CON+j+ScreenWidth-1]=(byte)bgcolor;
				j+=LCD_WIDTH;
			}
		} else if (t==1) {
			j=0;
			for (i=0;i<ScreenHeight;i++) {
				memmove(BmpData+SCROLL_CON+j+1,BmpData+SCROLL_CON+j,ScreenWidth-1);
				BmpData[SCROLL_CON+j]=(byte)bgcolor;
				j+=LCD_WIDTH;
			}
		} else if (t==2) {
			memcpy(BmpData+SCROLL_CON,BmpData+SCROLL_CON+LCD_WIDTH,LCD_WIDTH*(ScreenHeight-1));
			memset(BmpData+SCROLL_CON+LCD_WIDTH*(ScreenHeight-1),bgcolor,LCD_WIDTH);
		} else if (t==3) {
			memmove(BmpData+SCROLL_CON+LCD_WIDTH,BmpData+SCROLL_CON,LCD_WIDTH*(ScreenHeight-1));
			memset(BmpData+SCROLL_CON,bgcolor,LCD_WIDTH);
		} else if (t==4) {
			for (i=0;i<ScreenHeight;i++)
				for (j=0;j<ScreenWidth/2;j++) {
					t=BmpData[SCROLL_CON+i*LCD_WIDTH+j];
					BmpData[SCROLL_CON+i*LCD_WIDTH+j]=BmpData[SCROLL_CON+i*LCD_WIDTH+ScreenWidth-1-j];
					BmpData[SCROLL_CON+i*LCD_WIDTH+ScreenWidth-1-j]=t;
				}
		} else if (t==5) {
			for (i=0;i<ScreenHeight/2;i++) {
				memcpy(tb,BmpData+SCROLL_CON+i*LCD_WIDTH,ScreenWidth);
				memcpy(BmpData+SCROLL_CON+i*LCD_WIDTH,BmpData+SCROLL_CON+(ScreenHeight-1-i)*LCD_WIDTH,ScreenWidth);
				memcpy(BmpData+SCROLL_CON+(ScreenHeight-1-i)*LCD_WIDTH,tb,ScreenWidth);
			}
		} else if (t==6) {
			memcpy(BmpData+SCROLL_CON,BmpData,LCD_WIDTH*LCD_HEIGHT);
		}
	}
}

void c_getblock()
{
	word width,height;
	int i,j;
	byte t;

	get_bp();
	a3=bp[5];
	a1=bp[4];
	no_buf=a1&0x40;
	height=(word)bp[3];
	a1=bp[2];
	if (graph_mode==1)
		width=(word)a1>>3;
	else if (graph_mode==4)
		width=(word)(a1&0xfff8)>>1; //使宽度是8的整数倍
	else
		width=(word)a1;
	yy=(word)bp[1];
	a1=bp[0];
	if (graph_mode==8) xx=(word)a1;
	else xx=(word)(a1&0xfff8); //使xx是8的整数倍
	if (width==0 || height==0) return; //容错
	ByteAddr();
	if (graph_mode==1) {
		for (i=0;i<height;i++) {
			for (j=0;j<width;j++) {
				t=0;
				if (BmpData[m1l++]) t|=0x80;
				if (BmpData[m1l++]) t|=0x40;
				if (BmpData[m1l++]) t|=0x20;
				if (BmpData[m1l++]) t|=0x10;
				if (BmpData[m1l++]) t|=0x8;
				if (BmpData[m1l++]) t|=0x4;
				if (BmpData[m1l++]) t|=0x2;
				if (BmpData[m1l++]) t|=0x1;
				lRam[a3++]=t;
			}
			m1l+=LCD_WIDTH-width*8;
		}
	} else if (graph_mode==4) {
		for (i=0;i<height;i++) {
			for (j=0;j<width;j++) {
				t=(BmpData[m1l++]&0xf)<<4;
				t|=BmpData[m1l++]&0xf;
				lRam[a3++]=t;
			}
			m1l+=LCD_WIDTH-width*2;
		}
	} else {
		for (i=0;i<height;i++) {
			memcpy(lRam+a3,BmpData+m1l,width);
			a3+=width;
			m1l+=LCD_WIDTH;
		}
	}
}

a32 c_sin()
{
	get_bp();
	a1=(bp[0]&0xffff)%360;
	if (a1<90) a1=sin90[a1];
	else if (a1<180) a1=sin90[180-a1];
	else if (a1<270) a1=0-sin90[a1-180];
	else a1=0-sin90[360-a1];
	return a1;
}

a32 c_cos()
{
	get_bp();
	a1=(bp[0]&0xffff)%360;
	if (a1>=270) a1-=270;
	else a1+=90;
	if (a1<90) a1=sin90[a1];
	else if (a1<180) a1=sin90[180-a1];
	else if (a1<270) a1=0-sin90[a1-180];
	else a1=0-sin90[360-a1];
	return a1;
}

void c_fill()
{
	;
}

a32 c_setgraphmode()
{
	int t;

	get_bp();
	t=graph_mode;
	a1=bp[0];
	a1&=0xff;
	if (a1==1 || a1==4 || a1==8) {
		if (graph_mode!=(word)a1) {
			if (a1==4) {bgcolor=0;fgcolor=15;}
			else if (a1==8) {bgcolor=0;fgcolor=255;}
			if ((word)a1==1)
				memset(BmpData,0,LCD_WIDTH*LCD_HEIGHT);
			else
				memset(BmpData,bgcolor,LCD_WIDTH*LCD_HEIGHT);
			graph_mode=(word)a1;
			SetPalette();
		}
		a1=t;
	} else if (a1==0) a1=t;
	else a1=0;
	return a1;
}

void c_setbgcolor()
{
	get_bp();
	a1=bp[0];
	if (graph_mode==8) bgcolor=a1&0xff;
	else bgcolor=a1&0xf;
}

void c_setfgcolor()
{
	get_bp();
	a1=bp[0];
	if (graph_mode==8) fgcolor=a1&0xff;
	else fgcolor=a1&0xf;
}

void c_fade()
{
	int i;
	byte t,fa;
	byte *src,*obj;

	get_bp();
	a1=bp[0];
	if (graph_mode==1) return;
	fa=(a1&0xf)^0xf;
	src=(byte *)BmpData+SCROLL_CON;
	obj=(byte *)BmpData;
	i=LCD_WIDTH*ScreenHeight;
	while (i) {
		t=*src++;
		*obj++=(t<fa)?fa:t;
		i--;
	}
}

void c_math()
{
	float t;

	a1=get_val();
	switch (a1) {
	case 0: //GetVertion
		a1=0x100;
		break;
	case 7: //sin
		a1=get_val();
		memcpy(&t,&a1,4);
		t=(float)sin(t);
		memcpy(&a1,&t,4);
		break;
	case 8: //cos
		a1=get_val();
		memcpy(&t,&a1,4);
		t=(float)cos(t);
		memcpy(&a1,&t,4);
		break;
	case 9: //tan
		a1=get_val();
		memcpy(&t,&a1,4);
		t=(float)tan(t);
		memcpy(&a1,&t,4);
		break;
	case 10: //asin
		a1=get_val();
		memcpy(&t,&a1,4);
		t=(float)asin(t);
		memcpy(&a1,&t,4);
		break;
	case 11: //acos
		a1=get_val();
		memcpy(&t,&a1,4);
		t=(float)acos(t);
		memcpy(&a1,&t,4);
		break;
	case 12: //atan
		a1=get_val();
		memcpy(&t,&a1,4);
		t=(float)atan(t);
		memcpy(&a1,&t,4);
		break;
	case 13: //sqrt
		a1=get_val();
		memcpy(&t,&a1,4);
		t=(float)sqrt(t);
		memcpy(&a1,&t,4);
		break;
	case 14: //exp
		a1=get_val();
		memcpy(&t,&a1,4);
		t=(float)exp(t);
		memcpy(&a1,&t,4);
		break;
	case 15: //log
		a1=get_val();
		memcpy(&t,&a1,4);
		t=(float)log(t);
		memcpy(&a1,&t,4);
		break;
	case 19: //abs
		a1=get_val();
		memcpy(&t,&a1,4);
		if (t<0) t=-t;
		memcpy(&a1,&t,4);
		break;
	default:
		a1=0;
	}
	put_val(a1);
}

//////////////////////////////

a32 c_getchar()
{
	while (lav_key<128) ;
	a1=lav_key&0x7f;
	lav_key&=0x7f;
	return a1;
}

a32 c_inkey()
{
	if (lav_key<128) a1=0;
	else {
		a1=lav_key&0x7f;
		lav_key&=0x7f;
	}
	return a1;
}

a32 c_checkkey()
{
	byte k;
	int i;

	get_bp();
	k=(byte)bp[0];
	a1=LFALSE;
	if (k<128) {
		k=c_keyid(k);
		if (k && cur_keyb[k]) a1=LTRUE;
	} else {
		for (i=0;i<sizeof(cur_keyb);i++) {
			if (cur_keyb[i]) {
				k=c_keyval((byte)i);
				if (k) {
					a1=k;
					break;
				}
			}
		}
	}
	return a1;
}

void c_releasekey()
{
	byte k,t;
	int i;

	get_bp();
	k=(byte)bp[0];
	t=k;
	if (k<128) {
		k=c_keyid(k);
		if (k) {
			cur_keyb[k]=0;
			if ((t|0x80)==lav_key) lav_key=t;
		}
	} else {
		lav_key=0;
		for (i=0;i<sizeof(cur_keyb);i++) cur_keyb[i]=0;
	}
}

void c_getword()
{
	c_getchar();
}

void c_delay()
{
	int delay;
	byte t;

	get_bp();
	delay=(bp[0]&0x7fff)*256/1000;
	t=Hz128;
	while (delay) {
		if (t!=Hz128) {
			t=Hz128;
			delay--;
		}
	}
}

a32 c_getms()
{
	return Hz128;
}

void c_gettime()
{
	get_bp();
	a1=bp[0];
	lRam[a1]=s_year;
	lRam[a1+1]=s_year>>8;
	lRam[a1+2]=s_month;
	lRam[a1+3]=s_day;
	lRam[a1+4]=s_hour;
	lRam[a1+5]=s_min;
	lRam[a1+6]=s_sec;
	//lRam[a1+7]=(byte)stime.wDayOfWeek;
}

void c_settime()
{
	get_bp();
	a1=bp[0];
	s_year=lRam[a1]+(lRam[a1+1]<<8);
	s_month=lRam[a1+2];
	s_day=lRam[a1+3];
	s_hour=lRam[a1+4];
	s_min=lRam[a1+5];
	s_sec=lRam[a1+6];
	//stime.wDayOfWeek=0;
}

byte *TaskOpen()
{
	a32 str;

	get_bp();
	str=bp[0];
	if (lRam[str]==0) {
		return NULL;
	}
	if (task_lev && (task[task_lev].attrib&0x80)) {
		return NULL;
	}
	return romdisk_fileaddr(lRam+str);
}

a32 TaskSet(a32 lsp)
{
	a32 dx;

	get_bp();
	a3=bp[1];
	
	task[task_lev+1].attrib=bp[2];
	//task[task_lev].local_bp=lbp;
	//task[task_lev].local_sp=lsp;
	task[task_lev].lRam=lRam;
	//task[task_lev].curr_file=curr_file;
	//task[task_lev].first_file=first_file;
	//task[task_lev].list_set=list_set;
	task[task_lev].graph_mode=graph_mode;
	task[task_lev].bgcolor=bgcolor;
	task[task_lev].fgcolor=fgcolor;
	Save_Palette();
	task[task_lev].ScreenWidth=ScreenWidth;
	task[task_lev].ScreenHeight=ScreenHeight;
	task[task_lev].WorkHandle=WorkHandle;
	//strcpy(task[task_lev].CD,CD);
	//strcpy(task[task_lev+1].name,FileName);
	dx=16-(lsp&0xf);
	//strcpy((byte *)lsp+dx+0x2000,lRam+a3);
	GetFullPathName(lRam+bp[0],(byte *)lsp+dx+CMDLINE);
	strcat((byte *)lsp+dx+CMDLINE," ");
	strcat((byte *)lsp+dx+CMDLINE,lRam+a3);
	*(byte *)(lsp+dx+CMDLINE+255)=0;
	wait_no_key();
	task_lev++;
	lRam=(byte *)lsp+dx;
	return (a32)lRam;
}

void c_getcmdline()
{
	get_bp();
	strcpy(lRam+bp[0],lRam+CMDLINE);
}

void FlmDecode()
{
	get_val();
	get_val();
}

void PY2GB()
{
	a32 src,obj;

	a3=get_val();
	a1=get_val();
	src=a1;
	obj=a3;
	a1=get_val();
	a1=GetGBCodeByPY(a1,lRam+src,lRam+obj);
}

void c_system()
{
	a1=get_val();
	switch (a1) {
	case 0: //GetPID
		a1=('L'<<24)+1; //Lee的机器
		break;
	case 1: //SetBrightness
		get_val();
		break;
	case 2: //GetBrightness
		a1=0;
		break;
	case 3: //ComOpen;
	case 4: //ComClose;
	case 5: //ComWaitReady
		break;
	case 6: //ComSetTimer
		get_val();
		break;
	case 7: //ComGetc
		break;
	case 8: //ComPutc
		get_val();
		break;
	case 9: //ComRead
	case 10: //ComWrite
	case 11: //ComXor
		get_val();
		get_val();
		break;
	case 12: //RamRead
		get_val();
		get_val();
		get_val();
		break;
	case 13: //DiskReclaim
	case 14: //DiskCheck
		break;
	case 15: //FlmDecode
		FlmDecode();
		break;
	case 20: //PY2GB
		PY2GB();
		break;
	case 29: //FindFileEx
		sys_findfile_ex();
		break;
	case 30:
		sys_getfilenum_ex();
		break;	
	case 31: //GetTickCount
		a1=TickCount;
		break;
	case 33: //GetFileAttributes
		sys_GetFileAttributes();
		break;
	default:
		a1=0;
	}
	put_val(a1);
}

a32 c_setpalette()
{
	a32 addr;

	get_bp();
	addr=bp[2];
	a3=bp[1];
	a1=bp[0];
	a1&=0xff;
	a3&=0x7fff;
	if (a1+a3>256) a3=256-a1;
	lav_setpalette((byte)a1,a3,lRam+addr);
	return a3;
}

/*const CODES codes2[]={
	c_putchar,c_getchar,c_printf,c_strcpy,c_strlen,c_setscreen,c_updatelcd,
	c_delay,c_writeblock,scroll_to_lcd,c_textout,c_block,c_rectangle,
	c_exit,c_clearscreen,c_abs,c_rand,c_srand,c_locate,c_inkey,c_point,
	c_getpoint,c_line,c_box,c_circle,c_ellipse,c_beep,c_isalnum,c_isalpha,
	c_iscntrl,c_isdigit,c_isgraph,c_islower,c_isprint,c_ispunct,c_isspace,
	c_isupper,c_isxdigit,c_strcat,c_strchr,c_strcmp,c_strstr,c_tolower,
	c_toupper,c_memset,c_memcpy,c_fopen,c_fclose,c_fread,c_fwrite,
	c_fseek,c_ftell,c_feof,c_rewind,c_getc,c_putc,c_sprintf,c_makedir,
	c_deletefile,c_getms,c_checkkey,c_memmove,c_crc16,c_jiami,c_chdir,
	c_filelist,c_gettime,c_settime,c_getword,c_xdraw,c_releasekey,c_getblock,
	c_sin,c_cos,c_fill,c_setgraphmode,c_setbgcolor,c_setfgcolor,
	c_setlist,c_fade,c_exec,c_findfile,c_getfilenum,c_system,c_math,
	c_setpalette
};*/
