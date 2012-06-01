#ifndef _BOARD_H_
#define _BOARD_H_

//#define IN_CLK  	      50000000 // Hz
//#define IN_CLK  	      32000000 // Hz
//#define IN_CLK  	      30000000 // HZ
//#define IN_CLK  	      24000000 // HZ
#define IN_CLK  	      20000000 // HZ
//#define IN_CLK  	      18000000 // HZ
//#define IN_CLK  	      16000000 // HZ

//
// ROM bootloader
//
// Uncomment the appropriate bootloader define. This will effect the bootrom.S
// file, which is compiled and converted into Verilog for inclusion at 
// synthesis time. See bootloader/bootloader.S for details on each option.
#ifndef PRELOAD_RAM
#define BOOTROM_SPI_FLASH
//#define BOOTROM_GOTO_RESET
//#define BOOTROM_LOOP_AT_ZERO
//#define BOOTROM_LOOP_IN_ROM
#else
#define BOOTROM_GOTO_RESET
#endif

//
// Defines for each core (memory map base, OR1200 interrupt line number, etc.)
//
#define SDRAM_BASE                 0x0
//#define MT48LC32M16A2 // 64MB SDRAM part
#define MT48LC16M16A2 // 32MB SDRAM part
//#define MT48LC4M16A2 // 8MB SDRAM part

#define FLASHROM_BASE       0xcf000000
#define FLASHROM_SIZE            0x100

#define GPIO_0_BASE         0x91000000

#define UART0_BASE  	    0x90000000
#define UART0_IRQ                    2
#define UART0_BAUD_RATE 	115200

#define UART1_BASE  	    0x93000000
#define UART1_IRQ                    3
#define UART1_BAUD_RATE 	115200

#define UART2_BASE  	    0x94000000
#define UART2_IRQ                    5
#define UART2_BAUD_RATE 	115200

#define SPI0_BASE           0xb0000000
#define SPI0_IRQ                     6

#define SPI1_BASE           0xb1000000
#define SPI1_IRQ                     7

#define SPI2_BASE           0xb2000000
#define SPI2_IRQ                     8

#define I2C_0_BASE          0xa0000000
#define I2C_0_IRQ                   10

#define I2C_1_BASE          0xa1000000
#define I2C_1_IRQ                   11

#define I2C_2_BASE          0xa2000000
#define I2C_2_IRQ                   12

#define I2C_3_BASE          0xa3000000
#define I2C_3_IRQ                   13

#define USB0_BASE            0x9c000000
#define USB0_HOST_IRQ                20
#define USB0_SLAVE_IRQ               21

#define USB1_BASE            0x9d000000
#define USB1_HOST_IRQ                22
#define USB1_SLAVE_IRQ               23

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
// UART driver initialisation
// 
#define UART_NUM_CORES 3

#define UART_BASE_ADDRESSES_CSV						\
	UART0_BASE, UART2_BASE, UART2_BASE

#define UART_BAUD_RATES_CSV						\
	UART0_BAUD_RATE, UART1_BAUD_RATE, UART1_BAUD_RATE

// 
// i2c_master_slave core driver configuration
//

#define I2C_MASTER_SLAVE_NUM_CORES 4

#define I2C_MASTER_SLAVE_BASE_ADDRESSES_CSV		\
	I2C_0_BASE, I2C_1_BASE, I2C_2_BASE,I2C_3_BASE



#endif
