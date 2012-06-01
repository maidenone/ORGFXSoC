/*
	Exception test.

	The following list outlines the tests performed
	
	Execption test
	- 0x100 reset                        - start addr after reset 
	- 0x200 bus error                    - unimplemented addr is accessed
	- 0x300 data page fault              - NOT tested here 
	- 0x400 instruction page fault       - NOT tested here 
	- 0x500 tick timer                   - NOT tested here 
	- 0x600 alignment                    - write to unaligned addr
	- 0x700 illegal instruction          - use unimplemented inst. l.div 
	- 0x800 external interrupt           - int triggered from wb slave 
	- 0x900 d-tlb miss                   - translation tests
	- 0xA00 i-tlb miss                   - NOT tested here 
	- 0xB00 range                        - NOT tested here 
	- 0xC00 system call                  - NOT tested here 
	- 0xD00 floating point               - NOT tested here 
	- 0xE00 trap                         - NOT tested here 
	- 0xF00 RESERVED 
	
	r11 - 1st exception counter incremented inside exception 
	r12 - 2nd exception counter incremented outside and rigth after
	      exception
	... other register use to be documented...

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
	.global stack
	l.movhi r1, hi(stack)
	l.ori 	r1, r1, lo(stack)
	l.addi  r2, r0, -3
	l.and   r1, r1, r2
	/* Jump to program initialisation code */
	.global _start
	l.movhi r4, hi(_start)
	l.ori 	r4, r4, lo(_start)
	l.jr    r4
	l.nop

  
/* ---[ 0x200: BUS error ]------------------------------------------------ */
	.org 0x200
	.global	_bus_handler
_bus_handler:	
	l.mfspr	r3,r0,SPR_EPCR_BASE	/* Get EPC */
	l.nop	2
	l.mfspr	r3,r0,SPR_EEAR_BASE	/* Get EEA */
	l.nop	2
	l.addi	r11,r11,1		/* Increment 1st exception counter */
	l.sfeqi r2, 0xd /* Is this a data bus test, if so return with l.rfe */
	l.bf 1f
	l.movhi r5, 0    /* r5 should be the one causing the error on dbus */
	/* Instruction bus error test return */
	l.movhi r5, hi(0x44004800) /* Put "l.jr r9" instruction in r5 */
	l.ori r5, r5, lo(0x44004800)
	l.sw 0x0(r0), r5 /* Write "l.j r9" to address 0x0 in RAM */
	l.movhi	r5,0x1500	/* Put a "l.nop"  instruction in r5 */
	l.sw	0x4(r0),r5	/* Write l.nop instruction to RAM addr 0x4 */
	l.mtspr	r0,r0,SPR_ICBIR		/* Invalidate line 0 of cache */
	l.mtspr	r0,r0,SPR_EPCR_BASE	/* RFE to 0x0, which is l.jr r9 */
1:	l.rfe

/* ---[ 0x600: ALIGN error ]------------------------------------------------ */
	.org 0x600
	.global	_align_handler
_align_handler:	
	l.mfspr	r3,r0,SPR_EPCR_BASE	/* Get EPC */
	l.nop	2
	l.addi	r11,r11,1		/* Increment 1st exception counter */
	l.movhi	r5,0			/* Clear the pointer register */
	l.rfe


/* ---[ 0x700: ILLEGAL INSN exception ]------------------------------------- */
	.org 0x700
	.global _illinsn_handler
_illinsn_handler:
	l.mfspr	r3,r0,SPR_EPCR_BASE	/* Get EPC */
	l.nop	2
	l.addi	r11,r11,1		/* Increment 1st exception counter */
	/* Delay slot test needs this instruction "fixed" */
	l.movhi	r5,0x1500		/* Put a "l.nop"  instruction in r5 */
	l.sw	0x4(r0),r5	/* Write l.nop instruction to RAM addr 0x4 */
	l.mtspr	r0,r0,SPR_ICBIR		/* Invalidate line 0 of cache */
	l.mtspr	r0,r0,SPR_EPCR_BASE	/* Jump to 0, which is l.jr r9 */
	l.rfe

