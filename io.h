#ifndef	_IO_H

#define	_IO_H	1

#include "type.h"

#define IRQ_ADDR		*(u32*)0x0300fffc
#define BGPAL_P			(u16*)0x05000000
#define VRAM_START		0x06000000

#define REG_DISPCNT		*(u16*)0x04000000
#define REG_DISPSTAT	*(u16*)0x04000004

#define REG_BG2CNT		*(u16*)0x0400000c

#define REG_BG2PA		*(u16*)0x04000020
#define REG_BG2PB		*(u16*)0x04000022
#define REG_BG2PC		*(u16*)0x04000024
#define REG_BG2PD		*(u16*)0x04000026
#define REG_BG2X_L		*(u16*)0x04000028
#define REG_BG2X_H		*(u16*)0x0400002a
#define REG_BG2Y_L		*(u16*)0x0400002c
#define REG_BG2Y_H		*(u16*)0x0400002e
#define REG_BG2X		*(u32*)0x04000028
#define REG_BG2Y		*(u32*)0x0400002c

#define REG_MOSAIC		*(u16*)0x0400004c

#define REG_DM0SAD		*(u32*)0x040000b0
#define REG_DM0DAD		*(u32*)0x040000b4
#define REG_DM0CNT_L	*(u16*)0x040000b8
#define REG_DM0CNT_H	*(u16*)0x040000ba

#define REG_TM2D		*(u16*)0x04000108
#define REG_TM2CNT		*(u16*)0x0400010a
#define REG_TM3D		*(u16*)0x0400010c
#define REG_TM3CNT		*(u16*)0x0400010e

#define REG_P1			*(u16*)0x04000130
#define REG_P1CNT		*(u16*)0x04000132

#define REG_IE			*(u16*)0x04000200
#define REG_IF			*(u16*)0x04000202
#define REG_WSCNT		*(u16*)0x04000204
#define REG_IME			*(u16*)0x04000208

#endif	/* io.h */
