/*
 * UART loopback interrupt test
 *
 * Tests UART and interrupt routines servicing them.
 *
 * Relies on testbench having uart0's lines in loopback (rx = tx)
 *
 * Julius Baxter, julius.baxter@orsoc.se
 *
*/


#include "cpu-utils.h"
#include "spr-defs.h"
#include "board.h"
#include "uart.h"
#include "int.h"
#include "orpsoc-defines.h"

#ifndef UART0
# error
# error UART0 missing and is required for UART interrupt (loopback) test
# error
#endif

struct uart_tx_ctrl
{
  char *bufptr;
  int busy;
};

volatile struct uart_tx_ctrl uart0_tx_ctrl;

void uart_int_handler(void* corenum);

void uart_int_handler(void* corenum)
{

  int core = *((int*)corenum);

  if (core)report(core);
  
  char iir = uart_get_iir(core);

  if ( (iir & UART_IIR_RLSI)  == UART_IIR_RLSI)
    uart_get_lsr(core); // Should clear this interrupt
  else if ( (iir & UART_IIR_RDI) == UART_IIR_RDI )
    {
      // Was potentially also a timeout. Do we care?
      
      // Data received. Pull all from the FIFO buffer, here we just report it
      // and throw it away
      char rxchar;
      while (uart_check_for_char(core))
	{
	  rxchar = uart_getc(core);
	  report(0xff & rxchar);
	  if (rxchar == 0x2a) // Exit simulation when RX char is '*'
	    {
	      report(0x8000000d);
	      exit(0);
	    }
	}
    }
  else if ( (iir & UART_IIR_THRI) ==  UART_IIR_THRI)
    {
      // Only trigered if we've set something to be transmitted
      // and enabled the interrupt.
      // Put next thing to be transmitted into buffer, check if it's
      // the last, if so, disable interrupts.
      if (uart0_tx_ctrl.bufptr[0] == 0) // EOL, disable interrupt after this char
	{
	  uart_txint_disable(core);
	  uart0_tx_ctrl.busy = 0;
	}
      else // Transmit this byte
	{
	  uart_putc_noblock(core, uart0_tx_ctrl.bufptr[0]);
	  uart0_tx_ctrl.bufptr++;
	}
    }
  else if ( (iir & UART_IIR_MSI) == UART_IIR_MSI )
    {
      // Just read the modem status register to clear this
      uart_get_msr(core);
    }
}


void uart0_tx_buffer(char* buf)
{
  while (uart0_tx_ctrl.busy); // Wait until we can transmit more
  uart0_tx_ctrl.bufptr = buf;
  uart0_tx_ctrl.busy = 1;
  uart_txint_enable(0);
}

int main()
{
  int uart0_core = 0;
  int uart1_core = 1;
  uart0_tx_ctrl.busy = 0;
  
  /* Set up interrupt handler */
  int_init();

  /* Install UART core 0 interrupt handler */
  int_add(UART0_IRQ, uart_int_handler,(void*) &uart0_core);
  
  /* Install UART core 1 interrupt handler */
  //int_add(UART1_IRQ, uart_int_handler,(void*) &uart1_core);

  /* Enable interrupts in supervisor register */
  mtspr (SPR_SR, mfspr (SPR_SR) | SPR_SR_IEE);
  
  uart_init(uart0_core);
  //uart_init(uart1_core);
  
  //uart_rxint_enable(uart1_core);
  uart_rxint_enable(uart0_core);
  
  char* teststring = "\n\tHello world from UART 0\n\0";

  uart0_tx_buffer(teststring);

  // Do other things while we transmit
  float f1, f2, f3; int i;
  f1 = 0.2382; f2 = 4342.65; f3=0;
  for(i=0;i<32;i++) f3 += f1*f3 + f2;

  report(f3);
  report(0x4aaaaa1f);
  
  char* done_calculating = "\tDone with the number crunching!\n\0";

  uart0_tx_buffer(done_calculating);
  
  // Character '*', which will be received in the interrupt handler and cause
  // the simulation to exit.
  char* finish = "*\n\0";
  
  uart0_tx_buffer(finish);
  
  while(1); // will exit in the rx interrupt routine

}