/* ---[ 0x900: DTLB exception ]--------------------------------------------- */
	.org 0x900
	.global _dtlb_handler
/* Exception handler - DMMU TLB miss */
/* Assume 64-entry TLB cache */	
_dtlb_handler:
	l.sw	-4(r1),r4
	l.sw	-8(r1),r5
	l.sw	-12(r1),r6
	l.sw	-16(r1),r7
	l.sw	-20(r1),r8
	l.mfspr r2, r0, SPR_EEAR_BASE
	/* Find the entry/set for this address */
	l.srli r13, r2, 13 /* r13 = VPN, shift by size 8192 = 2**13 */
	l.andi r4, r13, 0x3f /* 64 entries = 6 bit mask, r4 = set number */
	/* If page is in the 0xc0000000 space we map to 4MB part of
	memory, ie 0x0 => 0x00400000, otherwise 1-1 mapping */
	l.movhi r5, hi(0xc0000000)
	l.ori r5, r5, lo(0xc0000000)
	l.srli r5, r5, 13 /* Get page address, shift by page size, 13 bits */
	l.movhi r6, hi(0xff << 11) /* Mask for top byte of VPN */
	l.ori r6, r6, lo(0xff << 11)
	l.and r6, r6, r13 /* Mask in only top byte of VPN */
	l.sfeq r5, r6 /* Decide if it's in our special mapped region or not*/
	
	/* First, Setup value for DTLBM (match) reg, is same for both cases */
	l.movhi r6, hi(SPR_ITLBMR_VPN) /* VPN mask into r6 */
	l.ori r6, r6, lo(SPR_ITLBMR_VPN)
	l.and r7, r2, r6 /* AND address with VPN mask */
	l.ori r7, r7, SPR_DTLBMR_V /* OR in valid bit */
	l.mtspr r4, r7, SPR_DTLBMR_BASE(0) /* Write to DTLBR register */

	l.bf _highmem_map
	l.nop
	
_lomem_map:
	/* Do 1:1 mapping for this request */
	/* Setup value for translate register */
	l.movhi r6, hi(SPR_ITLBTR_PPN) /* PPN mask into r6 */
	l.ori r6, r6, lo(SPR_ITLBTR_PPN)
	l.and r7, r2, r6 /* AND address with PPN mask */
	l.ori r7, r7, DTLB_PR_NOLIMIT /* Set all execute enables, no lims. */
	l.mtspr r4, r7, SPR_DTLBTR_BASE(0) /* Write to DTLTR register */
	l.j _dtlb_done
	l.addi r14, r14, 1 /* Incremement low-mapping counter */

_highmem_map:
	/* Do top byte, 0xc00->0x004, mapping for this request */
	/* Setup value for translate register */
	l.movhi r6, hi(SPR_ITLBTR_PPN) /* PPN mask into r6 */
	l.ori r6, r6, lo(SPR_ITLBTR_PPN)
	l.and r7, r2, r6 /* AND address with PPN mask */
	l.movhi r8, hi(0xffff0000) /* Top byte address mask */
	l.or r7, r8, r7 /* Set top byte to 0xff */
	l.xor r7, r8, r7 /* Now clear top byte with XOR */
	l.movhi r8, hi(0x00400000) /* Top address byte */
	l.or r7, r8, r7 /* Set top address byte */
	l.ori r7, r7, DTLB_PR_NOLIMIT /* Set all execute enables, no lims. */
	l.mtspr r4, r7, SPR_DTLBTR_BASE(0) /* Write to DTLTR register */
	l.addi r15, r15, 1 /* Incremement low-mapping counter */
	
