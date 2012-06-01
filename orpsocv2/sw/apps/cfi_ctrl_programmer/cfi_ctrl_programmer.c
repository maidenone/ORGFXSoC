// Program that will erase and program a flash via cfi_ctrl module

#include "cpu-utils.h"
#include "board.h"
#include "uart.h"
#include "printf.h"
#include "cfi_ctrl.h"

// TODO: detect this manually - it is CFI after all!
#define BLOCK_SIZE_BYTES (128*1024)

//#define VERBOSE_PROGRAMMING
#define VERBOSE_VERIFY_ERRORS
//#define VERIFY_WHEN_WRITE

unsigned long programming_file_start;
unsigned long programming_file_end;
unsigned long programming_file_length;

extern unsigned long userprogram_data, _userprogram_data;
extern unsigned long end_userprogram_data, _end_userprogram_data;

// Globals
int timeout_counter;
volatile char dump;
int programming_base_offset;

#define RESET_PC 0xf0000100

int
console_get_num(void)
{
  char c = 0x30;
  int num_nums = 0;
  int num_nums_total;
  char nums[16]; // up to 16 decimal digits long
  int retval = 0;
  int decimal_multiplier;
  int i;
	
  printf("Enter decimal value: ");
	
  while (c >= 0x30 && c < 0x40)
    {
      c = uart_getc(DEFAULT_UART);  
		
      if (c >= 0x30 && c < 0x40)
	{
	  printf("%d", c-0x30);
	  nums[num_nums] = c-0x30;
	  num_nums++;
	}
		
    }
  printf("\n");

  num_nums_total = num_nums;

  while(num_nums--)
    {
      decimal_multiplier = 1;
      for(i=1;i<num_nums_total - num_nums;i++)
	decimal_multiplier *= 10;
      //printf("%d * %d\n",decimal_multiplier,nums[num_nums]);
		
      retval += (decimal_multiplier * nums[num_nums]);
    }
  //printf("%d\n",retval);
  return retval;
}

void
console_browse_buffer(char* buf)
{
  char c = 0;
  int offset = 0;
  const int linesize = 16;
  int i;
  printf("Press space to scroll through buffer, q to return\n");
  printf("+/- alter address offset\n");
  cfi_ctrl_enable_data_read();
  while (1)
    {
      c = uart_getc(DEFAULT_UART);  

      if (c == 'q')
	return;
      else if (c == 'r')
	offset=0;
      else if (c == '+')
	{
	  offset+=linesize;
	  printf("%04x:\r",offset);
	}
      else if (c == '-')
	{
	  if (offset >=linesize)
	    offset-=linesize;

	  printf("%04x:\r",offset);
	}
      else if (c == 0x20) // space, print al ine
	{
	  printf("%04x:",offset);
	  // print another line of the buffer
	  for (i=0;i<linesize;i++)
	    {
	      printf(" %02x", buf[offset+i]&0xff);
	    }
	  printf("\n");
			
	  offset += linesize;
			
	}
    }
	
}

void
delay(int n)
{
  volatile int i=0;
  while(i<n)
    i++;
}

void
timeout_reset(void)
{
  timeout_counter = 0;
}

int
timeout(void)
{
  timeout_counter++;
  if (timeout_counter == 20000)
    {
      printf("timeout\n");
      cfi_ctrl_reset_flash();

      return 1;
    }
  return 0;
}

void
print_cfi_status(void)
{
  char flashid[5];
  int i;
  printf("\tcfi_ctrl flash status:\n");

  printf("Device status byte: 0x%02x\n",cfi_ctrl_get_status()&0xff);
  printf("\n");
  printf("Programming file from 0x%x-0x%x, %d bytes\n",
	 programming_file_start, programming_file_end,
	 programming_file_length);
  //	printf("Embedded length: %d\n", (unsigned long) userprogram_data);
  printf("Page programming base: %d (set with 'o' command)\n",
	 programming_base_offset);
}

void
verify_image(int base_offset, char* data, int length_bytes)
{

  int base_block = base_offset / BLOCK_SIZE_BYTES;
  
  int num_blocks = (length_bytes/BLOCK_SIZE_BYTES) + 1;
  int i;
	
  int verify_error = 0;		

  printf("\tVerifying %d bytes of image from 0x%08x\n",length_bytes,
	 base_offset);

  unsigned int waddr;
  short * data_shorts;
  int short_counter;
  data_shorts = (short*) data;
  
  // Read the pages
  cfi_ctrl_enable_data_read();
  for(waddr=base_offset, short_counter=0;
      waddr<(base_offset + length_bytes); 
      waddr +=2, short_counter++)
    {
      /* Check what we wrote */
      short verify_data = REG16(CFI_CTRL_BASE+waddr);
      
      if (verify_data != data_shorts[short_counter])
	{
	  printf("\tERROR: word verify failed at 0x%08x.\n",waddr);
	  printf("\tERROR: Read 0x%04x instead of 0x%04x.\n",verify_data,
		 data_shorts[short_counter]);
	  verify_error ++;
	}
      
    }
	
  if (verify_error)
    printf("\tVerify complete - %d errors were detected\n",
	   verify_error);
  else
    printf("\tImage verified. Press esacpe to boot from 0x%08x.\n",RESET_PC);
}

