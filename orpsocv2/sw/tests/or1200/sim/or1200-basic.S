/*

	Basic instruction set test

	Requires hardware multiply (uses l.muli and l.mul)

	We specify our own reset and initialisation routines as we don't link
	in the usual initialisation code.

	Based on original or1200 instruction set test

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
#include "or1200-defines.h"

// Check multiply unit is enabled before trying to run this test
#ifndef OR1200_MULT_IMPLEMENTED	
# error
# error No multiply unit detected. This test requires hardware multiply support
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
	l.jal _cache_init
	l.nop
	
	// Kick off test
	l.jal   _main
	l.nop
	


/* =================================================== [ main ] === */
	
	.global _main	
_main:
	l.nop
	l.j	_regs
	l.nop
	

_regs:
	l.addi	r1,r0,0x1
	l.addi	r2,r1,0x2
	l.addi	r3,r2,0x4
	l.addi	r4,r3,0x8
	l.addi	r5,r4,0x10
	l.addi	r6,r5,0x20
	l.addi	r7,r6,0x40
	l.addi	r8,r7,0x80
	l.addi	r9,r8,0x100
	l.addi	r10,r9,0x200
	l.addi	r11,r10,0x400
	l.addi	r12,r11,0x800
	l.addi	r13,r12,0x1000
	l.addi	r14,r13,0x2000
	l.addi	r15,r14,0x4000
	l.addi	r16,r15,0x8000

	l.sub	r31,r0,r1
	l.sub	r30,r31,r2
	l.sub	r29,r30,r3
	l.sub	r28,r29,r4
	l.sub	r27,r28,r5
	l.sub	r26,r27,r6
	l.sub	r25,r26,r7
	l.sub	r24,r25,r8
	l.sub	r23,r24,r9
	l.sub	r22,r23,r10
	l.sub	r21,r22,r11
	l.sub	r20,r21,r12
	l.sub	r19,r20,r13
	l.sub	r18,r19,r14
	l.sub	r17,r18,r15
	l.sub	r16,r17,r16
	l.movhi r31,0x0000
	l.ori	r31,r31,0x0040

	l.mtspr	r0,r16,0x1234	/* Should be 0xffff0012 */

	l.sw	0(r31),r16


_mem:	l.movhi	r3,0x1234
	l.ori	r3,r3,0x5678

	l.sw	4(r31),r3

	l.lbz	r4,4(r31)
	l.add	r8,r8,r4
	l.sb	11(r31),r4
	l.lbz	r4,5(r31)
	l.add	r8,r8,r4
	l.sb	10(r31),r4
	l.lbz	r4,6(r31)
	l.add	r8,r8,r4
	l.sb	9(r31),r4
	l.lbz	r4,7(r31)
	l.add	r8,r8,r4
	l.sb	8(r31),r4

	l.lbs   r4,8(r31)
        l.add   r8,r8,r4
        l.sb    7(r31),r4
        l.lbs   r4,9(r31)
        l.add   r8,r8,r4
        l.sb    6(r31),r4
        l.lbs   r4,10(r31)
        l.add   r8,r8,r4
        l.sb    5(r31),r4
        l.lbs   r4,11(r31)
        l.add   r8,r8,r4
        l.sb    4(r31),r4
	
	l.lhz   r4,4(r31)
        l.add   r8,r8,r4
        l.sh    10(r31),r4
        l.lhz   r4,6(r31)
        l.add   r8,r8,r4
        l.sh    8(r31),r4

	l.lhs   r4,8(r31)
        l.add   r8,r8,r4
        l.sh    6(r31),r4
        l.lhs   r4,10(r31)
        l.add   r8,r8,r4
        l.sh    4(r31),r4

	l.lwz	r4,4(r31)
	l.add   r8,r8,r4

	l.mtspr r0,r8,0x1234   /* Should be 0x12352af7 */

	l.lwz	r9,0(r31)
	l.add	r8,r9,r8
	l.sw	0(r31),r8
	

