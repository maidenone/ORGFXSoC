/*
  i2c loopback test

  Tests a i2c master/slave read/writeback.

  Interrupt handler for slave merely inverts the data we received and puts 
  it back onto the slave transmit register.

  Master writes and then reads back something from the same slave. 
  Successfull if the byte read back is inverted version of what was written.

  Currently hardcoded for 2 I2C devices.

  Julius Baxter, julius@opencores.org

*/
 

#include "board.h"
#include "cpu-utils.h"
#include "orpsoc-defines.h"
#include "printf.h"

#include "i2c_master_slave.h"

const int i2c_base_adr[4] = {I2C_0_BASE, I2C_1_BASE, I2C_2_BASE, I2C_3_BASE};

#define I2C_MASTER_SLAVE_PRESCALER 0x0010

#define NUM_I2C_MASTER_SLAVE_CORES 2

void i2c_master_slave_0_int_handler(void);
void i2c_master_slave_1_int_handler(void);

void i2c_master_slave_0_int_handler(void)
{
  static char regdata;

  char status = REG8(i2c_base_adr[0] + I2C_MASTER_SLAVE_SR);
  //printf("i2c core 0 interrupt - status: %x\n",status);
  if (status & I2C_MASTER_SLAVE_SR_SLAVE_DATA_REQ) // Read req. from master
    {
      // Put data into TX reg, inverted
      REG8(i2c_base_adr[0] + I2C_MASTER_SLAVE_TXR) = (~regdata);
      // set command (slave continue), and acknowledge interrupt
      REG8(i2c_base_adr[0] + I2C_MASTER_SLAVE_CR) = I2C_MASTER_SLAVE_CR_SL_CONT | 
	I2C_MASTER_SLAVE_CR_IACK;
    }
  else if ((status & I2C_MASTER_SLAVE_SR_SLAVE_DATA_AVAIL)) //Write req.
    {
      // Store the received value
      regdata =  REG8(i2c_base_adr[0] + I2C_MASTER_SLAVE_RXR);
      // set command (slave continue), and acknowledge interrupt
      REG8(i2c_base_adr[0] + I2C_MASTER_SLAVE_CR) = I2C_MASTER_SLAVE_CR_SL_CONT | 
	I2C_MASTER_SLAVE_CR_IACK;  
    }
  else
    {
      // Interrupt due to master receiving an OK for a transaction.
      // Just ACK and proceed for now.
      //printf("i2c 0 master RX ack\n");
      i2c_master_slave_ack_interrupt(0);
    }
  return;
}


void i2c_master_slave_1_int_handler(void)
{
  static char regdata;
  char status = REG8(i2c_base_adr[1] + I2C_MASTER_SLAVE_SR);
  //printf("i2c core 1 interrupt - status: %x\n",status);
  if (status & I2C_MASTER_SLAVE_SR_SLAVE_DATA_REQ) // Read req. from master
    {
      // Put data into TX reg, inverted
      REG8(i2c_base_adr[1] + I2C_MASTER_SLAVE_TXR) = (~regdata);
      // set command (slave continue), and acknowledge interrupt
      REG8(i2c_base_adr[1] + I2C_MASTER_SLAVE_CR) = I2C_MASTER_SLAVE_CR_SL_CONT | 
	I2C_MASTER_SLAVE_CR_IACK;
    }
  else if ((status & I2C_MASTER_SLAVE_SR_SLAVE_DATA_AVAIL)) //Write req.
    {
      // Store the received value
      regdata =  REG8(i2c_base_adr[1] + I2C_MASTER_SLAVE_RXR);
      // set command (slave continue), and acknowledge interrupt
      REG8(i2c_base_adr[1] + I2C_MASTER_SLAVE_CR) = I2C_MASTER_SLAVE_CR_SL_CONT | 
	I2C_MASTER_SLAVE_CR_IACK;  
    }
  else
    {
      // Interrupt due to master receiving an OK for a transaction.
      // Just ACK and proceed for now.
      // printf("i2c 1 master RX ack\n");
      i2c_master_slave_ack_interrupt(1);
    }

  return;
}

