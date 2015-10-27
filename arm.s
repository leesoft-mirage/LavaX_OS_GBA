	.text

	.align	2
	.global	IrqServe
IrqServe:
	mov	r0,#0x4000000
	add	r0,r0,#0x130
	ldrh	r0,[r0]
	tst	r0,#0x30c
	beq	to_exit
	b	IrqServe0
to_exit:
	ldr	r0,=c_exit1
	str	r0,[sp,#20]
	mov	pc,lr

	.align	2
	.global	get_val
get_val:
	mov	r0,r10
	ldr	r10,[r6,#-4]!
	bx	lr

	.align	2
	.global	put_val
put_val:
	str	r10,[r6],#4
	mov	r10,r0
	bx	lr

	.align	2
	.global	main_loop
main_loop:
	ldr	r0,=0x2020000
	mov	r9,r0
	bl	shell_init
	mov	r10,r0
	bl	syscall_init
	bl	lavReset
	mov	r0,r10
	bl	bin_exec
	b	main_loop

	.align	2
	.global	bin_exec
bin_exec:
	ldr	r1,=r9_bak
	str	r9,[r1]
	str	sp,[r9,#0x30]
	str	lr,[r9,#0x34]
	add	pc,r0,#0x1f8

	.align	2
	.global	c_exit
c_exit:
	ldr	r12,[r9,#0x20]
	ldr	r0,[r12,#8]
c_exit1:
	ldr	r1,=r9_bak
	ldr	r9,[r1]
	ldr	sp,[r9,#0x30]
	ldr	pc,[r9,#0x34]

	.align	2
	.global	c_exec
c_exec:
	stmfd	sp!,{lr}
	bl	TaskOpen
	tst	r0,r0
	beq	c_exec_fail
	ldr	r12,[r9,#0x20]
	mov	r8,r0
	str	r10,[r9,#0x28]
	ldr	r0,[r9,#0x20]
	bl	TaskSet
	mov	r9,r0
	bl	syscall_init
	bl	lavReset
	mov	r0,r8
	bl	bin_exec
	stmfd	sp!,{r0}
	bl	TaskExit
	mov	r9,r0
	ldr	r1,=r9_bak
	str	r9,[r1]
	ldr	r10,[r9,#0x28]
	sub	r11,r10,r9
	ldmfd	sp!,{r0,pc}
c_exec_fail:
	mvn	r0,#0
	ldmfd	sp!,{lr}

	.align	2
	.global	syscall_init
syscall_init:
	stmfd	sp!,{lr}
	add	r0,r9,#0x100
	mov	r2,#0x200
	add	r1,pc,#4
	bl	memcpy
	ldmfd	sp!,{pc}
	.word	c_putchar
	.word	c_getchar
	.word	c_printf
	.word	c_strcpy
	.word	c_strlen
	.word	c_setscreen
	.word	c_updatelcd
	.word	c_delay
	.word	c_writeblock
	.word	scroll_to_lcd
	.word	c_textout
	.word	c_block
	.word	c_rectangle
	.word	c_exit
	.word	c_clearscreen
	.word	c_abs
	.word	c_rand
	.word	c_srand
	.word	c_locate
	.word	c_inkey
	.word	c_point
	.word	c_getpoint
	.word	c_line
	.word	c_box
	.word	c_circle
	.word	c_ellipse
	.word	c_beep
	.word	c_isalnum
	.word	c_isalpha
	.word	c_iscntrl
	.word	c_isdigit
	.word	c_isgraph
	.word	c_islower
	.word	c_isprint
	.word	c_ispunct
	.word	c_isspace
	.word	c_isupper
	.word	c_isxdigit
	.word	c_strcat
	.word	c_strchr
	.word	c_strcmp
	.word	c_strstr
	.word	c_tolower
	.word	c_toupper
	.word	c_memset
	.word	c_memcpy
	.word	c_fopen
	.word	c_fclose
	.word	c_fread
	.word	c_fwrite
	.word	c_fseek
	.word	c_ftell
	.word	c_feof
	.word	c_rewind
	.word	c_getc
	.word	c_putc
	.word	c_sprintf
	.word	c_makedir
	.word	c_deletefile
	.word	c_getms
	.word	c_checkkey
	.word	c_memmove
	.word	c_crc16
	.word	c_jiami
	.word	c_chdir
	.word	c_filelist
	.word	c_gettime
	.word	c_settime
	.word	c_getword
	.word	c_xdraw
	.word	c_releasekey
	.word	c_getblock
	.word	c_sin
	.word	c_cos
	.word	c_fill
	.word	c_setgraphmode
	.word	c_setbgcolor
	.word	c_setfgcolor
	.word	c_setlist
	.word	c_fade
	.word	c_exec
	.word	c_findfile
	.word	c_getfilenum
	.word	c_system
	.word	c_math
	.word	c_setpalette
	.word	c_getcmdline
