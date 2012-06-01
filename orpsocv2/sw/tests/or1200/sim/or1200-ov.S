/*
	OR1200 overflow bit checking

	Very basic, testing

TODO:	 Check range exception handling in delay slots

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

#define INCREMENT_EXCEPTION_COUNTER	l.addi r11, r11, 0x1
#define CHECK_EXCEPTION_COUNTER		\
	l.sfne	r11, r12	;	\
	l.bf	_fail		;	\
	l.nop			;
#define EXPECT_RANGE_EXCEPT		\
	l.addi	r12, r12, 1	;	\
	CHECK_EXCEPTION_COUNTER
	
/* ---[ 0xb00: Range exception ]---------------------------------------- */
        .org 0xb00
	l.sw 	0(r0), r3	;// Save r3 - don't disrupt it during exceptions
	l.ori 	r3, r0, 0xaaee
	l.nop 	0x2
	// TODO - get instruction and decode to ensure it was an instruction
	// which is capable of causing a range exception. Remember delay slot!
	INCREMENT_EXCEPTION_COUNTER

	// Clear OV in ESR
	l.mfspr	r3,r0,SPR_ESR_BASE 	;// Get ESR
	l.nop	2
	l.xori	r3, r3,SPR_SR_OV	;// Clear OV bit
	l.mtspr	r0,r3,SPR_ESR_BASE 	;// Get EPC 
	
	l.mfspr	r3,r0,SPR_EPCR_BASE 	;// Get EPC 
	l.nop	2
	l.addi	r3, r3, 0x4		;// Increment
	l.mtspr	r0,r3,SPR_EPCR_BASE 	;// Get EPC 
	// For now, increment EPCR so we step over instruction and continue

	l.lwz	r3, 0(r0)
	l.rfe

/* =================================================== [ text ] === */
	.section .text

/* =================================================== [ start ] === */	

	.global _start
_start:
	// Clear all regs
	l.movhi r1, 0
	l.movhi	r2, 0
	l.movhi	r3, 0
	l.movhi	r4, 0
	l.movhi	r5, 0
	l.movhi	r6, 0
	l.movhi	r7, 0
	l.movhi	r8, 0
	l.movhi	r9, 0
	l.movhi	r10, 0
	l.movhi	r11, 0
	l.movhi	r12, 0
	l.movhi	r13, 0
	l.movhi	r14, 0
	l.movhi	r15, 0
	l.movhi	r16, 0
	l.movhi	r17, 0
	l.movhi	r18, 0
	l.movhi	r19, 0
	l.movhi	r20, 0
	l.movhi	r21, 0
	l.movhi	r22, 0
	l.movhi	r23, 0
	l.movhi	r24, 0
	l.movhi	r25, 0
	l.movhi	r26, 0
	l.movhi	r27, 0
	l.movhi	r28, 0
	l.movhi	r29, 0
	l.movhi	r30, 0
	l.movhi	r31, 0
	
#ifdef OR1200_IMPL_OV
	// Kick off test
	l.jal   _main
#else
	// Not supported, exit test
	l.j	_finish
#endif	
	l.nop


/* =================================================== [ main ] === */


#define CHECK_OV_CLEAR			\
	l.mfspr	r20, r0, SPR_SR	;	\
	l.andi	r21, r20, SPR_SR_OV ;	\
	l.sfne	r21, r0		  ;	\
	l.bf	_fail		  ;	\
	l.nop				

#define CHECK_OV_SET			\
	l.mfspr	r20, r0, SPR_SR	;	\
	l.andi	r21, r20, SPR_SR_OV ;	\
	l.sfnei	r21, SPR_SR_OV	  ;	\
	l.bf	_fail		  ;	\
	l.addi	r12, r12, 0x1	  ;	// Increment expected counter
	
	.global _main	
