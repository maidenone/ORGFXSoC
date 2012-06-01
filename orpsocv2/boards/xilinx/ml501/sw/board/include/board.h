#ifndef _BOARD_H_
#define _BOARD_H_

//#define IN_CLK  	      50000000 // Hz
#define IN_CLK  	      66666667 // Hz

//
// ROM bootloader
//
// Uncomment the appropriate bootloader define. This will effect the bootrom.S
// file, which is compiled and converted into Verilog for inclusion at 
// synthesis time. See bootloader/bootloader.S for details on each option.
#ifndef PRELOAD_RAM
//#define BOOTROM_SPI_FLASH
#define BOOTROM_GOTO_RESET
//#define BOOTROM_LOOP_AT_ZERO
//#define BOOTROM_LOOP_IN_ROM
#else
#define BOOTROM_GOTO_RESET
#endif

// Address bootloader should start from in FLASH
// Last 256KB of 2MB flash - offset 0x1c0000 (2MB-256KB)
#define BOOTROM_ADDR_BYTE2 0x1c
#define BOOTROM_ADDR_BYTE1 0x00
#define BOOTROM_ADDR_BYTE0 0x00
// Causes SPI bootloader to loop if SPI didn't give correct size of image
#define SPI_RETRY_IF_INSANE_SIZEWORD

//
// Defines for each core (memory map base, OR1200 interrupt line number, etc.)
//
#define SDRAM_BASE                 0x0

#define GPIO_0_BASE         0x91000000

#define UART0_BASE  	    0x90000000
#define UART0_IRQ                    2
#define UART0_BAUD_RATE 	115200


#define SPI0_BASE           0xb0000000
#define SPI0_IRQ                     6

#define I2C_0_BASE          0xa0000000
#define I2C_0_IRQ                   10

#define I2C_1_BASE          0xa1000000
#define I2C_1_IRQ                   11

#define ETH0_BASE            0x92000000
#define ETH0_IRQ                      4

#define ETH_MACADDR0	           0x00
#define ETH_MACADDR1	           0x12
#define ETH_MACADDR2  	           0x34
#define ETH_MACADDR3	           0x56
#define ETH_MACADDR4  	           0x78
#define ETH_MACADDR5	           0x9a

//
// OR1200 tick timer period define
//
#define TICKS_PER_SEC   100

//
// CFI flash controller base
//
#define CFI_CTRL_BASE 0xf0000000

//
// UART driver configuration
// 
#define UART_NUM_CORES 1
#define UART_BASE_ADDRESSES_CSV	UART0_BASE
#define UART_BAUD_RATES_CSV UART0_BAUD_RATE


// 
// i2c_master_slave core driver configuration
//

#define I2C_MASTER_SLAVE_NUM_CORES 2

#define I2C_MASTER_SLAVE_BASE_ADDRESSES_CSV		\
	I2C_0_BASE, I2C_1_BASE


#endif
