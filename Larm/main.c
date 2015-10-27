#include <windows.h>
#include <stdio.h>
#include "define.h"

typedef void (*CODES)();

char SrcName[MAX_PATH];
char ObjName[MAX_PATH];
FILE *out;
int ll;
byte *p;
int cur_reg;

byte *prog;
static int file_len;

byte *cmd_line(int argc,char *argv[])
{
	FILE *fp;
	int i,name_len;
	byte *p;
	byte fhead[16];

	p=NULL;
	if (argc<2) {
		printf("错误：没有指定输入文件\n");
		return NULL;
	}
	strcpy(SrcName,argv[1]);
	fp=fopen(SrcName,"rb");
	if (fp==NULL) {
		printf("错误：无法打开输入文件 %s\n",SrcName);
		return NULL;
	}
	name_len=strlen(SrcName);
	strcpy(ObjName,SrcName);
	for (i=name_len-1;i>=0;i--) {
		if (ObjName[i]=='.') {
			strcpy(ObjName+i,".s");
			break;
		} else if (ObjName[i]=='\\' || ObjName[i]=='/' || ObjName[i]==':' || i==0) {
			strcat(ObjName,".s");
			break;
		}
	}

	for (i=2;i<argc;i++) {
		if (argv[i][0]=='-') { //非-开始的参数忽略
			switch (argv[i][1]) {
			case 'o':
				if (i+1<argc)
					strcpy(ObjName,argv[i+1]);
				break;
			}
		}
	}

	fread(fhead,1,16,fp);
	if (fhead[0]!='L' || fhead[1]!='A' || fhead[2]!='V' || fhead[3]!=18) {
		printf("错误：非LavaX程序\n");
		fclose(fp);
		return NULL;
	}
	if ((fhead[8]&0x70)!=0x70 || fhead[9]*16!=240 || fhead[10]*16!=160) {
		printf("错误：该程序不适合在GBA上运行（要求：bigram，8位色，240x160屏幕分辨率）\n");
		fclose(fp);
		return NULL;
	}

	fseek(fp,0,SEEK_END);
	file_len=ftell(fp);
	rewind(fp);
	p=malloc(file_len+1024);
	if (p==NULL) {
		printf("错误：内存不足\n");
		fclose(fp);
		return NULL;
	}
	fread(p,1,file_len,fp);
	fclose(fp);
	*(p+file_len)=0;

	out=fopen(ObjName,"w");
	if (out==NULL) {
		printf("错误：无法打开输出文件 %s\n",ObjName);
		return NULL;
	}

	return p;
}

int is_liji(a32 t)
{
	int i;

	for (i=0;i<16;i++) {
		if ((t&0xffffff00)==0) return 1;
		t=(t>>1)|((t&1)<<31);
		t=(t>>1)|((t&1)<<31);
	}
	return 0;
}

int mov_liji(a32 t)
{
	if (is_liji(t)) return 1;
	if (is_liji(t^0xffffffff)) return 1;
	return 0;
}

int is_off12(a32 t)
{
	if (t&0xfffff000) return 0;
	return 1;
}

int is_off8(a32 t)
{
	if (t&0xffffff00) return 0;
	return 1;
}

void c_preset()
{
	int addr;
	int i,t,len,x,skip;

	fprintf(out,"\nC%x:",p-1-prog);
	skip=0;
	addr=*(a32 *)p&MASK_24BITS;
	p+=3;
	len=*(word *)p;
	p+=2;
	if (len==0) return;

	if (mov_liji(addr)) fprintf(out,"\n\tmov\tr0,#0x%x",addr);
	else {
		fprintf(out,"\n\tldr\tr0,.L%d",ll);
		skip+=4;
	}
	fprintf(out,"\n\tadd\tr0,r0,r9");
	if (mov_liji(len)) fprintf(out,"\n\tmov\tr2,#0x%x",len);
	else {
		fprintf(out,"\n\tldr\tr2,.L%d+%d",ll,skip);
		skip+=4;
	}
	fprintf(out,"\n\tadd\tr1,pc,#%d",4+skip);
	fprintf(out,"\n\tbl\tmemcpy");
	fprintf(out,"\n\tb\t.L%d",ll+1);

	fprintf(out,"\n.L%d:",ll);
	if (!mov_liji(addr)) fprintf(out,"\n\t.word\t0x%x",addr);
	if (!mov_liji(len)) fprintf(out,"\n\t.word\t0x%x",len);

	ll++;
	t=(len+3)/4;
	p-=4;
	for (i=0;i<t;i++) {
		p+=4;
		x=*(a32 *)p;
		fprintf(out,"\n\t.word\t0x%x",x);
	}
	if (len&3) p+=len&3;
	else p+=4;

	fprintf(out,"\n.L%d:",ll);
	ll++;
}

void set_sp()
{
	int sp;

	fprintf(out,"\nC%x:",p-1-prog);
	sp=*(a32 *)p&MASK_24BITS;
	p+=3;
	fprintf(out,"\n\tldr\tr0,=0x%x",sp);
	fprintf(out,"\n\tadd\tr0,r0,r9",sp);
	fprintf(out,"\n\tstr\tr0,[r9,#0x20]");
}

void c_call() //
{
	int addr;

	fprintf(out,"\nC%x:",p-1-prog);
	addr=*(a32 *)p&MASK_24BITS;
	p+=3;
	if (cur_reg==7) {
		fprintf(out,"\n\tstmfd\tsp!,{r8}");
		fprintf(out,"\n\tbl\tC%x",addr);
		fprintf(out,"\n\tldmfd\tsp!,{r8}");
	} else if (cur_reg<7) {
		fprintf(out,"\n\tstmfd\tsp!,{r%d-r8}",cur_reg+1);
		fprintf(out,"\n\tbl\tC%x",addr);
		fprintf(out,"\n\tldmfd\tsp!,{r%d-r8}",cur_reg+1);
	} else
		fprintf(out,"\n\tbl\tC%x",addr);
	if (*p!=TK_VOID) {
		fprintf(out,"\n\tmov\tr%d,r0",cur_reg);
		cur_reg--;
	}
}

void c_jmp()
{
	int addr;

	fprintf(out,"\nC%x:",p-1-prog);
	addr=*(a32 *)p&MASK_24BITS;
	p+=3;
	fprintf(out,"\n\tb\tC%x",addr);
	fprintf(out,"\n\t.ltorg");
}