_arith:
	l.addi	r3,r0,1
	l.addi	r4,r0,2
	l.addi	r5,r0,-1
	l.addi	r6,r0,-1
	l.addi	r8,r0,0
	
	l.sub	r7,r5,r3
	l.sub	r8,r3,r5
	l.add	r8,r8,r7
	
#	l.div	r7,r7,r4
	l.add	r9,r3,r4
	l.mul	r7,r9,r7
#	l.divu	r7,r7,r4
	l.add	r8,r8,r7

	l.mtspr r0,r8,0x1234   /* Should be 0x7ffffffe */

	l.lwz	r9,0(r31)
	l.add	r8,r9,r8
	l.sw	0(r31),r8
	

_log:
	l.addi  r3,r0,1
        l.addi  r4,r0,2
        l.addi	r5,r0,-1
        l.addi	r6,r0,-1
        l.addi	r8,r0,0 

	l.andi	r8,r8,1
	l.and	r8,r8,r3

	l.xori	r8,r5,0xa5a5
	l.xor	r8,r8,r5

	l.ori	r8,r8,2
	l.or	r8,r8,r4

	l.mtspr r0,r8,0x1234   /* Should be 0xffffa5a7 */

	l.lwz	r9,0(r31)
	l.add	r8,r9,r8
	l.sw	0(r31),r8


_shift:
	l.addi  r3,r0,1
        l.addi  r4,r0,2
        l.addi  r5,r0,-1
        l.addi  r6,r0,-1
        l.addi  r8,r0,0

	l.slli	r8,r5,6
	l.sll	r8,r8,r4
	
	l.srli	r8,r8,6
	l.srl	r8,r8,r4
	
	l.srai	r8,r8,2
	l.sra	r8,r8,r4	

	l.mtspr r0,r8,0x1234   /* Should be 0x000fffff */

	l.lwz	r9,0(r31)
	l.add	r8,r9,r8
	l.sw	0(r31),r8
	

_flag:
	l.addi  r3,r0,1
        l.addi  r4,r0,-2
        l.addi  r8,r0,0

	l.sfeq	r3,r3
	l.mfspr	r5,r0,17
	l.andi	r4,r5,0x200
	l.add	r8,r8,r4

	l.sfeq  r3,r4
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4

	l.sfeqi r3,1
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4
 
        l.sfeqi r3,-2
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4 
	
	l.sfne  r3,r3
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4
 
        l.sfne  r3,r4
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4 

	l.sfnei r3,1
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4
 
        l.sfnei r3,-2
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4

	l.sfgtu r3,r3
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4
 
        l.sfgtu r3,r4
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4

	l.sfgtui 	r3,1
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4
 
        l.sfgtui	r3,-2
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4

	l.sfgeu r3,r3
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4
 
        l.sfgeu r3,r4
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4

	l.sfgeui        r3,1
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4
 
        l.sfgeui        r3,-2
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4

	l.sfltu r3,r3
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4
 
        l.sfltu r3,r4
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4

	l.sfltui        r3,1
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4
 
        l.sfltui        r3,-2
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4

	l.sfleu r3,r3
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4
 
        l.sfleu r3,r4
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4
 
        l.sfleui        r3,1
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4
 
        l.sfleui        r3,-2
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4

	l.sfgts r3,r3
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4
 
        l.sfgts r3,r4
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4
 
        l.sfgtsi        r3,1
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4
 
        l.sfgtsi        r3,-2
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4

	l.sfges r3,r3
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4
 
        l.sfges r3,r4
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4
 
        l.sfgesi        r3,1
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4
 
        l.sfgesi        r3,-2
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4

	l.sflts r3,r3
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4
 
        l.sflts r3,r4
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4
 
        l.sfltsi        r3,1
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4
 
        l.sfltsi        r3,-2
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4

	l.sfles r3,r3
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4
 
        l.sfles r3,r4
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4
 
        l.sflesi        r3,1
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4
 
        l.sflesi        r3,-2
        l.mfspr r5,r0,17
        l.andi  r4,r5,0x200
        l.add   r8,r8,r4

	l.mtspr r0,r8,0x1234   /* Should be 0x00002800 */

	l.lwz	r9,0(r31)
	l.add	r8,r9,r8
	l.sw	0(r31),r8


