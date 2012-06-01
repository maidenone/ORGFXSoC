#ifndef _OR1200_UTILS_H_
#define _OR1200_UTILS_H_

// Pull in interrupt defines here
#include "int.h"

/* Register access macros */
#define REG8(add) *((volatile unsigned char *)(add))
#define REG16(add) *((volatile unsigned short *)(add))
#define REG32(add) *((volatile unsigned long *)(add))

/*
 * l.nop constants
 *
 */
#define NOP_NOP         0x0000      /* Normal nop instruction */
#define NOP_EXIT        0x0001      /* End of simulation */
#define NOP_REPORT      0x0002      /* Simple report */
#define NOP_PRINTF      0x0003      /* Simprintf instruction */
#define NOP_PUTC        0x0004      /* Simulation putc instruction */
#define NOP_REPORT_FIRST 0x0400     /* Report with number */
#define NOP_REPORT_LAST  0x03ff      /* Report with number */

/* For writing into SPR. */
void mtspr(unsigned long spr, unsigned long value);

/* For reading SPR. */
unsigned long mfspr(unsigned long spr);

/* Print out a character via simulator */
void sim_putc(unsigned char c);

/* Prints out a value */
void report(unsigned long value);

/* Loops/exits simulation */
void exit(int i);

/* Enable user interrupts */
void cpu_enable_user_interrupts(void);

/* Variable keeping track of timer ticks */
extern unsigned long timer_ticks;
/* Enable tick timer and interrupt generation */
void cpu_enable_timer(void);
/* Disable tick timer and interrupt generation */
void cpu_disable_timer(void);
/* Timer increment - called by interrupt routine */
void cpu_timer_tick(void);
/* Reset tick counter */
void cpu_reset_timer_ticks(void);
/* Get tick counter */
unsigned long cpu_get_timer_ticks(void);
/* Wait for 10ms */
void cpu_sleep_10ms(void);

#endif
