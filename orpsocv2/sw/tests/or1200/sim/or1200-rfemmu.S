/*
	Test of return from execption and MMU behavior 

	For now, just a simple test confirming that the instructions we l.rfe
	to are executed OK.

	In this test we are checking that execution of instructions after
	l.rfe enabling the MMU occurs OK.

	Mainly this is to test what happens when the first virtual address
	after a l.rfe is a cache "hit" - ensuring this instruction is not
	executed.

	Test method:
	Relocate a simple looping function to an unused page in memory, at
	offset 0xf00 within that page. Translate virtual page 0 to this page
	and execute it, with timer interrupts occuring, ensuring the l.rfe is
	being called regularly.

	Whenever the function in virtual space is so be resumed, code at the
	address matching the virtual space is executed before the l.rfe,
	ensuring it is in cache. If any of these instructions in the physical
	address are executed, then the code running in virtual space will
	detect this and crash.

	In this instance, the test function is only about 30 instructions
	long. It is mapped to virtual address 0xf00. The physical address
	0xf00 has the same number of instructions, setting r31 to -1 (0xfffffff)
	This string of 32 instructions is executed before any l.rfe back to the
	function running with MMUs on in the virtual page address. The last
	instruction before the l.rfe clears r31, so if any of the instructions
	in the physical address space get executed, r31 will return to
	0xffffffff and this can be detected by the code.

	The test finishes when the test function, running in virtual address
	space, has incremented a value in memory beyond the alue defined by
	TEST_LOOPS.

	The test has the possibility of being restarted and run with
	caches enabled.

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

#define ONCE_THROUGH_WITH_CACHE_ENABLED 0
	
#define TEST_LOOPS 15
	
#define PAGE_ADR_SHIFT 13

// Should be at least 0x1000
#define PAGE_OFFSET_FOR_TEST 0xf00

#define TIMER_CYCLES_RATE 1000
	
#define LOAD_SYMBOL_2_GPR(gpr,symbol)  \
	l.movhi gpr, hi(symbol) ;      \
	l.ori   gpr, gpr, lo(symbol)
	
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

/* ---[ 0x400: itlb fault ]--------------------------------------------- */
        .org 0x400
	l.ori	r3,r0,0xa00
	l.nop 	0x2
	l.movhi	r3,0xbaaa
	l.ori	r3,r3,0xaaad
	l.nop 	0x2
	l.nop 	0x1 /* We should never have a itlb miss */

/* ---[ 0x500: timer exception ]---------------------------------------- */	
	.org 0x500
	/* check test loop counter to see if test is over */
	LOAD_SYMBOL_2_GPR(r3,function_global)
	l.lwz	r3,0(r3)
	l.sfgeui r3,TEST_LOOPS
	l.bf	check_for_restart
	l.nop 	2
	/* Write to TTMR again to clear TTMR[IP] bit */
	l.jal	timer_load
	l.nop
	
	l.j	call_rfe	/* continue execution */
	l.nop

	.org 0x600
	l.j	fail
	l.nop

	.org 0x700
	l.j	fail
	l.nop

	.org 0x800
	l.j	fail
	l.nop
	
	
/* ---[ 0xa00: itlb miss ]---------------------------------------------- */
        .org 0xa00
	l.ori	r3,r0,0xa00
	l.nop 	0x2
	l.movhi	r3,0xbaaa
	l.ori	r3,r3,0xaaad
	l.nop 	0x2
	l.j	0
	l.nop 	0x1 /* We should never have a itlb miss */

/* ---[ 0xc00: system call ]-------------------------------------------- */
	.org 0xc00
	l.movhi	r3,0xbaaa
	l.ori	r3,r3,0xaaad
	l.nop 	0x2	/* Simulation report */
	l.j	0	/* Loop here */
	l.nop 	0x1	/* End simulation */	
	
/* ---[ 0xE00: TRAP exception ]----------------------------------------- */
        .org 0xe00
	l.j	0
	l.nop 	0x1

/* ---[ reserved space        ]------------------------------------ */
	.org PAGE_OFFSET_FOR_TEST
call_rfe:	
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1
	l.addi	r31,r0,-1	
	l.movhi	r31,0
	l.rfe
	

/* =================================================== [ text ] === */
	.section .data
immu_sets:
	.long -1
immu_ways:
	.long -1
dmmu_sets:
	.long -1
dmmu_ways:
	.long -1
test_page_num:
	.long -1
function_global:
	.long 0
	
	
/* =================================================== [ text ] === */
	.section .text

/* =================================================== [ start ] === */	

	.global _start	
_start:

#if ONCE_THROUGH_WITH_CACHE_ENABLED==1
	l.jal 	_cache_init
	l.nop
#endif

	// Kick off test
	l.jal   _main
	l.nop
	
/* =================================================== [ main ] === */
	.global _main