_dslot:
	l.addi	r14,r0,0x4      /* inc. loop cnt. - must be non-zero */
                                /* and operand at 1st operation */
	l.addi	r15,r0,0x14     /* inc. cnt. limit value (4+4+4+4+4) */
	l.addi	r21,r0,0x40     /* mul. by 2 cnt. limit value (4*2*2*2*2) */
	l.addi	r16,r0,0x10     /* dec. loop cnt. - limits at 0 */
                                /* loop counters are changed by 4 due */
                                /* to value is used by l.lwz/l.sw */
	l.addi	r17,r0,7        /* operand at 2nd operation */
                                /* and test result */
	l.addi	r18,r0,8        /* operand at 2nd operation */
	l.addi	r19,r0,9        /* operand at 3rd operation */
	l.addi	r20,r0,-1       /* xor every intermediate result by */
                                /* value 0xffffffff */

	l.sw	(0x0)(r14),r0   /* init RAM to zero */
	l.sw	(0x4)(r14),r0
	l.sw	(0x8)(r14),r0
	l.sw	(0xc)(r14),r0
	l.sw	(0x10)(r14),r0
	l.sw	(0x14)(r14),r0
	l.sw	(0x18)(r14),r0
	l.sw	(0x1c)(r14),r0
	l.sw	(0x20)(r14),r0
	l.sw	(0x24)(r14),r0
	l.sw	(0x28)(r14),r0
	l.sw	(0x2c)(r14),r0


        /* ins. in dslot - loop 1: */
        /* 1st operation operand before branch is NOT used to set flag */
        /* 1st and 2nd operation operands before and after branch are NOT related */
        /* 2nd and 3rd operation operands before and after jump are NOT related */
        /* 1st operation before branch is SINGLE cycle */
        /* 2nd operation after branch/before jump is SINGLE cycle */
        /* 3rd operation after jump is SINGLE cycle */
	l.addi	r14,r0,0x4      /* init inc. loop cnt. */
	l.addi	r16,r0,0x10     /* init dec. loop cnt. */
_D1:
	l.add	r17,r17,r14	/* merge test case operands */
	l.add	r17,r17,r19	/* merge test case operands */
	l.xor	r17,r17,r20	/* invert test result value */
	l.addi	r16,r16,-4	/* dec. cnt. */
	l.addi	r14,r14,4	/* inc. cnt. - r14..1st operation operand */
	l.sfeqi	r16,0		 
	l.bf	_D2s
	l.add	r17,r17,r18	/* r17,r18..2nd opertion operands */
	l.j	_D1
	l.addi	r19,r19,1	/* r19..3rd operation operand */


        /* ins. in dslot - loop 2: */
        /* 1st operation operand before branch is used to set flag */
        /* 1st and 2nd operation operands before and after branch are NOT related */
        /* 2nd and 3rd operation operands before and after jump are related */
        /* 1st operation before branch is SINGLE cycle */
        /* 2nd operation after branch/before jump is SINGLE cycle */
        /* 3rd operation after jump is SINGLE cycle */
_D2s:
	l.addi	r14,r0,0x4      /* init inc. loop cnt. */
	l.addi	r16,r0,0x10     /* init dec. loop cnt. */
_D2:
	l.add	r17,r17,r14	/* merge test case operands */
	l.add	r17,r17,r19	/* merge test case operands */
	l.xor	r17,r17,r20	/* invert test result value */
	l.addi	r16,r16,-4	/* dec. cnt. */
	l.addi	r14,r14,4	/* inc. cnt. - r14..1st operation operand */
	l.sfgeu	r14,r15		
	l.bf	_D3s
	l.add	r17,r17,r18	/* r17,r18..2nd opertion operands */
	l.j	_D2
	l.addi	r19,r17,1	/* r19,r17..3rd operation operand */


        /* ins. in dslot - loop 3: */
        /* 1st operation operand before branch is NOT used to set flag */
        /* 1st and 2nd operation operands before and after branch are related */
        /* 2nd and 3rd operation operands before and after jump are NOT related */
        /* 1st operation before branch is SINGLE cycle */
        /* 2nd operation after branch/before jump is SINGLE cycle */
        /* 3rd operation after jump is MULTI cycle */