void c_jmpe()
{
	int addr;

	fprintf(out,"\nC%x:",p-1-prog);
	addr=*(a32 *)p&MASK_24BITS;
	p+=3;
	fprintf(out,"\n\tbeq\tC%x",addr);
}

void add_bp()
{
	int add,num;

	fprintf(out,"\n\n\t.align\t2");
	fprintf(out,"\nC%x:",p-1-prog);
	fprintf(out,"\n\tstmfd\tsp!,{lr}");
	fprintf(out,"\n\tldr\tr0,[r9,#0x20]");
	fprintf(out,"\n\tstr\tr10,[r0,#4]");
	fprintf(out,"\n\tmov\tr10,r0");
	add=*(a32 *)p&MASK_24BITS;
	p+=3;
	num=*p;
	p++;
	if (is_liji(add)) fprintf(out,"\n\tadd\tr0,r10,#0x%x",add);
	else {
		fprintf(out,"\n\tldr\tr1,=0x%x",add);
		fprintf(out,"\n\tadd\tr0,r10,r1",add);
	}
	fprintf(out,"\n\tstr\tr0,[r9,#0x20]");
	fprintf(out,"\n\tsub\tr11,r10,r9");
}

void sub_bp() //
{
	fprintf(out,"\nC%x:",p-1-prog);
	if (cur_reg==7) fprintf(out,"\n\tmov\tr0,r8");
	cur_reg=8;
	fprintf(out,"\n\tstr\tr10,[r9,#0x20]");
	fprintf(out,"\n\tldr\tr10,[r10,#4]");
	fprintf(out,"\n\tsub\tr11,r10,r9");
	fprintf(out,"\n\tldmfd\tsp!,{pc}");
	fprintf(out,"\n\t.ltorg");
}

void push_char() //
{
	byte x;

	fprintf(out,"\nC%x:",p-1-prog);
	x=*p;
	p++;
	fprintf(out,"\n\tmov\tr%d,#0x%x",cur_reg,x);
	cur_reg--;
}

void push_int() //
{
	int x;

	fprintf(out,"\nC%x:",p-1-prog);
	x=*(short *)p;
	p+=2;
	if (mov_liji(x)) fprintf(out,"\n\tmov\tr%d,#0x%x",cur_reg,x);
	else fprintf(out,"\n\tldr\tr%d,=0x%x",cur_reg,x);
	cur_reg--;
}

void push_long() //
{
	a32 x;

	fprintf(out,"\nC%x:",p-1-prog);
	x=*(a32 *)p;
	p+=4;
	if (mov_liji(x)) fprintf(out,"\n\tmov\tr%d,#0x%x",cur_reg,x);
	else fprintf(out,"\n\tldr\tr%d,=0x%x",cur_reg,x);
	cur_reg--;
}

void push_vchar() //
{
	a32 x;

	fprintf(out,"\nC%x:",p-1-prog);
	x=*(a32 *)p&MASK_24BITS;
	p+=3;
	if (is_off12(x)) fprintf(out,"\n\tldrb\tr%d,[r9,#0x%x]",cur_reg,x);
	else {
		fprintf(out,"\n\tldr\tr0,=0x%x",x);
		fprintf(out,"\n\tldrb\tr%d,[r9,r0]",cur_reg);
	}
	cur_reg--;
}

void push_vint() //
{
	a32 x;

	fprintf(out,"\nC%x:",p-1-prog);
	x=*(a32 *)p&MASK_24BITS;
	p+=3;
	if (is_off8(x)) fprintf(out,"\n\tldrsh\tr%d,[r9,#0x%x]",cur_reg,x);
	else {
		fprintf(out,"\n\tldr\tr0,=0x%x",x);
		fprintf(out,"\n\tldrsh\tr%d,[r9,r0]",cur_reg);
	}
	cur_reg--;
}

void push_vlong() //
{
	a32 x;

	fprintf(out,"\nC%x:",p-1-prog);
	x=*(a32 *)p&MASK_24BITS;
	p+=3;
	if (is_off12(x)) fprintf(out,"\n\tldr\tr%d,[r9,#0x%x]",cur_reg,x);
	else {
		fprintf(out,"\n\tldr\tr0,=0x%x",x);
		fprintf(out,"\n\tldr\tr%d,[r9,r0]",cur_reg);
	}
	cur_reg--;
}

void push_lvchar() //
{
	a32 x;

	fprintf(out,"\nC%x:",p-1-prog);
	x=*(a32 *)p&MASK_24BITS;
	p+=3;
	if (is_off12(x)) fprintf(out,"\n\tldrb\tr%d,[r10,#0x%x]",cur_reg,x);
	else {
		fprintf(out,"\n\tldr\tr0,=0x%x",x);
		fprintf(out,"\n\tldrb\tr%d,[r10,r0]",cur_reg);
	}
	cur_reg--;
}

void push_lvint() //
{
	a32 x;

	fprintf(out,"\nC%x:",p-1-prog);
	x=*(a32 *)p&MASK_24BITS;
	p+=3;
	if (is_off8(x)) fprintf(out,"\n\tldrsh\tr%d,[r10,#0x%x]",cur_reg,x);
	else {
		fprintf(out,"\n\tldr\tr0,=0x%x",x);
		fprintf(out,"\n\tldrsh\tr%d,[r10,r0]",cur_reg);
	}
	cur_reg--;
}

void push_lvlong() //
{
	a32 x;

	fprintf(out,"\nC%x:",p-1-prog);
	x=*(a32 *)p&MASK_24BITS;
	p+=3;
	if (is_off12(x)) fprintf(out,"\n\tldr\tr%d,[r10,#0x%x]",cur_reg,x);
	else {
		fprintf(out,"\n\tldr\tr0,=0x%x",x);
		fprintf(out,"\n\tldr\tr%d,[r10,r0]",cur_reg);
	}
	cur_reg--;
}

void push_offset() //
{
	a32 x;

	fprintf(out,"\nC%x:",p-1-prog);
	x=*(a32 *)p&MASK_24BITS;
	p+=3;
	if (is_liji(x)) fprintf(out,"\n\tadd\tr%d,r%d,#0x%x",cur_reg+1,cur_reg+1,x);
	else {
		fprintf(out,"\n\tldr\tr0,=0x%x",x);
		fprintf(out,"\n\tadd\tr%d,r%d,r0",cur_reg+1,cur_reg+1);
	}
}