_dtlb_done:
	l.lwz	r4,-4(r1)
	l.lwz	r5,-8(r1)
	l.lwz	r6,-12(r1)
	l.lwz	r7,-16(r1)
	l.lwz	r8,-20(r1)
	l.rfe



/* =================================================== [ text section ] === */
	.section  .text

/* =================================================== [ start ] === */	

	.global _start
_start:	
	l.jal 	_cache_init
	l.nop
	// Kick off test
	l.jal   _main
	l.nop

/* ========================================================= [ main ] === */

	.global	_main
	.global _dmmu_invalidate
	.global _dmmu_except1
	
_main:		
	l.nop	
        l.addi	r3,r0,0
        l.addi	r5,r0,0
        l.addi	r11,r0,0 /* exception counter 1 */
        l.addi	r12,r0,0 /* exception counter 2 */
        l.addi	r13,r0,0
	l.addi	r14,r0,0 /* DMMU exception counter for low mem mapping */
	l.addi	r15,r0,0 /* DMMU exception counter for hi mem mapping */
	l.sw	0x0(r0),r0	/* Initialize RAM */	
	l.sw	0x4(r0),r0	/* Initialize RAM */	
	l.sw	0x8(r0),r0	/* Initialize RAM */	
	l.sw	0xc(r0),r0	/* Initialize RAM */	
	l.sw	0x10(r0),r0	/* Initialize RAM */	
	l.sw	0x14(r0),r0	/* Initialize RAM */	
	l.sw	0x18(r0),r0	/* Initialize RAM */	
	l.sw	0x1c(r0),r0	/* Initialize RAM */	
	l.sw	0x20(r0),r0	/* Initialize RAM */	
	l.sw	0x24(r0),r0	/* Initialize RAM */	
	l.sw	0x28(r0),r0	/* Initialize RAM */	
	l.sw	0x2c(r0),r0	/* Initialize RAM */	
	l.sw	0x30(r0),r0	/* Initialize RAM */	
	l.sw	0x34(r0),r0	/* Initialize RAM */	
	l.sw	0x38(r0),r0	/* Initialize RAM */	
	l.sw	0x3c(r0),r0	/* Initialize RAM */	
	l.sw	0x40(r0),r0	/* Initialize RAM */	
	l.sw	0x44(r0),r0	/* Initialize RAM */	
	l.sw	0x48(r0),r0	/* Initialize RAM */	
	l.sw	0x4c(r0),r0	/* Initialize RAM */	
	l.j	_align1
	//l.j _dmmu_except1
	l.nop


        /* Exceptions followed by NOPs */

	/* SW alignment exception */
_align1:	
	l.movhi r11,0
	l.nop
	/* Test l.sh */
	l.ori r5,r0,0x1 /* Half-word access, offset 1 */
	l.sh 0x0(r5),r0
	l.addi r12,r12,1 /* Increment 2nd exception counter */
	l.ori r5,r0,0x3 /* Half-word access, offset 3 */
	l.sh 0x0(r5),r0
	l.addi r12,r12,1 /* Increment 2nd exception counter */
	/* Test l.lhz */
	l.ori r5,r0,0x1 /* Half-word access, offset 1 */
	l.lhz r3,0x0(r5)
	l.addi r12,r12,1 /* Increment 2nd exception counter */
	l.ori r5,r0,0x3 /* Half-word access, offset 3 */
	l.lhz r3,0x0(r5)
	l.addi r12,r12,1 /* Increment 2nd exception counter */
	/* Test l.lhs */
	l.ori r5,r0,0x1 /* Half-word access, offset 1 */
	l.lhs r3,0x0(r5)
	l.addi r12,r12,1 /* Increment 2nd exception counter */
	l.ori r5,r0,0x3 /* Half-word access, offset 3 */
	l.lhs r3,0x0(r5)	
	l.addi r12,r12,1 /* Increment 2nd exception counter */
	/* Test l.sw */
	l.ori r5,r0,0x1 /* Word access, offset 1 */	
	l.sw 0x0(r5), r0
	l.addi r12,r12,1 /* Increment 2nd exception counter */
	l.ori r5,r0,0x2 /* Word access, offset 2 */	
	l.sw 0x0(r5), r0
	l.addi r12,r12,1 /* Increment 2nd exception counter */
	l.ori r5,r0,0x3 /* Word access, offset 3 */	
	l.sw 0x0(r5), r0
	l.addi r12,r12,1 /* Increment 2nd exception counter */
	/* Test l.lwz */
	l.ori r5,r0,0x1 /* Word access, offset 1 */	
	l.lwz r3,0x0(r5)
	l.addi r12,r12,1 /* Increment 2nd exception counter */
	l.ori r5,r0,0x2 /* Word access, offset 2 */	
	l.lwz r3,0x0(r5)
	l.addi r12,r12,1 /* Increment 2nd exception counter */
	l.ori r5,r0,0x3 /* Word access, offset 3 */	
	l.lwz r3,0x0(r5)
	l.addi r12,r12,1 /* Increment 2nd exception counter */