_D3s:
	l.addi	r14,r0,0x4      /* init inc. loop cnt. */
	l.addi	r16,r0,0x10     /* init dec. loop cnt. */
_D3:
	l.add	r17,r17,r14	/* merge test case operands */
	l.add	r17,r17,r19	/* merge test case operands */
	l.xor	r17,r17,r20	/* invert test result value */
	l.addi	r16,r16,-4	/* dec. cnt. */
	l.addi	r14,r14,4	/* inc. cnt. - r14..1st operation operand */
	l.sfeqi	r16,0		
	l.bf	_D4s
	l.add	r17,r17,r14	/* r17,r14..2nd opertion operands */
	l.j	_D3
	l.muli	r19,r19,2	/* r19..3rd operation operand */


        /* ins. in dslot - loop 4: */
        /* 1st operation operand before branch is used to set flag */
        /* 1st and 2nd operation operands before and after branch are related */
        /* 2nd and 3rd operation operands before and after jump are related */
        /* 1st operation before branch is SINGLE cycle */
        /* 2nd operation after branch/before jump is SINGLE cycle */
        /* 3rd operation after jump is MULTI cycle */
_D4s:
	l.addi	r14,r0,0x4      /* init inc. loop cnt. */
	l.addi	r16,r0,0x10     /* init dec. loop cnt. */
_D4:
	l.add	r17,r17,r14	/* merge test case operands */
	l.add	r17,r17,r19	/* merge test case operands */
	l.xor	r17,r17,r20	/* invert test result value */
	l.addi	r16,r16,-4	/* dec. cnt. */
	l.addi	r14,r14,4	/* inc. cnt. - r14..1st operation operand */
	l.sfgeu	r14,r15
	l.bf	_D5s
	l.add	r17,r17,r14	/* r17,r14..2nd opertion operands */
	l.j	_D4
	l.muli	r19,r17,1	/* r19,r17..3rd operation operand */


        /* ins. in dslot - loop 5: */
        /* 1st operation operand before branch is NOT used to set flag */
        /* 1st and 2nd operation operands before and after branch are NOT related */
        /* 2nd and 3rd operation operands before and after jump are NOT related */
        /* 1st operation before branch is SINGLE cycle */
        /* 2nd operation after branch/before jump is MULTI cycle */
        /* 3rd operation after jump is SINGLE cycle */
_D5s:
	l.addi	r14,r0,0x4      /* init inc. loop cnt. */
	l.addi	r16,r0,0x10     /* init dec. loop cnt. */
_D5:
	l.add	r17,r17,r14	/* merge test case operands */
	l.add	r17,r17,r19	/* merge test case operands */
	l.xor	r17,r17,r20	/* invert test result value */
	l.addi	r16,r16,-4	/* dec. cnt. */
	l.addi	r14,r14,4	/* inc. cnt. - r14..1st operation operand */
	l.sfeqi	r16,0		
	l.bf	_D6s
	l.mul	r17,r17,r18	/* r17,r18..2nd opertion operands */
	l.j	_D5
	l.addi	r19,r19,1	/* r19..3rd operation operand */


        /* ins. in dslot - loop 6: */
        /* 1st operation operand before branch is used to set flag */
        /* 1st and 2nd operation operands before and after branch are NOT related */
        /* 2nd and 3rd operation operands before and after jump are related */
        /* 1st operation before branch is SINGLE cycle */
        /* 2nd operation after branch/before jump is MULTI cycle */
        /* 3rd operation after jump is SINGLE cycle */
_D6s:
	l.addi	r14,r0,0x4      /* init inc. loop cnt. */
	l.addi	r16,r0,0x10     /* init dec. loop cnt. */
