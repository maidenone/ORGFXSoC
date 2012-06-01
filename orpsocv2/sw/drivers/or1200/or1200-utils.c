#include "spr-defs.h"
#include "or1200-utils.h"
#include "board.h" // For timer rate (IN_CLK, TICKS_PER_SEC)

/* For writing into SPR. */
void 
mtspr(unsigned long spr, unsigned long value)
{	
  asm("l.mtspr\t\t%0,%1,0": : "r" (spr), "r" (value));
}

/* For reading SPR. */
unsigned long 
mfspr(unsigned long spr)
{	
  unsigned long value;
  asm("l.mfspr\t\t%0,%1,0" : "=r" (value) : "r" (spr));
  return value;
}

/* Print out a character via simulator */
void 
sim_putc(unsigned char c)
{
  asm("l.addi\tr3,%0,0": :"r" (c));
  asm("l.nop %0": :"K" (NOP_PUTC));
}

/* print long */
void 
report(unsigned long value)
{
  asm("l.addi\tr3,%0,0": :"r" (value));
  asm("l.nop %0": :"K" (NOP_REPORT));
}

/* Loops/exits simulation */
void 
exit (int i)
{
  asm("l.add r3,r0,%0": : "r" (i));
  asm("l.nop %0": :"K" (NOP_EXIT));
  while (1);
}

/* Enable user interrupts */
void
cpu_enable_user_interrupts(void)
{
  /* Enable interrupts in supervisor register */
  mtspr (SPR_SR, mfspr (SPR_SR) | SPR_SR_IEE);
}

/* Tick timer variable */
unsigned long timer_ticks;

/* Tick timer functions */
/* Enable tick timer and interrupt generation */
void 
cpu_enable_timer(void)
{
  mtspr(SPR_TTMR, SPR_TTMR_IE | SPR_TTMR_RT | ((IN_CLK/TICKS_PER_SEC) & SPR_TTMR_PERIOD));
  mtspr(SPR_SR, SPR_SR_TEE | mfspr(SPR_SR));

}

/* Disable tick timer and interrupt generation */
void 
cpu_disable_timer(void)
{
  // Disable timer: clear it all!
  mtspr (SPR_SR, mfspr (SPR_SR) & ~SPR_SR_TEE);
  mtspr(SPR_TTMR, 0);

}

/* Timer increment - called by interrupt routine */
void 
cpu_timer_tick(void)
{
  timer_ticks++;
  // Reset timer mode register to interrupt with same interval
  mtspr(SPR_TTMR, SPR_TTMR_IE | SPR_TTMR_RT | 
	((IN_CLK/TICKS_PER_SEC) & SPR_TTMR_PERIOD));
}

/* Reset tick counter */
void 
cpu_reset_timer_ticks(void)
{
  timer_ticks=0;
}

/* Get tick counter */
unsigned long 
cpu_get_timer_ticks(void)
{
  return timer_ticks;
}

/* Wait for 10ms, assumes CLK_HZ is 100, which it usually is.
   Will be slightly inaccurate!*/
void 
cpu_sleep_10ms(void)
{
  unsigned long ttcr = mfspr(SPR_TTCR) & SPR_TTCR_PERIOD;
  unsigned long first_time = cpu_get_timer_ticks();
  while (first_time == cpu_get_timer_ticks()); // Wait for tick to occur
  // Now wait until we're past the tick value we read before to know we've
  // gone at least enough
  while(ttcr > (mfspr(SPR_TTCR) & SPR_TTCR_PERIOD));

}
  