#if OR1200_HAS_LWS==1	
	/* Test l.lws */
	l.ori r5,r0,0x1 /* Word access, offset 1 */	
	l.lws r3,0x0(r5)
	l.addi r12,r12,1 /* Increment 2nd exception counter */
	l.ori r5,r0,0x2 /* Word access, offset 2 */	
	l.lws r3,0x0(r5)
	l.addi r12,r12,1 /* Increment 2nd exception counter */
	l.ori r5,r0,0x3 /* Word access, offset 3 */	
	l.lws r3,0x0(r5)	
	l.addi r12,r12,1 /* Increment 2nd exception counter */
	l.nop
#endif	
	/* Now test them in delay slots */
	
	l.ori r5,r0,0x1 /* Half-word access, offset 1 */
	l.j 1f  
	l.sh 0x0(r5),r0
	l.nop
1:	l.addi r12,r12,1 /* Increment 2nd exception counter */
	l.ori r5,r0,0x3 /* Half-word access, offset 3 */
	l.j 2f  	
	l.sh 0x0(r5),r0
	l.nop
2:	l.addi r12,r12,1 /* Increment 2nd exception counter */

	/* Test l.lhz */
	l.ori r5,r0,0x1 /* Half-word access, offset 1 */
	l.j 3f
	l.lhz r3,0x0(r5)
	l.nop
3:	l.addi r12,r12,1 /* Increment 2nd exception counter */
	l.ori r5,r0,0x3 /* Half-word access, offset 3 */
	l.j 4f
	l.lhz r3,0x0(r5)
	l.nop
4:	l.addi r12,r12,1 /* Increment 2nd exception counter */
	
	/* Test l.lhs */
	l.ori r5,r0,0x1 /* Half-word access, offset 1 */
	l.j 5f
	l.lhs r3,0x0(r5)
	l.nop
5:	l.addi r12,r12,1 /* Increment 2nd exception counter */
	l.ori r5,r0,0x3 /* Half-word access, offset 3 */
	l.j 6f
	l.lhs r3,0x0(r5)
	l.nop
6:	l.addi r12,r12,1 /* Increment 2nd exception counter */
	
	/* Test l.sw */
	l.ori 	r5,r0,0x1 /* Word access, offset 1 */     
	l.j 	7f
	l.sw 	0x0(r5), r0
	l.nop	
7:	l.addi 	r12,r12,1 /* Increment 2nd exception counter */
	l.ori 	r5,r0,0x2 /* Word access, offset 2 */     
	l.j 	8f
	l.sw 	0x0(r5), r0
	l.nop	
8:	l.addi 	r12,r12,1 /* Increment 2nd exception counter */
	l.ori 	r5,r0,0x3 /* Word access, offset 3 */     
	l.j 	9f
	l.sh 	0x0(r5), r0
	l.nop
