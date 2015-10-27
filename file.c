#include "define.h"

#define MAX_OPEN_FILES	3

#define LTRUE 0xffffffff
#define LFALSE 0
#define NULL 0
#define GENERIC_READ 1
#define GENERIC_WRITE 2

struct DirEnt
{
	char name[15];
	byte Attrib;
	a32 address;
	a32 filelen;
	char baoliu[8];
};

extern a32 *bp;
extern long a1;

char *smode[]={"r","rb","r+","rb+","w","wb","w+","wb+","a","ab","a+","ab+"};
int modeAccess[]={GENERIC_READ,GENERIC_READ,GENERIC_READ|GENERIC_WRITE,GENERIC_READ|GENERIC_WRITE,
	GENERIC_WRITE,GENERIC_WRITE,GENERIC_READ|GENERIC_WRITE,GENERIC_READ|GENERIC_WRITE,
	GENERIC_WRITE,GENERIC_WRITE,GENERIC_READ|GENERIC_WRITE,GENERIC_READ|GENERIC_WRITE};
char WorkDir[MAX_PATH];
a32 WorkHandle;
byte *FileSystemStart;
byte fhave[MAX_OPEN_FILES],fmode[MAX_OPEN_FILES],ftask[MAX_OPEN_FILES];
a32 fsize[MAX_OPEN_FILES];
a32 foffset[MAX_OPEN_FILES];
int curr_fnum;
struct DirEnt *fhandle[MAX_OPEN_FILES];
struct DirEnt *FileHandle;
struct DirEnt RootDir;

int romdisk_init()
{
	char *p,*e;

	p=(char *)0x8000000;
	e=p+0x100000;
	while (p<e) {
		if (!strncmp(p,"RomDisk",7)) {
			FileSystemStart=p;
			memset(&RootDir,0,sizeof(RootDir));
			RootDir.Attrib=0x80;
			return 1;
		}
		p+=0x1000;
	}
	FileSystemStart=NULL;
	return 0;
}

void filesys_init()
{
	memset(fhave,0,MAX_OPEN_FILES);
	WorkHandle=0;
}

void workdir_init()
{
	WorkHandle=0;
}

struct DirEnt *romdisk_dirfindname(char *name,a32 DirHandle)
{
	struct DirEnt *p;

	p=(struct DirEnt *)(FileSystemStart+DirHandle);
	while (p->name[0]) {
		if (!strcmp(p->name,name)) { //找到
			return p;
		}
		p++;
	}
	return NULL;
}

int romdisk_getfilenum(a32 DirHandle)
{
	int num;
	struct DirEnt *p;

	p=(struct DirEnt *)(FileSystemStart+DirHandle);
	num=0;
	while (p->name[0]) {
		if (p->name[0]!='.' && !(p->Attrib&0x40)) {
			num++;
		}
		p++;
	}
	return num;
}

int romdisk_findfile(int id,int nums,byte *pf,a32 DirHandle)
{
	int num,cur;
	struct DirEnt *p;

	if (nums==0) return 0;
	p=(struct DirEnt *)(FileSystemStart+DirHandle);
	num=0;
	cur=1;
	if (id==0) {
		strcpy(pf,"..");
		pf+=16;
		id=1;
		num=1;
	}
	while (p->name[0]) {
		if (p->name[0]!='.' && !(p->Attrib&0x40)) {
			if (cur>=id) {
				strcpy(pf,p->name);
				pf+=16;
				num++;
				if (num>=nums) break;
			}
			cur++;
		}
		p++;
	}
	return num;
}

struct DirEnt *romdisk_findname(char *name)
{
	char fullname[MAX_PATH];
	a32 DirHandle;
	char s[16];
	int i,len;
	struct DirEnt *dirent;