void push_loffset() //
{
	a32 x;

	fprintf(out,"\nC%x:",p-1-prog);
	x=*(a32 *)p&MASK_24BITS;
	p+=3;
	if (is_liji(x)) fprintf(out,"\n\tadd\tr%d,r%d,#0x%x",cur_reg+1,cur_reg+1,x);
	else {
		fprintf(out,"\n\tldr\tr0,=0x%x",x);
		fprintf(out,"\n\tadd\tr%d,r%d,r0",cur_reg+1,cur_reg+1);
	}
	fprintf(out,"\n\tadd\tr%d,r%d,r11",cur_reg+1,cur_reg+1);
}

void push_ax() //
{
	a32 x;

	fprintf(out,"\nC%x:",p-1-prog);
	x=*(a32 *)p&MASK_24BITS;
	p+=3;
	if (is_liji(x)) fprintf(out,"\n\tadd\tr%d,r%d,#0x%x",cur_reg+1,cur_reg+1,x);
	else {
		fprintf(out,"\n\tldr\tr0,=0x%x",x);
		fprintf(out,"\n\tadd\tr%d,r%d,r0",cur_reg+1,cur_reg+1);
	}
}

void push_gchar() //
{
	a32 x;

	fprintf(out,"\nC%x:",p-1-prog);
	x=*(a32 *)p&MASK_24BITS;
	p+=3;
	if (is_off12(x)) {
		fprintf(out,"\n\tadd\tr%d,r%d,r9",cur_reg+1,cur_reg+1);
		fprintf(out,"\n\tldrb\tr%d,[r%d,#0x%x]",cur_reg+1,cur_reg+1,x);
	} else if (is_liji(x)) {
		fprintf(out,"\n\tadd\tr0,r9,#0x%x",x);
		fprintf(out,"\n\tldrb\tr%d,[r%d,r0]",cur_reg+1,cur_reg+1);
	} else {
		fprintf(out,"\n\tldr\tr0,=0x%x",x);
		fprintf(out,"\n\tadd\tr0,r9,r0");
		fprintf(out,"\n\tldrb\tr%d,[r%d,r0]",cur_reg+1,cur_reg+1);
	}
}

void push_gint() //
{
	a32 x;

	fprintf(out,"\nC%x:",p-1-prog);
	x=*(a32 *)p&MASK_24BITS;
	p+=3;
	if (is_liji(x)) fprintf(out,"\n\tadd\tr0,r9,#0x%x",x);
	else {
		fprintf(out,"\n\tldr\tr0,=0x%x",x);
		fprintf(out,"\n\tadd\tr0,r9,r0");
	}
	fprintf(out,"\n\tldrsh\tr%d,[r%d,r0]",cur_reg+1,cur_reg+1);
}

void push_glong() //
{
	a32 x;

	fprintf(out,"\nC%x:",p-1-prog);
	x=*(a32 *)p&MASK_24BITS;
	p+=3;
	if (is_off12(x)) {
		fprintf(out,"\n\tadd\tr%d,r%d,r9",cur_reg+1,cur_reg+1);
		fprintf(out,"\n\tldr\tr%d,[r%d,#0x%x]",cur_reg+1,cur_reg+1,x);
	} else if (is_liji(x)) {
		fprintf(out,"\n\tadd\tr0,r9,#0x%x",x);
		fprintf(out,"\n\tldr\tr%d,[r%d,r0]",cur_reg+1,cur_reg+1);
	} else {
		fprintf(out,"\n\tldr\tr0,=0x%x",x);
		fprintf(out,"\n\tadd\tr0,r9,r0");
		fprintf(out,"\n\tldr\tr%d,[r%d,r0]",cur_reg+1,cur_reg+1);
	}
}

void push_lgchar() //
{
	a32 x;

	fprintf(out,"\nC%x:",p-1-prog);
	x=*(a32 *)p&MASK_24BITS;
	p+=3;
	if (is_off12(x)) {
		fprintf(out,"\n\tadd\tr%d,r%d,r10",cur_reg+1,cur_reg+1);
		fprintf(out,"\n\tldrb\tr%d,[r%d,#0x%x]",cur_reg+1,cur_reg+1,x);
	} else if (is_liji(x)) {
		fprintf(out,"\n\tadd\tr0,r10,#0x%x",x);
		fprintf(out,"\n\tldrb\tr%d,[r%d,r0]",cur_reg+1,cur_reg+1);
	} else {
		fprintf(out,"\n\tldr\tr0,=0x%x",x);
		fprintf(out,"\n\tadd\tr0,r10,r0");
		fprintf(out,"\n\tldrb\tr%d,[r%d,r0]",cur_reg+1,cur_reg+1);
	}
}

void push_lgint() //
{
	a32 x;

	fprintf(out,"\nC%x:",p-1-prog);
	x=*(a32 *)p&MASK_24BITS;
	p+=3;
	if (is_liji(x)) fprintf(out,"\n\tadd\tr0,r10,#0x%x",x);
	else {
		fprintf(out,"\n\tldr\tr0,=0x%x",x);
		fprintf(out,"\n\tadd\tr0,r10,r0");
	}
	fprintf(out,"\n\tldrsh\tr%d,[r%d,r0]",cur_reg+1,cur_reg+1);
}

void push_lglong() //
{
	a32 x;

	fprintf(out,"\nC%x:",p-1-prog);
	x=*(a32 *)p&MASK_24BITS;
	p+=3;
	if (is_off12(x)) {
		fprintf(out,"\n\tadd\tr%d,r%d,r10",cur_reg+1,cur_reg+1);
		fprintf(out,"\n\tldr\tr%d,[r%d,#0x%x]",cur_reg+1,cur_reg+1,x);
	} else if (is_liji(x)) {
		fprintf(out,"\n\tadd\tr0,r10,#0x%x",x);
		fprintf(out,"\n\tldr\tr%d,[r%d,r0]",cur_reg+1,cur_reg+1);
	} else {
		fprintf(out,"\n\tldr\tr0,=0x%x",x);
		fprintf(out,"\n\tadd\tr0,r10,r0");
		fprintf(out,"\n\tldr\tr%d,[r%d,r0]",cur_reg+1,cur_reg+1);
	}
}

void push_llong() //
{
	a32 x;

	fprintf(out,"\nC%x:",p-1-prog);
	x=*(a32 *)p&MASK_24BITS;
	p+=3;
	if (is_liji(x)) fprintf(out,"\n\tadd\tr%d,r11,#0x%x",cur_reg,x);
	else {
		fprintf(out,"\n\tldr\tr%d,=0x%x",cur_reg,x);
		fprintf(out,"\n\tadd\tr%d,r11,r%d",cur_reg,cur_reg);
	}
	cur_reg--;
}

