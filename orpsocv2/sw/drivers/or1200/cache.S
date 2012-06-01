#include "spr-defs.h"
	/* Cache init. To be called during init ONLY */
	
	.global	_cache_init
        .type	_cache_init,@function

_cache_init:	
	/* Instruction cache enable */
	/* Check if IC present and skip enabling otherwise */
	l.mfspr r3,r0,SPR_UPR
	l.andi  r4,r3,SPR_UPR_ICP
	l.sfeq  r4,r0
	l.bf    .L8
	l.nop
	
	/* Disable IC */
	l.mfspr r6,r0,SPR_SR
	l.addi  r5,r0,-1
	l.xori  r5,r5,SPR_SR_ICE
	l.and   r5,r6,r5
	l.mtspr r0,r5,SPR_SR
	
	/* Establish cache block size
	If BS=0, 16;
	If BS=1, 32;
	r14 contain block size
	*/
	l.mfspr r3,r0,SPR_ICCFGR
	l.andi  r4,r3,SPR_ICCFGR_CBS
	l.srli  r5,r4,7
	l.ori   r6,r0,16
	l.sll   r14,r6,r5
	
	/* Establish number of cache sets
	r7 contains number of cache sets
	r5 contains log(# of cache sets)
	*/
	l.andi  r4,r3,SPR_ICCFGR_NCS
	l.srli  r5,r4,3
	l.ori   r6,r0,1
	l.sll   r7,r6,r5
	
	/* Invalidate IC */
	l.addi  r6,r0,0
	l.sll   r5,r14,r5
	
.L7:
	l.mtspr r0,r6,SPR_ICBIR
	l.sfne  r6,r5
	l.bf    .L7
	l.add   r6,r6,r14
	
	/* Enable IC */
	l.mfspr r6,r0,SPR_SR
	l.ori   r6,r6,SPR_SR_ICE
	l.mtspr r0,r6,SPR_SR
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop
	l.nop

.L8:
	/* Data cache enable */
        /* Check if DC present and skip enabling otherwise */
        l.mfspr r3,r0,SPR_UPR
        l.andi  r4,r3,SPR_UPR_DCP
        l.sfeq  r4,r0
        l.bf    .L10
        l.nop
        /* Disable DC */
        l.mfspr r6,r0,SPR_SR
        l.addi  r5,r0,-1
        l.xori  r5,r5,SPR_SR_DCE
	l.and   r5,r6,r5
        l.mtspr r0,r5,SPR_SR
        /* Establish cache block size
           If BS=0, 16;
           If BS=1, 32;
           r14 contain block size
        */
        l.mfspr r3,r0,SPR_DCCFGR
        l.andi  r4,r3,SPR_DCCFGR_CBS
        l.srli  r5,r4,7
        l.ori   r6,r0,16
        l.sll   r14,r6,r5
        /* Establish number of cache sets
           r7 contains number of cache sets
           r5 contains log(# of cache sets)
        */
	l.andi  r4,r3,SPR_DCCFGR_NCS
	l.srli  r5,r4,3
        l.ori   r6,r0,1
        l.sll   r7,r6,r5
        /* Invalidate DC */
        l.addi  r6,r0,0
        l.sll   r5,r14,r5
.L9:
        l.mtspr r0,r6,SPR_DCBIR
        l.sfne  r6,r5
        l.bf    .L9
	l.add   r6,r6,r14
        /* Enable DC */
        l.mfspr r6,r0,SPR_SR
        l.ori   r6,r6,SPR_SR_DCE
        l.mtspr r0,r6,SPR_SR

.L10:
	/* Return */
	l.jr	r9
	l.nop