_main:

	// Set up some values, check the OV bit is cleared
	CHECK_OV_CLEAR

	l.movhi	r4, 0x7fff
	l.ori	r4, r4, 0xefff

	l.ori	r5, r0,	0xffff

	l.add	r3, r5, r4	;// Should set overflow
	l.nop	0x2
	CHECK_OV_SET

	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_OV_CLEAR

	l.addi	r3, r4, 0x7fff	;// Should set overflow
	l.nop	0x2
	CHECK_OV_SET

	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_OV_CLEAR
	
	// Now test negative numbers

	l.movhi	r4, 0x8000
	l.ori	r4, r4, 0x0000

	l.movhi	r5, 0xffff
	l.ori	r5, r5,	0xffff

	l.add	r3, r4, r5 	// Biggest and smallest negative number
	l.nop   0x2
	CHECK_OV_SET

	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_OV_CLEAR

	l.addi	r3, r4, 0xffff 	// Biggest and smallest negative number
	l.nop   0x2
	CHECK_OV_SET

	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_OV_CLEAR

	l.add	r3, r4, r0 	// Biggest negative number, and zero
	l.nop   0x2
	CHECK_OV_CLEAR
	
	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_OV_CLEAR

	l.movhi	r5, 0xffff
	l.ori	r5, r5,	0xfffe

	l.add	r3, r4, r5 	// Biggest and second smallest negative number
	l.nop   0x2
	CHECK_OV_SET

	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_OV_CLEAR

#ifdef OR1200_IMPL_SUB

	// report indicator that we're at l.sub section 0x55555555
	l.movhi	r3, 0x5555
	l.ori	r3, r3, 0x5555
	l.nop 	0x2
	
	// Quick subtract check
	// Check largest negative number -1 tripping overflow
	l.ori 	r5, r0, 1		; // +1
	l.sub	r3, r4, r5		; // -2147483647 - 1
	l.nop	0x2
	CHECK_OV_SET

	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_OV_CLEAR
#endif	
	
	l.movhi	r4, 0x8000
	l.ori	r4, r4, 0x0437

	l.movhi	r5, 0xffff
	l.ori	r5, r5,	0xfbc7

	l.add	r3, r4, r5 	// Very big negative number, another one big
	                        // enough to cause overflow hopefully
	l.nop   0x2
	CHECK_OV_SET

	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_OV_CLEAR

	l.movhi	r5, 0xffff	
	l.ori	r5, r5,	0xfff7

	l.add	r3, r4, r5	// Two negative numbers but shouldn't overflow
	l.nop   0x2
	CHECK_OV_CLEAR

#ifdef OR1200_DIV_IMPLEMENTED

	// report indicator that we're at l.div section 0xdddddddd
	l.movhi	r3, 0xdddd
	l.ori	r3, r3, 0xdddd
	l.nop 	0x2

	// Test divide by zero
	l.div	r3, r5, r0
	l.nop 	0x2

	l.sfne	r3, r0		;// Check result was 0
	l.bf	_fail	
	
	CHECK_OV_SET

	l.add 	r3, r5, r0	;// Should clear overflow
	l.nop	0x2
	
	CHECK_OV_CLEAR

	// Test divide by zero
	l.divu	r3, r0, r0
	l.nop 	0x2
	CHECK_OV_SET

	l.sfne	r3, r0		;// Check result was 0
	l.bf	_fail	

	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_OV_CLEAR
	
#endif

#ifdef OR1200_MULT_IMPLEMENTED

	// report indicator that we're at l.multiply section 0x11111111
	l.movhi	r3, 0x1111
	l.ori	r3, r3, 0x1111
	l.nop 	0x2
	
	// Check multiplying two large numbers, which will cause overflow,
	// trigger the flag appropriately

	// First signed multiply.
	l.movhi	r4, 0xd555	;//-(((2^32)-1)/3 + 2)
	l.ori	r4, r4, 0x5552
	l.ori	r5, r0, 2
	l.ori	r6, r0, 3

	// First multiply big negative number by 2 - shouldn't overflow
	l.mul	r3, r4, r5
	l.nop 	0x2
	CHECK_OV_CLEAR

	// Now multiply by 3 - should just overflow negative
	l.mul	r3, r4, r6
	l.nop 	0x2
	CHECK_OV_SET

	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_OV_CLEAR
	
	// Now some big positive values
	l.movhi	r4, 0x2aaa	;//((2^32)-1)/3 + 2
	l.ori	r4, r4, 0xaaae
	l.ori	r5, r0, 2
	l.ori	r6, r0, 3

	// First multiply big number by 2 - shouldn't overflow
	l.mul	r3, r4, r5
	l.nop 	0x2
	CHECK_OV_CLEAR

	// Now multiply by 3 - should only just overflow
	l.mul	r3, r4, r6
	l.nop 	0x2
	CHECK_OV_SET

	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_OV_CLEAR

	// First multiply big number by 2 - shouldn't overflow
	l.muli	r3, r4, 0x2
	l.nop 	0x2
	CHECK_OV_CLEAR

	// Now multiply by 3 - should just overflow negative
	l.muli	r3, r4, 0x3
	l.nop 	0x2
	CHECK_OV_SET

	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_OV_CLEAR


	// Now check overflow on  unsigned multiply

	// Some stimulus to make a 32-bit multiply overflow
	l.movhi	r4, 0x5555	;//((2^32))/3 + 2
	l.ori	r4, r4, 0x5557
	l.ori	r5, r0, 2
	l.ori	r6, r0, 3

	// First multiply big negative number by 2 - shouldn't overflow
	l.mulu	r3, r4, r5
	l.nop 	0x2
	CHECK_OV_CLEAR

	// Now multiply by 3 - should just overflow negative
	l.mulu	r3, r4, r6
	l.nop 	0x2
	CHECK_OV_SET

	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_OV_CLEAR
	
