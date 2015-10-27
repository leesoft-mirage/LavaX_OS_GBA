#include "define.h"

extern byte *ram_base;

extern void lav_font();
extern void main_loop();

byte *lav_fonts;

a32 r9_bak;

a32 shell_init(a32 rr9)
{
	filesys_init(); //文件系统初始化
	lRam=(u8*)rr9;
	task_lev=0;
	return (a32)romdisk_fileaddr("/System/shell.sys");
}

int main()
{
	lav_fonts=(byte *)lav_font;
	REG_WSCNT=0x4002; //使SuperCard正常快速运行的最佳值

	InitIrq();
	IrqVBlankEnable();
	IrqTimerEnable();
	romdisk_init(); //磁盘初始化
	Color256Init();

	TickCount=Hz128=0;
	s_year=2015;
	s_month=10;
	s_day=22;
	s_hour=9;
	s_min=0;
	s_sec=0;
	
	ram_base=(u8*)0x02016000;
	VRam=(u8*)0x02020000;
	main_loop();
}
