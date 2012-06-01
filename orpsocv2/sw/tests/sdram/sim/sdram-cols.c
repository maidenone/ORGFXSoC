/*
 * SDRAM column test
 *
 * Tests accessing beginning and middle of column. Should detect any mismatch
 * between SDRAM row size configuration.
 *
*/

#include "cpu-utils.h"
#include "board.h"
#include "sdram.h"

// Start some rows after the program/data

#define STACK_AT (128*1024)
#define START_ROW  ((STACK_AT/SDRAM_ROW_SIZE)+1)


// For a short test, set the following to 1
#define SHORT_TEST 0

#if SHORT_TEST==1
# define SDRAM_NUM_ROWS (START_ROW + 512)
#else
# define SDRAM_NUM_ROWS (SDRAM_NUM_ROWS_PER_BANK * SDRAM_NUM_BANKS)
#endif

#define SDRAM_ROW_BEGINNING 0
#define SDRAM_ROW_MIDDLE (SDRAM_ROW_SIZE/2)

int main()
{
  
  int i; // Skip first 64KB, code/stack resides there
  for(i=START_ROW;i<(SDRAM_NUM_ROWS);i++)
    {
      REG32((i*(SDRAM_ROW_SIZE))+SDRAM_ROW_BEGINNING) = i;
      REG32((i*(SDRAM_ROW_SIZE))+SDRAM_ROW_MIDDLE) = ~i;
    }

  int read_result = 0;
  int read_result_inv = 0;

  for(i=START_ROW;i<(SDRAM_NUM_ROWS);i++)
    {
      read_result = REG32((i*(SDRAM_ROW_SIZE))+SDRAM_ROW_BEGINNING);
      read_result_inv = REG32((i*(SDRAM_ROW_SIZE))+SDRAM_ROW_MIDDLE);
      if ((read_result != i) || (read_result_inv != ~i))
	{
	  report(0xbaaaaaad);
	  report(i);
	  report(read_result);
	  exit(0xbaaaaaad);
	}
    }
  exit(0x8000000d);  
}
