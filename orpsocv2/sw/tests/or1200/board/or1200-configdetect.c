//////////////////////////////////////////////////////////////////////
////                                                              ////
////                                                              ////
//// OR1K configuration detection software                        ////
////                                                              ////
//// Description:                                                 ////
////             Determine capabilities of OR1K processor.        ////
////                                                              ////
//// Author(s):                                                   ////
////            Julius Baxter, julius@opencores.org               ////
////                                                              ////
//// TODO:                                                        ////
////      Add further tests for optional parts of OR1K.           ////
////                                                              ////
////                                                              ////
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

#include "cpu-utils.h"
#include "spr-defs.h"
#include "board.h"
#include "uart.h"
#include "printf.h"


int illegal_instruction;

int
print_or1k_upr(unsigned long upr)
{

	if (!(SPR_UPR_UP & upr))
	{
		printf("UPR not present. Cannot determine present units\n");
		return 1;
	}
	
	printf("\nOptional units present:\n");
	

	if (upr & SPR_UPR_DCP)
		printf("Data Cache\n");

	if (upr & SPR_UPR_ICP)
		printf("Instruction Cache\n");

	if (upr & SPR_UPR_DMP)
		printf("Data MMU\n");

	if (upr & SPR_UPR_IMP)
		printf("Instruction MMU\n");

	if (upr & SPR_UPR_MP)
		printf("MAC Unit\n");

	if (upr & SPR_UPR_DUP)
		printf("Debug Unit\n");

	if (upr & SPR_UPR_PCUP)
		printf("Performance Counters Unit\n");

	if (upr & SPR_UPR_PMP)
		printf("Power Management Unit\n");

	if (upr & SPR_UPR_PICP)
		printf("Programmable Interrupt Controller\n");

	if (upr & SPR_UPR_TTP)
		printf("Tick Timer\n");

	if (upr & SPR_UPR_CUP)
		printf("Context units\n");
	
	return 0;
}

void
print_or1k_dc_config (void)
{
	unsigned long dccfgr = mfspr(SPR_DCCFGR);

	unsigned long ways, sets, bs;
	
	ways = (1 << (dccfgr & SPR_DCCFGR_NCW));

	sets = (1 << ((dccfgr & SPR_DCCFGR_NCS)>> SPR_DCCFGR_NCS_OFF));

	bs = ((dccfgr & SPR_DCCFGR_CBS) == SPR_DCCFGR_CBS) ? 32 : 16;
	
	printf("\nData Cache capabilities:\n");

	printf("Ways:\t%d\t", ways);

	printf("Sets:\t%d\t", sets);

	printf("Block size: %d\n", bs);
	
	printf("Total size: %d kByte\n", (ways * sets * bs) / 1024);

	printf("Write strategy:\t\t\t\t\t%s\n",
	       ((dccfgr & SPR_DCCFGR_CWS) == SPR_DCCFGR_CWS) ?
	       "write-back" : "write-through");

	printf("Cache control register:\t\t\t\t%s\n",
	       ((dccfgr & SPR_DCCFGR_CCRI) == SPR_DCCFGR_CCRI) ?
	       "implemented" : "not implemented");

	printf("Cache block invalidate capability:\t\t%s\n",
	       ((dccfgr & SPR_DCCFGR_CBIRI) == SPR_DCCFGR_CBIRI) ?
	       "implemented" : "not implemented");

	printf("Cache block prefetch capability:\t\t%s\n",
	       ((dccfgr & SPR_DCCFGR_CBPRI) == SPR_DCCFGR_CBPRI) ?
	       "implemented" : "not implemented");

	printf("Cache block lock capability:\t\t\t%s\n",
	       ((dccfgr & SPR_DCCFGR_CBLRI) == SPR_DCCFGR_CBLRI) ?
	       "implemented" : "not implemented");

	printf("Cache block flush capability:\t\t\t%s\n",
	       ((dccfgr & SPR_DCCFGR_CBFRI) == SPR_DCCFGR_CBFRI) ?
	       "implemented" : "not implemented");

	printf("Cache block write-back capability:\t\t%s\n",
	       ((dccfgr & SPR_DCCFGR_CBWBRI) == SPR_DCCFGR_CBWBRI) ?
	       "implemented" : "not implemented");

}