_D6:
	l.add	r17,r17,r14	/* merge test case operands */
	l.add	r17,r17,r19	/* merge test case operands */
	l.xor	r17,r17,r20	/* invert test result value */
	l.addi	r16,r16,-4	/* dec. cnt. */
	l.addi	r14,r14,4	/* inc. cnt. - r14..1st operation operand */
	l.sfgeu	r14,r15
	l.bf	_D7s
	l.mul	r17,r17,r18	/* r17,r18..2nd opertion operands */
	l.j	_D6
	l.addi	r19,r17,1	/* r19,r17..3rd operation operand */


        /* ins. in dslot - loop 7: */
        /* 1st operation operand before branch is NOT used to set flag */
        /* 1st and 2nd operation operands before and after branch are related */
        /* 2nd and 3rd operation operands before and after jump are NOT related */
        /* 1st operation before branch is SINGLE cycle */
        /* 2nd operation after branch/before jump is MULTI cycle */
        /* 3rd operation after jump is MULTI cycle */
_D7s:
	l.addi	r14,r0,0x4      /* init inc. loop cnt. */
	l.addi	r16,r0,0x10     /* init dec. loop cnt. */
_D7:
	l.add	r17,r17,r14	/* merge test case operands */
	l.add	r17,r17,r19	/* merge test case operands */
	l.xor	r17,r17,r20	/* invert test result value */
	l.addi	r16,r16,-4	/* dec. cnt. */
	l.addi	r14,r14,4	/* inc. cnt. - r14..1st operation operand */
	l.sfeqi	r16,0		
	l.bf	_D8s
	l.mul	r17,r17,r14	/* r17,r14..2nd opertion operands */
	l.j	_D7
	l.muli	r19,r19,2	/* r19..3rd operation operand */


        /* ins. in dslot - loop 8: */
        /* 1st operation operand before branch is used to set flag */
        /* 1st and 2nd operation operands before and after branch are related */
        /* 2nd and 3rd operation operands before and after jump are related */
        /* 1st operation before branch is SINGLE cycle */
        /* 2nd operation after branch/before jump is MULTI cycle */
        /* 3rd operation after jump is MULTI cycle */
_D8s:
	l.addi	r14,r0,0x4      /* init inc. loop cnt. */
	l.addi	r16,r0,0x10     /* init dec. loop cnt. */
_D8:
	l.add	r17,r17,r14	/* merge test case operands */
	l.add	r17,r17,r19	/* merge test case operands */
	l.xor	r17,r17,r20	/* invert test result value */
	l.addi	r16,r16,-4	/* dec. cnt. */
	l.addi	r14,r14,4	/* inc. cnt. - r14..1st operation operand */
	l.sfgeu	r14,r15
	l.bf	_D9s
	l.mul	r17,r17,r14	/* r17,r14..2nd opertion operands */
	l.j	_D8
	l.muli	r19,r17,1	/* r19,r17..3rd operation operand */


        /* ins. in dslot - loop 9: */
        /* 1st operation operand before branch is NOT used to set flag */
        /* 1st and 2nd operation operands before and after branch are NOT related */
        /* 2nd and 3rd operation operands before and after jump are NOT related */
        /* 1st operation before branch is MULTI cycle */
        /* 2nd operation after branch/before jump is SINGLE cycle */
        /* 3rd operation after jump is SINGLE cycle */
_D9s:
	l.addi	r14,r0,0x4      /* init inc. loop cnt. */
	l.addi	r16,r0,0x10     /* init dec. loop cnt. */
_D9:
	l.add	r17,r17,r14	/* merge test case operands */
	l.add	r17,r17,r19	/* merge test case operands */
	l.xor	r17,r17,r20	/* invert test result value */
	l.addi	r16,r16,-4	/* dec. cnt. */
	l.muli	r14,r14,2	/* mul. cnt. - r14..1st operation operand */
	l.sfeqi	r16,0		 
	l.bf	_D10s
	l.add	r17,r17,r18	/* r17,r18..2nd opertion operands */
	l.j	_D9
	l.addi	r19,r19,1	/* r19..3rd operation operand */


        /* ins. in dslot - loop 10: */
        /* 1st operation operand before branch is used to set flag */
        /* 1st and 2nd operation operands before and after branch are NOT related */
        /* 2nd and 3rd operation operands before and after jump are related */
        /* 1st operation before branch is MULTI cycle */
        /* 2nd operation after branch/before jump is SINGLE cycle */
        /* 3rd operation after jump is SINGLE cycle */
