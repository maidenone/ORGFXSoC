/*
 * SDRAM row test
 *
 * Tests accessing every row
 *
*/

#include "cpu-utils.h"
#include "board.h"
#include "sdram.h"

#define SDRAM_NUM_ROWS (SDRAM_NUM_ROWS_PER_BANK * SDRAM_NUM_BANKS)

#define STACK_AT (128*1024)
#define START_ROW  ((STACK_AT/SDRAM_ROW_SIZE)+1)

int main()
{
  
  int i; // Skip first 64KB, code/stack resides there
  for(i=START_ROW;i<(SDRAM_NUM_ROWS);i++)
    REG32((i*(SDRAM_ROW_SIZE))) = i;

  int read_result = 0;

  for(i=START_ROW;i<(SDRAM_NUM_ROWS);i++)
    {
      read_result = REG32((i*(SDRAM_ROW_SIZE)));
      if (read_result != i)
	{
	  report(0xbaaaaaad);
	  report(i);
	  report(read_result);
	  exit(0xbaaaaaad);
	}
    }
  exit(0x8000000d);  
}
