#include "board.h"
#include "cpu-utils.h"
#include "cfi_ctrl.h"
#include "uart.h"
#include "printf.h"
#include "orpsoc-defines.h"


int main(void)
{
  uart_init(DEFAULT_UART);

  printf("cfi_ctrl-simple test");

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

  /* test for number of blocks... */
  for(j=0;j<NUM_PAGES;j++)
    {
      page_base = j*(128*1024);
      
      printf("Status before clear: %02x\n",cfi_ctrl_get_status()&0xff);
      cfi_ctrl_clear_status();
      printf("Status after clear: %02x\n",cfi_ctrl_get_status()&0xff);

      /* Erase block  */
      if (cfi_ctrl_erase_block(page_base))
	{
	  //exit(0xbaaaaaad);
	  printf("Error erasing block at %08x\n",page_base);
	  printf("Aborting test\n");
	  exit(1);
	}

      cfi_ctrl_clear_status();
      
      for (i=0;i<TEST_LENGTH;i++)
	{
	  tmp = (unsigned short)rand();
	  //report(tmp&0xffff);
	  printf("writing data: %04x\n", tmp&0xffff);
	  cfi_ctrl_write_short(tmp,page_base+i*2);
	  check_data[i] = tmp;
      
	}

      /* Read back the data and check it */
      cfi_ctrl_enable_data_read();
      for (i=0;i<TEST_LENGTH;i++)
	{
	  tmp = REG16(CFI_CTRL_BASE + page_base + (i*2));
	  if (tmp != check_data[i])
	    //exit(0xbaaaaaad);
	    printf("read data was not written data: %04x != %04x\n", tmp&0xffff,
		   check_data[i]);
	}
    }

  report(0x8000000d);
  exit(0);
}