void
print_or1k_dmmu_config(void)
{
	unsigned long dmmucfgr = mfspr(SPR_DMMUCFGR);

	printf("\nDMMU Capabilities:\n");

	printf("TLB Ways:\t%d",
	       (dmmucfgr & SPR_DMMUCFGR_NTW) + 1);

	printf("\tTLB Sets:\t%d",
	       1 << ((dmmucfgr & SPR_DMMUCFGR_NTS)>> SPR_DMMUCFGR_NTS_OFF));

	printf("\tATB:\t");
	if (!(dmmucfgr & SPR_DMMUCFGR_NAE))
		printf("none");
	else
		printf("%d", (dmmucfgr & SPR_DMMUCFGR_NAE)>>5);

	printf("\n");

	printf("Control register:\t\t\t\t%s\n",
	       ((dmmucfgr & SPR_DMMUCFGR_CRI) == SPR_DMMUCFGR_CRI) ?
	       "implemented" : "not implemented");
	
	printf("Protection register:\t\t\t\t%s\n",
	       ((dmmucfgr & SPR_DMMUCFGR_PRI) == SPR_DMMUCFGR_PRI) ?
	       "implemented" : "not implemented");

	printf("Invalidate entry register:\t\t\t%s\n",
	       ((dmmucfgr & SPR_DMMUCFGR_TEIRI) == SPR_DMMUCFGR_TEIRI) ?
	       "implemented" : "not implemented");

	printf("TLB reload:\t\t\t\t\t%sware\n",
	       ((dmmucfgr & SPR_DMMUCFGR_HTR) == SPR_DMMUCFGR_HTR) ?
	       "hard" : "soft");
	
}

void
print_or1k_immu_config(void)
{
	unsigned long immucfgr = mfspr(SPR_IMMUCFGR);

	printf("\nIMMU Capabilities:\n");

	printf("TLB Ways:\t%d",
	       (immucfgr & SPR_IMMUCFGR_NTW) + 1);

	printf("\tTLB Sets:\t%d",
	       1 << ((immucfgr & SPR_IMMUCFGR_NTS)>> SPR_IMMUCFGR_NTS_OFF));

	printf("\tATB:\t");
	if (!(immucfgr & SPR_IMMUCFGR_NAE))
		printf("none");
	else
		printf("%d", (immucfgr & SPR_IMMUCFGR_NAE)>>5);

	printf("\n");

	printf("Control register:\t\t\t\t%s\n",
	       ((immucfgr & SPR_IMMUCFGR_CRI) == SPR_IMMUCFGR_CRI) ?
	       "implemented" : "not implemented");
	
	printf("Protection register:\t\t\t\t%s\n",
	       ((immucfgr & SPR_IMMUCFGR_PRI) == SPR_IMMUCFGR_PRI) ?
	       "implemented" : "not implemented");

	printf("Invalidate entry register:\t\t\t%s\n",
	       ((immucfgr & SPR_IMMUCFGR_TEIRI) == SPR_IMMUCFGR_TEIRI) ?
	       "implemented" : "not implemented");

	printf("TLB reload:\t\t\t\t\t%sware\n",
	       ((immucfgr & SPR_IMMUCFGR_HTR) == SPR_IMMUCFGR_HTR) ?
	       "hard" : "soft");
	
}

void
print_or1k_ic_config (void)
{
	unsigned long iccfgr = mfspr(SPR_ICCFGR);

	unsigned long ways, sets, bs;

	printf("\nInstruction Cache capabilities:\n");
	
	ways = (1 << (iccfgr & SPR_ICCFGR_NCW));

	sets = (1 << ((iccfgr & SPR_ICCFGR_NCS)>> SPR_ICCFGR_NCS_OFF));

	bs = ((iccfgr & SPR_ICCFGR_CBS) == SPR_ICCFGR_CBS) ? 32 : 16;	

	printf("Ways:\t%d\t", ways);

	printf("Sets:\t%d\t", sets);

	printf("Block size: %d\n", bs);
	
	printf("Total size: %d kByte\n", (ways * sets * bs) / 1024);
	
	printf("Cache control register:\t\t\t\t%s\n",
	       ((iccfgr & SPR_ICCFGR_CCRI) == SPR_ICCFGR_CCRI) ?
	       "implemented" : "not implemented");

	printf("Cache block invalidate capability:\t\t%s\n",
	       ((iccfgr & SPR_ICCFGR_CBIRI) == SPR_ICCFGR_CBIRI) ?
	       "implemented" : "not implemented");

	printf("Cache block prefetch capability:\t\t%s\n",
	       ((iccfgr & SPR_ICCFGR_CBPRI) == SPR_ICCFGR_CBPRI) ?
	       "implemented" : "not implemented");

	printf("Cache block lock capability:\t\t\t%s\n",
	       ((iccfgr & SPR_ICCFGR_CBLRI) == SPR_ICCFGR_CBLRI) ?
	       "implemented" : "not implemented");

}

void
print_or1k_cpu_config(void)
{
	unsigned long cpucfgr = mfspr(SPR_CPUCFGR);
	
	printf("\nCPU Capabilities:\n");

	printf("Shadow GPR files: %d\n",(cpucfgr & SPR_CPUCFGR_NSGF));
	
	printf("GPR file size: %s\n",
	       ((cpucfgr & SPR_CPUCFGR_CGF)==SPR_CPUCFGR_CGF) ? "<32" : "32");

	printf("ORBIS32: %ssupported\n",
	       ((cpucfgr & SPR_CPUCFGR_OB32S)==SPR_CPUCFGR_OB32S) ?"":"not ");

	printf("ORBIS64: %ssupported\n",
	       ((cpucfgr & SPR_CPUCFGR_OB64S)==SPR_CPUCFGR_OB64S) ?"":"not ");

	printf("ORFPX32: %ssupported\n",
	       ((cpucfgr & SPR_CPUCFGR_OF32S)==SPR_CPUCFGR_OF32S) ?"":"not ");

	printf("ORFPX64: %ssupported\n",
	       ((cpucfgr & SPR_CPUCFGR_OF64S)==SPR_CPUCFGR_OF64S) ?"":"not ");

	printf("ORVDX64: %ssupported\n",
	       ((cpucfgr & SPR_CPUCFGR_OV64S)==SPR_CPUCFGR_OV64S) ?"":"not ");
}

