/*
  SPI controller simple test

  Do some reads from the SPI slaves. Select a different slave each loop
  and does a read and reports the value.

  Nothing much actually gets tested here.

  Ensure the slave selects for the spi are enabled in orpsoc-defines.v

*/


#include "cpu-utils.h"
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

int main()
{
  int spi_master = FIRST_SPI_CORE;
  int spi_slave = 2;
  int i;
  
  // Init the masters
  for (spi_master = FIRST_SPI_CORE; 
       spi_master < FIRST_SPI_CORE+ NUM_SPI_CORES;
       spi_master++)
    {
      spi_core_clock_setup(spi_master, 0, 0, 2, 0);
      spi_core_enable(spi_master);
    }

  
  // Play with the slaves
  for(i=0;i<64;i++)
    {
      for (spi_master = FIRST_SPI_CORE; 
	   spi_master < FIRST_SPI_CORE+ NUM_SPI_CORES;
	   spi_master++)
	{
	  spi_slave = i % 3;
	  spi_slave = (1 << spi_slave);
	  // Select slave on SPI bus
	  spi_core_slave_select(spi_master, spi_slave);
	  // Do a SPI bus transaction - we're only interested in the read data
	  // coming back
	  while (!spi_core_write_avail(spi_master));      
	  spi_core_write_data(spi_master, (i&0xff));
	  while (!spi_core_data_avail(spi_master));
	  report(spi_core_read_data(spi_master));
	  // Deselect slaves
	  spi_core_slave_select(spi_master, 0);
	}
    }
  
  exit(0x8000000d);
  
}