9:	l.addi 	r12,r12,1 /* Increment 2nd exception counter */

	/* Test l.lwz */
	l.ori 	r5,r0,0x1 /* Word access, offset 1 */
	l.j 	10f
	l.lwz 	r3,0x0(r5)
	l.nop
10:	l.addi 	r12,r12,1 /* Increment 2nd exception counter */
	l.ori 	r5,r0,0x2 /* Word access, offset 2 */
	l.j 	11f
	l.lwz 	r3,0x0(r5)
	l.nop
11:	l.addi 	r12,r12,1 /* Increment 2nd exception counter */
	l.ori 	r5,r0,0x3 /* Word access, offset 3 */
	l.j 	12f
	l.lwz 	r3,0x0(r5)
	l.nop
12:	l.addi 	r12,r12,1 /* Increment 2nd exception counter */
#if OR1200_HAS_LWS==1
	/* Test l.lws */
	l.ori 	r5,r0,0x1 /* Word access, offset 1 */
	l.j 	13f
	l.lws 	r3,0x0(r5)
	l.nop
13:	l.addi 	r12,r12,1 /* Increment 2nd exception counter */
	l.ori 	r5,r0,0x2 /* Word access, offset 2 */
	l.j 	14f
	l.lws 	r3,0x0(r5)
	l.nop
14:	l.addi 	r12,r12,1 /* Increment 2nd exception counter */
	l.ori 	r5,r0,0x3 /* Word access, offset 3 */
	l.j 	15f
	l.lws 	r3,0x0(r5)
	l.nop
15:	l.addi 	r12,r12,1 /* Increment 2nd exception counter */
		
#endif
	/* Check 1st and 2nd exception counters are equal */
	l.sfeq	r11,r12			/* Should be equal */
	l.bf	17f
	l.nop
	l.nop	1
17:	l.nop	2
	l.nop

	/* Illegal instruction exception */
_illegal1:	
	l.nop
	l.nop
	l.movhi r5, hi(0x44004800) /* Put "l.jr r9" instruction in r5 */
	l.ori 	r5, r5, lo(0x44004800)
	l.sw 	0x0(r0), r5 	/* Write "l.j r9" to address 0x0 in RAM */	
	l.movhi r5, 0xee00 	/* Put an illegal instruction in r5 */
	l.sw 	0x4(r0), r5   	/* Write illegal instruction to RAM addr 0x4 */
	l.movhi r5, 0x1500 	/* l.nop after illegal instruction */
	l.sw 	0x8(r0), r5   	/* Write nop to RAM addr 0x8 */
	l.mtspr r0,r0,SPR_ICBIR /* Invalidate line 0 of cache */
	/* Jump to 0x4 - illegal opcode instruction */
	l.ori 	r6, r0, 0x4
	l.jalr 	r6 		/* Jump to address 0x4, land on illegal insn */
	l.addi 	r12,r12,1 	/* Increment 2nd exception counter */
	l.nop 			/* Should return here */	
	l.nop

	/* Test in delay slot */
	
	l.movhi r5, 0xee00 	/* Put an illegal instruction in r5 */
	l.sw 	0x4(r0), r5   	/* Write illegal instruction to RAM addr 0x4 */
	l.mtspr r0,r0,SPR_ICBIR /* Invalidate line 0 of cache */	
	l.jalr 	r0 /* Jump to address 0, will be a jump back but with an illegal
	             dslot instruction which will befixed by handler */
	l.addi 	r12,r12,1 	/* Increment 2nd exception counter */
	l.nop 			/* Should return here */

	/* Check 1st and 2nd exception counters are equal */
	l.sfeq	r11,r12		/* Should be equal */
	l.bf 	1f
	l.nop
	l.or 	r3, r12, r12
	l.nop 	2 		/* Report expected exception count */
	l.or 	r3, r11, r11
	l.nop 	2 		/* Report actual exception count */
	l.nop 	1
