/*
 * Trivial test of floating point capability, be it hardware or software.
 *
 */

#include "cpu-utils.h"

int main()
{

  volatile int a = 5;
  volatile float a_float;
  volatile int * int_ptr;
  volatile float b = 9232.25;

  report((int)b);
  // test float to int
  if ((int) b != 9232)
    exit(1);

  a_float = (float) a; // Should convert to float
  int_ptr = (int*) &a_float;
  //report((int)*((int*)void_ptr));
  report(*int_ptr);

  if (*int_ptr != 0x40A00000)// is decimal 5 in single prec. float
    exit(2);
  
  // Should be 343.3 * 8.6
  volatile float c = (float) 343.3 * (float) ((float) 79397.35 / (float) b );

  volatile float* d = 0x0;

  *d = c;
  
  // C should be 0x45388615
  int_ptr = (int*) &c;
  report(*int_ptr);  
  if (*int_ptr != 0x45388615)
    exit(3);
  
  report(0x8000000d);

  exit(0);
  
}
