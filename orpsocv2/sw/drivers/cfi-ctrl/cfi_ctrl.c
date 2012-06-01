/* Driver functions for cfi_ctrl module which is to control
   CFI flash devices.
*/

#include "board.h"
#include "cpu-utils.h"
#include "cfi_ctrl.h"

#ifndef CFI_CTRL_BASE
#define CFI_CTRL_BASE -1
#endif

void cfi_ctrl_reset_flash(void)
{
  //REG32((CFI_CTRL_BASE + CFI_CTRL_SCR_OFFSET)) = CFI_CTRL_SCR_RESET_DEVICE;
  // Put in array read mode, like reset would
  REG16(CFI_CTRL_BASE) = 0x00ff;
  
}

int cfi_ctrl_busy(void)
{
  //  return REG32((CFI_CTRL_BASE + CFI_CTRL_SCR_OFFSET)) & 
  //CFI_CTRL_SCR_CONTROLLER_BUSY;
  return 0;
}

void cfi_ctrl_clear_status(void)
{
  //REG32((CFI_CTRL_BASE + CFI_CTRL_SCR_OFFSET)) = CFI_CTRL_SCR_CLEAR_FSR;
  REG16(CFI_CTRL_BASE) = 0x0050;
}

unsigned char cfi_ctrl_get_status(void)
{
  //return (unsigned char) (REG32((CFI_CTRL_BASE + CFI_CTRL_FSR_OFFSET)) & 0xff);
  REG16(CFI_CTRL_BASE) = 0x0070;
  return (unsigned char) REG16(CFI_CTRL_BASE);
}

void cfi_ctrl_unlock_block(unsigned int addr)
{
  //REG32((CFI_CTRL_BASE + CFI_CTRL_UNLOCKBLOCK_OFFSET + addr)) = 0;
  REG16(CFI_CTRL_BASE + addr) = 0x0060;
  REG16(CFI_CTRL_BASE + addr) = 0x00d0;
}

int cfi_ctrl_erase_block(unsigned int addr)
{
  cfi_ctrl_clear_status();

  /* Unlock block first */
  cfi_ctrl_unlock_block(addr);
  
  //REG32((CFI_CTRL_BASE + CFI_CTRL_ERASEBLOCK_OFFSET + addr)) = 0;
  REG16(CFI_CTRL_BASE + addr) = 0x0020;
  REG16(CFI_CTRL_BASE + addr) = 0x00d0;

  /* Wait for device to be finished erasing */
  while(!(cfi_ctrl_get_status() & CFI_FSR_DWS));

  /* Check if programming was successful */
  return !!(cfi_ctrl_get_status() & CFI_FSR_ES);
    
}

void cfi_ctrl_erase_block_no_wait(unsigned int addr)
{
  cfi_ctrl_clear_status();

  /* Unlock block first */
  cfi_ctrl_unlock_block(addr);
  
  /* Now erase the block */
  REG16(CFI_CTRL_BASE + addr) = 0x0020;
  REG16(CFI_CTRL_BASE + addr) = 0x00d0;
  //  REG32((CFI_CTRL_BASE + CFI_CTRL_ERASEBLOCK_OFFSET + addr)) = 0;
  
  return;
    
}

int cfi_ctrl_write_short(short data, unsigned int addr)
{

  cfi_ctrl_clear_status();

  REG16(CFI_CTRL_BASE + addr) = 0x0040;
  REG16(CFI_CTRL_BASE + addr) = data;
  
  /* Wait for device to write */
  while(!(cfi_ctrl_get_status() & CFI_FSR_DWS));

  /* Check if programming was successful */
  return !!(cfi_ctrl_get_status() & CFI_FSR_PS);
}

void cfi_ctrl_enable_data_read(void)
{
  REG16(CFI_CTRL_BASE) = 0x00ff;
}

short cfi_ctrl_read_identifier(unsigned int addr)
{
  //return REG16(CFI_CTRL_BASE + CFI_CTRL_DEVICEIDENT_OFFSET + (addr<<1));
  REG16(CFI_CTRL_BASE) = 0x0090;
  return REG16(CFI_CTRL_BASE + (addr<<1));
}

short cfi_ctrl_query_info(unsigned int addr)
{
  REG16(CFI_CTRL_BASE) = 0x0098;  
  return REG16(CFI_CTRL_BASE + (addr<<1));
}