_D10s:
	l.addi	r14,r0,0x4      /* init inc. loop cnt. */
	l.addi	r16,r0,0x10     /* init dec. loop cnt. */
_D10:
	l.add	r17,r17,r14	/* merge test case operands */
	l.add	r17,r17,r19	/* merge test case operands */
	l.xor	r17,r17,r20	/* invert test result value */
	l.addi	r16,r16,-4	/* dec. cnt. */
	l.muli	r14,r14,2	/* mul. cnt. - r14..1st operation operand */
	l.sfgeu	r14,r21		
	l.bf	_D11s
	l.add	r17,r17,r18	/* r17,r18..2nd opertion operands */
	l.j	_D10
	l.addi	r19,r17,1	/* r19,r17..3rd operation operand */


        /* ins. in dslot - loop 11: */
        /* 1st operation operand before branch is NOT used to set flag */
        /* 1st and 2nd operation operands before and after branch are related */
        /* 2nd and 3rd operation operands before and after jump are NOT related */
        /* 1st operation before branch is MULTI cycle */
        /* 2nd operation after branch/before jump is SINGLE cycle */
        /* 3rd operation after jump is MULTI cycle */
_D11s:
	l.addi	r14,r0,0x4      /* init inc. loop cnt. */
	l.addi	r16,r0,0x10     /* init dec. loop cnt. */
_D11:
	l.add	r17,r17,r14	/* merge test case operands */
	l.add	r17,r17,r19	/* merge test case operands */
	l.xor	r17,r17,r20	/* invert test result value */
	l.addi	r16,r16,-4	/* dec. cnt. */
	l.muli	r14,r14,2	/* mul. cnt. - r14..1st operation operand */
	l.sfeqi	r16,0		
	l.bf	_D12s
	l.add	r17,r17,r14	/* r17,r14..2nd opertion operands */
	l.j	_D11
	l.muli	r19,r19,2	/* r19..3rd operation operand */


        /* ins. in dslot - loop 12: */
        /* 1st operation operand before branch is used to set flag */
        /* 1st and 2nd operation operands before and after branch are related */
        /* 2nd and 3rd operation operands before and after jump are related */
        /* 1st operation before branch is MULTI cycle */
        /* 2nd operation after branch/before jump is SINGLE cycle */
        /* 3rd operation after jump is MULTI cycle */
_D12s:
	l.addi	r14,r0,0x4      /* init inc. loop cnt. */
	l.addi	r16,r0,0x10     /* init dec. loop cnt. */
_D12:
	l.add	r17,r17,r14	/* merge test case operands */
	l.add	r17,r17,r19	/* merge test case operands */
	l.xor	r17,r17,r20	/* invert test result value */
	l.addi	r16,r16,-4	/* dec. cnt. */
	l.muli	r14,r14,2	/* mul. cnt. - r14..1st operation operand */
	l.sfgeu	r14,r21
	l.bf	_D13s
	l.add	r17,r17,r14	/* r17,r14..2nd opertion operands */
	l.j	_D12
	l.muli	r19,r17,1	/* r19,r17..3rd operation operand */


        /* ins. in dslot - loop 13: */
        /* 1st operation operand before branch is NOT used to set flag */
        /* 1st and 2nd operation operands before and after branch are NOT related */
        /* 2nd and 3rd operation operands before and after jump are NOT related */
        /* 1st operation before branch is MULTI cycle */
        /* 2nd operation after branch/before jump is MULTI cycle */
        /* 3rd operation after jump is SINGLE cycle */
_D13s:
	l.addi	r14,r0,0x4      /* init inc. loop cnt. */
	l.addi	r16,r0,0x10     /* init dec. loop cnt. */
