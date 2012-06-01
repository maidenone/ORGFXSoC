/*************************************************************
* I2C functions for the Herveille i2c controller            *
*                                                           *
* Provides functions to read from and write to the I2C bus. *
* Master and slave mode are both supported                  *
*                                                           *
* Julius Baxter, julius@opencores.org                       *
*                                                           *
************************************************************/

#include "board.h"
#include "cpu-utils.h"
#include "i2c_master_slave.h"


// Ensure board.h defines I2C_MASTER_SLAVE_NUM_CORES and 
// I2C_MASTER_SLAVE_BASE_ADDRESSES_CSV which should be the base address values
// separated with commas
#ifdef I2C_MASTER_SLAVE_NUM_CORES

const int I2C_MASTER_SLAVE_BASE_ADR[I2C_MASTER_SLAVE_NUM_CORES] = { 
	I2C_MASTER_SLAVE_BASE_ADDRESSES_CSV };
#else

const int I2C_MASTER_SLAVE_BASE_ADR[1] = {-1};

#endif

inline unsigned char i2c_master_slave_read_reg(int core, unsigned char addr)
{
	return REG8((I2C_MASTER_SLAVE_BASE_ADR[core] + addr));
}

inline void i2c_master_slave_write_reg(int core, unsigned char addr,
				       unsigned char data)
{
	REG8((I2C_MASTER_SLAVE_BASE_ADR[core] + addr)) = data;
}

int i2c_master_slave_wait_for_busy(int core)
{
	while (1) {
		// Check for busy flag in i2c status reg
		if (!
		    (i2c_master_slave_read_reg(core, I2C_MASTER_SLAVE_SR) &
		     I2C_MASTER_SLAVE_SR_BUSY))
			return 0;
	}
}

int i2c_master_slave_wait_for_transfer(int core)
{
	volatile unsigned char status;
	// Wait for ongoing transmission to finish
	while (1) {
		status = i2c_master_slave_read_reg(core, I2C_MASTER_SLAVE_SR);
		// If arbitration lost
		if ((status & I2C_MASTER_SLAVE_SR_ARB_LOST) ==
		    I2C_MASTER_SLAVE_SR_ARB_LOST)
			return 2;
		// If TIP bit = o , stop waiting
		else if (!(status & I2C_MASTER_SLAVE_SR_TRANSFER_IN_PRG))
			return 0;
	}
}

/***********************************************************
* i2c_master_slave_init_core                               *
*                                                          *
* Setup i2c core:                                          *
* Write prescaler register with parmeter passed, enable    *
* core in control register, optionally enable interrupts   *
************************************************************/
int i2c_master_slave_init_core(int core, unsigned short prescaler,
			       int interrupt_enable)
{

	// Setup I2C prescaler,
	i2c_master_slave_write_reg(core, I2C_MASTER_SLAVE_PRERlo,
				   prescaler & 0xff);
	i2c_master_slave_write_reg(core, I2C_MASTER_SLAVE_PRERhi,
				   (prescaler >> 8) & 0xff);

	// Enable I2C controller and optionally interrupts
	if (interrupt_enable)
		i2c_master_slave_write_reg(core, I2C_MASTER_SLAVE_CTR,
					   I2C_MASTER_SLAVE_CTR_CORE_ENABLE |
					   I2C_MASTER_SLAVE_CTR_INTR_ENABLE);
	else
		i2c_master_slave_write_reg(core, I2C_MASTER_SLAVE_CTR,
					   I2C_MASTER_SLAVE_CTR_CORE_ENABLE);

	return 0;

}

/***********************************************************
* i2c_master_slave_deact_core                              *
*                                                          *
* Deactivate i2c core:                                     *
* Clear core enable and interrupt enable bits              *
************************************************************/
int i2c_master_slave_deact_core(int core)
{

  
  i2c_master_slave_write_reg(core, I2C_MASTER_SLAVE_CTR,
			     i2c_master_slave_read_reg(core,
						       I2C_MASTER_SLAVE_CTR) & 
			     ~(I2C_MASTER_SLAVE_CTR_CORE_ENABLE |
			       I2C_MASTER_SLAVE_CTR_INTR_ENABLE));


	return 0;

}

/***********************************************************
* i2c_master_slave_init_as_slave                           *
*                                                          *
* Setup i2c core to service slave accesses                 *
* OR in slave enable bit to control register               *
* Set slave address                                        *
************************************************************/
int i2c_master_slave_init_as_slave(int core, char addr)
{

	// Set slave enable bit
	i2c_master_slave_write_reg(core, I2C_MASTER_SLAVE_CTR,
				   i2c_master_slave_read_reg(core,
							     I2C_MASTER_SLAVE_CTR)
				   | I2C_MASTER_SLAVE_CTR_SLAVE_ENABLE);
	// Set slave address
	i2c_master_slave_write_reg(core, I2C_MASTER_SLAVE_SLADR, addr);

	return 0;

}