void c_ptr() //
{
	fprintf(out,"\n\tldrb\tr%d,[r9,r%d]",cur_reg+1,cur_reg+1);
}

void c_iptr() //
{
	fprintf(out,"\n\tldrsh\tr%d,[r9,r%d]",cur_reg+1,cur_reg+1);
}

void c_lptr() //
{
	fprintf(out,"\n\tldr\tr%d,[r9,r%d]",cur_reg+1,cur_reg+1);
}

void cal_add() //
{
	fprintf(out,"\n\tadd\tr%d,r%d,r%d",cur_reg+2,cur_reg+2,cur_reg+1);
	cur_reg++;
}

void cal_sub() //
{
	fprintf(out,"\n\tsub\tr%d,r%d,r%d",cur_reg+2,cur_reg+2,cur_reg+1);
	cur_reg++;
}

void push_sub0() //
{
	fprintf(out,"\n\trsb\tr%d,r%d,#0",cur_reg+1,cur_reg+1);
}

void cal_mul() //
{
	fprintf(out,"\n\tmul\tr%d,r%d,r%d",cur_reg+2,cur_reg+1,cur_reg+2); //待查?要不要考虑符号?
	cur_reg++;
}

void cal_div() //
{
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+2);
	fprintf(out,"\n\tmov\tr1,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__divsi3");
	fprintf(out,"\n\tmov\tr%d,r0",cur_reg+2);
	cur_reg++;
}

void cal_mod() //
{
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+2);
	fprintf(out,"\n\tmov\tr1,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__modsi3");
	fprintf(out,"\n\tmov\tr%d,r0",cur_reg+2);
	cur_reg++;
}

void cal_lshift() //
{
	fprintf(out,"\n\tmov\tr%d,r%d,lsl r%d",cur_reg+2,cur_reg+2,cur_reg+1);
	cur_reg++;
}

void cal_rshift() //
{
	fprintf(out,"\n\tmov\tr%d,r%d,lsr r%d",cur_reg+2,cur_reg+2,cur_reg+1);
	cur_reg++;
}

void cal_and() //
{
	fprintf(out,"\n\tand\tr%d,r%d,r%d",cur_reg+2,cur_reg+2,cur_reg+1);
	cur_reg++;
}

void cal_or() //
{
	fprintf(out,"\n\torr\tr%d,r%d,r%d",cur_reg+2,cur_reg+2,cur_reg+1);
	cur_reg++;
}

void push_not() //
{
	fprintf(out,"\n\tmvn\tr%d,r%d",cur_reg+1,cur_reg+1);
}

void cal_xor() //
{
	fprintf(out,"\n\teor\tr%d,r%d,r%d",cur_reg+2,cur_reg+2,cur_reg+1);
	cur_reg++;
}

void cal_land() //
{
	fprintf(out,"\n\ttst\tr%d,r%d",cur_reg+2,cur_reg+2);
	fprintf(out,"\n\tbeq\t.Lland%d",ll);
	fprintf(out,"\n\ttst\tr%d,r%d",cur_reg+1,cur_reg+1);
	fprintf(out,"\n\tmvnne\tr%d,#0",cur_reg+2);
	fprintf(out,"\n\tmoveq\tr%d,#0",cur_reg+2);
	fprintf(out,"\n.Lland%d:",ll);
	ll++;
	cur_reg++;
}

void cal_lor() //
{
	fprintf(out,"\n\torrs\tr%d,r%d,r%d",cur_reg+2,cur_reg+2,cur_reg+1);
	fprintf(out,"\n\tmvnne\tr%d,#0",cur_reg+2);
	cur_reg++;
}

void cal_lnot() //
{
	fprintf(out,"\n\ttst\tr%d,r%d",cur_reg+1,cur_reg+1);
	fprintf(out,"\n\tmvneq\tr%d,#0",cur_reg+1);
	fprintf(out,"\n\tmovne\tr%d,#0",cur_reg+1);
}

void cal_equ() //
{
	fprintf(out,"\n\tcmp\tr%d,r%d",cur_reg+2,cur_reg+1);
	fprintf(out,"\n\tmvneq\tr%d,#0",cur_reg+2);
	fprintf(out,"\n\tmovne\tr%d,#0",cur_reg+2);
	cur_reg++;
}

void cal_neq() //
{
	fprintf(out,"\n\tcmp\tr%d,r%d",cur_reg+2,cur_reg+1);
	fprintf(out,"\n\tmvnne\tr%d,#0",cur_reg+2);
	fprintf(out,"\n\tmoveq\tr%d,#0",cur_reg+2);
	cur_reg++;
}

void cal_great() //
{
	fprintf(out,"\n\tcmp\tr%d,r%d",cur_reg+2,cur_reg+1);
	fprintf(out,"\n\tmvngt\tr%d,#0",cur_reg+2);
	fprintf(out,"\n\tmovle\tr%d,#0",cur_reg+2);
	cur_reg++;
}

void cal_le() //
{
	fprintf(out,"\n\tcmp\tr%d,r%d",cur_reg+2,cur_reg+1);
	fprintf(out,"\n\tmvnle\tr%d,#0",cur_reg+2);
	fprintf(out,"\n\tmovgt\tr%d,#0",cur_reg+2);
	cur_reg++;
}

void cal_less() //
{
	fprintf(out,"\n\tcmp\tr%d,r%d",cur_reg+2,cur_reg+1);
	fprintf(out,"\n\tmvnlt\tr%d,#0",cur_reg+2);
	fprintf(out,"\n\tmovge\tr%d,#0",cur_reg+2);
	cur_reg++;
}

void cal_ge() //
{
	fprintf(out,"\n\tcmp\tr%d,r%d",cur_reg+2,cur_reg+1);
	fprintf(out,"\n\tmvnge\tr%d,#0",cur_reg+2);
	fprintf(out,"\n\tmovlt\tr%d,#0",cur_reg+2);
	cur_reg++;
}

void cal_qadd() //
{
	int x;

	x=*(short *)p;
	p+=2;
	if (is_liji(x)) {
		fprintf(out,"\n\tadd\tr%d,r%d,#0x%x",cur_reg+1,cur_reg+1,x);
	} else {
		fprintf(out,"\n\tldr\tr0,=0x%x",x);
		fprintf(out,"\n\tadd\tr%d,r%d,r0",cur_reg+1,cur_reg+1);
	}
}

