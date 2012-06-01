#include "board.h"
#include "cpu-utils.h"
#include "cfi_ctrl.h"
#include "uart.h"
#include "printf.h"

#include "orpsoc-defines.h"


int main(void)
{
  uart_init(DEFAULT_UART);

  printf("\nReading some CFI ID\n");
  printf("manufacturer code: %04x\n",cfi_ctrl_read_identifier(0x00)&0xffff);
  printf("Device ID: %04x\n",cfi_ctrl_read_identifier(0x01)&0xffff);
  printf("RCR: %04x\n",cfi_ctrl_read_identifier(0x05)&0xffff);
  
  printf("query info: %04x\n",cfi_ctrl_query_info(0x10)&0xffff);
  printf("query info: %04x\n",cfi_ctrl_query_info(0x11)&0xffff);
  printf("query info: %04x\n",cfi_ctrl_query_info(0x12)&0xffff);
  printf("device timing & voltage info: %04x\n",
	 cfi_ctrl_query_info(0x1b)&0xffff);

  report(0x8000000d);
  exit(0);
}
