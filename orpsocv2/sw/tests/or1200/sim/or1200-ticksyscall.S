#include "spr-defs.h"
#include "board.h"
	
/*

	Tick timer and system call simultaneous interrupt test
	
	Within the test we'll use following global variables:

	r15 syscall interrupt counter
	r16 syscall function counter	
	r17 timer interrupt counter


	The test do the following:
	Setup tick interrupts to occur regularly, and then do a bunch of l.sys
	systems calls, checking that they all occur OK

        Note: if this test appears to continue without counting, it's most
	likely due to a tick counter value that's too small (processor is
	executing too slowly, due to lack of cache or similar) and always
	interrupting before execution can continue. Try increasing the
	TICK_COUNTER_VALUE #define to give the processor time to continue.

	Julius Baxter, julius@opencores.org
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


#define TICK_COUNTER_VALUE 16



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

	
/* =================================================== [ tick interrupt ] === */
	.org 0x500
	.global _tick_handler
_tick_handler:	
	l.addi r17, r17, 1
# Set r7 to hold value of TTMR, one-shot mode/single run (SR)
	l.addi r5, r0, TICK_COUNTER_VALUE /* Tick timer counter value */
	l.movhi r6,hi(SPR_TTMR_SR | SPR_TTMR_IE)
	l.add  r7,r5,r6
	/* Report values , 0x00000500 == tick timer report*/
	l.ori r3, r0, 0x0500
	l.nop 2
	l.or r3, r0, r17
	l.nop 2
# Init the tick timer
	l.mtspr r0,r0,SPR_TTCR 		# clear TTCR
	l.mtspr r0,r7,SPR_TTMR		# set TTMR
	l.rfe

/* ========================================================= [ syscall ] === */	
	.org 0xC00
	.extern _syscall_function
	.global _syscall_handler
_syscall_handler:	
	l.addi r15, r15, 1
	l.mfspr r7, r0, SPR_ESR_BASE /* Put ESR in r7, set back to ESR later */
	l.mfspr r8, r0, SPR_EPCR_BASE/* Put EPCR in r8,set back to EPCR later*/
	/* Unset IEE and TEE bits of SR */
	l.ori r4, r0, SPR_SR_IEE|SPR_SR_TEE
	l.ori r5, r0, 0xffff
	l.xor r5, r5, r4
	l.and r5, r7, r5 /* New SR without interrupt bits set */
	l.mtspr r0, r5, SPR_ESR_BASE /* SR after l.rfe */
	/* Report values , 0x00000c00 == tick timer report*/
	l.ori r3, r0, 0x0c00
	l.nop 2
	/* Get syscall number */
	l.lwz r3, -4(r8) /* r8 = load(EPCR-4)= PC of l.sys that caused this */
	l.andi r3, r3, 0xffff /* get 16-bit immediate syscall number */
	l.nop 2
	l.movhi r4, hi(_syscall_function)
	l.ori r4, r4, lo(_syscall_function)
	l.mtspr r0, r4, SPR_EPCR_BASE
	l.rfe
	


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

/* =================================================== [ main ] === */	
.global _main
_main:
	l.movhi r15, 0
	l.movhi r16, 0
	l.movhi r17, 0
	
	#
	# unmask all ints
	#
	l.movhi r5,0xffff
	l.ori   r5,r5,0xffff
	l.mtspr r0,r5,SPR_PICMR		# set PICMR
	
	# Set r20 to hold enable exceptions and interrupts
	l.mfspr	r20,r0,SPR_SR
	l.ori r20,r20,SPR_SR_SM|SPR_SR_TEE
	
	# Enable exceptions and interrupts
	l.mtspr r0,r20,SPR_SR	# set SR
	
	# Set r7 to hold value of TTMR, one-shot mode/single run (SR)
	l.addi r5, r0, TICK_COUNTER_VALUE /* Tick timer counter value */
	l.movhi r6,hi(SPR_TTMR_SR | SPR_TTMR_IE)
	l.add  r7,r5,r6
	
	# Init the tick timer
	l.mtspr r0,r0,SPR_TTCR 		# clear TTCR
	l.mtspr r0,r7,SPR_TTMR		# set TTMR
	
_wait_loop:
	l.sfeqi r17, 0x10
	l.bnf _wait_loop
	l.nop
	
	/* Timer is working, let's start with some syscalls */
	/* These should occur before tick timer's cycle is up */
	l.nop
	l.sys 0x1
	l.nop
	l.sys 0x2
	l.nop
	l.sys 0x3
	l.nop
	l.sys 0x4
	l.nop
	l.sys 0x5
	l.nop
	l.sfnei r16, 0xf /* Should equal 15, 0xf */
	l.bf _fail
	l.nop
	/* Continue, hopefuly now intercept tick timer cycles */
	l.nop
	l.nop
	l.sys 0x6
	l.nop
	l.nop
	l.nop
	l.nop
	l.sys 0x7
	l.nop
	l.nop
	l.sys 0x8
	l.nop
	l.nop
	l.sys 0x9
	l.nop
	l.nop
	l.sys 0xa
	l.nop
	l.nop
	l.nop
	l.sys 0xb
	l.nop
	l.nop
	l.nop
	l.nop
	l.sys 0xc
	l.nop
	l.nop
	l.sys 0xd
	l.nop
	l.nop
	l.sys 0xe
	l.nop
	l.nop
	l.sys 0xf
	l.nop
	/* Now turn off tick timer */
	l.mtspr r0,r0,SPR_TTMR		# clear TTMR
	l.sfnei r16, 0x78 /* Should equal 120, 0x78 */
	l.bf _fail
	l.nop	
	l.movhi r3, hi(0x8000000d)
	l.ori r3, r3, lo(0x8000000d)
	l.nop 2
	l.ori r3, r0, 0
	l.nop 1
	
_fail:
	l.movhi r3, hi(0xbaaaaaad)
	l.ori r3, r3, lo(0xbaaaaaad)
	l.nop 1
	
	.global _syscall_function
_syscall_function:
	/* r7 and r8 hold actual real ESR and EPCR, respectively */
	/* We'll restore them now */
	l.mtspr r0, r7, SPR_ESR_BASE /* SR before syscall */
	l.mtspr r0, r8, SPR_EPCR_BASE
	l.add r16, r16, r3 /* Add syscall number to our counter */
	l.movhi r4, hi(0x00400000) /* 4MB mark of memory */
	/* Ensure memory access OK */
	l.slli r3, r3, 2 /* Turn syscall number into a word address (<< 2) */
	l.add r4, r4, r3 /* Access this offset from 4MB mark */
	l.sw 0(r4), r16 /* Do a write to memory */
	l.lwz r16, 0(r4) /* Do a read from memory */
	/* Report running value of syscall counter */
	l.or r3, r0, r16
	l.nop 2
	l.rfe /* Now continue from where we had the l.sys */
	
