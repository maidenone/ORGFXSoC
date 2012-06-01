//////////////////////////////////////////////////////////////////////
////                                                              ////
////                                                              ////
//// OR1K Timer Demo Software                                     ////
////                                                              ////
//// Description:                                                 ////
////             Demonstrate timer.                               ////
////             First block main loop while checking ticks, then ////
////             install custom tick handler and do other things  ////
////             while tick timer fires. The "other things" use   ////
////             the UART, so too does the custom tick handler,   ////
////             so the tick interrupt is blocked while the main  ////
////             loop is using the UART.                          ////
////                                                              ////
//// Author(s):                                                   ////
////            Julius Baxter, julius@opencores.org               ////
////                                                              ////
////                                                              ////
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

#include "cpu-utils.h"
#include "spr-defs.h"
#include "board.h"
#include "uart.h"
#include "printf.h"

/* RTOS-like critical section enter and exit functions */
static inline void disable_ttint(void)
{
	// Disable timer interrupt in supervisor register
	mtspr (SPR_SR, mfspr (SPR_SR) & ~SPR_SR_TEE);
	// Disable timer interrupt generation in tick timer mode register
	//mtspr(SPR_TTMR, mfspr (SPR_TTMR) & ~SPR_TTMR_IE);
}

static inline void enable_ttint(void)
{
	// Enable timer interrupt in supervisor register
	mtspr(SPR_SR, SPR_SR_TEE | mfspr(SPR_SR));
	// Enable timer interrupt generation in tick timer mode register
	//mtspr(SPR_TTMR, mfspr (SPR_TTMR) | SPR_TTMR_IE);
}

#define MAIN_PRINT_ENTER disable_ttint
#define MAIN_PRINT_EXIT enable_ttint


void 
print_time(void)
{
	static int ms_counter = 0;
	static int s_counter = 0;
	// Position the cursor on the line and print the time so far.
	
	// Usually we go on 100 ticks per second, which is 10ms each:
	if (TICKS_PER_SEC == 100)
		ms_counter += 10;

	if (ms_counter >= 1000)
	{
		s_counter++;
		ms_counter = 0;			
	}
	
	// Sometimes print hasn't finished properly...
	printf("\r");
	// ANSI Escape sequence "\esc[40C" - cursor forward 40 places
	uart_putc(DEFAULT_UART,0x1b);
	uart_putc(DEFAULT_UART,0x5b);
	uart_putc(DEFAULT_UART,'4');
	uart_putc(DEFAULT_UART,'0');
	uart_putc(DEFAULT_UART,'C');
	// ANSI Escape sequence "\esc[K" - delete rest of line
	uart_putc(DEFAULT_UART,0x1b);
	uart_putc(DEFAULT_UART,0x5b);
	uart_putc(DEFAULT_UART,'K');
	printf("%2d.%03d",s_counter,ms_counter);
	printf("\r");

}

void our_timer_handler(void);

void our_timer_handler(void)
{
	// Call time output function
	print_time();

	// can potentially also call cpu_timer_tick() here to hook back into
	// the CPU's timer tick function.
	
	// Reset timer mode register to interrupt with same interval
	mtspr(SPR_TTMR, SPR_TTMR_IE | SPR_TTMR_RT | 
	      ((IN_CLK/TICKS_PER_SEC) & SPR_TTMR_PERIOD));
}


int
main (void)
{
	
	int seconds;
	unsigned long *adr;
	unsigned long data;
	volatile int i;
	
	uart_init(DEFAULT_UART);

	printf("\nOR1200 Timer Demo\n");

	printf("\nInitialising Timer\n");
	// Reset timing variables
	seconds = 0;
	cpu_reset_timer_ticks();
	cpu_enable_timer();

	printf("\nBlocking main() loop for 5 seconds\n");
	
	printf("Elapsed: %ds",seconds);

	while(seconds < 5)
	{
		while (cpu_get_timer_ticks() < (TICKS_PER_SEC * (seconds+1)));
		seconds++;
		printf("\rElapsed: %ds",seconds);
	}
	printf("\n");

	printf("\nInstalling our timer handler\n");

	printf("\nmain() loop will dump mem. contents, timer interrupt will print time\n");
	
	add_handler(0x5 /* Timer */, our_timer_handler);
	
	adr=0;
	while(1)
	{
		data = *(adr++);
		
		// Disable tick timer interrupt here, as UART printing is not
		// re-entrant!
		MAIN_PRINT_ENTER;
		printf("0x%08x: 0x%08x\r",adr,data);
		MAIN_PRINT_EXIT;
		// Delay a little bit..
		for(i=0;i<40000;i++);
	}

	return 0;
}