_D13:
	l.add	r17,r17,r14	/* merge test case operands */
	l.add	r17,r17,r19	/* merge test case operands */
	l.xor	r17,r17,r20	/* invert test result value */
	l.addi	r16,r16,-4	/* dec. cnt. */
	l.muli	r14,r14,2	/* mul. cnt. - r14..1st operation operand */
	l.sfeqi	r16,0		
	l.bf	_D14s
	l.mul	r17,r17,r18	/* r17,r18..2nd opertion operands */
	l.j	_D13
	l.addi	r19,r19,1	/* r19..3rd operation operand */


        /* ins. in dslot - loop 14: */
        /* 1st operation operand before branch is used to set flag */
        /* 1st and 2nd operation operands before and after branch are NOT related */
        /* 2nd and 3rd operation operands before and after jump are related */
        /* 1st operation before branch is MULTI cycle */
        /* 2nd operation after branch/before jump is MULTI cycle */
        /* 3rd operation after jump is SINGLE cycle */
_D14s:
	l.addi	r14,r0,0x4      /* init inc. loop cnt. */
	l.addi	r16,r0,0x10     /* init dec. loop cnt. */
_D14:
	l.add	r17,r17,r14	/* merge test case operands */
	l.add	r17,r17,r19	/* merge test case operands */
	l.xor	r17,r17,r20	/* invert test result value */
	l.addi	r16,r16,-4	/* dec. cnt. */
	l.muli	r14,r14,2	/* mul. cnt. - r14..1st operation operand */
	l.sfgeu	r14,r21
	l.bf	_D15s
	l.mul	r17,r17,r18	/* r17,r18..2nd opertion operands */
	l.j	_D14
	l.addi	r19,r17,1	/* r19,r17..3rd operation operand */


        /* ins. in dslot - loop 15: */
        /* 1st operation operand before branch is NOT used to set flag */
        /* 1st and 2nd operation operands before and after branch are related */
        /* 2nd and 3rd operation operands before and after jump are NOT related */
        /* 1st operation before branch is MULTI cycle */
        /* 2nd operation after branch/before jump is MULTI cycle */
        /* 3rd operation after jump is MULTI cycle */
_D15s:
	l.addi	r14,r0,0x4      /* init inc. loop cnt. */
	l.addi	r16,r0,0x10     /* init dec. loop cnt. */
_D15:
	l.add	r17,r17,r14	/* merge test case operands */
	l.add	r17,r17,r19	/* merge test case operands */
	l.xor	r17,r17,r20	/* invert test result value */
	l.addi	r16,r16,-4	/* dec. cnt. */
	l.muli	r14,r14,2	/* mul. cnt. - r14..1st operation operand */
	l.sfeqi	r16,0		
	l.bf	_D16s
	l.mul	r17,r17,r14	/* r17,r14..2nd opertion operands */
	l.j	_D15
	l.muli	r19,r19,2	/* r19..3rd operation operand */


        /* ins. in dslot - loop 16: */
        /* 1st operation operand before branch is used to set flag */
        /* 1st and 2nd operation operands before and after branch are related */
        /* 2nd and 3rd operation operands before and after jump are related */
        /* 1st operation before branch is MULTI cycle */
        /* 2nd operation after branch/before jump is MULTI cycle */
        /* 3rd operation after jump is MULTI cycle */
_D16s:
	l.addi	r14,r0,0x4      /* init inc. loop cnt. */
	l.addi	r16,r0,0x10     /* init dec. loop cnt. */
_D16:
	l.add	r17,r17,r14	/* merge test case operands */
	l.add	r17,r17,r19	/* merge test case operands */
	l.xor	r17,r17,r20	/* invert test result value */
	l.addi	r16,r16,-4	/* dec. cnt. */
	l.muli	r14,r14,2	/* mul. cnt. - r14..1st operation operand */
	l.sfgeu	r14,r21
	l.bf	_D17s
	l.mul	r17,r17,r14	/* r17,r14..2nd opertion operands */
	l.j	_D16
	l.muli	r19,r17,1	/* r19,r17..3rd operation operand */


        /* ins. in dslot - loop 17: */
        /* 1st operation operand before branch is used to set flag */
        /* 1st and 2nd operation operands before and after branch are NOT related */
        /* 1st operation before branch is SINGLE cycle */
        /* 2nd operation after branch/before jump is SW */
_D17s:
	l.addi	r14,r0,0x4      /* init inc. loop cnt. */
	l.addi	r16,r0,0x10     /* init dec. loop cnt. */
