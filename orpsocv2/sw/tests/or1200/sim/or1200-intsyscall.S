#include "spr-defs.h"
#include "board.h"
	
/*

	User IRQ and system call simultaneous interrupt test
	
	Within the test we'll use following global variables:

	r15 syscall interrupt counter
	r6 syscall function counter	
	r10 irq interrupt counter
	r12 intgen's base address


	The test does the following:
	Uses the intgen module to schedule interrupts to see if they clash
	with system calls.

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

	
/* =================================================== [ User interrupt ] === */
	.org 0x800
	.global _user_irq_handler
_user_irq_handler:	
	l.addi r10, r10, 1
	/* Report values , 0x00000800 == user interrupt report*/
	l.ori r3, r0, 0x0800
	l.nop 2
	l.or r3, r0, r10
	l.nop 2
	/* TODO - propably confirm it was intgen's IRQ that caused this */
	/* Clear interrupt source */
	l.ori	r7, r12, 0x1 	/* intgen IRQ clear address */
	l.sb	0(r7), r0 	/* Any write clears the bit */
	/* Clear OR1200 PICSR */
	l.mfspr r7, r0, SPR_PICSR
	l.mtspr r0, r7, SPR_PICSR

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
	/* Report values , 0x00000c00 == system call report*/
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
	// Kick off test
	l.jal   _main
	l.nop

/* =================================================== [ main ] === */	
.global _main
_main:
	
	#
	# unmask (enable) all ints
	#
	l.movhi r5,0xffff
	l.ori   r5,r5,0xffff
	l.mtspr r0,r5,SPR_PICMR		# set PICMR

	/* Enable Interrupts */
        l.mfspr r6,r0,SPR_SR
        l.ori   r6,r6,SPR_SR_IEE
        l.mtspr r0,r6,SPR_SR

	l.movhi r15, 0
	l.movhi r6, 0
	l.movhi r10, 0


	
	// Assumes r12 is intgen's base address
	l.movhi	r12,hi(INTGEN_BASE)
	
#define INTGEN_LOAD(x)	\
	l.ori	r5,r0,lo(x)	;\
	l.sb	0(r12),r5
	
	
	/* Test begin */
	
	l.nop
	INTGEN_LOAD(1)
	l.sys 0x1
	l.nop
	INTGEN_LOAD(1)
	l.nop
	l.sys 0x2
	l.nop
	INTGEN_LOAD(2)	
	l.sys 0x3
	l.nop
	INTGEN_LOAD(2)
	l.nop
	l.sys 0x4
	l.nop
	l.ori	r5,r0,1	
	l.sys 0x5
	l.sb	0(r12),r5
	l.nop
	l.nop
	l.nop
	l.sfnei r6, 0xf /* Should equal 15, 0xf */
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
	l.add r6, r6, r3 /* Add syscall number to our counter */
	l.movhi r4, hi(0x00400000) /* 4MB mark of memory */
	/* Ensure memory access OK */
	l.slli r3, r3, 2 /* Turn syscall number into a word address (<< 2) */
	l.add r4, r4, r3 /* Access this offset from 4MB mark */
	l.sw 0(r4), r6 /* Do a write to memory */
	l.lwz r6, 0(r4) /* Do a read from memory */
	/* Report running value of syscall counter */
	l.or r3, r0, r6
	l.nop 2
	l.rfe /* Now continue from where we had the l.sys */
	