void
program_image(int base_offset, char* data, int length_bytes)
{

  int base_block = base_offset / BLOCK_SIZE_BYTES;
  
  int num_blocks = (length_bytes/BLOCK_SIZE_BYTES) + 1;
  
  int i;

  unsigned int waddr;
  
  short * data_shorts;
  int short_counter;
	
  printf("\tErasing blocks (%d - %d)\n",base_block, 
	 base_block + num_blocks-1);
  
  // Erase the appropriate blocks
  for(i=base_block;i<base_block + num_blocks;i++)
    {
      printf("\tErasing block %d\n",i);
      cfi_ctrl_unlock_block(i*BLOCK_SIZE_BYTES);
      if (cfi_ctrl_erase_block(i*BLOCK_SIZE_BYTES))
	{
	  printf("\tErase failed, trying again\n");
	  i--; // Try erasing again
	  continue;
	}
    }
  printf("\tErase complete\n");
  
  printf("\n\tProgramming %d bytes\n", length_bytes);

  data_shorts = (short*) data;

  // Program the pages
  for(waddr=base_offset, short_counter=0;
      waddr<(base_offset + length_bytes); 
      waddr +=2, short_counter++)
    {
      if (cfi_ctrl_write_short(data_shorts[short_counter],waddr))
	{
	  printf("\tERROR: word program failed at 0x%08x\n",waddr);
	  return;
	}
      else
	{
#ifdef VERIFY_WHEN_WRITE
	  /* Check what we wrote */
	  cfi_ctrl_enable_data_read();
	  short verify_data = REG16(CFI_CTRL_BASE+waddr);
	  if (verify_data != data_shorts[short_counter])
	    {
	      printf("\tERROR: word verify failed at 0x%08x.\n",waddr);
	      printf("\tERROR: Read 0x%04x instead of 0x%04x.\n",verify_data,
		     data_shorts[short_counter]);
	      return;
	    }
#endif
	}
    }
	
  printf("\tProgramming %d bytes complete\n", length_bytes);
}

#define printhelp() printf("\nUsage: \n \
\t[p]rogram\t\terase req. blocks, write image to flash\n \
\t[o]ffset\t\tset page offset to write to\n \
\t[v]erify\t\tverify written program\n \
\t[s]tatus\t\tprint status of CFI flash\n \
\t[e]rase\t\t\terase block of CFI flash\n \
\t[r]ead\t\t\tread, browse page of CFI flash\n \
\t[i]nspect\t\tinspect page of image in RAM\n \
\t[R]eset\t\t\treset CFI flash controller\n \
\t[ESC]\t\t\treset by jumping to 0x%08x\n \
\n",RESET_PC)

int 
main()
{
  uart_init(0); // init the UART before we can printf
  
  volatile char c;
  int i,j;

  programming_base_offset = 0;
  
  programming_file_start = (unsigned long) &userprogram_data;
  programming_file_end = (unsigned long) &end_userprogram_data;
  programming_file_length = programming_file_end - programming_file_start;

  printf("\n\n\tcfi_ctrl flash programming app\n\n");
  printf("\ttype 'h' for help menu\n\n");

  while(1){
    printf(" > ");
    c = uart_getc(DEFAULT_UART);  
    printf("%c",c);
    printf("\n");
    
    if (c == 'h')
      printhelp();
    else if (c == 's')
      print_cfi_status();
    else if (c == 'c')
      cfi_ctrl_clear_status();
    else if (c == 'p')
      program_image(programming_base_offset, 
		    (char *) &userprogram_data, 
		    programming_file_length);
    else if (c == 'v')
      verify_image(programming_base_offset, 
		   (char *) &userprogram_data, 
		   programming_file_length);
    else if (c == 'o')
      {
	printf("Enter byte offset to program from.\n");
	programming_base_offset = console_get_num();
      }
    // Support/debug commands:
    else if (c == 'e')
      {
	printf("Erase a block.\n");
	i = console_get_num();
	// program a specific page
	cfi_ctrl_erase_block_no_wait(i*BLOCK_SIZE_BYTES);
      }
    else if (c == 'r')
      {
	printf("Inspect memory.\n");
	i = console_get_num();
	// read a page
	console_browse_buffer((char*)CFI_CTRL_BASE + i);
      }
    else if (c == 'i')
      {
	printf("Inspect image to program.\n");
	console_browse_buffer((char*) &userprogram_data);
      }
    else if (c == 'R')
      {
	printf("Reset command to controller\n");
	cfi_ctrl_reset_flash();
      }
    else if (c == 0x1b) // Esacpe key
      {
	// Reset
	void (*reset_function) (void) = 
	  (void *)RESET_PC;
	(*reset_function)();
      }

  }
  
  return 0;

}
