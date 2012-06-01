/*
	OR1200 Find First/Last '1' Test

	Checks l.ff1 and l.fl1 outputs for every bit position

	Julius Baxter, julius.baxter@orsoc.se
	
*/
//////////////////////////////////////////////////////////////////////
////                                                              ////
//// Copyright (C) 2010 Authors and OPENCORES.ORG                 ////
////                                                              ////
//// This source file may be used and distributed without         ////
//// restriction provided that this copyright statement is not    ////
//// removed from the file and that any derivative work contains  ////
//// the original copyright notice and the associated disclaimer. ////
////                                                              ////
//// This source file is free software; you can redistribute it   ////
//// and/or modify it under the terms of the GNU Lesser General   ////
//// Public License as published by the Free Software Foundation; ////
//// either version 2.1 of the License, or (at your option) any   ////
//// later version.                                               ////
////                                                              ////
//// This source is distributed in the hope that it will be       ////
//// useful, but WITHOUT ANY WARRANTY; without even the implied   ////
//// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR      ////
//// PURPOSE.  See the GNU Lesser General Public License for more ////
//// details.                                                     ////
////                                                              ////
//// You should have received a copy of the GNU Lesser General    ////
//// Public License along with this source; if not, download it   ////
//// from http://www.opencores.org/lgpl.shtml                     ////
////                                                              ////
//////////////////////////////////////////////////////////////////////
	
	
#include "spr-defs.h"
#include "board.h"
#include "or1200-defines.h"

	
/* =================================================== [ exceptions ] === */
	.section .vectors, "ax"


/* ---[ 0x100: RESET exception ]----------------------------------------- */
        .org 0x100 	
	l.movhi r0, 0
	/* Clear status register */
	l.ori r1, r0, SPR_SR_SM
	l.mtspr r0, r1, SPR_SR
	/* Clear timer  */
	l.mtspr r0, r0, SPR_TTMR

	/* Jump to program initialisation code */
	.global _start
	l.movhi r4, hi(_start)
	l.ori r4, r4, lo(_start)
	l.jr    r4
	l.nop

/* ---[ 0x700: Illegal instruction exception ]-------------------------- */
        .org 0x700
#ifndef OR1200_IMPL_ALU_FFL1
	// No problem - instruction not supported
	l.movhi r3, hi(0x8000000d)
	l.ori 	r3, r3, lo(0x8000000d)
	l.nop 	0x2
	l.ori 	r3, r0, 0
#else
	l.ori 	r3, r0, 1
#endif
	l.nop 	0x1

	
/* =================================================== [ text ] === */
	.section .text

/* =================================================== [ start ] === */	

	.global _start
_start:	

	/* Instruction cache enable */
	/* Check if IC present and skip enabling otherwise */
	l.mfspr r24,r0,SPR_UPR
	l.andi  r26,r24,SPR_UPR_ICP
	l.sfeq  r26,r0
	l.bf    .L8
	l.nop
	
	/* Disable IC */
	l.mfspr r6,r0,SPR_SR
	l.addi  r5,r0,-1
	l.xori  r5,r5,SPR_SR_ICE
	l.and   r5,r6,r5
	l.mtspr r0,r5,SPR_SR
	
	/* Establish cache block size
	If BS=0, 16;
	If BS=1, 32;
	r14 contain block size
	*/
	l.mfspr r24,r0,SPR_ICCFGR
	l.andi  r26,r24,SPR_ICCFGR_CBS
	l.srli  r28,r26,7
	l.ori   r30,r0,16
	l.sll   r14,r30,r28
	
	/* Establish number of cache sets
	r16 contains number of cache sets
	r28 contains log(# of cache sets)
	*/
	l.andi  r26,r24,SPR_ICCFGR_NCS
	l.srli  r28,r26,3
	l.ori   r30,r0,1
	l.sll   r16,r30,r28
	
	/* Invalidate IC */
	l.addi  r6,r0,0
	l.sll   r5,r14,r28
	
.L7:
	l.mtspr r0,r6,SPR_ICBIR
	l.sfne  r6,r5
	l.bf    .L7
	l.add   r6,r6,r14
	
	/* Enable IC */
	l.mfspr r6,r0,SPR_SR
	l.ori   r6,r6,SPR_SR_ICE
	l.mtspr r0,r6,SPR_SR
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop

