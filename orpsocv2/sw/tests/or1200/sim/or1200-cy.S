/*
	OR1200 carry bit checking

	Carry generated on all adds which we interpret to be
	unsigned. The CPU will generate both CY and OV.
	CY is generated when unsigned values generate an extra bit.
	OV is when the values, interpreted as signed, cannot have
	the result displayed as it is too large.

	OV is not checked here. Just CY generation and inclusion by
	the l.addc and l.addic instructions.

	Very basic, testing.

TODO:	 Substraction carry out testing.

	Julius Baxter, ORSoC AB, julius.baxter@orsoc.se
	
*/
//////////////////////////////////////////////////////////////////////
////                                                              ////
//// Copyright (C) 2011 Authors and OPENCORES.ORG                 ////
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
#ifndef OR1200_IMPL_ADDC
	// No problem - instruction not supported
	l.movhi r3, hi(0x8000000d)
	l.ori 	r3, r3, lo(0x8000000d)
	l.nop 	0x2
	l.ori 	r3, r0, 0
#else
	l.ori 	r3, r0, 1
#endif
	l.nop 	0x1

/* ---[ 0xb00: Range exception ]---------------------------------------- */
        .org 0xb00
	l.sw 	0(r0), r3
	l.ori 	r3, r0, 0xaaee
	l.nop 	0x2
	l.lwz	r3, 0(r0)
	l.rfe

/* =================================================== [ text ] === */
	.section .text

/* =================================================== [ start ] === */	

	.global _start
_start:
	// Clear regs
	l.movhi r1, 0
	l.movhi	r2, 0
	l.movhi	r3, 0
	l.movhi	r4, 0
	l.movhi	r5, 0
	l.movhi	r6, 0
	
#ifdef OR1200_IMPL_CY
	// Kick off test
	l.jal   _main
#else
	// Not supported, exit test
	l.j	_finish
#endif	
	l.nop


/* =================================================== [ main ] === */


#define CHECK_CY_CLEAR			\
	l.mfspr	r6, r0, SPR_SR	;	\
	l.andi	r6, r6, SPR_SR_CY ;	\
	l.sfne	r6, r0		  ;	\
	l.bf	_fail		  ;	\
	l.nop				

#define CHECK_CY_SET			\
	l.mfspr	r6, r0, SPR_SR	;	\
	l.andi	r6, r6, SPR_SR_CY ;	\
	l.sfnei	r6, SPR_SR_CY	  ;	\
	l.bf	_fail		  ;	\
	l.nop				
	
	.global _main	
_main:

	// Set up some values, check the CY bit is cleared from reset
	CHECK_CY_CLEAR

	// A large unsigned value
	l.movhi	r4, 0xffff
	l.ori	r4, r4, 0xefff

	// A value large enough to cause carry
	l.ori	r5, r0,	0x1001

	l.add	r3, r5, r4	;// Should set CY
	l.nop 	0x2
	CHECK_CY_SET

	l.add	r3, r0, r0	;// Should clear CY
	CHECK_CY_CLEAR
	
	l.addi	r3, r4, 0x1001	;// Should set CY
	l.nop 	0x2
	CHECK_CY_SET

	l.addi	r3, r4, 0x1000	;// Shouldn't set CY
	l.nop 	0x2
	CHECK_CY_CLEAR

	l.add	r3, r0, r0	;// Should clear CY
	CHECK_CY_CLEAR

	// Check use of carry - l.addc
	l.addi	r3, r4, 0x1001	;// Should set CY
	;; // Consequtive instructions
	l.addc	r3, r3,	r5	;// r3 should be 0x1002
	l.nop 	0x2		;// Report

	l.sfnei	r3, 0x1002
	l.bf	_fail
	l.nop

	l.add	r3, r4, r5	;// Should set CY
	l.nop			;// 1 delay instruction
	l.addc	r3, r3,	r5	;// r3 should be 0x1002
	l.nop 	0x2		;// Report

	l.sfnei	r3, 0x1002
	l.bf	_fail
	l.nop

	l.add	r3, r4, r5	;// Should set
	l.nop	0x2		;// 1 delay instruction
	l.nop			;// 2nd delay instruction
	l.addc	r3, r3,	r5	;// r3 should be 0x1002
	l.nop 	0x2		;// Report

	l.sfnei	r3, 0x1002
	l.bf	_fail
	l.nop

	l.add	r3, r0, r0	;// Should clear CY
	CHECK_CY_CLEAR	
	
	// Check use of carry - l.addic
	l.addi	r3, r4, 0x1001	;// Should set CY
	;; // Consequtive instructions
	l.addic	r3, r3,	0x1	;// r3 should be 2
	l.nop 	0x2		;// Report

	l.sfnei	r3, 0x2
	l.bf	_fail
	l.nop

	l.add	r3, r0, r0	;// Should clear CY
	CHECK_CY_CLEAR

	l.add	r3, r4, r5	;// Should set CY
	l.nop			;// 1 delay instruction
	l.addic	r3, r3,	0x1	;// r3 should be 2
	l.nop 	0x2		;// Report

	l.sfnei	r3, 0x2
	l.bf	_fail
	l.nop

	l.add	r3, r0, r0	;// Should clear CY
	CHECK_CY_CLEAR

	l.add	r3, r4, r5	;// Should set
	l.nop	0x2		;// 1 delay instruction
	l.nop			;// 2nd delay instruction
	l.addic	r3, r3,	0x1	;// r3 should be 2
	l.nop 	0x2		;// Report

	l.sfnei	r3, 0x2
	l.bf	_fail
	l.nop

	l.add	r3, r0, r0	;// Should clear CY
	CHECK_CY_CLEAR	

	// Add with carry and generate carry with l.addc

	l.add	r3, r4, r5
	l.addc	r3, r4, r5
	l.nop	0x2

	l.sfnei	r3, 0x1
	l.bf	_fail
	l.nop

	CHECK_CY_SET

	l.add	r3, r0, r0	;// Should clear CY
	CHECK_CY_CLEAR		

	// Add with carry and generate carry with l.addic
	
	l.addi	r3, r4, 0x1001
	l.addic	r3, r4, 0x1001
	l.nop	0x2

	l.sfnei	r3, 0x1
	l.bf	_fail
	l.nop

	CHECK_CY_SET
	
_finish:	
	l.movhi r3, hi(0x8000000d)
	l.ori 	r3, r3, lo(0x8000000d)
	l.nop 	0x2
	l.ori 	r3, r0, 0	
	l.nop 	0x1

_fail:
	l.ori 	r3, r0, 1
	l.nop 	0x1