_main:
	// Get MMU configuration
	l.jal	immu_get_config
	l.nop
	l.jal	dmmu_get_config
	l.nop

	// Clear IMMU
	l.jal	immu_clear
	l.nop
	// Clear DMMU
	l.jal	dmmu_clear
	l.nop

	// Set up stack pointer, but kind of don't need it
	LOAD_SYMBOL_2_GPR(r1,_stack)

	// What is next page above stack?
	l.srli	r4,r1,PAGE_ADR_SHIFT
	l.addi	r4,r4,1		/* Next page past */
	LOAD_SYMBOL_2_GPR(r5,test_page_num)
	l.sw	0(r5),r4	/* We will do tests with code in this PPN */

	
	LOAD_SYMBOL_2_GPR(r2,test_function)
	LOAD_SYMBOL_2_GPR(r3,test_function_end)
	/* Convert physical page number back to physical address */
	l.slli	r4,r4,PAGE_ADR_SHIFT
	/* r4 has physical address of where we'll relocate our test program to
	but we want it at an offset of PAGE_OFFSET_FOR_TEST inside that page.
	*/

	l.addi	r4,r4,PAGE_OFFSET_FOR_TEST
program_relocate_loop:
	l.lwz	r5,0(r2)
	l.sw	0(r4),r5
	l.sfeq	r2,r3
	l.mtspr	r0,r4,SPR_DCBIR	/* flush dcache */
	l.addi	r4,r4,4		/* increment dest ptr */
	l.bnf	program_relocate_loop
	l.addi	r2,r2,4		/* increment src ptr */

mmu_setup:	
	/* Program relocated, setup MMU mappings */
	/* Physical page number of test-code is in test_page_num. We need
	to set up a mapping for this page in the MMU, then launch execution
	of it.
	*/
	LOAD_SYMBOL_2_GPR(r5,test_page_num)
	l.lwz	r5,0(r5)
	/* Basically should be match-PPN of 0 translates to page
	number test_page_num.
	Set up entry 0, then, and have translate address to test_page_num*/
	// Match register
	l.ori	r3,r0,SPR_ITLBMR_V 	/* Just this */
	l.mtspr	r0,r3,SPR_ITLBMR_BASE(0)
	// Setup TR page number
	l.slli	r3,r5,PAGE_ADR_SHIFT //r5 = page number of target page
	l.ori	r3,r3,ITLB_PR_NOLIMIT
	l.mtspr	r0,r3,SPR_ITLBTR_BASE(0)

timer_setup:	
	/* Init timer to fire at rate of TIMER_CYCLES_RATE */
	l.jal	timer_load
	l.nop
	
sr_setup:	
	// Start test - set up EPCR to jump to PAGE_OFFSET_FOR_TEST
	l.ori	r3,r0,PAGE_OFFSET_FOR_TEST
	l.mtspr r0,r3,SPR_EPCR_BASE

	l.mfspr r3,r0,SPR_SR
	l.ori	r3,r3,(SPR_SR_IME|SPR_SR_TEE)	/* IMMU, tick timer enable */
	l.mtspr	r0,r3,SPR_ESR_BASE

	// Ensure global counter variable is zero
	LOAD_SYMBOL_2_GPR(r3,function_global)
	l.sw	0(r3),r0
	
	// Call function entry point, check bug doesn't occur
	l.j	call_rfe
	l.nop

	// Should never get here - test is detected as finished from the
	// timer interrupt.

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
	l.j 	0
	l.nop 	0x1

fail:
	l.movhi	r3,0xbaaa
	l.ori	r3,r3,0xaaad
	l.nop 	0x2
	l.j	0
	l.nop	0x1

	
	/* A stilly test function - doing some loads/stores, and some
	branches
	only touches r13 and r14*/
	/* Should be relocatable */
test_function:
	// Check for error - r31 should be 0
	l.sfne	r31,r0
	l.bf	test_function_fail
	
	LOAD_SYMBOL_2_GPR(r13,function_global)
	l.lwz	r14,0(r13) 	/* Load value of global */
	l.addi	r14,r14,1
	l.sw	0(r13),r14	/* Store it back */
	l.mtspr	r0,r13,SPR_DCBIR /* flush dcache */
	/* Every 8, branch away and back */
	l.andi	r14,r14,0xf
	l.sfeq	r14,r0
	l.bf	test_function_sub
	l.nop
	l.j	test_function
	l.nop

test_function_sub:
	l.j	test_function
	l.nop

test_function_fail:
	l.sys	0 /* Bail out with syscall */

test_function_end:
	l.nop	0x1


	
restart_with_caches:
	l.jal 	_cache_init
	l.nop
	l.movhi r4, hi(_start)
	l.ori r4, r4, lo(_start)
	l.jr    r4
	l.nop	


	/* MMU configuration functions */

immu_get_config:
	l.mfspr	r3,r0,SPR_IMMUCFGR
	// Number of ways
	l.andi	r4,r3,SPR_IMMUCFGR_NTW
	l.addi	r4,r4,1
	// Store the ways in immu_ways	
	l.movhi	r5,hi(immu_ways)
	l.ori	r5,r5,lo(immu_ways)
	l.sw	0(r5),r4
	// Number of sets
	l.andi	r4,r3,SPR_IMMUCFGR_NTS
	l.srli	r4,r4,SPR_IMMUCFGR_NTS_OFF
	l.ori	r6,r0,1
	l.sll	r4,r6,r4
	// Store the ways in immu_ways	
	l.movhi	r5,hi(immu_sets)
	l.ori	r5,r5,lo(immu_sets)
	l.jr	r9	// Return
	l.sw	0(r5),r4

