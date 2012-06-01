/*
 * Defines and prototypes for Simple SPI module driver
 *
 * Julius Baxter, julius.baxter@orsoc.se
 *
 */

#ifndef _SIMPLE_SPI_H_
#define _SIMPLE_SPI_H_

#define SIMPLESPI_SPCR 0x00 // Control register
#define SIMPLESPI_SPSR 0x01 // Status register
#define SIMPLESPI_SPDR 0x02 // Data register
#define SIMPLESPI_SPER 0x03 // Extensions Register
#define SIMPLESPI_SSPU 0x04 // Slave select pickups

//
// Bit masks for each register //
//

// Control Register
#define SIMPLESPI_SPCR_SPR  0x03 // Clock rate select
#define SIMPLESPI_SPCR_CPHA 0x04 // Clock phase
#define SIMPLESPI_SPCR_CPOL 0x08 // Clock polarity
#define SIMPLESPI_SPCR_MSTR 0x10 // Master mode select
#define SIMPLESPI_SPCR_SPE  0x40 // Serial peripheral enable (core enable)
#define SIMPLESPI_SPCR_SPIE 0x80 // Interrupt enable

// Status Register
#define SIMPLESPI_SPSR_RFEMPTY 0x01 // Read FIFO buffer empty
#define SIMPLESPI_SPSR_RFFULL  0x02 // Read FIFO buffer full
#define SIMPLESPI_SPSR_WFEMPTY 0x04 // Write FIFO buffer empty
#define SIMPLESPI_SPSR_WFFULL  0x08 // Write FIFO buffer full
#define SIMPLESPI_SPSR_WCOL    0x40 // Write collision
#define SIMPLESPI_SPSR_SPIF    0x80 // Interrupt flag

// Extensions register
#define SIMPLESPI_SPER_ESPR    0x03 // Extended clock rate select
#define SIMPLESPI_SPER_ICNT    0xc0 // Interrupt count (IRQ set after no. xfer)

// Pass these to the spi_core_set_int_count() function
#define SIMPLESPI_SPER_ICNT_EVERY 0x00
#define SIMPLESPI_SPER_ICNT_TWO   0x40
#define SIMPLESPI_SPER_ICNT_THREE 0x80
#define SIMPLESPI_SPER_ICNT_FOUR  0xC0


void spi_core_enable(int core);
void spi_core_disable(int core);
void spi_core_interrupt_enable(int core);
void spi_core_interrupt_disable(int core);
void spi_core_interrupt_flag_clear(int core);
void spi_core_clock_setup(int core, char polarity, char phase, char rate,char ext_rate);
void spi_core_set_int_count(int core, char cnt);
void spi_core_slave_select(int core, char slave_sel_dec);
void spi_core_write_data(int core, char data);
int spi_core_data_avail(int core);
int spi_core_write_avail(int core);
char spi_core_read_data(int core);



#endif
