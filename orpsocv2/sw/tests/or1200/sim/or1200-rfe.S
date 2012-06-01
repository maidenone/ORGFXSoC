/*
	Test of return from execption behavior

	For now, just a simple test confirming that the instructions we l.rfe
	to are executed OK.

	In this test we just increment a counter and confirm this occurred.

	Julius Baxter, ORSoC AB, julius.baxter@orsoc.se

	Register usage:

r1: 	 function call address
r2: 	 SR when function is called
r3:	 test counter
r4:	 temp register
r5:	 temp register

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

/* ---[ 0xE00: TRAP exception ]----------------------------------------- */
        .org 0xe00 	
	/* Traps occur when we want to call a function - function address will
	be in r1, desired SR will be in r2.
	Put EPCR+4 into r9 - link register, function will return
	that way*/
	l.mfspr	r9,r0,SPR_EPCR_BASE
	l.addi	r9,r9,4 /* One instruction past l.trap that got us here */
	l.mtspr	r0,r1,SPR_EPCR_BASE
	l.mtspr	r0,r2,SPR_ESR_BASE
	l.rfe
	/* An unsupported instruction in delay slot, which should not be
	executed */
	lf.add.d r1,r2,r3

/* =================================================== [ text ] === */
	.section .text

/* =================================================== [ start ] === */	

	.global _start
_start:	

	// Kick off test
	l.jal   _main
	l.nop
	
/* =================================================== [ main ] === */

/* Call a function with l.rfe */
#define CALL_FN_WITH_RFE(fn)		\
	l.movhi	r1,hi(fn)		;\
	l.ori	r1,r1,lo(fn)		;\
	l.mfspr	r2,r0,SPR_SR		;\
	l.trap	15
	
	.global _main
_main:
	/* First test, call some functions by l.rfe'ing */
	l.movhi	r3,0 /* r3 = function call counter */

	CALL_FN_WITH_RFE(function1)
	CALL_FN_WITH_RFE(function1)


#define EXPECTED_RESULT	 2
	/* Check result in r3 against the define */
	l.sfnei	r3,EXPECTED_RESULT
	l.bf	fail
	l.nop

check_for_restart:
	l.mfspr	r4,r0,SPR_SR
	l.andi	r4,r4,SPR_SR_ICE /* is instruction cache enabled? */
	l.sfgtu	r4,r0
	l.bnf	restart_with_caches
	l.nop
	
finish:
	l.movhi	r3,0x8000
	l.ori	r3,r3,0x000d
	l.nop	0x2
	l.movhi	r3,0
	l.nop 	0x2
	l.nop 	0x1
	l.nop

fail:
	l.movhi	r3,0xbaaa
	l.ori	r3,r3,0xaaad
	l.nop 	0x2
	l.nop	0x1
	
function1:
	l.addi	r3,r3,1 /* Increment function call counter */
	l.jr	r9
	l.nop	0x2 /* Report value */

restart_with_caches:
	l.jal 	_cache_init
	l.nop
	l.movhi r4, hi(_start)
	l.ori r4, r4, lo(_start)
	l.jr    r4
	l.nop	

	