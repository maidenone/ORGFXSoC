// Little program to dump the contents of the SPI flash memory it's connected 
// to on the board

#include "cpu-utils.h"

#include "board.h"
#include "uart.h"
#include "simple-spi.h"
#include "printf.h"


int spi_master;
char slave;


void
spi_write_ignore_read(int core, char dat)
{
  spi_core_write_data(core,dat);
  while (!(spi_core_data_avail(core))); // Wait for the transaction (should 
                                        // generate a byte)
  spi_core_read_data(core);
}

char
spi_read_ignore_write(int core)
{
  spi_core_write_data(core, 0x00);
  while (!(spi_core_data_avail(core))); // Wait for the transaction (should 
                                        // generate a byte)
  return spi_core_read_data(core);
}


unsigned long
spi_read_id(int core, char slave_sel)
{
  unsigned long rdid;
  char* rdid_ptr = (char*) &rdid;
  int i;
  spi_core_slave_select(core, slave_sel); // Select slave
  rdid_ptr[3] = 0;
  // Send the RDID command
  spi_write_ignore_read(core,0x9f); // 0x9f is READ ID command
  // Now we read the next 3 bytes
  for(i=0;i<3;i++)
    {
      rdid_ptr[i] = spi_read_ignore_write(core);
    }
  spi_core_slave_select(core, 0); // Deselect slave
  return rdid;
}

// Read status regsiter
char
spi_read_sr(int core, char slave_sel)
{
  char rdsr;
  spi_core_slave_select(core, slave_sel); // Select slave
  // Send the RDSR command
  spi_write_ignore_read(core,0x05); // 0x05 is READ status register command
  rdsr = spi_read_ignore_write(core);
  spi_core_slave_select(core, 0); // Deselect slave  
  return rdsr;
}


void
spi_read_block(int core, char slave_sel, unsigned int addr, int num_bytes, 
	       char* buf)
{
  int i;
  spi_core_slave_select(core, slave_sel); // Select slave
  spi_write_ignore_read(core, 0x3); // READ command
  spi_write_ignore_read(core,((addr >> 16) & 0xff)); // addres high byte
  spi_write_ignore_read(core,((addr >> 8) & 0xff)); // addres middle byte
  spi_write_ignore_read(core,((addr >> 0) & 0xff)); // addres low byte
  for(i=0;i<num_bytes;i++)
    buf[i] = spi_read_ignore_write(core);
  
  spi_core_slave_select(core, 0); // Deselect slave  
}


#define printhelp() printf("\nUsage: \n\t[d]ump\t\tdump 256 bytes of data to screen from flash\n\t[+/-]\t\tIncrease/decrease dump address by 256 bytes\n\t[</>]\t\tIncrease/decrease dump address by 4096 bytes\n\t[s]tatus\t\tprint status of SPI flash\n\n")

void 
print_spi_status(void)
{

  printf("SPI core: %d\n",spi_master);
  printf("SPI slave select: 0x%x\n",slave&0xff);

  printf("SPI slave info:\n");
  printf("\tID:\t%x\n", spi_read_id(spi_master, slave));
  printf("\tSR:\t%x\n", spi_read_sr(spi_master, slave));  
  printf("\n");

}



int 
main()
{

  uart_init(0); // init the UART before we can printf
  
  volatile char c;
  int i,j;
  spi_master = 0;
  slave = 1;

  spi_core_slave_select(spi_master, 0); // Deselect slaves

  // Clear the read FIFO
  while (spi_core_data_avail(spi_master))
    c = spi_core_read_data(spi_master);

  // SPI core 0, should already be configured to read out data
  // when we reset.

  printf("\n\n\tSPI dumping app\n\n");
  
  unsigned long dump_addr = 0;
  char read_buf[256];
  int dump_amount = 0x100;

  while(1){
    printf("[d,+,-,s,h] > ");
    c = uart_getc(DEFAULT_UART);  

    if ((c != '+') && (c != '-') && (c != '<') && (c != '>') )
      {
	printf("%c ",c);
	printf("\n");
      }
    
    if (c == 'h')
      printhelp();
    else if (c == 's')
      print_spi_status();
    else if (c == '+')
      {
	dump_addr += dump_amount;
	printf("dump_addr= 0x%x\r", dump_addr);
      }
    else if (c == '-')
      {
	dump_addr -= dump_amount;
	printf("dump_addr= 0x%x\r", dump_addr);
      }
    else if (c == '>')
      {
	dump_addr += dump_amount*16;
	printf("dump_addr= 0x%x\r", dump_addr);
      }
    else if (c == '<')
      {
	dump_addr -= dump_amount*16;
	printf("dump_addr= 0x%x\r", dump_addr);
      }
    else if (c == 'd')
      {
	spi_read_block(spi_master, 1, dump_addr, dump_amount, read_buf);
	// Print it out, 32 bytes across each time
	for(i=0;i<(dump_amount/32);i++)
	  {
	    printf("%.5x: ", (i*32)+dump_addr);
	    for(j=0;j<32;j++)
	      printf("%.2x", read_buf[(i*32)+j] & 0xff);
	    printf("\n");
	  }
	dump_addr += dump_amount;
	       

      }
    

  }
  
  return 0;

}