void cal_qsub() //
{
	int x;

	x=*(short *)p;
	p+=2;
	if (is_liji(x)) {
		fprintf(out,"\n\tsub\tr%d,r%d,#0x%x",cur_reg+1,cur_reg+1,x);
	} else {
		fprintf(out,"\n\tldr\tr0,=0x%x",x);
		fprintf(out,"\n\tsub\tr%d,r%d,r0",cur_reg+1,cur_reg+1);
	}
}

void cal_qmul() //
{
	int x;

	x=*(short *)p;
	p+=2;
	if (x==1) ;
	else if (x) {
		fprintf(out,"\n\tldr\tr0,=0x%x",x);
		fprintf(out,"\n\tmul\tr%d,r0,r%d",cur_reg+1,cur_reg+1); //待查?要不要考虑符号?
	} else fprintf(out,"\n\tmov\tr%d,#0",cur_reg+1);
}

void cal_qdiv() //
{
	int x;

	x=*(short *)p;
	p+=2;
	if (is_liji(x)) {
		fprintf(out,"\n\tmov\tr1,#0x%x",x);
	} else {
		fprintf(out,"\n\tldr\tr1,=0x%x",x);
	}
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__divsi3");
	fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
}

void cal_qmod() //
{
	int x;

	x=*(short *)p;
	p+=2;
	if (is_liji(x)) {
		fprintf(out,"\n\tmov\tr1,#0x%x",x);
	} else {
		fprintf(out,"\n\tldr\tr1,=0x%x",x);
	}
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__modsi3");
	fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
}

void cal_qlshift() //
{
	a32 x;

	x=*(word *)p;
	p+=2;
	if (x>=32) {
		fprintf(out,"\n\tmov\tr%d,#0",cur_reg+1);
	} else if (x) {
		fprintf(out,"\n\tmov\tr%d,r%d,lsl #%d",cur_reg+1,cur_reg+1,x);
	}
}

void cal_qrshift() //
{
	a32 x;

	x=*(word *)p;
	p+=2;
	if (x>=32) {
		fprintf(out,"\n\tmov\tr%d,#0",cur_reg+1);
	} else if (x) {
		fprintf(out,"\n\tmov\tr%d,r%d,lsr #%d",cur_reg+1,cur_reg+1,x);
	}
}

void cal_qequ() //
{
	int x;

	x=*(short *)p;
	p+=2;
	if (is_liji(x)) {
		fprintf(out,"\n\tcmp\tr%d,#0x%x",cur_reg+1,x);
	} else {
		fprintf(out,"\n\tldr\tr0,=0x%x",x);
		fprintf(out,"\n\tcmp\tr%d,r0",cur_reg+1);
	}
	fprintf(out,"\n\tmvneq\tr%d,#0",cur_reg+1);
	fprintf(out,"\n\tmovne\tr%d,#0",cur_reg+1);
}

void cal_qneq() //
{
	int x;

	x=*(short *)p;
	p+=2;
	if (is_liji(x)) {
		fprintf(out,"\n\tcmp\tr%d,#0x%x",cur_reg+1,x);
	} else {
		fprintf(out,"\n\tldr\tr0,=0x%x",x);
		fprintf(out,"\n\tcmp\tr%d,r0",cur_reg+1);
	}
	fprintf(out,"\n\tmvnne\tr%d,#0",cur_reg+1);
	fprintf(out,"\n\tmoveq\tr%d,#0",cur_reg+1);
}

void cal_qless() //
{
	int x;

	x=*(short *)p;
	p+=2;
	if (is_liji(x)) {
		fprintf(out,"\n\tcmp\tr%d,#0x%x",cur_reg+1,x);
	} else {
		fprintf(out,"\n\tldr\tr0,=0x%x",x);
		fprintf(out,"\n\tcmp\tr%d,r0",cur_reg+1);
	}
	fprintf(out,"\n\tmvnlt\tr%d,#0",cur_reg+1);
	fprintf(out,"\n\tmovge\tr%d,#0",cur_reg+1);
}

void cal_qge() //
{
	int x;

	x=*(short *)p;
	p+=2;
	if (is_liji(x)) {
		fprintf(out,"\n\tcmp\tr%d,#0x%x",cur_reg+1,x);
	} else {
		fprintf(out,"\n\tldr\tr0,=0x%x",x);
		fprintf(out,"\n\tcmp\tr%d,r0",cur_reg+1);
	}
	fprintf(out,"\n\tmvnge\tr%d,#0",cur_reg+1);
	fprintf(out,"\n\tmovlt\tr%d,#0",cur_reg+1);
}

void cal_qle() //
{
	int x;

	x=*(short *)p;
	p+=2;
	if (is_liji(x)) {
		fprintf(out,"\n\tcmp\tr%d,#0x%x",cur_reg+1,x);
	} else {
		fprintf(out,"\n\tldr\tr0,=0x%x",x);
		fprintf(out,"\n\tcmp\tr%d,r0",cur_reg+1);
	}
	fprintf(out,"\n\tmvnle\tr%d,#0",cur_reg+1);
	fprintf(out,"\n\tmovgt\tr%d,#0",cur_reg+1);
}

void cal_qgreat() //
{
	int x;

	x=*(short *)p;
	p+=2;
	if (is_liji(x)) {
		fprintf(out,"\n\tcmp\tr%d,#0x%x",cur_reg+1,x);
	} else {
		fprintf(out,"\n\tldr\tr0,=0x%x",x);
		fprintf(out,"\n\tcmp\tr%d,r0",cur_reg+1);
	}
	fprintf(out,"\n\tmvngt\tr%d,#0",cur_reg+1);
	fprintf(out,"\n\tmovle\tr%d,#0",cur_reg+1);
}

void c_letx() //
{
	byte x;

	x=*p++;
	if (x&0x80) {
		x&=0x7f;
		if (x==1) fprintf(out,"\n\tstrb\tr%d,[r10,r%d]",cur_reg+1,cur_reg+2);
		else if (x==2) fprintf(out,"\n\tstrh\tr%d,[r10,r%d]",cur_reg+1,cur_reg+2);
		else fprintf(out,"\n\tstr\tr%d,[r10,r%d]",cur_reg+1,cur_reg+2);
	} else {
		if (x==1) fprintf(out,"\n\tstrb\tr%d,[r9,r%d]",cur_reg+1,cur_reg+2);
		else if (x==2) fprintf(out,"\n\tstrh\tr%d,[r9,r%d]",cur_reg+1,cur_reg+2);
		else fprintf(out,"\n\tstr\tr%d,[r9,r%d]",cur_reg+1,cur_reg+2);
	}
	fprintf(out,"\n\tmov\tr%d,r%d",cur_reg+2,cur_reg+1);
	cur_reg++;
}

