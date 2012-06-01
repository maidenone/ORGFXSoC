/*************************************************************
 * I2C functions for Herveille i2c master_slave core         *
 *                                                           *
 * Provides functions to read from and write to the I2C bus. *
 * Master and slave mode are both supported                  *
 *                                                           *
 *                                                           *
 ************************************************************/

#ifndef _I2C_MASTER_SLAVE_H_
#define _I2C_MASTER_SLAVE_H_

//Memory mapping adresses

#define I2C_MASTER_SLAVE_PRERlo 0x0	// Clock prescaler register
#define I2C_MASTER_SLAVE_PRERhi 0x1	// Clock prescaler register
#define I2C_MASTER_SLAVE_CTR    0x2	// Control register
#define I2C_MASTER_SLAVE_TXR    0x3	// Transmit register
#define I2C_MASTER_SLAVE_RXR    0x3	// Recive register
#define I2C_MASTER_SLAVE_CR     0x4	// Control register
#define I2C_MASTER_SLAVE_SR     0x4	// Status register
#define I2C_MASTER_SLAVE_SLADR  0x7	// Slave address register

#define I2C_MASTER_SLAVE_CTR_CORE_ENABLE 0x80
#define I2C_MASTER_SLAVE_CTR_INTR_ENABLE 0x40
#define I2C_MASTER_SLAVE_CTR_SLAVE_ENABLE 0x20

#define I2C_MASTER_SLAVE_CR_START        0x80
#define I2C_MASTER_SLAVE_CR_STOP         0x40
#define I2C_MASTER_SLAVE_CR_READ         0x20
#define I2C_MASTER_SLAVE_CR_WRITE        0x10
#define I2C_MASTER_SLAVE_CR_ACK          0x08
#define I2C_MASTER_SLAVE_CR_SL_CONT      0x02
#define I2C_MASTER_SLAVE_CR_IACK         0x01

#define I2C_MASTER_SLAVE_SR_RXACK            0x80
#define I2C_MASTER_SLAVE_SR_BUSY             0x40
#define I2C_MASTER_SLAVE_SR_ARB_LOST         0x20
#define I2C_MASTER_SLAVE_SR_SLAVE_MODE       0x10
#define I2C_MASTER_SLAVE_SR_SLAVE_DATA_AVAIL 0x08
#define I2C_MASTER_SLAVE_SR_SLAVE_DATA_REQ   0x04
#define I2C_MASTER_SLAVE_SR_TRANSFER_IN_PRG  0x02
#define I2C_MASTER_SLAVE_SR_IRQ_FLAG         0x01

int i2c_master_slave_init_core(int core, unsigned short prescaler,
			       int interrupt_enable);
int i2c_master_slave_deact_core(int core);
int i2c_master_slave_init_as_slave(int core, char addr);
int i2c_master_slave_deact_as_slave(int core);
int i2c_master_slave_master_start(int core, unsigned char addr, int read);
int i2c_master_slave_master_write(int core, unsigned char data,
				  int check_prev_ack, int stop);
int i2c_master_slave_master_stop(int core);
int i2c_master_slave_master_read(int core, int check_prev_ack, int stop,
				 char *data);
int i2c_master_slave_ack_interrupt(int core);
#endif
