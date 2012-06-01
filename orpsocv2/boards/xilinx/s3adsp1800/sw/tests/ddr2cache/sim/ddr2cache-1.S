/*
	Simple test to exercise cache writeback

	Hardcoded for cache with 4 lines of 32 bytes.
	
	Julius Baxter, ORSoC AB, <julius.baxter@orsoc.se>
	
*/
#include "spr-defs.h"

#define NUM_LINES 4
#define BYTES_PER_LINE 32
	
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
	l.movhi r1,hi(_stack)
	l.ori	r1,r1,lo(_stack)
	l.addi  r2, r0, -3
	l.and   r1, r1, r2

	l.movhi	r2,0	/*r2 is counter */

	/* Write data into addresses that should step through the
	lines of the cache */

	l.addi	r5,r0,-(BYTES_PER_LINE-1)
	l.and	r4,r1,r5	/* r4 has base address of place to access */
	l.addi	r4,r4,BYTES_PER_LINE	/* Go to safe address, past top of stack */

	/* report this address */
	l.ori	r3,r4,0
	l.nop	0x2

wr_loop:
	l.muli	r5,r2,BYTES_PER_LINE	/* offset from base address */
	l.add	r6,r5,r4	/* Address to write to (line offset + base) */

	/* report this address */
	l.ori	r3,r6,0
	l.nop	0x2

	/* report counter */
	l.ori	r3,r2,0
	l.nop 	0x2

	/* do memory access */
	l.sw	0(r6),r2	/* Write counter to this address */

	l.sfeqi	r2,(NUM_LINES-1)	/* Done all lines? */
	l.bnf	wr_loop
	l.addi	r2,r2,1		/* increment line counter */

	/* end of write loop */
	
	/* reset counter */
	l.movhi	r2,0
	
rd_loop:
	l.muli	r5,r2,BYTES_PER_LINE	/* offset from base address */
	l.add	r6,r5,r4	/* Address to write to (line offset + base) */

	/* report this address */
	l.ori	r3,r6,0
	l.nop	0x2

	/* report counter */
	l.ori	r3,r2,0
	l.nop 	0x2

	/* do memory access */
	l.lwz	r7,0(r6)	/* load value */

	/* should equal r2 */
	l.sfne	r2,r7
	l.bf	fail
	l.nop	

	l.sfeqi	r2,(NUM_LINES-1)	/* Done all lines? */
	l.bnf	rd_loop
	l.addi	r2,r2,1		/* increment line counter */

	/* end of read loop */
pass:
	l.movhi	r3,0x8000
	l.ori	r3,r3,0x000d
	l.nop	0x2
	l.movhi	r3,0
	l.nop	0x1

fail:
	l.movhi	r3,0xbaaa
	l.ori	r3,r3,0xaaad
	l.nop	0x1