_D17:
	l.add	r17,r17,r14	/* merge test case operands */
	l.xor	r17,r17,r20	/* invert test result value */
	l.addi	r16,r16,-4	/* dec. cnt. */
	l.addi	r14,r14,4	/* inc. cnt. - r14..1st operation operand */
	l.sfgeu	r14,r15 
	l.bnf	_D17
	l.sw	0(r16),r17	/* r16,r17..2nd opertion operands */


        /* ins. in dslot - loop 18: */
        /* 1st operation operand before branch is used to set flag */
        /* 1st and 2nd operation operands before and after branch are NOT related */
        /* 1st operation before branch is SINGLE cycle */
        /* 2nd operation after branch/before jump is LWZ */
	l.addi	r14,r0,0x4      /* init inc. loop cnt. */
	l.addi	r16,r0,0x10     /* init dec. loop cnt. */
_D18:
	l.add	r17,r17,r22	/* merge test case operands */
	l.add	r17,r17,r14	/* merge test case operands */
	l.xor	r17,r17,r20	/* invert test result value */
	l.addi	r16,r16,-4	/* dec. cnt. */
	l.addi	r14,r14,4	/* inc. cnt. - r14..1st operation operand */
	l.sfgeu	r14,r15 
	l.bnf	_D18
	l.lwz	r22,0(r16)	/* r16,r22..2nd opertion operands */


        /* following are ins. in dslot in three different JUMPs: l.jal, l.jr, l.j */
	l.add	r17,r17,r22	/* merge test case operands */
	l.xor	r17,r17,r20	/* invert test result value */
	l.jal	_D19
	l.muli	r17,r17,2
	l.add	r17,r17,r18
	l.xor	r17,r17,r20	/* invert test result value */
	l.j	_D20
	l.sub	r18,r18,r17

_D19:	
	l.addi	r17,r17,1
	l.xor	r17,r17,r20	/* invert test result value */
	l.jr	r9
	l.addi	r18,r18,8

_D20:
	l.mul	r17,r17,r18		/* r17 shold be 0xb093a787 */
        l.movhi r18,hi(0xb093a787)
        l.ori   r18,r18,lo(0xb093a787)
	l.sub	r17,r17,r18		/* r17 used further in code */
	l.nop
	l.nop


_jump:
	l.add	r8,r0,r17

	l.j	_T1
	l.addi	r8,r8,1

_T2:	l.or	r10,r0,r9
	l.jalr	r10
	l.addi	r8,r8,1

_T1:	l.jal	_T2
	l.addi	r8,r8,1

	l.sfeqi	r0,0
	l.bf	_T3
	l.addi	r8,r8,1

_T3:	l.sfeqi	r0,1
	l.bf	_T4
	l.addi	r8,r8,1
	
	l.addi	r8,r8,1

_T4:	l.sfeqi r0,0
        l.bnf    _T5
        l.addi  r8,r8,1

	l.addi	r8,r8,1

_T5:	l.sfeqi r0,1
        l.bnf    _T6
        l.addi  r8,r8,1
 
        l.addi  r8,r8,1

_T6:	l.movhi r3,hi(_T7)
        l.ori  r3,r3,lo(_T7)
	l.mtspr r0,r3,32
	l.mfspr r5,r0,17
	l.mtspr r0,r5,64
	l.rfe
	l.addi  r8,r8,1

	l.addi  r8,r8,1

_T7:	l.mtspr r0,r8,0x1234   /* Should be 0x00000000a */

	l.lwz	r9,0(r31)
	l.add	r8,r9,r8
	l.sw	0(r31),r8

	l.lwz	r9,0(r31)
	l.movhi	r3,0xcc69
	l.ori	r3,r3,0xe5fb
	l.add	r3,r8,r3	/* Should be 0xdeaddead */

	l.movhi r4, 0x5ead
	l.ori 	r4, r4, 0xdea0
	l.sub 	r3, r3, r4 /* Should now be 0x8000000d */
	l.nop	0x2
	l.ori 	r3, r0, 0 /* Clear R3 */
	l.nop 	0x1 /* Exit simulation */
	l.nop
	l.nop
