/*
	Simple test to exercise cache writeback

	Fills all of cache, then reads data back.

	Julius Baxter, ORSoC AB, <julius.baxter@orsoc.se>
	
*/
#include "spr-defs.h"

#define NUM_LINES 4
#define BYTES_PER_LINE 32
#define WORDS_PER_LINE 8
	
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

	l.movhi	r2,0	/*r2 is line counter */

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

	/* r8 used as counter for words on line - do 128/4 = 32 writes */
	l.movhi r8,0
	
wr_words_loop:
	/* r9 is data - top 16-bits is line number, bottom 16 is word */
	l.slli	r9,r2,16
	l.or	r9,r9,r8

	/* report value we're writing */
	l.ori	r3,r9,0
	l.nop	0x2

	/* report address where writing */
	l.ori	r3,r6,0
	l.nop	0x2
	
	/* do memory access */
	l.sw	0(r6),r9	/* Write counter to this address */

	l.addi	r6,r6,4		/* Increment memory pointer */

	l.sfnei	r8,WORDS_PER_LINE-1	/* Finished filling this line? */
	l.bf	wr_words_loop
	l.addi	r8,r8,1		/* Increment word counter */

	/* end of word write loop */
	
	l.sfeqi	r2,NUM_LINES-1		/* Done all lines? */
	l.bnf	wr_loop
	l.addi	r2,r2,1		/* increment line counter */

	/* end of line write loop */
	
	/* reset line counter */
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

	/* r8 used as counter for words on line - do 128/4 = 32 reads */
	l.movhi r8,0
	
rd_words_loop:
	/* r9 is data - top 16-bits is line number, bottom 16 is word */
	l.slli	r9,r2,16
	l.or	r9,r9,r8

	/* report address where reading */
	l.ori	r3,r6,0
	l.nop	0x2
	
	/* do memory access */
	l.lwz	r10, 0(r6)	/* Read data */

	/* report what we *should* read */
	l.ori	r3,r9,0
	l.nop	0x2
	
	/* report what we read */
	l.ori	r3,r10,0
	l.nop	0x2

	l.sfne	r9,r10		/* Does data equal what it should? */
	l.bf	fail
	
	l.addi	r6,r6,4		/* Increment memory pointer */

	l.sfnei	r8,WORDS_PER_LINE-1	/* Finished reading this line? */
	l.bf	rd_words_loop
	l.addi	r8,r8,1		/* Increment word counter */

	/* end of word read loop */	

	l.sfeqi	r2,NUM_LINES-1		/* Done all lines? */
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

