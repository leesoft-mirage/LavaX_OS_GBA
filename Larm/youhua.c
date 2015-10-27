#include <windows.h>
#include <stdio.h>
#include "define.h"

byte *asm;

//从p开始查找一个标号，填到s，返回标号的位置
byte *get_biaohao(byte *p,byte *s)
{
	byte *t,*te;
	byte ts[32];

xxx:
	t=strstr(p,":");
	if (t==NULL) return NULL;
	te=t;
	while (*t>' ') t--;
	t++;
	memcpy(ts,t,te-t);
	ts[te-t]=0;
	if (ts[0]!='C') {
		p=te+1;
		goto xxx;
	}
	strcpy(s,ts);
	return t;
}

//从p开始查找一个转移目标，填到s，返回转移目标的位置
byte *get_mubiao(byte *p,byte *s)
{
	byte *t,*tf;
	byte ts[32];

xxx:
	t=strstr(p,"C");
	if (t==NULL) return NULL;
	tf=t;
	while (*t>' ') t++;
	if (*(t-1)==':') {
		p=t;
		goto xxx;
	}
	memcpy(ts,tf,t-tf);
	ts[t-tf]=0;
	strcpy(s,ts);
	return tf;
}

byte *find_biaohao(byte *bh)
{
	byte *p,*t;
	int len;

	len=strlen(bh);
	p=asm;
xxx:
	t=strstr(p,bh);
	if (t==NULL) return NULL;
	if (*(t+len)!=':') {
		p=t+len;
		goto xxx;
	}
	return t;
}

int find_mubiao(byte *bh)
{
	byte *p,*t;
	int len;

	len=strlen(bh);
	p=asm;
xxx:
	t=strstr(p,bh);
	if (t==NULL) return 0;
	if (*(t+len)==':' || *(t+len)>' ') {
		p=t+len;
		goto xxx;
	}
	return 1;
}

byte *find_mubiaox(byte *p,byte *bh)
{
	byte *t;
	int len;

	len=strlen(bh);
xxx:
	t=strstr(p,bh);
	if (t==NULL) return NULL;
	if (*(t+len)==':' || *(t+len)>' ') {
		p=t+len;
		goto xxx;
	}
	return t;
}

byte *next_line(byte *p)
{
	while (*p!=0xd && *p!=0xa && *p!=0) p++; 
	if (*p==0) return NULL;
	while (*p==0xd || *p==0xa) p++;
	if (*p==0) return NULL;
	return p;
}

byte *up_line(byte *p)
{
	p--;
	while (*p==0xd || *p==0xa) p--;
	while (*p!=0xd && *p!=0xa) p--;
	p++;
	return p;
}

void del_line(byte *p)
{
	byte *t;

	t=next_line(p);
	if (t!=NULL) strcpy(p,t);
}

void add_suzu(byte *sz[],int *num,byte *p)
{
	int i;

	if (p==NULL) return;
	for (i=0;i<*num;i++) {
		if (p==sz[i]) return;
	}
	sz[*num]=p;
	(*num)++;
}

void pai_suzu(byte *sz[],int num)
{
	int i,j;
	byte *t;

	for (i=0;i<num-1;i++) {
		for (j=i+1;j<num;j++) {
			if (sz[i]<sz[j]) {
				t=sz[i];
				sz[i]=sz[j];
				sz[j]=t;
			}
		}
	}
}

int pipei(byte *p,byte *mode)
{
	int i,len;

	len=strlen(mode);
	for (i=0;i<len;i++) {
		if (mode[i]=='?') continue;
		if (p[i]!=mode[i]) return 0;
	}
	return 1;
}

void tihuan(byte *p,byte *src,byte *obj)
{
	int len,len2;

	p=strstr(p,src);
	if (p==NULL) return;
	len=strlen(src);
	strcpy(p,p+len);
	len=strlen(p);
	len2=strlen(obj);
	memmove(p+len2,p,len+1);
	memcpy(p,obj,len2);
}

void biaohao() //消除多余标号
{
	byte *p,*cur;
	byte bh[32];

	p=asm;
	for (;;) {
		cur=get_biaohao(p,bh);
		if (cur==NULL) break;
		if (!find_mubiao(bh)) {
			del_line(cur);
			p=cur;
		} else p=cur+strlen(bh)+1;
	}
}

