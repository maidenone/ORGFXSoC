/*
	OR1200 MAC test

	Very basic, testing simple instructions and multiplication,
	accumulation values

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

// Check MAC unit is enabled before trying to run this test
#ifndef OR1200_MAC_IMPLEMENTED	
# error
# error No MAC unit detected. This test requires hardware MAC support
# error
#endif

	
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
	// First clear MAC
	l.macrc r3
	l.nop 0x2
	// Load a constant into r3
	l.ori 	r4, r0, 0x3	// r4 = 3
	l.ori 	r5, r0, 0x2	// r5 = 2
	l.ori 	r6, r0, 0	// r6 = 0
	l.ori 	r7, r0, 0x7	// r7 = 7
	// Test back-to-back l.mac operations
	l.mac	r4, r5 		// Multiply: MAC reg 6
	l.mac	r4, r7 		// Multiply: MAC reg 27
	l.mac	r5, r5 		// Multiply: MAC reg 31
	l.mac	r4, r0 		// Multiply: MAC reg 31
	l.mac	r4, r5 		// Multiply: MAC reg 37 (0x25)
	l.macrc r3
	l.nop 0x2
	l.sfeqi r3, 0x25
	l.bnf fail
	l.ori r3, r0, 0x1	// Test 1 failed

	// Test back-to-back l.maci operations
	l.maci	r4, 5 		// Multiply: MAC reg 15
	l.maci	r4, 8 		// Multiply: MAC reg 39
	l.maci	r5, 1 		// Multiply: MAC reg 41
	l.maci	r4, 0 		// Multiply: MAC reg 41
	l.maci	r4, 3 		// Multiply: MAC reg 50 (0x32)
	l.macrc r3
	l.nop 0x2
	l.sfeqi r3, 0x32
	l.bnf fail
	l.ori r3, r0, 0x2	// Test 2 failed
	
	
#define NUM_MAC_LOOPS 8
macloop:	
	l.mac	r4, r5 		// Multiply: MAC reg 6
	l.addi 	r6, r6, 1 	// Increment counter
	l.sfltui r6, NUM_MAC_LOOPS
	l.bf 	macloop
	l.nop
	l.macrc	r3
	l.nop 0x2
	l.sfeqi r3, (NUM_MAC_LOOPS*6)
	l.bnf 	fail
	l.ori 	r3, r0, 0x3	// Test 3 failed


	l.movhi	r3, 0x8000
	l.ori 	r3, r3, 0x000d
	l.nop 	0x2
	l.ori 	r3, r0, 0
	l.nop 	0x1

fail:
	l.nop 0x1

