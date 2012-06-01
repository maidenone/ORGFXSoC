#include "cpu-utils.h"
#include "uart.h"
#include "printf.h"

// Echo some characters until we recieve a '!' charecter, after which
// we will exit simulation.


int
main()
{

  uart_init(DEFAULT_UART); // init the UART before we can printf
  
  char c;
  printf("Echo - type and see it echo back:\n");
  while (1)
    {
      c = uart_getc(DEFAULT_UART);  
      printf("%c",c);
    }
}