dmmu_get_config:
	l.mfspr	r3,r0,SPR_DMMUCFGR
	// Number of ways
	l.andi	r4,r3,SPR_DMMUCFGR_NTW
	l.addi	r4,r4,1
	// Store the ways in dmmu_ways	
	l.movhi	r5,hi(dmmu_ways)
	l.ori	r5,r5,lo(dmmu_ways)
	l.sw	0(r5),r4
	// Number of sets
	l.andi	r4,r3,SPR_DMMUCFGR_NTS
	l.srli	r4,r4,SPR_DMMUCFGR_NTS_OFF
	l.ori	r6,r0,1
	l.sll	r4,r6,r4
	// Store the ways in dmmu_ways	
	l.movhi	r5,hi(dmmu_sets)
	l.ori	r5,r5,lo(dmmu_sets)
	l.jr	r9	// Return
	l.sw	0(r5),r4


immu_clear:
	// r3 - immu_ways
	// r4 - immu_sets
	// r5 - work
	// r6 - ways counter
	// r7 - sets counter
	// r10 - match regs spr address
	// r11 - translate regs spr address
	
	/* Clear all IMMU match/translate register pairs */
	l.movhi	r5,hi(immu_ways)
	l.ori	r5,r5,lo(immu_ways)
	l.lwz	r3,0(r5)
	l.movhi	r5,hi(immu_sets)
	l.ori	r5,r5,lo(immu_sets)
	l.lwz	r4,0(r5)

	l.movhi	r6, 0	// Clear ways counters

immu_clear_set:
	l.movhi	r7, 0   // Clear sets counter
	// Calculate base of IMMU match & translate registers
	l.ori	r10,r0,SPR_ITLBMR_BASE(0)
	l.ori	r11,r0,SPR_ITLBTR_BASE(0)	
	l.slli	r5,r6,8 /* Multiply ways by 256  (ways<<8) */
	l.add	r10,r10,r5
	l.add	r11,r11,r5
immu_clear_set_loop:
	l.mtspr	r10,r0,0 /* ITLBMR_BASE(way) + set = 0 */
	l.mtspr	r11,r0,0 /* ITLBTR_BASE(way) + set = 0 */
	l.addi	r7,r7,1
	l.sfne	r4,r7
	l.addi	r10,r10,1
	l.bf	immu_clear_set_loop
	l.addi	r11,r11,1

	l.addi	r6,r6,1 /* Increment ways counter */
	l.sfne	r3,r6 	/* Cleared all ways? */
	l.bf	immu_clear_set
	l.nop

	/* Finished */
	l.jr	r9
	l.nop

dmmu_clear:
	// r3 - dmmu_ways
	// r4 - dmmu_sets
	// r5 - work
	// r6 - ways counter
	// r7 - sets counter
	// r10 - match regs spr address
	// r11 - translate regs spr address
	
	/* Clear all DMMU match/translate register pairs */
	l.movhi	r5,hi(dmmu_ways)
	l.ori	r5,r5,lo(dmmu_ways)
	l.lwz	r3,0(r5)
	l.movhi	r5,hi(dmmu_sets)
	l.ori	r5,r5,lo(dmmu_sets)
	l.lwz	r4,0(r5)

	l.movhi	r6, 0	// Clear ways counters

dmmu_clear_set:
	l.movhi	r7, 0   // Clear sets counter
	// Calculate base of DMMU match & translate registers
	l.ori	r10,r0,SPR_DTLBMR_BASE(0)
	l.ori	r11,r0,SPR_DTLBTR_BASE(0)	
	l.slli	r5,r6,8 /* Multiply ways by 256  (ways<<8) */
	l.add	r10,r10,r5
	l.add	r11,r11,r5
dmmu_clear_set_loop:
	l.mtspr	r10,r0,0 /* ITLBMR_BASE(way) + set = 0 */
	l.mtspr	r11,r0,0 /* ITLBTR_BASE(way) + set = 0 */
	l.addi	r7,r7,1
	l.sfne	r4,r7
	l.addi	r10,r10,1
	l.bf	dmmu_clear_set_loop
	l.addi	r11,r11,1

	l.addi	r6,r6,1 /* Increment ways counter */
	l.sfne	r3,r6 	/* Cleared all ways? */
	l.bf	dmmu_clear_set
	l.nop

	/* Finished */
	l.jr	r9
	l.nop
	
	
	
#define TTMR_RELOAD_VALUE (SPR_TTMR_IE | SPR_TTMR_RT | TIMER_CYCLES_RATE)
timer_load:
	l.movhi	r3,hi(TTMR_RELOAD_VALUE)
	l.ori	r3,r3,lo(TTMR_RELOAD_VALUE)
	l.jr r9
	l.mtspr	r0,r3,SPR_TTMR
