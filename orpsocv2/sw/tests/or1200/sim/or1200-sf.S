/*

	Basic l.sfxx comparison instruction tests.

	Does not cover set-flag-against-immediate

	Julius Baxter, ORSoC AB, <julius.baxter@orsoc.se>
	
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

#define SHOULD_BE_SET	l.bnf	_fail ;	\
	l.nop 0
#define	SHOULDNT_BE_SET	l.bf	_fail ;	\
	l.nop 0

#define MOVE_TO_R4R5_AND_REPORT(a,b)	\
	l.movhi	r3,hi(a)	;	\
	l.ori	r3,r3,lo(a)	;	\
	l.nop	0x2		;	\
	l.or	r4,r3,r3	;	\
	l.movhi	r3,hi(b)	;	\
	l.ori	r3,r3,lo(b)	;	\
	l.nop	0x2		;	\
	l.or	r5,r3,r3

	
// Tests to check these values should be less than
#define SHOULD_BE_LESS_THAN_SIGNED(a,b)	\
	MOVE_TO_R4R5_AND_REPORT(a,b) ;	\
	l.sfeq	r4,r5		;	\
	SHOULDNT_BE_SET		;	\
	l.sfne	r4,r5		;	\
	SHOULD_BE_SET		;	\
	l.sfgts	r4,r5		;	\
	SHOULDNT_BE_SET		;	\
	l.sfges	r4,r5		;	\
	SHOULDNT_BE_SET		;	\
	l.sfles	r4,r5		;	\
	SHOULD_BE_SET		;	\
	l.sflts	r4,r5		;	\
	SHOULD_BE_SET

#define SHOULD_BE_GREATER_THAN_SIGNED(a,b)	\
	MOVE_TO_R4R5_AND_REPORT(a,b) ;	\
	l.sfeq	r4,r5		;	\
	SHOULDNT_BE_SET		;	\
	l.sfne	r4,r5		;	\
	SHOULD_BE_SET		;	\
	l.sfgts	r4,r5		;	\
	SHOULD_BE_SET		;	\
	l.sfges	r4,r5		;	\
	SHOULD_BE_SET		;	\
	l.sfles	r4,r5		;	\
	SHOULDNT_BE_SET		;	\
	l.sflts	r4,r5		;	\
	SHOULDNT_BE_SET
	
#define SHOULD_BE_LESS_THAN_UNSIGNED(a,b)	\
	MOVE_TO_R4R5_AND_REPORT(a,b) ;	\
	l.sfeq	r4,r5		;	\
	SHOULDNT_BE_SET		;	\
	l.sfne	r4,r5		;	\
	SHOULD_BE_SET		;	\
	l.sfgtu	r4,r5		;	\
	SHOULDNT_BE_SET		;	\
	l.sfgeu	r4,r5		;	\
	SHOULDNT_BE_SET		;	\
	l.sfleu	r4,r5		;	\
	SHOULD_BE_SET		;	\
	l.sfltu	r4,r5		;	\
	SHOULD_BE_SET

#define SHOULD_BE_GREATER_THAN_UNSIGNED(a,b)	\
	MOVE_TO_R4R5_AND_REPORT(a,b) ;	\
	l.sfeq	r4,r5		;	\
	SHOULDNT_BE_SET		;	\
	l.sfne	r4,r5		;	\
	SHOULD_BE_SET		;	\
	l.sfgtu	r4,r5		;	\
	SHOULD_BE_SET		;	\
	l.sfgeu	r4,r5		;	\
	SHOULD_BE_SET		;	\
	l.sfleu	r4,r5		;	\
	SHOULDNT_BE_SET		;	\
	l.sfltu	r4,r5		;	\
	SHOULDNT_BE_SET

#define INT_MAX		2147483647	/*0x7fffffff*/
#define INT_MAX_MIN1	2147483646	/*0x7ffffffe*/	
#define NEG_INT_MAX	-2147483648	/*0x80000000*/
#define NEG_INT_MAX_PL1	-2147483647	/*0x80000001*/
#define MIN1		-1		/*0xffffffff*/
	
