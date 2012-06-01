/*
	
	Tick timer interrupt test
	
	We specify our own reset and initialisation routines as we don't link
	in the usual initialisation code.

	Based on original or1200 tick timer test

	modified by

	Julius Baxter, julius@opencores.org
	Tadej Markovic, tadej@opencores.org
	
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
#define  RAM_START 0x0
	
/* 	Within the test we'll use following global variables:
	
	r16 interrupt counter
	r17 current tick timer comparison counter
	r18 sanity counter
	r19 loop counter
	r20 temp value of SR reg
	r21 temp value of TTMR reg.
	r23 RAM_START
	
	r25-r31 used by int handler
	
	The test do the following:
	We set up the tick timer to trigger once and then we trigger interrupts
	incrementally on every cycle in the specified test program; on
	interrupt handler we check if data computed so far exactly matches
	precalculated values. If interrupt has returned incorreclty, we can
	detect this using assertion routine at the end.
*/



/* =================================================== [ exceptions ] === */
	.section .vectors, "ax"


/* ---[ 0x100: RESET exception ]----------------------------------------- */
        .org 0x100 	
	l.movhi r0, 0
	/* Clear status register */
	l.ori 	r1, r0, SPR_SR_SM
	l.mtspr r0, r1, SPR_SR
	/* Clear timer  */
	l.mtspr r0, r0, SPR_TTMR
	/* Init the stack */
	.global _stack
	l.movhi r1, hi(_stack)
	l.ori 	r1, r1, lo(_stack)
	l.addi  r2, r0, -3
	l.and   r1, r1, r2
	/* Jump to program initialisation code */
	.global _start
	l.movhi r4, hi(_start)
	l.ori 	r4, r4, lo(_start)
	l.jr    r4
	l.nop

	
/* ================================================== [ tick interrupt ] === */
	.org 0x500

	.global _tick_handler
	
_tick_handler:	
	l.addi  r31,r3,0
	# get interrupted program pc
	l.mfspr r25,r0,SPR_EPCR_BASE

	# calculate instruction address
	l.movhi r26,hi(_ie_start)
	l.ori   r26,r26,lo(_ie_start)
	l.addi  r3,r25,0    #print insn index
	l.nop   2
	l.sub   r25,r25,r26
	l.addi  r3,r25,0    #print insn index
	l.nop   2
	
	l.addi  r3,r31,0    # restore r3
	l.sfeqi r25, 0x00
	l.bf    _i00
	l.sfeqi r25, 0x04
	l.bf    _i04
	l.sfeqi r25, 0x08
	l.bf    _i08
	l.sfeqi r25, 0x0c
	l.bf    _i0c
	l.sfeqi r25, 0x10
	l.bf    _i10
	l.sfeqi r25, 0x14
	l.bf    _i14
	l.sfeqi r25, 0x18
	l.bf    _i18
	l.sfeqi r25, 0x1c
	l.bf    _i1c
	l.sfeqi r25, 0x20
	l.bf    _i20
	l.sfeqi r25, 0x24
	l.bf    _i24
	l.sfeqi r25, 0x28
	l.bf    _i28
	l.sfeqi r25, 0x2c
	l.bf    _i2c
	l.sfeqi r25, 0x30
	l.bf    _i30
	l.sfeqi r25, 0x34
	l.bf    _i34
	l.sfeqi r25, 0x38
	l.bf    _i38
	l.nop
	
	# value not defined
_die:
	l.nop   2             #print r3
	
	l.addi  r3,r0,0xeeee
	l.nop   2
	l.addi  r3,r0,1
	l.jal	  exit
	l.nop
1:
	l.j     1b
	l.nop
	

/* =================================================== [ text section ] === */
	.section  .text

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

/* ========================================================= [ main ] === */

	.global _main
_main:
	#
	# set tick counter to initial 3 cycles
	#
	l.addi r16,r0,0
	l.addi r17,r0,1
	l.addi r18,r0,0
	l.addi r19,r0,0
	l.addi r22,r0,0
	
	l.movhi r23,hi(RAM_START)
	l.ori   r23,r23,lo(RAM_START)
	
	#
	# unmask all ints
	#
	l.movhi r5,0xffff
	l.ori   r5,r5,0xffff
	l.mtspr r0,r5,SPR_PICMR		# set PICMR
	
	# Set r20 to hold enable exceptions and interrupts
	l.mfspr	r20,r0,SPR_SR
	l.ori r20,r20,SPR_SR_SM|SPR_SR_TEE|SPR_SR_F
	
	# Set r21 to hold value of TTMR
	l.movhi r5,hi(SPR_TTMR_SR | SPR_TTMR_IE)
	l.add  r21,r5,r17
	
	#
	# MAIN LOOP
	#
_main_loop:
	# reinitialize memory and registers
	l.addi  r3,r0,0xaaaa
	l.addi  r9,r0,0xbbbb
	l.sw    0(r23),r3
	l.sw    4(r23),r9
	l.sw    8(r23),r3
	
	# Reinitializes tick timer  
	l.addi  r17,r17,1
	l.mtspr r0,r0,SPR_TTCR 		# set TTCR
	l.mtspr r0,r21,SPR_TTMR		# set TTMR
	l.mtspr r0,r0,SPR_TTCR 		# set TTCR
	l.addi  r21,r21,1
	
	# Enable exceptions and interrupts
	l.mtspr r0,r20,SPR_SR	# set SR
	
	##### TEST CODE #####
