/* 
   Test integer division 
   
   Use a software division algorithm to perform division and compare against
   the hardware calculated results

   TODO: Check the signed division software calculation stuff is 100% correct!

   Julius Baxter, julius@opencores.org

*/

#include "cpu-utils.h"
#include "printf.h"

static int sdiv_errors, udiv_errors;

#define VERBOSE_TESTS 0

// Make this bigger when running on FPGA target. For simulation it's enough.
#define NUM_TESTS 2000

int 
or1k_div(int dividend, int divisor)
{
  int result;
  asm ("l.div\t%0,%1,%2" : "=r" (result) : "r" (dividend), "r" (divisor));
  return result;
}

unsigned int 
or1k_divu(unsigned int dividend, unsigned int divisor)
{
  int result;
  asm ("l.divu\t%0,%1,%2" : "=r" (result) : "r" (dividend), "r" (divisor));
  return result;
}


void
check_div(int dividend, int divisor, int expected_result)
{
#if VERBOSE_TESTS
  printf("l.div 0x%.8x / 0x%.8x = (SW) 0x%.8x : ", dividend, divisor,
	 expected_result);
#endif
  int result =  or1k_div(dividend, divisor);
  report(result);
  if ( result != expected_result)
    {
      printf("l.div 0x%.8x / 0x%.8x = (SW) 0x%.8x : ", dividend, divisor,
	     expected_result);
      
      printf("(HW) 0x%.8x - MISMATCH\n",result);
      sdiv_errors++;
    }
#if VERBOSE_TESTS
  else
    printf("OK\n");
#endif
      
}

void
check_divu(unsigned int dividend, unsigned int divisor, 
	   unsigned int expected_result)
{
#if VERBOSE_TESTS
  printf("l.divu 0x%.8x / 0x%.8x = (SW) 0x%.8x : ", dividend, divisor,
	 expected_result);
#endif

  unsigned int result =  or1k_divu(dividend, divisor);
  report(result);
  if ( result != expected_result)
    {
      printf("l.divu 0x%.8x / 0x%.8x = (SW) 0x%.8x : ", dividend, divisor,
	     expected_result);
      
      printf("(HW) 0x%.8x - MISMATCH\n",result);
      udiv_errors++;
    }
#if VERBOSE_TESTS
  else
    printf("OK\n");
#endif
}


// Software implementation of division
unsigned int 
div_soft(unsigned int n, unsigned int d)
{

  // unsigned 32-bit restoring divide algo:
  unsigned long long p, dd;
  long long p_signed;
  unsigned int q = 0;

  p = (unsigned long long) n;
  dd = (unsigned long long) d << 32;

  int i;
  for(i=31; i>-1; i--){
    p_signed = (2*p) - dd;
    if (p_signed>=0)
      {
	p = (2*p) - dd;
	q |= 1 << i;
      }
    else
      {
	p = p_signed + dd;
      }
  }
  
  return q;

}

int
main(void)
{
#ifdef _UART_H_
  uart_init(DEFAULT_UART);
#endif
  
  udiv_errors = 0;
  sdiv_errors = 0;

  int i;

  unsigned long n, d;
  unsigned long expected_result;
  i=0;
  while(i < NUM_TESTS)
    {
      n = rand();
      d = rand();

      report(0x10101010);

      while ( d >= n )
	d >>= (rand() & 0xff);

      if (n&0x80000000) // numerator is negative
	{
	  // Calculate a value that's really smaller than the numerator
	  while ( d >= ~(n-1) )
	    d >>= (rand() & 0xff);

	  if (!d) d = 1;
	  // Processor thinks it's in 2's complement already, so we'll convert
	  // from the interpreted 2's complement to unsigned for our calculation
	  expected_result = div_soft(~(n-1), d);	  
	  // Answer will be an unsigned +ve value, but of course it has to be
	  // negative so convert back to 2's complment negative
	  expected_result = ~expected_result + 1; // 2's complement
	}
      else
	expected_result = div_soft(n, d);

      /* Report things */
      report(n);
      report(d);
      report(expected_result);

      /* Signed divide */
      check_div(n, d, expected_result);
      

      /* Unsigned divide test */
      /* Ensure numerator's bit 31 is clear */
      n >>= 1;

      /* If divisor is > numerator, shift it by a random amount */
      while ( d >= n )
	d >>= (rand() & 0xff);
      if (!d) d = 1;

      expected_result = div_soft(n, d);

      /* Report things */
      report(n);
      report(d);
      report(expected_result);
      
      /* Unsigned divide */
      check_divu(n, d, expected_result);
      
      i++;

    }


  printf("Division check complete\n");
  printf("Unsigned:\t%d tests\t %d errors\n",
	 NUM_TESTS, udiv_errors);
  printf("Signed:\t\t%d tests\t %d errors\n",
	 NUM_TESTS, sdiv_errors);

  if ((udiv_errors > 0) || (sdiv_errors > 0))
    report(0xbaaaaaad);
  else
    report(0x8000000d);

  return 0;
  
}