#define UINT_MAX	4294967295	/*0xffffffff*/
#define UINT_MAX_MIN1	4294967294	/*0xfffffffe*/	

	
	.global _start
_start:
	/* Signed tests */
	SHOULD_BE_LESS_THAN_SIGNED(0,1)
	
	SHOULD_BE_LESS_THAN_SIGNED(MIN1,0)
	
	SHOULD_BE_LESS_THAN_SIGNED(INT_MAX_MIN1,INT_MAX)
	
	SHOULD_BE_LESS_THAN_SIGNED(NEG_INT_MAX,INT_MAX)
	
	SHOULD_BE_LESS_THAN_SIGNED(NEG_INT_MAX,INT_MAX_MIN1)
	
	SHOULD_BE_LESS_THAN_SIGNED(NEG_INT_MAX_PL1,INT_MAX)
	
	SHOULD_BE_LESS_THAN_SIGNED(NEG_INT_MAX_PL1,INT_MAX_MIN1)

	SHOULD_BE_LESS_THAN_SIGNED(-7,-6)
	
	SHOULD_BE_LESS_THAN_SIGNED(NEG_INT_MAX,NEG_INT_MAX_PL1)
	
	SHOULD_BE_LESS_THAN_SIGNED(NEG_INT_MAX,MIN1)
	
	SHOULD_BE_LESS_THAN_SIGNED(NEG_INT_MAX,0)
	

	SHOULD_BE_GREATER_THAN_SIGNED(1,0)
	
	SHOULD_BE_GREATER_THAN_SIGNED(0,MIN1)
	
	SHOULD_BE_GREATER_THAN_SIGNED(INT_MAX,INT_MAX_MIN1)
	
	SHOULD_BE_GREATER_THAN_SIGNED(INT_MAX,NEG_INT_MAX)
	
	SHOULD_BE_GREATER_THAN_SIGNED(INT_MAX_MIN1,NEG_INT_MAX)
	
	SHOULD_BE_GREATER_THAN_SIGNED(INT_MAX,NEG_INT_MAX_PL1)
	
	SHOULD_BE_GREATER_THAN_SIGNED(INT_MAX_MIN1,NEG_INT_MAX_PL1)

	SHOULD_BE_GREATER_THAN_SIGNED(-6,-7)
	
	SHOULD_BE_GREATER_THAN_SIGNED(NEG_INT_MAX_PL1,NEG_INT_MAX)
	
	SHOULD_BE_GREATER_THAN_SIGNED(MIN1,NEG_INT_MAX)
	
	SHOULD_BE_GREATER_THAN_SIGNED(0,NEG_INT_MAX)

	/* Unsigned tests */
	SHOULD_BE_LESS_THAN_UNSIGNED(0,1)
	
	SHOULD_BE_LESS_THAN_UNSIGNED(UINT_MAX_MIN1,UINT_MAX)

	SHOULD_BE_GREATER_THAN_UNSIGNED(1,0)
	
	SHOULD_BE_GREATER_THAN_UNSIGNED(UINT_MAX,UINT_MAX_MIN1)

	SHOULD_BE_GREATER_THAN_UNSIGNED(UINT_MAX,0)

	SHOULD_BE_GREATER_THAN_UNSIGNED(0x80000001, 0x80000000)

	SHOULD_BE_LESS_THAN_UNSIGNED(0x80000000, 0x80000001)
	
	SHOULD_BE_GREATER_THAN_UNSIGNED(0x80000000,0x7fffffff)

	SHOULD_BE_LESS_THAN_UNSIGNED(0x7fffffff, 0x80000000)

	SHOULD_BE_GREATER_THAN_UNSIGNED(0x7fffffff, 0x7ffffffe)

	SHOULD_BE_LESS_THAN_UNSIGNED(0x7ffffffe, 0x7fffffff)

	SHOULD_BE_LESS_THAN_UNSIGNED(0x2024fae0, 0xfef03220)

_finish:	
	l.movhi r3, hi(0x8000000d)
	l.ori 	r3, r3, lo(0x8000000d)
	l.nop 	0x2
	l.ori 	r3, r0, 0	
	l.nop 	0x1

_fail:
	l.ori r3, r0, 0x1
	l.nop 0x1
	