void cal_idx() //
{
	byte x,t;

	x=*p++;
	t=x&0x1f;
	if (x&0x80) {
		switch ((x>>5)&3) {
		case 0:
			if (t==1) {
				fprintf(out,"\n\tldrb\tr0,[r10,r%d]",cur_reg+1);
				fprintf(out,"\n\tadd\tr0,r0,#1");
				fprintf(out,"\n\tstrb\tr0,[r10,r%d]",cur_reg+1);
				fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
			} else if (t==2) {
				fprintf(out,"\n\tldrsh\tr0,[r10,r%d]",cur_reg+1);
				fprintf(out,"\n\tadd\tr0,r0,#1");
				fprintf(out,"\n\tstrh\tr0,[r10,r%d]",cur_reg+1);
				fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
			} else {
				fprintf(out,"\n\tldr\tr0,[r10,r%d]",cur_reg+1);
				fprintf(out,"\n\tadd\tr0,r0,#1");
				fprintf(out,"\n\tstr\tr0,[r10,r%d]",cur_reg+1);
				fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
			}
			break;
		case 1:
			if (t==1) {
				fprintf(out,"\n\tldrb\tr0,[r10,r%d]",cur_reg+1);
				fprintf(out,"\n\tsub\tr0,r0,#1");
				fprintf(out,"\n\tstrb\tr0,[r10,r%d]",cur_reg+1);
				fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
			} else if (t==2) {
				fprintf(out,"\n\tldrsh\tr0,[r10,r%d]",cur_reg+1);
				fprintf(out,"\n\tsub\tr0,r0,#1");
				fprintf(out,"\n\tstrh\tr0,[r10,r%d]",cur_reg+1);
				fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
			} else {
				fprintf(out,"\n\tldr\tr0,[r10,r%d]",cur_reg+1);
				fprintf(out,"\n\tsub\tr0,r0,#1");
				fprintf(out,"\n\tstr\tr0,[r10,r%d]",cur_reg+1);
				fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
			}
			break;
		case 2:
			if (t==1) {
				fprintf(out,"\n\tldrb\tr0,[r10,r%d]",cur_reg+1);
				fprintf(out,"\n\tadd\tr1,r0,#1");
				fprintf(out,"\n\tstrb\tr1,[r10,r%d]",cur_reg+1);
				fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
			} else if (t==2) {
				fprintf(out,"\n\tldrsh\tr0,[r10,r%d]",cur_reg+1);
				fprintf(out,"\n\tadd\tr1,r0,#1");
				fprintf(out,"\n\tstrh\tr1,[r10,r%d]",cur_reg+1);
				fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
			} else {
				fprintf(out,"\n\tldr\tr0,[r10,r%d]",cur_reg+1);
				fprintf(out,"\n\tadd\tr1,r0,#1");
				fprintf(out,"\n\tstr\tr1,[r10,r%d]",cur_reg+1);
				fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
			}
			break;
		case 3:
			if (t==1) {
				fprintf(out,"\n\tldrb\tr0,[r10,r%d]",cur_reg+1);
				fprintf(out,"\n\tsub\tr1,r0,#1");
				fprintf(out,"\n\tstrb\tr1,[r10,r%d]",cur_reg+1);
				fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
			} else if (t==2) {
				fprintf(out,"\n\tldrsh\tr0,[r10,r%d]",cur_reg+1);
				fprintf(out,"\n\tsub\tr1,r0,#1");
				fprintf(out,"\n\tstrh\tr1,[r10,r%d]",cur_reg+1);
				fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
			} else {
				fprintf(out,"\n\tldr\tr0,[r10,r%d]",cur_reg+1);
				fprintf(out,"\n\tsub\tr1,r0,#1");
				fprintf(out,"\n\tstr\tr1,[r10,r%d]",cur_reg+1);
				fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
			}
			break;
		}
	} else {
		switch ((x>>5)&3) {
		case 0:
			if (t==1) {
				fprintf(out,"\n\tldrb\tr0,[r9,r%d]",cur_reg+1);
				fprintf(out,"\n\tadd\tr0,r0,#1");
				fprintf(out,"\n\tstrb\tr0,[r9,r%d]",cur_reg+1);
				fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
			} else if (t==2) {
				fprintf(out,"\n\tldrsh\tr0,[r9,r%d]",cur_reg+1);
				fprintf(out,"\n\tadd\tr0,r0,#1");
				fprintf(out,"\n\tstrh\tr0,[r9,r%d]",cur_reg+1);
				fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
			} else {
				fprintf(out,"\n\tldr\tr0,[r9,r%d]",cur_reg+1);
				fprintf(out,"\n\tadd\tr0,r0,#1");
				fprintf(out,"\n\tstr\tr0,[r9,r%d]",cur_reg+1);
				fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
			}
			break;
		case 1:
			if (t==1) {
				fprintf(out,"\n\tldrb\tr0,[r9,r%d]",cur_reg+1);
				fprintf(out,"\n\tsub\tr0,r0,#1");
				fprintf(out,"\n\tstrb\tr0,[r9,r%d]",cur_reg+1);
				fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
			} else if (t==2) {
				fprintf(out,"\n\tldrsh\tr0,[r9,r%d]",cur_reg+1);
				fprintf(out,"\n\tsub\tr0,r0,#1");
				fprintf(out,"\n\tstrh\tr0,[r9,r%d]",cur_reg+1);
				fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
			} else {
				fprintf(out,"\n\tldr\tr0,[r9,r%d]",cur_reg+1);
				fprintf(out,"\n\tsub\tr0,r0,#1");
				fprintf(out,"\n\tstr\tr0,[r9,r%d]",cur_reg+1);
				fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
			}
			break;
		case 2:
			if (t==1) {
				fprintf(out,"\n\tldrb\tr0,[r9,r%d]",cur_reg+1);
				fprintf(out,"\n\tadd\tr1,r0,#1");
				fprintf(out,"\n\tstrb\tr1,[r9,r%d]",cur_reg+1);
				fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
			} else if (t==2) {
				fprintf(out,"\n\tldrsh\tr0,[r9,r%d]",cur_reg+1);
				fprintf(out,"\n\tadd\tr1,r0,#1");
				fprintf(out,"\n\tstrh\tr1,[r9,r%d]",cur_reg+1);
				fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
			} else {
				fprintf(out,"\n\tldr\tr0,[r9,r%d]",cur_reg+1);
				fprintf(out,"\n\tadd\tr1,r0,#1");
				fprintf(out,"\n\tstr\tr1,[r9,r%d]",cur_reg+1);
				fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
			}
			break;
		case 3:
			if (t==1) {
				fprintf(out,"\n\tldrb\tr0,[r9,r%d]",cur_reg+1);
				fprintf(out,"\n\tsub\tr1,r0,#1");
				fprintf(out,"\n\tstrb\tr1,[r9,r%d]",cur_reg+1);
				fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
			} else if (t==2) {
				fprintf(out,"\n\tldrsh\tr0,[r9,r%d]",cur_reg+1);
				fprintf(out,"\n\tsub\tr1,r0,#1");
				fprintf(out,"\n\tstrh\tr1,[r9,r%d]",cur_reg+1);
				fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
			} else {
				fprintf(out,"\n\tldr\tr0,[r9,r%d]",cur_reg+1);
				fprintf(out,"\n\tsub\tr1,r0,#1");
				fprintf(out,"\n\tstr\tr1,[r9,r%d]",cur_reg+1);
				fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
			}
			break;
		}
	}
}