/***********************************************************
* i2c_master_slave_deact_as_slave                          *
*                                                          *
* Disable slave mode for this I2C core                     *
* Deassert slave eanble bit in control register            *
************************************************************/
int i2c_master_slave_deact_as_slave(int core)
{
	// Clear slave enable bit
	i2c_master_slave_write_reg(core, I2C_MASTER_SLAVE_CTR,
				   i2c_master_slave_read_reg(core,
							     I2C_MASTER_SLAVE_CTR)
				   & ~I2C_MASTER_SLAVE_CTR_SLAVE_ENABLE);

	return 0;
}

/***********************************************************
* i2c_master_slave_master_start                            *
*                                                          *
* Get the i2c bus.                                         *
************************************************************/
int i2c_master_slave_master_start(int core, unsigned char addr, int read)
{

	i2c_master_slave_wait_for_busy(core);

	// Set address in transfer register
	i2c_master_slave_write_reg(core, I2C_MASTER_SLAVE_TXR,
				   (addr << 1) | read);

	// Start and write the address
	i2c_master_slave_write_reg(core, I2C_MASTER_SLAVE_CR,
				   I2C_MASTER_SLAVE_CR_START |
				   I2C_MASTER_SLAVE_CR_WRITE);

	i2c_master_slave_wait_for_transfer(core);

	return 0;
}

/***********************************************************
* i2c_master_slave_master_write                            *
*                                                          *
* Send 1 byte of data					   *
************************************************************/
int i2c_master_slave_master_write(int core, unsigned char data,
				  int check_prev_ack, int stop)
{	
	if (i2c_master_slave_wait_for_transfer(core))
		return 1;

	// present data
	i2c_master_slave_write_reg(core, I2C_MASTER_SLAVE_TXR, data);

	if (!stop)
		// set command (write)
		i2c_master_slave_write_reg(core, I2C_MASTER_SLAVE_CR,
					   I2C_MASTER_SLAVE_CR_WRITE);
	else
		// set command (write) and stop
		i2c_master_slave_write_reg(core, I2C_MASTER_SLAVE_CR,
					   I2C_MASTER_SLAVE_CR_WRITE |
					   I2C_MASTER_SLAVE_CR_STOP);

	return i2c_master_slave_wait_for_transfer(core);

}

/***********************************************************
* i2c_master_slave_master_stop                             *
*                                                          *
* Send stop condition					   *
************************************************************/
int i2c_master_slave_master_stop(int core)
{
	unsigned char status;
	unsigned char ready = 0;

	// Make I2C controller wait at end of finished byte
	if (i2c_master_slave_wait_for_transfer(core))
		return 1;

	// Send stop condition
	i2c_master_slave_write_reg(core, I2C_MASTER_SLAVE_CR,
				   I2C_MASTER_SLAVE_CR_STOP);

	return 0;
}

/***********************************************************
* i2c_master_slave_master_read                             *
*                                                          *
* Read 1 byte of data    				   *
************************************************************/
int i2c_master_slave_master_read(int core, int check_prev_ack,
				 int stop, char *data)
{

	// Make I2C controller wait at end of finished byte
	if (i2c_master_slave_wait_for_transfer(core))
		return 1;

	if (stop)
		i2c_master_slave_write_reg(core, I2C_MASTER_SLAVE_CR,
					   I2C_MASTER_SLAVE_CR_READ |
					   // Final read, so send a NAK to slave
					   I2C_MASTER_SLAVE_CR_ACK |
					   I2C_MASTER_SLAVE_CR_STOP);
	else
		i2c_master_slave_write_reg(core, I2C_MASTER_SLAVE_CR,
					   I2C_MASTER_SLAVE_CR_READ);

	if (i2c_master_slave_wait_for_transfer(core))
		return 1;

	*data = i2c_master_slave_read_reg(core, I2C_MASTER_SLAVE_RXR);

	return i2c_master_slave_wait_for_transfer(core);
}

/***********************************************************
* i2c_master_slave_ack_interrupt                           *
*                                                          *
* Acknowledge interrupt has been serviced		   *
************************************************************/
int i2c_master_slave_ack_interrupt(int core)
{

	i2c_master_slave_write_reg(core, I2C_MASTER_SLAVE_CR,
				   I2C_MASTER_SLAVE_CR_IACK);

	return 0;
}
