/*
 *
 * User interrupt handler software for OR1200
 *
 */

#include "or1200-utils.h"
#include "spr-defs.h"
#include "int.h"

/* Interrupt handlers table */
struct ihnd int_handlers[MAX_INT_HANDLERS];

/* Initialize routine */
int int_init()
{
  int i;

  for(i = 0; i < MAX_INT_HANDLERS; i++) {
    int_handlers[i].handler = 0;
    int_handlers[i].arg = 0;
  }
  
  return 0;
}

/* Add interrupt handler */ 
int int_add(unsigned long irq, void (* handler)(void *), void *arg)
{
  if(irq >= MAX_INT_HANDLERS)
    return -1;

  int_handlers[irq].handler = handler;
  int_handlers[irq].arg = arg;

  mtspr(SPR_PICMR, mfspr(SPR_PICMR) | (0x00000001L << irq));
    
  return 0;
}

/* Disable interrupt */ 
int int_disable(unsigned long irq)
{
  if(irq >= MAX_INT_HANDLERS)
    return -1;
  
  mtspr(SPR_PICMR, mfspr(SPR_PICMR) & ~(0x00000001L << irq));
  
  return 0;
}

/* Enable interrupt */ 
int int_enable(unsigned long irq)
{
  if(irq >= MAX_INT_HANDLERS)
    return -1;

  mtspr(SPR_PICMR, mfspr(SPR_PICMR) | (0x00000001L << irq));
  
  return 0;
}

/* Main interrupt handler */
void int_main()
{
  unsigned long picsr = mfspr(SPR_PICSR);
  unsigned long i = 0;

  mtspr(SPR_PICSR, 0);

  while(i < 32) {
    if((picsr & (0x01L << i)) && (int_handlers[i].handler != 0)) {
      (*int_handlers[i].handler)(int_handlers[i].arg); 
#ifdef OR1200_INT_CHECK_BIT_CLEARED
      // Ensure PICSR bit is cleared, incase it takes some time for the
      // IRQ line going low to propagate back to PIC
      while (mfspr(SPR_PICSR) & (0x00000001L << i))
#endif
	      mtspr(SPR_PICSR, mfspr(SPR_PICSR) & ~(0x00000001L << i));
    }
    i++;
  }
}
  

