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

	.org 0x600 	
	l.nop 0x1

	.org 0x700 	
	l.nop 0x1


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
	// First clear MAC
	l.macrc r3

	/* Jeremy stuff. A l.maci which should not overflow */
	l.movhi	r4,hi(0x00000002)
	l.ori	r4,r4,lo(0x00000002)

	l.mtspr	r0,r0,SPR_MACHI
	l.mtspr	r0,r0,SPR_MACLO

	l.maci	r4,0x0002
	
	l.mfspr	r3,r0,SPR_MACHI
	l.nop 	0x2
	l.nop
	l.mfspr	r3,r0,SPR_MACLO
	l.nop 	0x2
	l.nop

	/* Jeremy stuff. A l.maci which should overflow and be truncated. */
	l.movhi	r4,hi(0x7fffffff)
	l.ori	r4,r4,lo(0x7fffffff)

	l.mtspr	r0,r0,SPR_MACHI
	l.mtspr	r0,r0,SPR_MACLO

	l.maci	r4,0x7fff
	
	l.mfspr	r3,r0,SPR_MACHI
	l.nop 	0x2
	l.nop
	l.mfspr	r3,r0,SPR_MACLO
	l.nop 	0x2
	l.nop
	l.movhi r3, hi(0x8000000d)
	l.ori 	r3, r3, lo(0x8000000d)
	l.nop 	0x2
	l.ori 	r3, r0, 0	
	l.nop 	0x1