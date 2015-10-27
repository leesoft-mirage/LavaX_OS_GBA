#include "type.h"
#include "io.h"
#include "ram.h"

#define LCD_WIDTH	240
#define LCD_HEIGHT	160
#define	SCROLL_CON	(LCD_WIDTH*LCD_HEIGHT)
#define	MAX_PATH	260

#define MY_LOCAL_RAM	0x000000 //240*160
#define MY_DATA_SIZE    0x20000 //128K

struct TASK
{
	int		attrib;
	a32		WorkHandle;
	int		RamBits;
	word	graph_mode;
	word	bgcolor;
	word	fgcolor;
	byte	palette[256*3];
	int		ScreenWidth;
	int		ScreenHeight;
	byte	*lRam;
};

//register a32 REG9 asm ("r9"); //用来保留寄存器

extern byte *lav_fonts;
extern byte *BmpData;
extern int ScreenWidth,ScreenHeight; //可变的屏幕宽高
extern byte *VRam;
extern byte *pLAVA,*pNextLAVA;
extern byte *lRam;
extern struct TASK task[]; //任务栈
extern int task_lev; //任务级
extern volatile byte lav_key;
extern byte cur_keyb[128];
extern volatile byte Hz128;
extern long TickCount;
extern int s_year,s_month,s_day,s_hour,s_min,s_sec;

extern byte *romdisk_fileaddr(char *name);
extern a32 c_fopen();
extern void c_fclose();
extern a32 c_fread();
extern a32 c_fwrite();
extern a32 c_fseek();
extern a32 c_ftell();
extern a32 c_feof();
extern void c_rewind();
extern a32 c_getc();
extern a32 c_putc();
extern a32 c_makedir();
extern a32 c_deletefile();
extern a32 c_chdir();
extern void c_filelist();
extern a32 c_findfile();
extern a32 c_getfilenum();
extern void c_setlist();
