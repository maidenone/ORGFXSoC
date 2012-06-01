/*
 * SDRAM bank test
 *
 * Tests the extremeties of the SDRAM banks
 *
*/

#include "cpu-utils.h"
#include "board.h"
#include "sdram.h"

#define SDRAM_BANK_SIZE (SDRAM_SIZE / SDRAM_NUM_BANKS)
#define SDRAM_BANK_START(bank) ((SDRAM_BANK_SIZE*bank) + SDRAM_BASE)
#define SDRAM_BANK_LAST_WORD(bank) ((SDRAM_BANK_START((bank+1)))-4)

int main()
{
  report (SDRAM_SIZE);
  report (SDRAM_BANK_SIZE);
  report (SDRAM_BANK_START(0));
  report (SDRAM_BANK_LAST_WORD(0));
  report (SDRAM_BANK_START(1));
  report (SDRAM_BANK_LAST_WORD(1));
  report (SDRAM_BANK_START(2));
  report (SDRAM_BANK_LAST_WORD(2));
  report (SDRAM_BANK_START(3));
  report (SDRAM_BANK_LAST_WORD(3));

  REG32(SDRAM_BANK_LAST_WORD(0)) = 0x11111111;
  REG32(SDRAM_BANK_LAST_WORD(1)) = 0x22222222;
  REG32(SDRAM_BANK_LAST_WORD(2)) = 0x33333333;
  REG32(SDRAM_BANK_LAST_WORD(3)) = 0x44444444;


  unsigned long read_result = 0;
  read_result += REG32(SDRAM_BANK_LAST_WORD(0));
  read_result += REG32(SDRAM_BANK_LAST_WORD(1));
  read_result += REG32(SDRAM_BANK_LAST_WORD(2));
  read_result += REG32(SDRAM_BANK_LAST_WORD(3));

  exit((read_result-0x2aaaaa9d)); /* should result in 8000000d */
}
