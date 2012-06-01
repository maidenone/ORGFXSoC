#include "board.h"
#include "cpu-utils.h"
#include "cfi_ctrl.h"

#include "orpsoc-defines.h"


int main(void)
{
  /* Reset the flash */
  cfi_ctrl_reset_flash();

  /* wait for the controller to be done */
  while (cfi_ctrl_busy());

#define NUM_PAGES 8
#define TEST_LENGTH 64
  int i,j;
  unsigned int page_base;
  unsigned short check_data[TEST_LENGTH];
  unsigned short tmp;
  unsigned short* flash_data_short_ptr;

  /* test for number of blocks... */
  for(j=0;j<NUM_PAGES;j++)
    {
      page_base = j*(128*1024);
      
      cfi_ctrl_clear_status();

      /* Erase block  */
      if (cfi_ctrl_erase_block(page_base))
	exit(0xbaaaaaad);

      cfi_ctrl_clear_status();

      for (i=0;i<TEST_LENGTH;i++)
	{
	  tmp = (unsigned short)rand();
	  report(tmp&0xffff);
	  cfi_ctrl_write_short(tmp,page_base+i*2);
	  check_data[i] = tmp;
      
	}

      /* Read back as shorts and check it */
      cfi_ctrl_enable_data_read();
      for (i=0;i<TEST_LENGTH;i++)
	{
	  //tmp = REG16(CFI_CTRL_BASE + page_base + (i*2));
	  flash_data_short_ptr = (short*) (CFI_CTRL_BASE + page_base);
	  //if (tmp != check_data[i])
	  if (flash_data_short_ptr[i] != check_data[i])
	    exit(0xbaaaaaad);
	  
	}
      
      char * flash_data_char_ptr = (char*) CFI_CTRL_BASE + page_base;
      char * chardata = (char*) check_data;
      /* Read back as bytes and check it */
      cfi_ctrl_enable_data_read();
      for (i=0;i<TEST_LENGTH*2;i++)
	{
	  if (flash_data_char_ptr[i] != chardata[i])
	    exit(0xbaaaaaad);
	  
	}

      long * flash_data_long_ptr =  CFI_CTRL_BASE + page_base;
      long * longdata = (long*) check_data;
      /* Read back as longs and check it */
      cfi_ctrl_enable_data_read();
      for (i=0;i<TEST_LENGTH/2;i++)
	{
	  if (flash_data_long_ptr[i] != longdata[i])
	    exit(0xbaaaaaad);
	  
	}


    }

  report(0x8000000d);
  exit(0);
}