.L8:
	/* Data cache enable */
        /* Check if DC present and skip enabling otherwise */
        l.mfspr r24,r0,SPR_UPR
        l.andi  r26,r24,SPR_UPR_DCP
        l.sfeq  r26,r0
        l.bf    .L10
        l.nop
        /* Disable DC */
        l.mfspr r6,r0,SPR_SR
        l.addi  r5,r0,-1
        l.xori  r5,r5,SPR_SR_DCE
	l.and   r5,r6,r5
        l.mtspr r0,r5,SPR_SR
        /* Establish cache block size
           If BS=0, 16;
           If BS=1, 32;
           r14 contain block size
        */
        l.mfspr r24,r0,SPR_DCCFGR
        l.andi  r26,r24,SPR_DCCFGR_CBS
        l.srli  r28,r26,7
        l.ori   r30,r0,16
        l.sll   r14,r30,r28
        /* Establish number of cache sets
           r16 contains number of cache sets
           r28 contains log(# of cache sets)
        */
	l.andi  r26,r24,SPR_DCCFGR_NCS
	l.srli  r28,r26,3
        l.ori   r30,r0,1
        l.sll   r16,r30,r28
        /* Invalidate DC */
        l.addi  r6,r0,0
        l.sll   r5,r14,r28
.L9:
        l.mtspr r0,r6,SPR_DCBIR
        l.sfne  r6,r5
        l.bf    .L9
	l.add   r6,r6,r14
        /* Enable DC */
        l.mfspr r6,r0,SPR_SR
        l.ori   r6,r6,SPR_SR_DCE
        l.mtspr r0,r6,SPR_SR
.L10:
	// Kick off test
	l.jal   _main
	l.nop
	


/* =================================================== [ main ] === */
	
	.global _main	
_main:
	l.movhi r3, 0
	l.movhi r4, 0 // Bit we're checking works
	l.movhi r5, 0
	l.ori	r6, r0, 32
	l.movhi r7, 0 // Register we'll put a value in to check with l.ff1

#define REPORT(reg)	l.or r3, reg, r0 ; \
			l.nop 0x2
	
	
ff1_loop:	
	// Set a loop going, creating a register with a '1' in a known position
	// and checking the output of the l.ff1
	l.ori 	r7, r0, 1 	// Put  1 in bit 0
	l.sll	r7, r7, r4 	// Shift '1' by r4
	REPORT(r7)		// Report value
	l.ff1	r5, r7		// Do Find First '1' op
	l.fl1	r8, r7		// Do Find Last '1' op
	REPORT(r5)		// Report value
	REPORT(r8)		// Report value
	l.addi 	r4, r4, 1 	// Increment bit we're checking (will also be
				// result from l.ff1)
	REPORT(r4)		// Report value
	l.sfne	r5, r4		// r5 should = r4
	l.bf	ff1_error
	l.sfne	r8, r4		// r8 should = r4
	l.bf	fl1_error
	l.sfne	r6, r4		// Check if loop is finished
	l.bf 	ff1_loop	// Keep checking
	l.nop
	l.j	ffl1_test2	// All OK, next test
	l.nop	

ffl1_test2:
	// Try 3 values - all '0', all '1' and a block of values in between
	l.movhi r4, 0
	l.movhi r5, 0xffff
	l.ori	r5, r5,  0xffff
	l.movhi r6, 0x00ff
	l.ori	r6, r6,  0xff00
	// Test '0'
	REPORT(r4)
	l.ff1	r7, r4
	l.fl1	r8, r4
	REPORT(r7)
	REPORT(r8)
	l.sfnei	r7, 0
	l.bf	ff1_error
	l.sfnei	r8, 0
	l.bf	fl1_error

	// Test '0xffffffff'
	REPORT(r5)
	l.ff1	r7, r5
	l.fl1	r8, r5
	REPORT(r7)
	REPORT(r8)
	l.sfnei	r7, 1
	l.bf	ff1_error
	l.sfnei	r8, 32
	l.bf	fl1_error

	// Test '0x00ffff00'
	REPORT(r6)
	l.ff1	r7, r6
	l.fl1	r8, r6
	REPORT(r7)
	REPORT(r8)
	l.sfnei	r7, 9
	l.bf	ff1_error
	l.sfnei	r8, 24
	l.bf	fl1_error
	l.nop
	l.j ffl1_ok	// Tests OK

ff1_error:
	l.movhi r3, hi(0xbaaadff1)
	l.ori	 r3, r3, lo(0xbaaadff1)
	l.nop	0x1


fl1_error:
	l.movhi r3, hi(0xbaaadf11)
	l.ori	r3, r3, lo(0xbaaadf11)
	l.nop	0x1
	
	
ffl1_ok:
	
	l.movhi r3, hi(0x8000000d)
	l.ori	r3, r3, lo(0x8000000d)
	l.nop 	0x2 /* Report */
	l.ori 	r3, r0, 0 /* Return 0 */
	l.nop 	0x1
	