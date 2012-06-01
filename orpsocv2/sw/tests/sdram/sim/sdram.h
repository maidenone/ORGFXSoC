#ifndef _SDRAM_H_
#define _SDRAM_H_

#ifdef  MT48LC32M16A2 // 64MB SDRAM part
#define SDRAM_SIZE 0x04000000
#define SDRAM_ROW_SIZE 2048 // in bytes (10 bits col addr, 2 bytes per)
#define SDRAM_NUM_ROWS_PER_BANK (8192) // 13-bit row address
#define SDRAM_NUM_BANKS 4
#endif

#ifdef  MT48LC16M16A2 // 32MB SDRAM part
#define SDRAM_SIZE 0x02000000
#define SDRAM_ROW_SIZE 1024 // in bytes (9 bits col addr, 2 bytes per)
#define SDRAM_NUM_ROWS_PER_BANK (8192) // 13-bit row address
#define SDRAM_NUM_BANKS 4
#endif

#ifdef MT48LC4M16A2 // 8MB SDRAM part
#define SDRAM_SIZE 0x800000
#define SDRAM_ROW_SIZE 512 // in bytes (8 bits col addr, 2 bytes per)
#define SDRAM_NUM_ROWS_PER_BANK (4096) // 12-bit row address
#define SDRAM_NUM_BANKS 4
#endif


#endif