_ie_start:
	l.movhi r3,0x1234         #00
	l.sw    0(r23),r3         #04
	l.movhi r3,hi(RAM_START)  #08
	l.lwz   r3,0(r3)          #0c
	l.movhi r3,hi(RAM_START)  #10
	l.addi  r3,r3,4           #14
	l.j     1f                #18
	l.lwz   r3,0(r3)          #1c
	l.addi  r3,r3,1           #20
1:
	l.sfeqi r3,0xdead         #24
	l.jal   2f                #28
	l.addi  r3,r0,0x5678      #2c
	
_return_addr:
2:
	l.bf    _die              #30
	l.sw    8(r23),r3         #34
_ie_end:
	l.nop                     #38
	##### END OF TEST CODE #####
	
	# do some testing
	
	l.j     _main_loop
	l.nop
	
_i00:
	l.sfeqi r3,0xaaaa
	l.bnf   _die
	l.nop
	l.j     _resume
	l.nop
_i04:
	l.movhi  r26,0x1234
	l.sfeq   r3,r26
	l.bnf   _die
	l.nop
	l.lwz   r26,0(r23)
	l.sfeqi r26,0xaaaa
	l.bnf   _die
	l.nop
	l.j     _resume
	l.nop
_i08:
	l.movhi r26,0x1234
	l.sfeq  r3,r26
	l.bnf   _die
	l.nop
	l.lwz   r27,0(r23)
	l.sfeq  r27,r26
	l.bnf   _die
	l.nop
	l.j     _resume
	l.nop
_i0c:
	l.sfeq  r3,r23
	l.bnf   _die
	l.nop
	l.j     _resume
	l.nop
_i10:
	l.movhi r26,0x1234
	l.sfeq  r26,r3
	l.bnf   _die
	l.nop
	l.j     _resume
	l.nop
_i14:
	l.sfeq  r3,r23
	l.bnf   _die
	l.nop
	l.j     _resume
	l.nop
_i18:
	l.addi  r26,r23,4
	l.sfeq  r3,r26
	l.bnf   _die
	l.nop
	l.j     _resume
	l.nop
_i1c:
	l.j     _die
	l.nop
_i20:
	l.j     _die
	l.nop
_i24:
	l.mfspr r26,r0,SPR_ESR_BASE
	l.addi  r30,r3,0
	l.addi  r3,r26,0
	l.nop   2
	l.addi  r3,r30,0
	l.andi  r26,r26,SPR_SR_F
	l.sfeq  r26,r0
	l.bf   _die
	l.nop
	l.sfeqi  r3,0xbbbb
	l.bnf   _die
	l.nop
	l.j     _resume
	l.nop
_i28:
	l.mfspr r26,r0,SPR_ESR_BASE
	l.addi  r30,r3,0
	l.addi  r3,r26,0
	l.nop   2
	l.addi  r3,r30,0
	l.andi  r26,r26,SPR_SR_F
	l.sfeq  r26,r0
	l.bnf    _die
	l.nop
	l.sfeqi  r22,1
	l.bf     _resume
	l.addi   r22,r0,1
	l.sfeqi  r9,0xbbbb
	l.bnf   _die
	l.nop
	l.j     _resume
	l.nop
_i2c:
	l.movhi  r26,hi(_return_addr)
	l.ori    r26,r26,lo(_return_addr)
	l.sfeq   r9,r26
	l.bnf   _die
	l.nop
	l.sfeqi  r3,0xbbbb
	l.bnf   _die
	l.nop
	l.j     _resume
	l.nop
_i30:
	l.sfeqi  r3,0x5678
	l.bnf   _die
	l.nop
	l.j     _resume
	l.nop  
_i34:
	l.sfeqi  r3,0x5678
	l.bnf   _die
	l.nop
	l.lwz    r26,8(r23)
	l.sfeqi  r26,0xaaaa
	l.bnf   _die
	l.nop
	l.j     _resume
	l.nop  
_i38:
	l.lwz    r26,8(r23)
	l.sfeqi  r26,0x5678
	l.bnf   _die
	l.nop
	#
	# mark finished ok
	#
	l.movhi r3,hi(0xdeaddead)
	l.ori   r3,r3,lo(0xdeaddead)
	l.nop   2
	l.movhi r3,hi(0x8000000d)
	l.ori   r3,r3,lo(0x8000000d)
	l.nop   2	
	l.addi  r3,r0,0

	l.jal	  exit
	l.nop   
_ok:
	l.j     _ok
	l.nop
	
_resume:
	l.mfspr  r27,r0,SPR_ESR_BASE
	l.addi   r26,r0,SPR_SR_TEE
	l.addi   r28,r0,-1
	l.xor    r26,r26,r28
	l.and    r26,r26,r27
	l.mtspr  r0,r26,SPR_ESR_BASE
	
	l.rfe
	l.addi    r3,r3,5         # should not be executed
