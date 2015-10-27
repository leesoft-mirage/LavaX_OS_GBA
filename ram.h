#ifndef	_RAM_H

#define	_RAM_H	1

#include "type.h"

#define base		lav_fonts
#define ascii		(u8*)(base)
#define ascii8		(u8*)(base+0x600)
#define gbfont		(u8*)(base+0xe00)
#define gbfont16	(u8*)(base+0x2d7d0)

#endif	/* ram.h */