void pop_val() //
{
	fprintf(out,"\n\tmovs\tr%d,r%d",cur_reg+1,cur_reg+1); //仅为改变状态
	cur_reg=8;
}

void push_string() //
{
	int i,len,t,x;

	fprintf(out,"\nC%x:",p-1-prog);
	len=strlen(p)+1;
	t=(len+3)/4;
	fprintf(out,"\n\tsub\tr%d,pc,r9",cur_reg);
	fprintf(out,"\n\tb\t.L%d",ll);

	p-=4;
	for (i=0;i<t;i++) {
		p+=4;
		x=*(a32 *)p;
		fprintf(out,"\n\t.word\t0x%x",x);
	}
	if (len&3) p+=len&3;
	else p+=4;

	fprintf(out,"\n.L%d:",ll);
	ll++;
	cur_reg--;
}

void c_icf() //
{
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__floatsisf");
	fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
}

void c_fci() //
{
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__fixsfsi");
	fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
}

void cal_addff() //
{
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+2);
	fprintf(out,"\n\tmov\tr1,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__addsf3");
	fprintf(out,"\n\tmov\tr%d,r0",cur_reg+2);
	cur_reg++;
}

void cal_addf() //
{
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__floatsisf");
	fprintf(out,"\n\tmov\tr1,r%d",cur_reg+2);
	fprintf(out,"\n\tbl\t__addsf3");
	fprintf(out,"\n\tmov\tr%d,r0",cur_reg+2);
	cur_reg++;
}

void cal_add0f() //
{	
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+2);
	fprintf(out,"\n\tbl\t__floatsisf");
	fprintf(out,"\n\tmov\tr1,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__addsf3");
	fprintf(out,"\n\tmov\tr%d,r0",cur_reg+2);
	cur_reg++;
}

void cal_subff() //
{
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+2);
	fprintf(out,"\n\tmov\tr1,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__subsf3");
	fprintf(out,"\n\tmov\tr%d,r0",cur_reg+2);
	cur_reg++;
}

void cal_subf() //
{
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__floatsisf");
	fprintf(out,"\n\tmov\tr1,r0");
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+2);
	fprintf(out,"\n\tbl\t__subsf3");
	fprintf(out,"\n\tmov\tr%d,r0",cur_reg+2);
	cur_reg++;
}

void cal_sub0f() //
{	
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+2);
	fprintf(out,"\n\tbl\t__floatsisf");
	fprintf(out,"\n\tmov\tr1,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__subsf3");
	fprintf(out,"\n\tmov\tr%d,r0",cur_reg+2);
	cur_reg++;
}

void push_sub0f() //
{
	fprintf(out,"\n\tmov\tr0,#0");
	fprintf(out,"\n\tmov\tr1,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__subsf3");
	fprintf(out,"\n\tmov\tr%d,r0",cur_reg+1);
}

void cal_mulff() //
{
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+2);
	fprintf(out,"\n\tmov\tr1,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__mulsf3");
	fprintf(out,"\n\tmov\tr%d,r0",cur_reg+2);
	cur_reg++;
}

void cal_mulf() //
{
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__floatsisf");
	fprintf(out,"\n\tmov\tr1,r0");
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+2);
	fprintf(out,"\n\tbl\t__mulsf3");
	fprintf(out,"\n\tmov\tr%d,r0",cur_reg+2);
	cur_reg++;
}

void cal_mul0f() //
{	
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+2);
	fprintf(out,"\n\tbl\t__floatsisf");
	fprintf(out,"\n\tmov\tr1,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__mulsf3");
	fprintf(out,"\n\tmov\tr%d,r0",cur_reg+2);
	cur_reg++;
}

void cal_divff() //
{
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+2);
	fprintf(out,"\n\tmov\tr1,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__divsf3");
	fprintf(out,"\n\tmov\tr%d,r0",cur_reg+2);
	cur_reg++;
}

void cal_divf() //
{
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__floatsisf");
	fprintf(out,"\n\tmov\tr1,r0");
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+2);
	fprintf(out,"\n\tbl\t__divsf3");
	fprintf(out,"\n\tmov\tr%d,r0",cur_reg+2);
	cur_reg++;
}

void cal_div0f() //
{	
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+2);
	fprintf(out,"\n\tbl\t__floatsisf");
	fprintf(out,"\n\tmov\tr1,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__divsf3");
	fprintf(out,"\n\tmov\tr%d,r0",cur_reg+2);
	cur_reg++;
}

void cal_lessf() //
{
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+2);
	fprintf(out,"\n\tmov\tr1,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__ltsf2");
	fprintf(out,"\n\tcmp\tr0,#0");
	fprintf(out,"\n\tmvnlt\tr%d,#0",cur_reg+2);
	fprintf(out,"\n\tmovge\tr%d,#0",cur_reg+2);
	cur_reg++;
}

void cal_greatf() //
{
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+2);
	fprintf(out,"\n\tmov\tr1,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__gtsf2");
	fprintf(out,"\n\tcmp\tr0,#0");
	fprintf(out,"\n\tmvngt\tr%d,#0",cur_reg+2);
	fprintf(out,"\n\tmovle\tr%d,#0",cur_reg+2);
	cur_reg++;
}

void cal_equf() //
{
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+2);
	fprintf(out,"\n\tmov\tr1,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__eqsf2");
	fprintf(out,"\n\tcmp\tr0,#0");
	fprintf(out,"\n\tmvneq\tr%d,#0",cur_reg+2);
	fprintf(out,"\n\tmovne\tr%d,#0",cur_reg+2);
	cur_reg++;
}