void
print_or1k_version_reg(void)
{
	unsigned long vr = mfspr(SPR_VR);
	
	printf("\nVersion Register\n");
	
	printf("Version:\t0x%2x",(vr & SPR_VR_VER) >> SPR_VR_VER_OFF);
	printf("\tConfig:\t%d",(vr & SPR_VR_CFG) >> SPR_VR_CFG_OFF);
	printf("\tRevision:\t%d",(vr & SPR_VR_REV));
	printf("\n");
}
void
print_or1k_du_config(void)
{
	unsigned long dcfgr = mfspr(SPR_DCFGR);
	
	printf("\nDebug Unit Capabilities:\n");

	printf("Debug Pairs: %d\n",
	       (dcfgr & SPR_DCFGR_NDP) + 1);

	printf("Watchpoint counters:\t\t\t\t%simplemented\n",
	       ((dcfgr & SPR_DCFGR_WPCI)==SPR_DCFGR_WPCI) ?"":"not ");
}

/* Handler for illegal instructions, to indicate if the exception was tripped */
void
illegal_insn_handler(void)
{
	// Step past illegal instruction
	unsigned long epcr = mfspr(SPR_EPCR_BASE);
	epcr += 4;
	mtspr(SPR_EPCR_BASE,epcr);
	illegal_instruction = 1;
	return;
}

/* Call function we passed, return 1 if supported by hardware, 0 if not */
int
or1k_insn_hw_support_detect(void (* function)(void))
{
	/* Install illegal instruction handler */
	add_handler(7, illegal_insn_handler);

	/* Clear illegal instruction indicator */
	illegal_instruction = 0;

	/* Call function that tests hardware support of an instruction,
	   potentially resulting in illegal instruction exception occurring. */
	(*function)();

	return !illegal_instruction;	
}

void
or1k_test_mul (void)
{
	volatile int a, b, c;
	a = b = 12;
        asm ("l.mul %0,%1,%2" : "=r" (c) : "r" (a), "r" (b));
	return;
}

void
or1k_test_div (void)
{
	volatile int a, b, c;
	a = b = 12;
        asm ("l.div %0,%1,%2" : "=r" (c) : "r" (a), "r" (b));
	return;
}

void
or1k_test_ff1 (void)
{
	volatile int a, b;
	a = b = 12;
        asm ("l.ff1 %0,%1" : "=r" (a) : "r" (b));
	return;
}


void
or1k_test_fl1 (void)
{
	volatile int a, b;
	a = b = 12;
        asm ("l.fl1 %0,%1" : "=r" (a) : "r" (b));
	return;
}

void
or1k_test_lfrem (void)
{
	volatile int a, b, c;
	a = b = 12;
        asm ("lf.rem.s %0,%1,%2" : "=r" (c) : "r" (a), "r" (b));
	return;
}

void 
print_or1k_insn_support (char* insnname,void (* function)(void))
{

	printf("\n%s instruction:\t %ssupported by hardware\n", insnname,
	       or1k_insn_hw_support_detect(function) ? "" : "not ");
}

int
main (void)
{
	
	unsigned long upr;

	uart_init(DEFAULT_UART);

	printf("\nOR1200 capability detection\n");

	print_or1k_cpu_config();

	print_or1k_version_reg();

	/* Unit present register */
	upr = mfspr(SPR_UPR);

	if (print_or1k_upr(upr))
		/* No UPR - can't tell what's here */
		return 0;

	if (upr & SPR_UPR_DCP)
		print_or1k_dc_config();

	if (upr & SPR_UPR_ICP)
		print_or1k_ic_config();

	if (upr & SPR_UPR_DMP)
		print_or1k_dmmu_config();

	if (upr & SPR_UPR_IMP)
		print_or1k_immu_config();

	if (upr & SPR_UPR_DUP)
		print_or1k_du_config();

	/* Now test for supported instructions */
	print_or1k_insn_support("Multiply", or1k_test_mul);
	print_or1k_insn_support("Multiply", or1k_test_mul);
	print_or1k_insn_support("Divide", or1k_test_div);
	print_or1k_insn_support("Find first '1'", or1k_test_ff1);
	print_or1k_insn_support("Find last '1'", or1k_test_fl1);
	print_or1k_insn_support("Floating remainder", or1k_test_lfrem);
	
	printf("End of capabilities testing\n");

}