1:	l.nop
	l.nop
	

_dbus1:
	l.nop
	l.movhi r12, 0 /* Reset exception counters */
	l.movhi r11, 0
	l.ori r2, r0, 0xd /* put 0xd in r2, indicate it's databus test */
	/* Cause access error */
	/* Load word */	
	l.movhi r5, 0xee00 /* Address to cause an error */
	l.lwz r6, 0(r5) 
	l.addi r12, r12, 1 /* Incremement secondary exception counter */
	/* Load half */	
	l.movhi r5, 0xee00 /* Address to cause an error */
	l.lhz r6, 0(r5)
	l.addi r12, r12, 1 /* Incremement secondary exception counter */
	/* Load byte */	
	l.movhi r5, 0xee00 /* Address to cause an error */
	l.lbz r6, 0(r5)
	l.addi r12, r12, 1 /* Incremement secondary exception counter */
	/* Store word */	
	l.movhi r5, 0xee00 /* Address to cause an error */
	l.sw 0(r5), r6
	l.addi r12, r12, 1 /* Incremement secondary exception counter */
	/* Store half */	
	l.movhi r5, 0xee00 /* Address to cause an error */
	l.sh 0(r5), r6
	l.addi r12, r12, 1 /* Incremement secondary exception counter */
	/* Store byte */	
	l.movhi r5, 0xee00 /* Address to cause an error */
	l.sb 0(r5), r6
	l.addi r12, r12, 1 /* Incremement secondary exception counter */	
	l.nop
	/* Delay slot tests */
	/* Load word */	
	l.movhi r5, 0xee00 /* Address to cause an error */	
	l.j 1f
	l.lwz r6, 0(r5) /* Data bus error in delay slot */
	l.nop
1:	l.addi r12, r12, 1
	/* Load half */
	l.movhi r5, 0xee00 /* Address to cause an error */	
	l.j 2f
	l.lhz r6, 0(r5) /* Data bus error in delay slot */
	l.nop
2:	l.addi r12, r12, 1	
	/* Load byte */
	l.movhi r5, 0xee00 /* Address to cause an error */	
	l.j 3f
	l.lbz r6, 0(r5) /* Data bus error in delay slot */
	l.nop
3:	l.addi r12, r12, 1	
	/* Store word */
	l.movhi r5, 0xee00 /* Address to cause an error */	
	l.j 4f
	l.sw 0(r5), r6  /* Data bus error in delay slot */
	l.nop
4:	l.addi r12, r12, 1	
	/* Store half */
	l.movhi r5, 0xee00 /* Address to cause an error */	
	l.j 5f
	l.sh 0(r5), r6  /* Data bus error in delay slot */
	l.nop
5:	l.addi r12, r12, 1	
	/* Store byte */
	l.movhi r5, 0xee00 /* Address to cause an error */	
	l.j 6f
	l.sb 0(r5), r6  /* Data bus error in delay slot */
	l.nop
6:	l.addi r12, r12, 1	


/* Check 1st and 2nd exception counters are equal */
	l.sfeq	r11,r12			/* Should be equal */
	l.bf 7f
	l.nop
	l.or r3, r12, r12
	l.nop 2 /* Report expected exception count */
	l.or r3, r11, r11
	l.nop 2 /* Report actual exception count */
	l.nop 1
7:	l.nop

	
	
_ibus1:
	/* TODO: do this it with cache enabled/disabled */	
	l.movhi r12, 0 /* Reset exception counters */
	l.movhi r11, 0
	l.movhi r2, 0x0 /* put 0x0 in r2,indicate it's instruction bus test*/
	/* Cause access error */
	l.movhi r5, 0xee00 /* Address to cause an error */
	l.jalr r5 /* Jump and link to bad address */
	l.nop
	l.addi r12, r12, 1 /* Incremement secondary exception counter */
	/* Check 1st and 2nd exception counters are equal */
	l.sfeq	r11,r12			/* Should be equal */
	l.bf 1f
	l.nop
	l.or r3, r12, r12
	l.nop 2 /* Report expected exception count */
	l.or r3, r11, r11
	l.nop 2 /* Report actual exception count */
	l.nop 1