	if (name[0]=='/') {
		DirHandle=0;
		name++;
		if (name[0]==0) {
			return &RootDir;
		}
	} else DirHandle=WorkHandle;
	for (;;) {
		if (*name==0) break;
		for (i=0;;i++) {
			s[i]=*name++;
			if (s[i]==0) {
				name--;
				break;
			} else if (s[i]=='/') {
				s[i]=0;
				break;
			}
		}
		if (s[0]) {
			dirent=romdisk_dirfindname(s,DirHandle);
			if (dirent==NULL) return NULL;
			if (*name) {
				if (dirent->Attrib&0x80) {
					DirHandle=dirent->address;
				} else return NULL;
			} else return dirent;
		}
	}
	return NULL;
}

byte *romdisk_fileaddr(char *name)
{
	struct DirEnt *dir;

	dir=romdisk_findname(name);
	if (dir==NULL) return NULL;
	return FileSystemStart+dir->address;
}

a32 romdisk_findfather(a32 dirh)
{
	struct DirEnt *dir;

	dir=(struct DirEnt *)(FileSystemStart+dirh);
	while (strcmp(dir->name,"..")) {
		dir++;
	}
	return dir->address;
}

void romdisk_findsun(a32 fah,a32 sunh,char *s)
{
	struct DirEnt *dir;

	dir=(struct DirEnt *)(FileSystemStart+fah);
	while (dir->address!=sunh) {
		dir++;
	}
	strcpy(s,dir->name);
}

void romdisk_fullpath(a32 dirh,char *name)
{
	a32 father;
	struct DirEnt *dir;
	char s[16];
	int len,lens;

	name[0]='/';
	name[1]=0;
	while (dirh) {
		father=romdisk_findfather(dirh);
		romdisk_findsun(father,dirh,s);
		len=strlen(name);
		lens=strlen(s);
		memmove(name+lens+1,name,len+1);
		memcpy(name+1,s,lens);
		name[0]='/';	
		dirh=father;
	}
}

void GetFullPathName(char *name,char *full)
{
	char path;

	if (name[0]=='/') strcpy(full,name);
	else {
		romdisk_fullpath(WorkHandle,full);
		strcat(full,name);
	}
}

byte judge_mode()
{
	a32 str;
	byte mode;
	int i;

	str=bp[1];
	for (i=0;i<12;i++)
		if (strcmp(lRam+str,smode[i])==0) break;
	mode=i+1;
	if (mode>12) mode=0;
	return mode;
}

int check_handle(a32 t)
{
	if (t>=0x80 && t<0x80+MAX_OPEN_FILES) {
		t&=0x7f;
		if (fhave[t] && ftask[t]==task_lev) //不允许操作别的任务打开的文件
			FileHandle=fhandle[t];
		else return 0;
	} else return 0;
	curr_fnum=t;
	return t+1;
}

a32 c_fopen()
{
	byte mode,this_file;
	int i;
	struct DirEnt *dir;

	get_bp();
	mode=judge_mode();
	if (mode) mode--;
	else return 0; //文件打开模式错误
	for (i=0;i<MAX_OPEN_FILES;i++) if (fhave[i]==0) break;
	if (i==MAX_OPEN_FILES) return 0; //超过同时打开文件数限制
	this_file=i;
	fmode[this_file]=mode;
	if (!lRam[bp[0]]) return 0; //文件名为空
	dir=romdisk_findname(lRam+bp[0]);
	if (dir==NULL) return 0; //找不到文件
	if (dir->Attrib&0x80) return 0; //目录
	for (i=0;i<MAX_OPEN_FILES;i++) {
		if (fhave[i]) {
			if (fhandle[i]==dir) { //文件已打开
				return 0;
			}
		}
	}
	fhave[this_file]=1;
	fsize[this_file]=dir->filelen;
	if (mode>=8) foffset[this_file]=fsize[this_file];
	else foffset[this_file]=0;
	fhandle[this_file]=dir;
	ftask[this_file]=task_lev;
	return this_file|0x80;
}