#endif

	/////////////////////////////////////////////////////////////
	//                                                         //
	// Range Exception Tests                                   //
	//                                                         //
	/////////////////////////////////////////////////////////////
#ifdef OR1200_IMPL_OVE	
	// report indicator that we're at exception section 0xeeeeeeee
	l.movhi	r3, 0xeeee
	l.ori	r3, r3, 0xeeee
	l.nop 	0x2
	
	// First enable OV exception in SR
	
	l.mfspr	r20, r0, SPR_SR
	l.ori	r21, r20, SPR_SR_OVE
	l.mtspr r0, r21, SPR_SR

	// Check it's set
	l.mfspr	r20, r0, SPR_SR
	l.andi	r21, r20, SPR_SR_OVE
	l.sfnei	r21, SPR_SR_OVE
	l.bf	_fail
	l.nop

	// now set r11 to r12 to know how many tests we've done so far
	l.or	r11, r12, r12
	
	l.movhi	r4, 0x7fff
	l.ori	r4, r4, 0xefff

	l.ori	r5, r0,	0xffff

	l.add	r3, r5, r4	;// Should set overflow
	l.nop	0x2
	EXPECT_RANGE_EXCEPT

	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_EXCEPTION_COUNTER
	

	l.addi	r3, r4, 0x7fff	;// Should set overflow
	l.nop	0x2
	EXPECT_RANGE_EXCEPT
	

	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_EXCEPTION_COUNTER
	
	
	// Now test negative numbers

	l.movhi	r4, 0x8000
	l.ori	r4, r4, 0x0000

	l.movhi	r5, 0xffff
	l.ori	r5, r5,	0xffff

	l.add	r3, r4, r5 	// Biggest and smallest negative number
	l.nop   0x2
	EXPECT_RANGE_EXCEPT
	

	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_EXCEPTION_COUNTER

	l.addi	r3, r4, 0xffff 	// Biggest and smallest negative number
	l.nop   0x2
	EXPECT_RANGE_EXCEPT
	

	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_EXCEPTION_COUNTER

	l.add	r3, r4, r0 	// Biggest negative number, and zero
	l.nop   0x2
	CHECK_EXCEPTION_COUNTER
		
	
	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_EXCEPTION_COUNTER
		

	l.movhi	r5, 0xffff
	l.ori	r5, r5,	0xfffe

	l.add	r3, r4, r5 	// Biggest and second smallest negative number
	l.nop   0x2
	EXPECT_RANGE_EXCEPT
		

	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_EXCEPTION_COUNTER

#ifdef OR1200_IMPL_SUB

	// report indicator that we're at l.sub section 0x55555555
	l.movhi	r3, 0x5555
	l.ori	r3, r3, 0x5555
	l.nop 	0x2
	
	// Quick subtract check
	// Check largest negative number -1 tripping overflow
	l.ori 	r5, r0, 1		; // +1
	l.sub	r3, r4, r5		; // -2147483647 - 1
	l.nop	0x2
	EXPECT_RANGE_EXCEPT
		

	l.sub 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_EXCEPTION_COUNTER
		
