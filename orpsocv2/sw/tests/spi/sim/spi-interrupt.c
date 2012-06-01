#include "board.h"
#include "spr-defs.h"
#include "cpu-utils.h"
#include "simple-spi.h"
#include "int.h"

#include "simple-spi.h"


#include "orpsoc-defines.h"

// Detect which of the SPI cores are enabled, tailor the test for that
#ifndef SPI1
# ifndef SPI2
#  error
#  error No SPI cores to test with! Please enable SPI1 and/or SPI2
#  error
# else
#  define NUM_SPI_CORES 1
#  define FIRST_SPI_CORE 2
# endif
#else
# ifdef SPI2
#  define NUM_SPI_CORES 2
#  define FIRST_SPI_CORE 1
# else
#  define NUM_SPI_CORES 1
#  define FIRST_SPI_CORE 1
# endif
#endif


struct spi_int_data{
  unsigned long data;
  char retrieved;
};

volatile struct spi_int_data spi1_int_data;

void spi1_int_handler(void);

void spi1_int_handler(void)
{
  
  // Should have recieved 4 bytes, let's pull them off and put them into
  // the data struct
  char* spi1_data = (char*) &spi1_int_data.data;
  int i;
  for (i=0;i<4;i++)
    {
      while (!spi_core_data_avail(1));
      spi1_data[i] = spi_core_read_data(1);
    }
  
  spi1_int_data.retrieved = 1;

  // Clear the interrupt flag
  spi_core_interrupt_flag_clear(1);
  
  return;
}

volatile struct spi_int_data spi2_int_data;

void spi2_int_handler(void);

void spi2_int_handler(void)
{
  
  // Should have recieved 4 bytes, let's pull them off and put them into
  // the data struct
  char* spi2_data = (char*) &spi2_int_data.data;
  int i;
  for (i=0;i<4;i++)
    {
      while (!spi_core_data_avail(2));
      spi2_data[i] = spi_core_read_data(2);
    }
  
  spi2_int_data.retrieved = 1;

  // Clear the interrupt flag
  spi_core_interrupt_flag_clear(2);
  
  return;
}


int main()
{
  int spi_master;
  int spi_slave = 2;
  int i,j;

  spi1_int_data.retrieved = 0;
  spi2_int_data.retrieved = 0;

  int_init();

  /* Install SPI core 1 interrupt handler */
  int_add(SPI1_IRQ, spi1_int_handler, 0);

  /* Install SPI core 2 interrupt handler */
  int_add(SPI2_IRQ, spi2_int_handler, 0);

  /* Enable interrupts in supervisor register */
  mtspr (SPR_SR, mfspr (SPR_SR) | SPR_SR_IEE);

  
  for (spi_master = FIRST_SPI_CORE; 
       spi_master < FIRST_SPI_CORE+ NUM_SPI_CORES;
       spi_master++)
    {
      
      // Init master -- disable it first
      spi_core_disable(spi_master);
      // polarity, phase, dividers :
      spi_core_clock_setup(spi_master, 0, 0, 0, 0);
      // Set number of transfers per interrupt to 4
      spi_core_set_int_count(spi_master, SIMPLESPI_SPER_ICNT_FOUR);

      // Enable interrupts on the core
      spi_core_interrupt_enable(spi_master);
    
      // Now enable core after configuration, before using it
      spi_core_enable(spi_master);
    }

  // Play with the slaves
  for(i=0;i<16;i++)
    {
      for (spi_master = FIRST_SPI_CORE; 
	   spi_master < FIRST_SPI_CORE+ NUM_SPI_CORES;
	   spi_master++)
	{
	  
	  spi_slave = i % 3;
	  spi_slave = (1 << spi_slave);
	  for(j=0;j<4;j++)
	    {
	      // Select a slave on SPI bus
	      spi_core_slave_select(spi_master, spi_slave);
	      // Do a SPI bus transaction - we're only reading
	      // coming back
	      while (!spi_core_write_avail(spi_master));      
	      spi_core_write_data(spi_master, (i&0xff));
	      // Deselect all masters
	      spi_core_slave_select(spi_master, 0);
	    }
	  // Wait for the interrupt to retrieve the data and signal this
	 
	  if (spi_master == 1){
	    while (!spi1_int_data.retrieved);
	    // Now report it
	    report(spi1_int_data.data);
	    // And reset the retrieved bit
	    spi1_int_data.retrieved = 0;
	  }
	  if (spi_master == 2){
	    while (!spi2_int_data.retrieved);
	    // Now report it
	    report(spi2_int_data.data);
	    // And reset the retrieved bit
	    spi2_int_data.retrieved = 0;
	  }
	}
    }
  
  exit(0x8000000d);
  
}