void c_fclose()
{
	int t;

	get_bp();
	t=check_handle(bp[0]);
	if (t) {
		fhave[curr_fnum]=0;
	}
}

void c_closeall()
{
	int i;

	for (i=0;i<MAX_OPEN_FILES;i++) {
		if (fhave[i]) {
			if (ftask[i]==task_lev) { //只关闭当前任务打开的文件
				fhave[i]=0;
			}
		}
	}
}

a32 c_fread()
{
	a32 len,str;

	get_bp();
	if (!check_handle(bp[3])) {
		return 0;
	}
	len=bp[2];
	str=bp[0];
	if (modeAccess[fmode[curr_fnum]]&GENERIC_READ) {
		if (foffset[curr_fnum]+len>fsize[curr_fnum])
			len=fsize[curr_fnum]-foffset[curr_fnum]; //读文件不超过文件尾
		if (len) {
			memcpy(lRam+str,FileSystemStart+FileHandle->address+foffset[curr_fnum],len);
			foffset[curr_fnum]+=len;
		}
	} else len=0;
	return len;
}

a32 c_fwrite()
{
	return 0;
}

a32 c_fseek()
{
	byte mode;
	long p;

	get_bp();
	mode=(byte)bp[2];
	p=bp[1];
	if (!check_handle(bp[0]) || mode>2) {
		return -1;
	}
	switch (mode) {
	case 0:
		break;
	case 1:
		p+=foffset[curr_fnum];
		break;
	case 2:
		p+=fsize[curr_fnum];
		break;
	}
	if (p>=0 && p<=(long)fsize[curr_fnum]) {
		foffset[curr_fnum]=p;
		return p;
	}
	return -1;
}

a32 c_ftell()
{
	get_bp();
	if (!check_handle(bp[0])) {
		return -1;
	}
	return foffset[curr_fnum];
}

a32 c_feof()
{
	get_bp();
	if (!check_handle(bp[0])) {
		return -1;
	}
	if (foffset[curr_fnum]==fsize[curr_fnum]) return -1;
	return 0;
}

void c_rewind()
{
	get_bp();
	if (check_handle(bp[0])) {
		foffset[curr_fnum]=0;
	}
}

a32 c_getc()
{
	byte c;

	get_bp();
	if (!check_handle(bp[0])) {
		return -1;
	}
	if (foffset[curr_fnum]==fsize[curr_fnum]) a1=-1; //结束
	else if (modeAccess[fmode[curr_fnum]]&GENERIC_READ) {
		a1=*(FileSystemStart+FileHandle->address+foffset[curr_fnum]);
		foffset[curr_fnum]++;
	} else a1=-1;
	return a1;
}

a32 c_putc()
{
	return -1;
}

a32 c_makedir()
{
	return LFALSE;
}

a32 c_deletefile()
{
	return LFALSE;
}

a32 c_chdir()
{
	struct DirEnt *dir;

	get_bp();
	dir=romdisk_findname(lRam+bp[0]);
	if (dir==NULL) return LFALSE;
	if (dir->Attrib&0x80) {
		WorkHandle=dir->address;
		return LTRUE;
	}
	return LFALSE;
}

void c_filelist()
{
	;
}

a32 c_findfile()
{
	struct DirEnt *dir;

	get_bp();
	return romdisk_findfile(bp[0],bp[1],lRam+bp[2],WorkHandle);
}

a32 c_getfilenum()
{
	struct DirEnt *dir;

	get_bp();
	dir=romdisk_findname(lRam+bp[0]);
	if (dir==NULL) return -1;
	if (dir->Attrib&0x80) {
		return romdisk_getfilenum(dir->address);
	}
	return -1;
}

void c_setlist()
{
	;
}

void sys_findfile_ex()
{
	;
}

void sys_getfilenum_ex()
{
	;
}

void sys_GetFileAttributes()
{
	;
}

/*void c_exec()
{
	;
}*/
