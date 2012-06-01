/*

	Tests of link register behavior in delay slot of l.jal instruction
	
	Testing this:
	l.jal _place
	l.sw 4(r1) r9
	What happens...... (r9 is link register)
Result:
	Appears to confirm that the r9 becomes the return value immediately
	( at the write-back stage of the l.jal instruction)

	Then testing this:
	l.jal _place
	l.or r9, r0, r0
Result:	
	Writing	to link register (r9) in delay slot works, so should be
	avoided (it was illegal according to spec, anyway)
	
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


/* =================================================== [ text ] === */
	.section .text

/* =================================================== [ start ] === */	

	.global _start
	.global _testjalfunc
_start:
	l.addi 	r1, r1, -4
	l.movhi r9, hi(0x01234567)
	l.ori 	r9, r9, lo(0x01234567)
	l.or 	r3, r1, r1 /* copy stack pointer to r3 so we can report it */
	l.nop 	0x2
	l.jal 	_testjalfunc
	l.sw 	0(r1), r9
	l.nop
	l.nop
	l.movhi r3, hi(0x8000000d)
	l.ori 	r3, r3, lo(0x8000000d)
	/* Setup some code at address 0x0 */
	l.movhi r4, hi(0x15000000) /* standard l.nop */
	l.ori 	r5, r4, 0x2 /* l.nop that will report value in r3 */
	l.sw	0x0(r0), r5 /* Write "l.nop 0x2" to 0x0 */
	l.movhi	r6, hi(0xa8600000) /*Assemble register with l.ori r3,r0,0 */
	l.sw	0x4(r0), r6 /* Write "l.ori r3,r0,0" to 0x4*/
	l.ori 	r5, r4, 0x1 /* l.nop that will exit simulation */
	l.sw	0x8(r0), r5 /* Write l.nop 0x1 to 0x8 */
	l.sw	0xc(r0), r0 /* Write "l.j 0" to address 0xc */
	l.sw	0x10(r0), r4 /* Write l.nop to 0xc */
	l.nop
	/* Try writing to r9 during delay slot... */
	l.jal 	_testjalfunc
	l.or 	r9, r0, r0 /* Clear r9 - cause jump to 0 on return */
	l.nop 	1


_testjalfunc:
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	l.jr 	r9
	l.nop

