/* 
   Test integer multiply 
   
   Use a software multiplication algorithm to compare against hardware 
   calculated results

   Julius Baxter, julius@opencores.org

*/

#include "cpu-utils.h"
#include "printf.h"

static int smul_errors, umul_errors;

#define VERBOSE_TESTS 0

// Make this bigger when running on FPGA target. For simulation it's enough.
#define NUM_TESTS 2000

int 
or1k_mul(int multiplicant, int multiplier)
{
  int result;
  asm ("l.mul\t%0,%1,%2" : "=r" (result) : "r" (multiplicant), 
       "r" (multiplier));
  return result;
}

unsigned int 
or1k_mulu(unsigned int mulidend, unsigned int mulisor)
{
  int result;
  asm ("l.mulu\t%0,%1,%2" : "=r" (result) : "r" (mulidend), "r" (mulisor));
  return result;
}


void
check_mul(int multiplicand, int multiplier, int expected_result)
{
#if VERBOSE_TESTS
  printf("l.mul 0x%.8x * 0x%.8x = (SW) 0x%.8x : ", multiplicand, multiplier,
	 expected_result);
#endif
  int result =  or1k_mul(multiplicand, multiplier);
  report(result);
  if ( result != expected_result)
    {
      printf("l.mul  0x%.8x * 0x%.8x = (SW) 0x%.8x : ", multiplicand, multiplier,
	     expected_result);
      
      printf("(HW) 0x%.8x - MISMATCH\n",result);
      smul_errors++;
    }
#if VERBOSE_TESTS
  else
    printf("OK\n");
#endif
      
}

void
check_mulu(unsigned int multiplicand, unsigned int multiplier, 
	   unsigned int expected_result)
{
#if VERBOSE_TESTS
  printf("l.mulu 0x%.8x * 0x%.8x = (SW) 0x%.8x : ", multiplicand, multiplier,
	 expected_result);
#endif

  unsigned int result =  or1k_mulu(multiplicand, multiplier);
  report(result);
  if ( result != expected_result)
    {
      printf("l.mulu 0x%.8x * 0x%.8x = (SW) 0x%.8x : ", multiplicand, multiplier,
	     expected_result);
      
      printf("(HW) 0x%.8x - MISMATCH\n",result);
      umul_errors++;
    }
#if VERBOSE_TESTS
  else
    printf("OK\n");
#endif
}


// Software implementation of multiply
unsigned int 
mul_soft(unsigned int n, unsigned int d)
{

  unsigned int m = 0;
  //printf("sft: 0x%x 0x%xd\n",n,d);
  int i;
  for(i=0; i<32; i++)
    {
      //printf("bit %d: 0x%x\n",i, (((1<<i) & d)));
      if ((1<<i) & d)
	{
	  m += (unsigned int) (n << i);
	}      
    }

  return (unsigned int) m;
}

int
main(void)
{
#ifdef _UART_H_
  uart_init(DEFAULT_UART);
#endif
  
  umul_errors = 0;
  smul_errors = 0;

  int i;

  unsigned int n, d;
  unsigned int expected_result;
  i=0;
  n=0;d=0;
  while(i < NUM_TESTS)
    {

      n = rand() >> 20;
      d = (rand() >> 24);

      report(0x10101010);

      
      if (n&0x10) // Randomly select if we should negate n
	{
	  // 2's complement of n
	  n = ~n + 1;
	}
      
      if (d&0x80) // Randomly select if we should negate d
	{
	  // 2's complement of d
	  d = ~d + 1;
	}
      
      if ((n & 0x80000000) && (d & 0x80000000))
	expected_result = mul_soft(~(n-1), ~(d-1));
      else if ((n & 0x80000000) && !(d & 0x80000000))
	{
	  expected_result = mul_soft(~(n-1), d);
	  expected_result = ~expected_result + 1; // 2's complement
	}
      else if (!(n & 0x80000000) && (d & 0x80000000))
	{
	  expected_result = mul_soft(n, ~(d-1));
	  expected_result = ~expected_result + 1; // 2's complement
	}
      else if (!(n & 0x80000000) && !(d & 0x80000000))
	expected_result = mul_soft(n, d);


      /* Report things */
      report(n);
      report(d);
      report(expected_result);


      /* Signed mulide */
      check_mul(n, d, expected_result);
      

      /* Unsigned mulide test */
      /* Ensure numerator's bit 31 is clear */
      n >>= 1;

      expected_result = mul_soft(n, d);

      /* Report things */
      report(n);
      report(d);
      report(expected_result);
      
      /* Unsigned mulide */
      check_mulu(n, d, expected_result);
      
      report(i);
      i++;

    }


  printf("Integer multiply check complete\n");
  printf("Unsigned:\t%d tests\t %d errors\n",
	 NUM_TESTS, umul_errors);
  printf("Signed:\t\t%d tests\t %d errors\n",
	 NUM_TESTS, smul_errors);

  if ((umul_errors > 0) || (smul_errors > 0))
    report(0xbaaaaaad);
  else
    report(0x8000000d);

  return 0;
  
}
