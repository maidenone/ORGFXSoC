#include "spr-defs.h"
#include "board.h"

/* ======================================================= [ macros ] === */

#define REDZONE 128
#define EXCEPTION_STACK_SIZE (128 + REDZONE)
	
#define CLEAR_GPR(gpr) \
	l.or    gpr, r0, r0
    
#define ENTRY(symbol)    \
	.global symbol ; \
symbol:

#define LOAD_SYMBOL_2_GPR(gpr,symbol)  \
	.global symbol ;               \
	l.movhi gpr, hi(symbol) ;      \
	l.ori   gpr, gpr, lo(symbol)

	// Really goes to configurable interrupt handler
#define EXCEPTION_HANDLER            \
	l.addi  r1, r1, -EXCEPTION_STACK_SIZE;          \
	l.sw    4(r1), r3;             \
	l.sw    8(r1), r4;             \
	l.mfspr r3,r0,SPR_NPC;          \
	l.mfspr r4,r0,SPR_EPCR_BASE;   \
	l.j default_exception_handler; \
	l.nop

/* =================================================== [ exceptions ] === */
	.section .vectors, "ax"

/* ---[ 0x100: RESET exception ]----------------------------------------- */
        .org 0x100 	
	l.movhi r0, 0
	l.movhi r1, 0
	l.movhi r2, 0
	l.movhi r3, 0
	l.movhi r4, 0
	l.movhi r5, 0
	l.movhi r6, 0
	l.movhi r7, 0
	l.movhi r8, 0
	l.movhi r9, 0
	l.movhi r10, 0
	l.movhi r11, 0
	l.movhi r12, 0
	l.movhi r13, 0
	l.movhi r14, 0
	l.movhi r15, 0
	l.movhi r16, 0
	l.movhi r17, 0
	l.movhi r18, 0
	l.movhi r19, 0
	l.movhi r20, 0
	l.movhi r21, 0
	l.movhi r22, 0
	l.movhi r23, 0
	l.movhi r24, 0
	l.movhi r25, 0
	l.movhi r26, 0
	l.movhi r27, 0
	l.movhi r28, 0
	l.movhi r29, 0
	l.movhi r30, 0
	l.movhi r31, 0

	/* Clear status register, set supervisor mode */
	l.ori r1, r0, SPR_SR_SM
	l.mtspr r0, r1, SPR_SR
	/* Clear timer  */
	l.mtspr r0, r0, SPR_TTMR
	/* Early Stack initilization */
	LOAD_SYMBOL_2_GPR(r1, _stack)
	l.addi  r2, r0, -3
	l.and   r1, r1, r2

	/* Jump to program initialisation code */
	LOAD_SYMBOL_2_GPR(r4, _start)
	l.jr    r4
	l.nop

/* ---[ 0x200: BUS exception ]------------------------------------------- */
	.org 0x200
	EXCEPTION_HANDLER

/* ---[ 0x300: Data Page Fault exception ]------------------------------- */
	.org 0x300
	EXCEPTION_HANDLER
	
/* ---[ 0x400: Insn Page Fault exception ]------------------------------- */
	.org 0x400
	EXCEPTION_HANDLER

/* ---[ 0x500: Timer exception ]----------------------------------------- */
	.org 0x500
	EXCEPTION_HANDLER
	
/* ---[ 0x600: Aligment exception ]-------------------------------------- */
	.org 0x600
	EXCEPTION_HANDLER
	
/* ---[ 0x700: Illegal insn exception ]---------------------------------- */
	.org 0x700
	EXCEPTION_HANDLER

/* ---[ 0x800: External interrupt exception ]---------------------------- */
	.org 0x800
	EXCEPTION_HANDLER

/* ---[ 0x900: DTLB miss exception ]------------------------------------- */
	.org 0x900
	EXCEPTION_HANDLER
	
/* ---[ 0xa00: ITLB miss exception ]------------------------------------- */
	.org 0xa00
	EXCEPTION_HANDLER

/* ---[ 0xb00: Range exception ]----------------------------------------- */
	.org 0xb00
	EXCEPTION_HANDLER

/* ---[ 0xc00: Syscall exception ]--------------------------------------- */
	.org 0xc00
	EXCEPTION_HANDLER

/* ---[ 0xd00: FPU exception ]------------------------------------------- */
	.org 0xd00
	EXCEPTION_HANDLER

/* ---[ 0xe00: Trap exception ]------------------------------------------ */
	.org 0xe00
	EXCEPTION_HANDLER
	