#endif	
	
	l.movhi	r4, 0x8000
	l.ori	r4, r4, 0x0437

	l.movhi	r5, 0xffff
	l.ori	r5, r5,	0xfbc7

	l.add	r3, r4, r5 	// Very big negative number, another one big
	                        // enough to cause overflow hopefully
	l.nop   0x2
	EXPECT_RANGE_EXCEPT
		

	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_EXCEPTION_COUNTER

	l.movhi	r5, 0xffff	
	l.ori	r5, r5,	0xfff7

	l.add	r3, r4, r5	// Two negative numbers but shouldn't overflow
	l.nop   0x2
	CHECK_EXCEPTION_COUNTER

#ifdef OR1200_DIV_IMPLEMENTED

	// report indicator that we're at l.div section 0xdddddddd
	l.movhi	r3, 0xdddd
	l.ori	r3, r3, 0xdddd
	l.nop 	0x2

	// Test divide by zero
	l.div	r3, r5, r0
	l.nop 	0x2

	l.sfne	r3, r0		;// Check result was 0
	l.bf	_fail	
	
	EXPECT_RANGE_EXCEPT
		

	l.add 	r3, r5, r0	;// Should clear overflow
	l.nop	0x2
	
	CHECK_EXCEPTION_COUNTER

	// Test divide by zero
	l.divu	r3, r0, r0
	l.nop 	0x2
	EXPECT_RANGE_EXCEPT
		

	l.sfne	r3, r0		;// Check result was 0
	l.bf	_fail	

	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_EXCEPTION_COUNTER
	
#endif

#ifdef OR1200_MULT_IMPLEMENTED

	// report indicator that we're at l.multiply section 0x11111111
	l.movhi	r3, 0x1111
	l.ori	r3, r3, 0x1111
	l.nop 	0x2
	
	// Check multiplying two large numbers, which will cause overflow,
	// trigger the flag appropriately

	// First signed multiply.
	l.movhi	r4, 0xd555	;//-(((2^32)-1)/3 + 2)
	l.ori	r4, r4, 0x5552
	l.ori	r5, r0, 2
	l.ori	r6, r0, 3

	// First multiply big negative number by 2 - shouldn't overflow
	l.mul	r3, r4, r5
	l.nop 	0x2
	CHECK_EXCEPTION_COUNTER

	// Now multiply by 3 - should just overflow negative
	l.mul	r3, r4, r6
	l.nop 	0x2
	EXPECT_RANGE_EXCEPT

	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_EXCEPTION_COUNTER
	
	// Now some big positive values
	l.movhi	r4, 0x2aaa	;//((2^32)-1)/3 + 2
	l.ori	r4, r4, 0xaaae
	l.ori	r5, r0, 2
	l.ori	r6, r0, 3

	// First multiply big number by 2 - shouldn't overflow
	l.mul	r3, r4, r5
	l.nop 	0x2
	CHECK_EXCEPTION_COUNTER

	// Now multiply by 3 - should only just overflow
	l.mul	r3, r4, r6
	l.nop 	0x2
	EXPECT_RANGE_EXCEPT

	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_EXCEPTION_COUNTER

	// First multiply big number by 2 - shouldn't overflow
	l.muli	r3, r4, 0x2
	l.nop 	0x2
	CHECK_EXCEPTION_COUNTER

	// Now multiply by 3 - should just overflow negative
	l.muli	r3, r4, 0x3
	l.nop 	0x2
	EXPECT_RANGE_EXCEPT

	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_EXCEPTION_COUNTER


	// Now check overflow on  unsigned multiply

	// Some stimulus to make a 32-bit multiply overflow
	l.movhi	r4, 0x5555	;//((2^32))/3 + 2
	l.ori	r4, r4, 0x5557
	l.ori	r5, r0, 2
	l.ori	r6, r0, 3

	// First multiply big negative number by 2 - shouldn't overflow
	l.mulu	r3, r4, r5
	l.nop 	0x2
	CHECK_EXCEPTION_COUNTER

	// Now multiply by 3 - should just overflow negative
	l.mulu	r3, r4, r6
	l.nop 	0x2
	EXPECT_RANGE_EXCEPT

	l.add 	r3, r0, r0	;// Should clear overflow
	l.nop	0x2
	CHECK_EXCEPTION_COUNTER
	
#endif

#endif
	

_finish:
	l.movhi r3, hi(0x8000000d)
	l.ori 	r3, r3, lo(0x8000000d)
	l.nop 	0x2
	l.ori 	r3, r0, 0	
	l.nop 	0x1

_fail:	
	l.or 	r3, r12, r0	;// Fail and report test number we were up to
	l.nop 	0x1