void ctl_movs()
{
	byte *p,*n0,*n,*n2,*n3,*nx;
	byte mbs[32],lmbs[32];
	int i;

	p=asm;
	for (;;) {
		n0=strstr(p,"\tmovs\tr8,r8");
		if (n0==NULL) break;
		n=next_line(n0);
		if (n==NULL) break;

		nx=n;

		n2=up_line(n0);
		if (!strncmp(n2,".Lland",6)) {
			if ((!strncmp(nx,"\tbeq\t",5))) { //逻辑与优化
				get_mubiao(nx,mbs);
				for (i=0;;i++) {
					lmbs[i]=n2[i];
					if (n2[i]==':') {
						lmbs[i]=0;
						break;
					}
				}
				nx=up_line(n2);
				n3=up_line(nx);
				nx=up_line(n3);
				nx=up_line(nx);
				del_line(n3);
				del_line(n3);
				del_line(n3);
				del_line(n3);
				tihuan(nx,lmbs,mbs);
				p=nx;
				continue;
			} else {
				p=n;
				continue;
			}
		}

		if (!pipei(n2,"\tmov??\tr8,#0")) {
			p=n;
			continue;
		}
		n3=up_line(n2);
		if (!pipei(n3,"\tmvn??\tr8,#0")) {
			p=n;
			continue;
		}

		if ((!strncmp(nx,"\tbeq\t",5))) {
			nx[2]=n2[4];
			nx[3]=n2[5];
			del_line(n3);
			del_line(n3);
			del_line(n3);
			p=n0;
		} else if ((!strncmp(nx,"\tbne\t",5))) {
			nx[2]=n3[4];
			nx[3]=n3[5];
			del_line(n3);
			del_line(n3);
			del_line(n3);
			p=n0;
		} else p=n;
	}
}

void ctl_movs2()
{
	byte *p,*n0,*n,*n2,*nx;

	p=asm;
	for (;;) {
		n0=strstr(p,"\tmovs\tr8,r8");
		if (n0==NULL) break;
		n=next_line(n0);
		if (n==NULL) break;

		nx=n;
		n2=up_line(n0);
		
		if ((!strncmp(nx,"\tbeq\t",5))) {
			if ((!strncmp(n2,"\tmov\tr8,",8))) {
				del_line(n0);
				tihuan(n2,"mov","movs");
				p=n2;
			} else if ((!strncmp(n2,"\tand\tr8,",8))) {
				del_line(n0);
				tihuan(n2,"and","ands");
				p=n2;
			} else
				p=n;
		} else if ((!strncmp(nx,"\tbne\t",5))) {
			if ((!strncmp(n2,"\tmov\tr8,",8))) {
				del_line(n0);
				tihuan(n2,"mov","movs");
				p=n2;
			} else if ((!strncmp(n2,"\tand\tr8,",8))) {
				del_line(n0);
				tihuan(n2,"and","ands");
				p=n2;
			} else
				p=n;
		} else {
			del_line(n0);
			if ((!strncmp(n2,"\tmov\t",5))) {
				del_line(n2);
			}
			p=n2;
		}
	}
}

void ctl_movs3()
{
	byte *p,*n0,*n,*n2,*n3,*nx;

	p=asm;
	for (;;) {
		n0=strstr(p,"\tmovs\tr8,r8");
		if (n0==NULL) break;
		n=next_line(n0);
		if (n==NULL) break;

		nx=n;
		n3=up_line(n0);
		n2=up_line(n3);
		
		if ((!strncmp(nx,"\tbeq\t",5))) {
			if ((!strncmp(n2,"\torrs\tr8,",9))) {
				del_line(n3);
				del_line(n3);
				p=n3;
			} else
				p=n;
		} else if ((!strncmp(nx,"\tbne\t",5))) {
			if ((!strncmp(n2,"\torrs\tr8,",9))) {
				del_line(n3);
				del_line(n3);
				p=n3;
			} else
				p=n;
		} else {
			p=n;
		}
	}
}

void ctl_sp()
{
	static byte load_sp[]="\tldr\tr12,[r9,#0x20]";
	byte *p,*n0,*n;
	int len;

	len=strlen(load_sp);
	p=asm;
	for (;;) {
		n0=strstr(p,"\tldr\tr12,[r9,#0x20]");
		if (n0==NULL) break;
		n=n0;
		for (;;) {
			n=next_line(n);
			if (n==NULL) return;
			if (!strncmp(n,load_sp,len)) {
				del_line(n);
				continue;
			} else if (!strncmp(n,"\tbl\t",4)) {
				p=n;
				break;
			} else if (!strncmp(n,"\tb\t",3)) {
				p=n;
				break;
			} else if (!strncmp(n,"\tldr\tpc,",8)) {
				p=n;
				break;
			} else if (!strncmp(n,"C",1)) {
				p=n;
				break;
			}
		}
	}
}

void youhua_main(char *InName,char *OutName)
{
	FILE *fp;
	int file_len;

	fp=fopen(InName,"rb");
	if (fp==NULL) {
		printf("错误：无法打开输入文件 %s\n",InName);
		return;
	}
	fseek(fp,0,SEEK_END);
	file_len=ftell(fp);
	rewind(fp);
	asm=malloc(file_len+1024);
	if (asm==NULL) {
		printf("错误：内存不足\n");
		fclose(fp);
		return;
	}
	fread(asm,1,file_len,fp);
	fclose(fp);
	*(asm+file_len)=0;

	biaohao();
	ctl_movs();
	ctl_movs2();
	ctl_movs3();
	ctl_sp();

	fp=fopen(OutName,"wb");
	if (fp==NULL) {
		printf("错误：无法打开输出文件 %s\n",OutName);
		return;
	}
	file_len=strlen(asm);
	fwrite(asm,1,file_len,fp);
	fclose(fp);
}