/* ---[ 0xf00 - 0x1400: Reserved exceptions ]---------------------------- */
/*	
	.org 0xf00
	EXCEPTION_HANDLER

	.org 0x1000
	EXCEPTION_HANDLER
	
	.org 0x1100
	EXCEPTION_HANDLER
	
	.org 0x1200
	EXCEPTION_HANDLER
	
	.org 0x1300
	EXCEPTION_HANDLER
	
	.org 0x1400
	EXCEPTION_HANDLER
	
*/
/* ---[ 0x1500 - 0x1800: Implementation-specific exceptions ]------------ */
/*	
	.org 0x1500
	EXCEPTION_HANDLER
	
	.org 0x1600
	EXCEPTION_HANDLER
	
	.org 0x1700
	EXCEPTION_HANDLER
	
	.org 0x1800
	EXCEPTION_HANDLER
*/
/* ---[ 0x1500 - 0x1F00: Custom exceptions ]----------------------------- */
/*	
	.org 0x1900
	EXCEPTION_HANDLER
	
	.org 0x1a00
	EXCEPTION_HANDLER
	
	.org 0x1b00
	EXCEPTION_HANDLER
	
	.org 0x1c00
	EXCEPTION_HANDLER
	
	.org 0x1d00
	EXCEPTION_HANDLER
	
	.org 0x1e00
	EXCEPTION_HANDLER
	
	.org 0x1f00
	EXCEPTION_HANDLER
*/

/* ========================================================= [ entry ] === */
	.section .text

ENTRY(_start)

	/* Cache initialisation */
	l.jal _cache_init
	l.nop
	
	/* Clear BSS */
        LOAD_SYMBOL_2_GPR(r5, _bss_start)
        LOAD_SYMBOL_2_GPR(r6, _bss_end)
1:
        l.sw    (0)(r5), r0
        l.sfltu r5, r6
        l.bf    1b
        l.addi  r5, r5, 4
        
	/* Jump to main program entry point (argc = argv = 0) */
	CLEAR_GPR(r3)
	CLEAR_GPR(r4)
	l.jal   main
	l.nop
	
	/* If program exits, call exit routine */
	l.addi  r3, r11, 0
	l.jal   exit
	l.nop


/* ====================================== [ default exception handler ] === */

default_exception_handler:
	l.sw    0x00(r1), r2
	l.sw    0x0c(r1), r5
	l.sw    0x10(r1), r6
	l.sw    0x14(r1), r7
	l.sw    0x18(r1), r8
	l.sw    0x1c(r1), r9
	l.sw    0x20(r1), r10
	l.sw    0x24(r1), r11
	l.sw    0x28(r1), r12
	l.sw    0x2c(r1), r13
	l.sw    0x30(r1), r14
	l.sw    0x34(r1), r15
	l.sw    0x38(r1), r16
	l.sw    0x3c(r1), r17
	l.sw    0x40(r1), r18
	l.sw    0x44(r1), r19
	l.sw    0x48(r1), r20
	l.sw    0x4c(r1), r21
	l.sw    0x50(r1), r22
	l.sw    0x54(r1), r23
	l.sw    0x58(r1), r24
	l.sw    0x5c(r1), r25
	l.sw    0x60(r1), r26
	l.sw    0x64(r1), r27
	l.sw    0x68(r1), r28
	l.sw    0x6c(r1), r29
	l.sw    0x70(r1), r30
	l.sw    0x74(r1), r31
	l.sw    0x78(r1), r32

	l.jal   default_exception_handler_c
	l.nop
	
	l.lwz    r2, 0x00(r1)
	l.lwz    r3, 0x04(r1)
	l.lwz    r4, 0x08(r1)
	l.lwz    r5, 0x0c(r1)
	l.lwz    r6, 0x10(r1)
	l.lwz    r7, 0x14(r1)
	l.lwz    r8, 0x18(r1)
	l.lwz    r9, 0x1c(r1)
	l.lwz    r10, 0x20(r1)
	l.lwz    r11, 0x24(r1)
	l.lwz    r12, 0x28(r1)
	l.lwz    r13, 0x2c(r1)
	l.lwz    r14, 0x30(r1)
	l.lwz    r15, 0x34(r1)
	l.lwz    r16, 0x38(r1)
	l.lwz    r17, 0x3c(r1)
	l.lwz    r18, 0x40(r1)
	l.lwz    r19, 0x44(r1)
	l.lwz    r20, 0x48(r1)
	l.lwz    r21, 0x4c(r1)
	l.lwz    r22, 0x50(r1)
	l.lwz    r23, 0x54(r1)
	l.lwz    r24, 0x58(r1)
	l.lwz    r25, 0x5c(r1)
	l.lwz    r26, 0x60(r1)
	l.lwz    r27, 0x64(r1)
	l.lwz    r28, 0x68(r1)
	l.lwz    r29, 0x6c(r1)
	l.lwz    r30, 0x70(r1)
	l.lwz    r31, 0x74(r1)
	l.lwz    r32, 0x78(r1)

	l.addi  r1, r1, EXCEPTION_STACK_SIZE

	l.rfe
	l.nop

