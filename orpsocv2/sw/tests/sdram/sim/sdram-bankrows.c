/*
 * SDRAM row/bank test
 *
 * Tests accessing different rows on different banks
 *
*/

#include "cpu-utils.h"
#include "board.h"
#include "sdram.h"

#define NUM_SDRAM_BANKS 4
#define SDRAM_BANK_SIZE (SDRAM_SIZE / NUM_SDRAM_BANKS)
#define SDRAM_BANK_START(bank) ((SDRAM_BANK_SIZE*bank) + SDRAM_BASE)
#define SDRAM_BANK_LAST_WORD(bank) ((SDRAM_BANK_START((bank+1)))-4)
#define SDRAM_BANK_MIDDLE_WORD(bank) ((SDRAM_BANK_START((bank)))+(SDRAM_BANK_SIZE/2))

int main()
{

  REG32(SDRAM_BANK_LAST_WORD(0))   = 0x00001111;
  REG32(SDRAM_BANK_MIDDLE_WORD(1)) = 0x22223333;
  REG32(SDRAM_BANK_LAST_WORD(1))   = 0x44445555;
  REG32(SDRAM_BANK_MIDDLE_WORD(2)) = 0x66667777;
  REG32(SDRAM_BANK_LAST_WORD(2))   = 0x88889999;
  REG32(SDRAM_BANK_MIDDLE_WORD(3)) = 0xaaaabbbb;
  REG32(SDRAM_BANK_LAST_WORD(3))   = 0xccccdddd;

  unsigned long read_result = 0;
  read_result += REG32(SDRAM_BANK_LAST_WORD(0));
  read_result += REG32(SDRAM_BANK_MIDDLE_WORD(1));
  read_result += REG32(SDRAM_BANK_LAST_WORD(1));
  read_result += REG32(SDRAM_BANK_MIDDLE_WORD(2));
  read_result += REG32(SDRAM_BANK_LAST_WORD(2));
  read_result += REG32(SDRAM_BANK_MIDDLE_WORD(3));
  read_result += REG32(SDRAM_BANK_LAST_WORD(3));
  report(read_result);
  // read_result should be 0xCCCD4441
  exit((read_result^0x4ccd444c)); /* should result in 8000000d */
}
