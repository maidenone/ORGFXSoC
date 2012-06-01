/*
	OR1200 zero and sign extension instruction tests

	Very basic, testing

	Julius Baxter, ORSoC AB, julius.baxter@orsoc.se
	
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

	.org 0x600 	
	l.nop 0x1

	
/* ---[ 0x700: Illegal instruction exception ]-------------------------- */
        .org 0x700
#ifndef OR1200_IMPL_ALU_EXT
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
	// Kick off test
	l.jal   _main
	l.nop
	


/* =================================================== [ main ] === */
	
	.global _main	
_main:
	// l.exth tests first
	l.ori 	r4,r0,0xffff
	l.ori 	r5,r0,0x7fff

	l.exths	r3,r4

	l.movhi r8,0xffff
	l.ori	r8,r8,0xffff

	l.nop 	0x2
	l.sfne	r8,r3
	l.bf	fail
	l.nop

	l.ori	r8,r0,0xffff

	l.exthz	r3,r4

	l.nop 	0x2
	
	l.sfne	r8,r3
	l.bf	fail
	l.nop

	l.exths	r3,r5

	l.nop 	0x2

	l.sfne	r3,r5
	l.bf	fail
	l.nop

	l.exthz	r3,r5

	l.nop 	0x2

	l.sfne	r3,r5
	l.bf	fail
	l.nop

	// l.extb tests
	
	l.ori 	r4,r0,0x00ff
	l.ori 	r5,r0,0x007f
	
	l.extbs	r3,r4

	l.nop 	0x2

	l.movhi r8,0xffff
	l.ori	r8,r8,0xffff

	l.sfne	r8,r3
	l.bf	fail
	l.nop

	l.ori	r8,r0,0x00ff

	l.extbz	r3,r4

	l.nop 	0x2
	
	l.sfne	r8,r3
	l.bf	fail
	l.nop

	l.extbs	r3,r5

	l.nop 	0x2

	l.sfne	r3,r5
	l.bf	fail
	l.nop

	l.extbz	r3,r5

	l.nop 	0x2

	l.sfne	r3,r5
	l.bf	fail
	l.nop

	// l.extw tests - shouldn't change anything
	l.movhi	r4,0xffff
	l.ori 	r4,r4,0xffff

	l.extws	r3,r4

	l.nop 	0x2
	
	l.sfne	r3,r4
	l.bf	fail
	l.nop

	l.extwz	r3,r4

	l.nop 	0x2
	
	l.sfne	r3,r4
	l.bf	fail
	l.nop

	
	l.movhi r3, hi(0x8000000d)
	l.ori 	r3, r3, lo(0x8000000d)
	l.nop 	0x2
	l.ori 	r3, r0, 0	
	l.nop 	0x1

fail:
	l.ori 	r3, r0, 1
	l.nop 	0x1
