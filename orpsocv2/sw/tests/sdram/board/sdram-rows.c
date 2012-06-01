/*
 * SDRAM row test, for running on the board (with printfs)
 *
 * Tests accessing every row
 *
*/

#include "cpu-utils.h"
#include "board.h"
#include "sdram.h"
#include "uart.h"
#include "printf.h"

#ifndef _SDRAM_H_
#define _SDRAM_H_

#ifdef  MT48LC32M16A2 // 64MB SDRAM part
#define SDRAM_SIZE 0x04000000
#define SDRAM_ROW_SIZE 2048 // in bytes (10 bits col addr, 2 bytes per)
#define SDRAM_NUM_ROWS_PER_BANK (8192) // 13-bit row address
#define SDRAM_NUM_BANKS 4
#endif

#ifdef  MT48LC16M16A2 // 32MB SDRAM part
#define SDRAM_SIZE 0x02000000
#define SDRAM_ROW_SIZE 1024 // in bytes (9 bits col addr, 2 bytes per)
#define SDRAM_NUM_ROWS_PER_BANK (8192) // 13-bit row address
#define SDRAM_NUM_BANKS 4
#endif

#ifdef MT48LC4M16A2 // 8MB SDRAM part
#define SDRAM_SIZE 0x800000
#define SDRAM_ROW_SIZE 512 // in bytes (8 bits col addr, 2 bytes per)
#define SDRAM_NUM_ROWS_PER_BANK (4096) // 12-bit row address
#define SDRAM_NUM_BANKS 4
#endif


#endif


#define SDRAM_NUM_ROWS (SDRAM_NUM_ROWS_PER_BANK * SDRAM_NUM_BANKS)

#define START_ROW 128

int main()
{

  uart_init(DEFAULT_UART);


  printf("\n\tSDRAM rows test\n");

  printf("\n\tWriting\n");
  
  int i; // Skip first 64KB, code/stack resides there - TODO determine this from
  // stack linker variable!
  for(i=START_ROW;i<(SDRAM_NUM_ROWS);i++)
    {
      REG32((i*(SDRAM_ROW_SIZE))) = i;
      printf("\r\t0x%x", i);
    }

  printf("\n\tReading\n");

  int read_result = 0;

  for(i=START_ROW;i<(SDRAM_NUM_ROWS);i++)
    {
      printf("\r\t0x%x", i);
      read_result = REG32((i*(SDRAM_ROW_SIZE)));
      if (read_result != i)
	{
	  printf("\n\Error at 0x%x, read 0x%x, expected 0x%x\n",
		 (i*SDRAM_ROW_SIZE), read_result, i);
	  report(0xbaaaaaad);
	  report(i);
	  report(read_result);
	  exit(0xbaaaaaad);
	}
    }
  printf("\n\tTest OK.\n");
  exit(0x8000000d);  
}