1:	l.nop
	l.nop	

	
	/* Data MMU exception - try case where we need to translate address as
	   we l.rfe to it */
	// Check the DMMU is the in the design, otherwise don't compile in this
	// test.
#ifndef OR1200_NO_DMMU	
	.extern lo_dmmu_en
_dmmu_except1:
	/* Call DMMU invalidate function */
	l.movhi r4, hi(_dmmu_invalidate)
	l.ori r4, r4, lo(_dmmu_invalidate)
	l.jalr r4
	l.ori r3, r0, 64 /* Put number of entries in r3 */

	l.movhi r5, hi(0x00400000)
	/* Write a word to the place where we'll translate to */
	l.movhi r7, hi(0xaabbccdd)
	l.ori r7, r7, lo(0xaabbccdd)
	l.sw 0(r5), r7 /* Shouldn't trigger MMU */
	l.sfne r14, r0
	l.bf _dmmu_test_error
	l.nop
	l.sfne r15, r0
	l.bf _dmmu_test_error
	l.nop
	
	/* Now enable DMMU */
	l.movhi r4, hi(lo_dmmu_en)
	l.ori r4, r4, lo(lo_dmmu_en)
	l.jalr r4
	l.nop

	/* Now start test. 0xc0000000 should go to 0x00400000 */
	l.lwz r8, 0(r5) /* Should cause DMMU miss, lomem */
	/* Check value was OK */
	l.sfne r7, r8
	l.bf _dmmu_test_error
	l.nop
	l.sfnei r14, 0x1 /* Check for lo mem mapping */
	l.bf _dmmu_test_error
	l.nop
	l.sfne r15, r0 /* hi-mem counter should still be 0 */
	l.bf _dmmu_test_error
	l.nop

	/* Test accesses to mapped area */
	l.movhi r6, hi(0xc0000000)
	l.lwz r8, 0(r6) /* Should cause DMMU miss, himem */
	/* Check value was OK */
	l.sfne r7, r8
	l.bf _dmmu_test_error
	l.nop
	l.sfnei r14, 0x1 /* Check for lo mem mapping */
	l.bf _dmmu_test_error
	l.nop
	l.sfnei r15, 0x1 /* hi-mem counter should still be 0 */
	l.bf _dmmu_test_error
	l.nop

	/* Now start test. 0xc0000000 should go to 0x00400000 */
	l.lwz r8, 0(r5) /* Should cause DMMU miss, lomem */
	/* Check value was OK */
	l.sfne r7, r8
	l.bf _dmmu_test_error
	l.nop
	l.sfnei r14, 0x2 /* Check for lo mem mapping increment */
	l.bf _dmmu_test_error
	l.nop
	l.sfnei r15, 0x1 /* hi-mem counter should still be 1 */
	l.bf _dmmu_test_error
	l.nop

	l.addi r7, r7, 0x1111 /* Incremement value we're writing */
	
	l.sw 4(r6), r7 /* Should cause DMMU miss, himem */	
	l.sfnei r14, 0x2 /* Check for lo mem mapping */
	l.bf _dmmu_test_error
	l.nop
	l.sfnei r15, 0x2 /* hi-mem counter should be 2 */
	l.bf _dmmu_test_error
	l.nop

	l.lwz r8, 4(r5) /* Should cause DMMU miss, lomem */
	/* Check value was OK */
	l.sfne r7, r8
	l.bf _dmmu_test_error
	l.nop
	l.sfnei r14, 0x3 /* Check for lo mem mapping increment */
	l.bf _dmmu_test_error
	l.nop
	l.sfnei r15, 0x2 /* hi-mem counter should still be 2 */
	l.bf _dmmu_test_error
	l.nop

	/* Fast DMMU exceptions should follow */
	l.addi r7, r7, 0x1111 /* Incremement value we're writing */
	l.sw 8(r6), r7 /* Should cause DMMU miss, himem */
	l.lwz r8, 8(r5) /* Should cause DMMU miss, lomem */
	l.addi r7, r7, 0x1111 /* Incremement value we're writing */
	l.sw 0xc(r6), r7 /* Should cause DMMU miss, himem */
	l.lwz r8, 0xc(r5) /* Should cause DMMU miss, lomem */
	l.addi r7, r7, 0x1111 /* Incremement value we're writing */
	l.sw 0x10(r6), r7 /* Should cause DMMU miss, himem */
	l.lwz r8, 0x10(r5) /* Should cause DMMU miss, lomem */
	l.addi r7, r7, 0x1111 /* Incremement value we're writing */
	l.sw 0x14(r6), r7 /* Should cause DMMU miss, himem */
	l.lwz r8, 0x14(r5) /* Should cause DMMU miss, lomem */
	l.addi r7, r7, 0x1111 /* Incremement value we're writing */
	l.sw 0x18(r6), r7 /* Should cause DMMU miss, himem */
	l.lwz r8, 0x18(r5) /* Should cause DMMU miss, lomem */
	l.addi r7, r7, 0x1111 /* Incremement value we're writing */
	l.sw 0x1c(r6), r7 /* Should cause DMMU miss, himem */
	l.lwz r8, 0x1c(r5) /* Should cause DMMU miss, lomem */
	l.addi r7, r7, 0x1111 /* Incremement value we're writing */
	l.sw 0x20(r6), r7 /* Should cause DMMU miss, himem */
	l.lwz r8, 0x20(r5) /* Should cause DMMU miss, lomem */
	l.addi r7, r7, 0x1111 /* Incremement value we're writing */
	l.sw 0x24(r6), r7 /* Should cause DMMU miss, himem */
	l.lwz r8, 0x24(r5) /* Should cause DMMU miss, lomem */
	/* Should now be 11 lowmem DTLB misses and 10 for high memory space */
	l.sfne r7, r8
	l.bf _dmmu_test_error
	l.nop
	l.sfnei r14, 0xb /* Check for lo mem mapping increment to 11 */
	l.bf _dmmu_test_error
	l.nop
	l.sfnei r15, 0xa /* hi-mem counter should be 10 */
	l.bf _dmmu_test_error
	l.nop
	
	l.j _dmmu_test_ok
	l.nop
	
	
_dmmu_test_error:
	l.movhi r3, hi(0xeeeeeeed)
	l.ori r3, r3, lo(0xeeeeeeed)
	l.nop 2
	l.nop 1

_dmmu_test_ok:
	l.nop
	
#endif // #ifndef OR1200_NO_DMMU
	
	/* End of tests - report and finish simulation */
	l.movhi	r3,hi(0xdeaddead)
	l.ori	r3,r3,lo(0xdeaddead)
	l.nop	2

	l.movhi	r3,hi(0x8000000d)
	l.ori	r3,r3,lo(0x8000000d)
	l.nop	2

	l.addi r3,r0,0
	l.jal	exit
	l.nop


	/* DMMU invalidate function */
	/* First parameter, r3, has number of DMMU entries (should be 64)*/

_dmmu_invalidate:
	/* Setup the Data MMU's TLBS */
	l.movhi r4, hi(SPR_DTLBMR_BASE(0))
	l.ori r4, r4, lo(SPR_DTLBMR_BASE(0))

	/* DTLB invalidate loop */
1:
	l.mtspr r4, r0, 0x0
	l.addi r4, r4, 0x1
	l.sfeq r3, r0
	l.bnf 1b
	l.addi r3, r3, -1
	l.jr r9
	l.nop

