/*
 * OR1200 Data cache test
 * Writes and checks various values in places to exercise data cache line 
 * swaps.
 *
 * Change LOOPS define to alter length of test (2048 is OK size)
 */


#include "cpu-utils.h"
#include "lib-utils.h"
#include "spr-defs.h"

#define LOOPS 64
#define WORD_STRIDE 8

extern unsigned long _stack;

unsigned long int my_lfsr;

unsigned long int next_rand()
{
  my_lfsr = (my_lfsr >> 1) ^ (unsigned long int)((0 - (my_lfsr & 1u)) & 0xd0000001u); 
  return my_lfsr;
}

int
main()
{

	unsigned long stack_top = (unsigned long) &_stack;

  // Check data cache is present and enabled
  if (!(mfspr(SPR_UPR)& SPR_UPR_DCP) | !(mfspr(SPR_SR) & SPR_SR_DCE))
    {
      // Not really a pass, but not really a fail, either.
      report(0x8000000d);
      return 0;
    }

  volatile char* ptr = (volatile char*) (stack_top + 256);
  int i;

  ptr[0] = 0xab;

  ptr[4096] = 0xcd;

  ptr[8192] = 0xef;

  report(ptr[0]);

  report(ptr[4096]);

  report(ptr[8192]);

  // If cache is write back, then test flush and writeback functionalities
  // Check cache write stategy bit (CWS) for write back
  if (mfspr(SPR_DCCFGR) & SPR_DCCFGR_CWS)
    {
      // TODO: Check flush and write back actually work by mapping the same
      // space as CI through DMMU. For now the following will aid checking on
      // waveform.
      volatile int * test_addr = (int *) 0xefaa10;

      // Fill some lines with data
      for (i=0;i<64;i++)
	test_addr[i] = 1+(i<<i)/(i+1);

      // Flush the lines
      int spr_addr = SPR_DCBFR;

      for (i=0;i<16;i++)
	asm("l.mtspr\t\t%0,%1,0": : "r" (spr_addr), "r" (test_addr+(i*4)));

      // Check the data
      for (i=0;i<64;i++)
	if (test_addr[i] != (1+(i<<i)/(i+1)))
	  return i;

      // Fill some lines with data
      for (i=0;i<64;i++)
	test_addr[i] = ~i;

      // Force writeback of the lines
      spr_addr = SPR_DCBWR;

      for (i=0;i<16;i++)
	asm("l.mtspr\t\t%0,%1,0": : "r" (spr_addr), "r" (test_addr+(i*4)));

      // Check the data
      for (i=0;i<64;i++)
	if (test_addr[i] != ~i) 
	  return ~i;

      
    }

  // Now generate some random numbers, write them in in strides that should 
  // execercise the cache's line reloading/storing mechanism.

  // init LFSR
  my_lfsr = RAND_LFSR_SEED;
  volatile unsigned long int *lptr = (volatile unsigned long int*) (stack_top + 256);
  for(i=0;i<LOOPS;i++)
    {
      lptr[(i*WORD_STRIDE)-1] = next_rand();
      lptr[(i*WORD_STRIDE)+0] = next_rand();
      lptr[(i*WORD_STRIDE)+1] = next_rand();
      lptr[(i*WORD_STRIDE)+2] = next_rand();
      lptr[(i*WORD_STRIDE)+3] = next_rand();
      lptr[(i*WORD_STRIDE)+4] = next_rand();
    }
  
  report(next_rand());

#define CHECK(off) expected=next_rand(); \
  if (lptr[(i*WORD_STRIDE)+off] != expected)

#define FAILURE(x,y) report(y); report(expected); \
  report(lptr[(i*WORD_STRIDE)+y]);exit(0xbaaaaaad)

  // reset lfsr seed
  my_lfsr = RAND_LFSR_SEED;
  unsigned long int expected;
  for (i=0;i<LOOPS;i++)
    {
      report(i);
      CHECK(-1)
	{
	  FAILURE(i,-1);
	}
      CHECK(0)
	{
	  FAILURE(i,0);
	}
      CHECK(1)
	{
	  FAILURE(i,1);
	}
      CHECK(2)
	{
	  FAILURE(i,2);
	}
      CHECK(3)
	{
	  FAILURE(i,3);
	}
      CHECK(4)
	{
	  FAILURE(i,4);
	}
    }

  report(next_rand());

  report(0x8000000d);

  exit(0);
}

  

  

  