int main()
{
  
  // Select which core should be master
  char i2c_master_core;
  char i2c_slave_core;
  
  // Slave addresses for each i2c core
  char i2c_slave_core_addresses[4] = {0x44, 0x45, 0x46, 0x47};
  char slave_reg_addr = 0;
  char test_write;
  char test_read;

  // Initialise software interrupt handler 
  int_init();
  
  printf("i2c loopback test.\n");

  printf("Init...\n");
  /* Install i2c core 0 interrupt handler */
  int_add(I2C_0_IRQ, i2c_master_slave_0_int_handler, 0);

  /* Install i2c core 1 interrupt handler */
  int_add(I2C_1_IRQ, i2c_master_slave_1_int_handler, 0);

  /* Install i2c core 2 interrupt handler */
  //int_add(I2C_2_IRQ, i2c_master_slave_2_int_handler, 0);

  /* Install i2c core 3 interrupt handler */
  //int_add(I2C_3_IRQ, i2c_master_slave_3_int_handler, 0);

  // Enable interrupts
  cpu_enable_user_interrupts();

  /* Set i2c core slave addresses - deact immediately again*/

  for(i2c_slave_core=0;i2c_slave_core<NUM_I2C_MASTER_SLAVE_CORES;
      i2c_slave_core++)
    {
      i2c_master_slave_init_core(i2c_slave_core, I2C_MASTER_SLAVE_PRESCALER, 
				 1);

      i2c_master_slave_init_as_slave(i2c_slave_core, 
				     i2c_slave_core_addresses[i2c_slave_core]);
    }

  for(i2c_master_core=0;i2c_master_core<NUM_I2C_MASTER_SLAVE_CORES;
      i2c_master_core++)
    {
      report(0x10000000);
      report(i2c_master_core);
      
      for(i2c_slave_core=0;i2c_slave_core<NUM_I2C_MASTER_SLAVE_CORES;
	  i2c_slave_core++)
	{
	  report(0x20000000);
	  report(i2c_slave_core);
	  // Master can't test own slave functionality, so skip
	  if (i2c_slave_core == i2c_master_core)
	    continue;

	  printf("i2c master %d: Testing write to slave %d ",i2c_master_core,
		 i2c_slave_core);

	  test_write = rand() & 0xff;

	  printf("writing: 0x%.2x\n",test_write & 0xff);
	  
	  // Start bus write to i2c slave
	  i2c_master_slave_master_start(i2c_master_core,
				       i2c_slave_core_addresses[i2c_slave_core],
					0);
	  // write some data (slave reg address)
	  i2c_master_slave_master_write(i2c_master_core, slave_reg_addr, 0, 0);

	  // write the data to slave's register, plus indicate stop
	  i2c_master_slave_master_write(i2c_master_core, test_write, 0, 1);

	  /* Send read access to slave */
	  printf("i2c master %d: Testing read from i2c slave core %d - ",
		 i2c_master_core, i2c_slave_core);

	  // Start bus read from i2c slave
	  i2c_master_slave_master_start(i2c_master_core,
				       i2c_slave_core_addresses[i2c_slave_core],
					1);
	  // Read + stop
	  i2c_master_slave_master_read(i2c_master_core, 0, 1, &test_read);
	  
	  report((0xff & test_read));
	  
	  printf("recv: 0x%.2x", test_read & 0xff);
	  
	  if (test_read != ~test_write)
	    {
	      printf(" - FAIL ( expected 0x%.2x)\n",~test_write & 0xff);
	      exit(0xbaaaaaad);
	    }
	  else
	    printf(" - OK\n");
	}
    }

  printf("Test completed\n");
  exit(0x8000000d);

}