void cal_neqf() //
{
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+2);
	fprintf(out,"\n\tmov\tr1,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__nesf2");
	fprintf(out,"\n\tcmp\tr0,#0");
	fprintf(out,"\n\tmvnne\tr%d,#0",cur_reg+2);
	fprintf(out,"\n\tmoveq\tr%d,#0",cur_reg+2);
	cur_reg++;
}

void cal_lef() //
{
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+2);
	fprintf(out,"\n\tmov\tr1,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__lesf2");
	fprintf(out,"\n\tcmp\tr0,#0");
	fprintf(out,"\n\tmvnle\tr%d,#0",cur_reg+2);
	fprintf(out,"\n\tmovgt\tr%d,#0",cur_reg+2);
	cur_reg++;
}

void cal_gef() //
{
	fprintf(out,"\n\tmov\tr0,r%d",cur_reg+2);
	fprintf(out,"\n\tmov\tr1,r%d",cur_reg+1);
	fprintf(out,"\n\tbl\t__gesf2");
	fprintf(out,"\n\tcmp\tr0,#0");
	fprintf(out,"\n\tmvnge\tr%d,#0",cur_reg+2);
	fprintf(out,"\n\tmovlt\tr%d,#0",cur_reg+2);
	cur_reg++;
}

void c_f0() //
{
	fprintf(out,"\n\tbic\tr%d,r%d,#0x80000000",cur_reg+1,cur_reg+1);
}

void c_lcc() //
{
	fprintf(out,"\n\tand\tr%d,r%d,#0xff",cur_reg+1,cur_reg+1);
}

void c_lci() //
{
	fprintf(out,"\n\tstr\tr%d,[r9,#0x24]",cur_reg+1);
	fprintf(out,"\n\tldrsh\tr%d,[r9,#0x24]",cur_reg+1);
}

void good_exit() //
{
	fprintf(out,"\nC%x:",p-1-prog);
	fprintf(out,"\n\tmov\tr0,#0");
	fprintf(out,"\n\tldr\tr12,[r9,#0x20]");
	fprintf(out,"\n\tstr\tr0,[r12,#8]");
	fprintf(out,"\n\tmov\tlr,pc");
	fprintf(out,"\n\tldr\tpc,[r9,#0x%x]",(TK_EXIT0-TK_PUTCHAR)*4+0x100);
}

void c_pass() //
{
	byte x;

	x=*p++;
	fprintf(out,"\n\tldr\tr12,[r9,#0x20]");
	fprintf(out,"\n\tstr\tr%d,[r12,#0x%x]",cur_reg+1,x);
	cur_reg++;
}

void c_void()
{
	cur_reg=8;
}

void sn_err()
{
	printf("错误：不支持的字节码0x%x发生在文件偏移0x%x处\n",p[-1],p-prog-1);
	fclose(out);
	exit(0);
}

CODES codes[]={
	sn_err,push_char,push_int,push_long,push_vchar,push_vint,push_vlong,push_gchar,
	push_gint,push_glong,sn_err,sn_err,sn_err,push_string,push_lvchar,push_lvint,
	push_lvlong,push_lgchar,push_lgint,push_lglong,sn_err,sn_err,sn_err,push_offset,
	push_loffset,push_llong,sn_err,sn_err,push_sub0,sn_err,sn_err,sn_err,

	sn_err,cal_add,cal_sub,cal_and,cal_or,push_not,cal_xor,cal_land,
	cal_lor,cal_lnot,cal_mul,cal_div,cal_mod,cal_lshift,cal_rshift,cal_equ,
	cal_neq,cal_le,cal_ge,cal_great,cal_less,sn_err,c_ptr,sn_err,
	pop_val,c_jmpe,sn_err,c_jmp,set_sp,c_call,add_bp,sub_bp,

	good_exit,c_preset,sn_err,sn_err,sn_err,cal_qadd,cal_qsub,cal_qmul,
	cal_qdiv,cal_qmod,cal_qlshift,cal_qrshift,cal_qequ,cal_qneq,cal_qgreat,cal_qless,
	cal_qge,cal_qle,c_iptr,c_lptr,c_icf,c_fci,cal_addff,cal_addf,
	cal_add0f,cal_subff,cal_subf,cal_sub0f,cal_mulff,cal_mulf,cal_mul0f,cal_divff,

	cal_divf,cal_div0f,push_sub0f,cal_lessf,cal_greatf,cal_equf,cal_neqf,cal_lef,
	cal_gef,c_f0,sn_err,sn_err,c_lcc,c_lci,c_letx,push_ax,
	cal_idx,c_pass,c_void,sn_err,sn_err,sn_err,sn_err,sn_err,
	sn_err,sn_err,sn_err,sn_err,sn_err,sn_err,sn_err,sn_err
};

void sys_call(int id)
{
	fprintf(out,"\nC%x:",p-1-prog);
	fprintf(out,"\n\tmov\tlr,pc");
	fprintf(out,"\n\tldr\tpc,[r9,#0x%x]",id*4+0x100);
	if (*p!=TK_VOID) {
		fprintf(out,"\n\tmov\tr%d,r0",cur_reg);
		cur_reg--;
	}
}

void code()
{
	CODES codew;
	byte w;

	cur_reg=8;
	while (w=*p++) {
		if (w&0x80) {
			sys_call(w&0x7f);
		} else {
			codew=codes[w];
			(*codew)();
		}
	}
}

void pre_code()
{
	byte w;
	CODES codew;

	p=prog+16;
	while (w=*p++) {
		if (w==0 || w==TK_ADDBP) {
			p--;
			break;
		}
		if (w&0x80) {
			sys_call(w&0x7f);
		} else {
			codew=codes[w];
			(*codew)();
		}
		if (w==TK_JMP) break;
	}
}

int main(int argc,char *argv[])
{
	printf("LavaX本机编译器(GBA) 3.5版\n");
	printf("版权所有 (C) 2003-2015 LeeSoft\n");
	prog=cmd_line(argc,argv);
	if (prog==NULL) return 1;

	ll=1;
	fprintf(out,"\t.text\n\n\t.align\t2\n\t.global\tmain\nmain:");
	pre_code();
	code();
	fprintf(out,"\n");
	fclose(out);

	if (prog) free(prog);

	youhua_main(ObjName,ObjName);

	printf("转换成功\n");
	return 0;
}