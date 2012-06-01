//////////////////////////////////////////////////////////////////////
////                                                              ////
////  Interrupt-driven Ethernet MAC transmit test code            ////
////                                                              ////
////  Description                                                 ////
////  Transmits packets, testing both 100mbit and 10mbit modes.   ////
////  Expects testbench to be checking each packet sent.          ////   
////  Define, ETH_TX_TEST_LENGTH, set further down, controls how  ////
////  many packets the test will send.                            ////
////                                                              ////
////  Author(s):                                                  ////
////      - jb, jb@orsoc.se, with parts taken from Linux kernel   ////
////        open_eth driver.                                      ////
////                                                              ////
////                                                              ////
//////////////////////////////////////////////////////////////////////
////                                                              ////
//// Copyright (C) 2009 Authors and OPENCORES.ORG                 ////
////                                                              ////
//// This source file may be used and distributed without         ////
//// restriction provided that this copyright statement is not    ////
//// removed from the file and that any derivative work contains  ////
//// the original copyright notice and the associated disclaimer. ////
////                                                              ////
//// This source file is free software; you can redistribute it   ////
//// and/or modify it under the terms of the GNU Lesser General   ////
//// Public License as published by the Free Software Foundation; ////
//// either version 2.1 of the License, or (at your option) any   ////
//// later version.                                               ////
////                                                              ////
//// This source is distributed in the hope that it will be       ////
//// useful, but WITHOUT ANY WARRANTY; without even the implied   ////
//// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR      ////
//// PURPOSE.  See the GNU Lesser General Public License for more ////
//// details.                                                     ////
////                                                              ////
//// You should have received a copy of the GNU Lesser General    ////
//// Public License along with this source; if not, download it   ////
//// from http://www.opencores.org/lgpl.shtml                     ////
////                                                              ////
//////////////////////////////////////////////////////////////////////

#include "cpu-utils.h"
#include "board.h"
#include "int.h"
#include "ethmac.h"
#include "eth-phy-mii.h"

volatile unsigned tx_done;
volatile unsigned rx_done;
static int next_tx_buf_num;

/* Functions in this file */
void ethmac_setup(void);
/* Interrupt functions */
void oeth_interrupt(void);
static void oeth_rx(void);
static void oeth_tx(void);

/* Let the ethernet packets use a space beginning here for buffering */
#define ETH_BUFF_BASE 0x200000;

#define RXBUFF_PREALLOC	1
#define TXBUFF_PREALLOC	1
//#undef RXBUFF_PREALLOC
//#undef TXBUFF_PREALLOC

/* The transmitter timeout
 */
#define TX_TIMEOUT	(2*HZ)

/* Buffer number (must be 2^n) 
 */
#define OETH_RXBD_NUM		16
#define OETH_TXBD_NUM		16
#define OETH_RXBD_NUM_MASK	(OETH_RXBD_NUM-1)
#define OETH_TXBD_NUM_MASK	(OETH_TXBD_NUM-1)

/* Buffer size 
 */
#define OETH_RX_BUFF_SIZE	0x600 - 4
#define OETH_TX_BUFF_SIZE	0x600 - 4

/* Buffer size  (if not XXBUF_PREALLOC 
 */
#define MAX_FRAME_SIZE		1518

/* The buffer descriptors track the ring buffers.   
 */
struct oeth_private {

  unsigned short	tx_next;/* Next buffer to be sent */
  unsigned short	tx_last;/* Next buffer to be checked if packet sent */
  unsigned short	tx_full;/* Buffer ring fuul indicator */
  unsigned short	rx_cur; /* Next buffer to check if packet received */
  
  oeth_regs	        *regs;		/* Address of controller registers. */
  oeth_bd		*rx_bd_base;		/* Address of Rx BDs. */
  oeth_bd		*tx_bd_base;		/* Address of Tx BDs. */
  
  //	struct net_device_stats stats;
};


// Data array of data to transmit, tx_data_array[]
// Not included in ORPSoC - #include "eth-rxtx-data.h"
//int tx_data_pointer;

#define PHYNUM 7

void 
eth_mii_write(char phynum, short regnum, short data)
{
  static volatile oeth_regs *regs = (oeth_regs *)(OETH_REG_BASE);
  regs->miiaddress = (regnum << 8) | phynum;
  regs->miitx_data = data;
  regs->miicommand = OETH_MIICOMMAND_WCTRLDATA;
  regs->miicommand = 0; 
  while(regs->miistatus & OETH_MIISTATUS_BUSY);
}

short 
eth_mii_read(char phynum, short regnum)
{
  static volatile oeth_regs *regs = (oeth_regs *)(OETH_REG_BASE);
  regs->miiaddress = (regnum << 8) | phynum;
  regs->miicommand = OETH_MIICOMMAND_RSTAT;
  regs->miicommand = 0; 
  while(regs->miistatus & OETH_MIISTATUS_BUSY);
  
  return regs->miirx_data;
}
	  

// Wait here until all packets have been transmitted
void wait_until_all_tx_clear(void)
{

  int i;
  volatile oeth_bd *tx_bd;
  tx_bd = (volatile oeth_bd *)OETH_BD_BASE; /* Search from beginning*/

  int some_tx_waiting = 1;
  
  while (some_tx_waiting)
    {
      some_tx_waiting = 0;
      /* Go through the TX buffs, search for unused one */
      for(i = 0; i < OETH_TXBD_NUM; i++) {
	
	if((tx_bd[i].len_status & OETH_TX_BD_READY)) // Looking for buffer ready for transmit
	  some_tx_waiting = 1;
	
      }
    }
  
}


void 
ethphy_set_10mbit(int phynum)
{
  wait_until_all_tx_clear();
  // Hardset PHY to just use 10Mbit mode
  short cr = eth_mii_read(phynum, MII_BMCR);
  cr &= ~BMCR_ANENABLE; // Clear auto negotiate bit
  cr &= ~BMCR_SPEED100; // Clear fast eth. bit
  eth_mii_write(phynum, MII_BMCR, cr);
}


void 
ethphy_set_100mbit(int phynum)
{
  wait_until_all_tx_clear();
  // Hardset PHY to just use 100Mbit mode
  short cr = eth_mii_read(phynum, MII_BMCR);
  cr |= BMCR_ANENABLE; // Clear auto negotiate bit
  cr |= BMCR_SPEED100; // Clear fast eth. bit
  eth_mii_write(phynum, MII_BMCR, cr);
}


void ethmac_setup(void)
{
  // from arch/or32/drivers/open_eth.c
  volatile oeth_regs *regs;
  
  regs = (oeth_regs *)(OETH_REG_BASE);
  
  /* Reset MII mode module */
  regs->miimoder = OETH_MIIMODER_RST; /* MII Reset ON */
  regs->miimoder &= ~OETH_MIIMODER_RST; /* MII Reset OFF */
  regs->miimoder = 0x64; /* Clock divider for MII Management interface */
  
  /* Reset the controller.
   */
  regs->moder = OETH_MODER_RST;	/* Reset ON */
  regs->moder &= ~OETH_MODER_RST;	/* Reset OFF */
  
  /* Setting TXBD base to OETH_TXBD_NUM.
   */
  regs->tx_bd_num = OETH_TXBD_NUM;
  
  
  /* Set min/max packet length 
   */
  regs->packet_len = 0x00400600;
  
  /* Set IPGT register to recomended value 
   */
  regs->ipgt = 0x12;
  
  /* Set IPGR1 register to recomended value 
   */
  regs->ipgr1 = 0x0000000c;
  
  /* Set IPGR2 register to recomended value 
   */
  regs->ipgr2 = 0x00000012;
  
  /* Set COLLCONF register to recomended value 
   */
  regs->collconf = 0x000f003f;
  
  /* Set control module mode 
   */
#if 0
  regs->ctrlmoder = OETH_CTRLMODER_TXFLOW | OETH_CTRLMODER_RXFLOW;
#else
  regs->ctrlmoder = 0;
#endif
  
  /* Clear MIIM registers */
  regs->miitx_data = 0;
  regs->miiaddress = 0;
  regs->miicommand = 0;
  
  regs->mac_addr1 = ETH_MACADDR0 << 8 | ETH_MACADDR1;
  regs->mac_addr0 = ETH_MACADDR2 << 24 | ETH_MACADDR3 << 16 | ETH_MACADDR4 << 8 | ETH_MACADDR5;
  
  /* Clear all pending interrupts 
   */
  regs->int_src = 0xffffffff;
  
  /* Promisc, IFG, CRCEn
   */
  regs->moder |= OETH_MODER_PRO | OETH_MODER_PAD | OETH_MODER_IFG | OETH_MODER_CRCEN | OETH_MODER_FULLD;
  
  /* Enable interrupt sources.
   */

  regs->int_mask = OETH_INT_MASK_TXB 	| 
    OETH_INT_MASK_TXE 	| 
    OETH_INT_MASK_RXF 	| 
    OETH_INT_MASK_RXE 	|
    OETH_INT_MASK_BUSY 	|
    OETH_INT_MASK_TXC	|
    OETH_INT_MASK_RXC;

  // Buffer setup stuff
  volatile oeth_bd *tx_bd, *rx_bd;
  int i,j,k;
  
  /* Initialize TXBD pointer
   */
  tx_bd = (volatile oeth_bd *)OETH_BD_BASE;
  
  /* Initialize RXBD pointer
   */
  rx_bd = ((volatile oeth_bd *)OETH_BD_BASE) + OETH_TXBD_NUM;
  
  /* Preallocated ethernet buffer setup */
  unsigned long mem_addr = ETH_BUFF_BASE; /* Defined at top */

 // Setup TX Buffers
  for(i = 0; i < OETH_TXBD_NUM; i++) {
      //tx_bd[i].len_status = OETH_TX_BD_PAD | OETH_TX_BD_CRC | OETH_RX_BD_IRQ;
      tx_bd[i].len_status = OETH_TX_BD_PAD | OETH_TX_BD_CRC;
      tx_bd[i].addr = mem_addr;
      mem_addr += OETH_TX_BUFF_SIZE;
  }
  tx_bd[OETH_TXBD_NUM - 1].len_status |= OETH_TX_BD_WRAP;

  // Setup RX buffers
  for(i = 0; i < OETH_RXBD_NUM; i++) {
    rx_bd[i].len_status = OETH_RX_BD_EMPTY | OETH_RX_BD_IRQ; // Init. with IRQ
    rx_bd[i].addr = mem_addr;
    mem_addr += OETH_RX_BUFF_SIZE;
  }
  rx_bd[OETH_RXBD_NUM - 1].len_status |= OETH_RX_BD_WRAP; // Last buffer wraps

  /* Enable JUST the transmiter 
   */
  regs->moder &= ~(OETH_MODER_RXEN | OETH_MODER_TXEN);
  regs->moder |= /*OETH_MODER_RXEN |*/ OETH_MODER_TXEN;

  next_tx_buf_num = 0; // init tx buffer pointer

  return;
}



/* Setup buffer descriptors with data */
/* length is in BYTES */
void tx_packet(void* data, int length)
{
  volatile oeth_regs *regs;
  regs = (oeth_regs *)(OETH_REG_BASE);
  
  volatile oeth_bd *tx_bd;
  volatile int i;

   tx_bd = (volatile oeth_bd *)OETH_BD_BASE;
   tx_bd = (struct oeth_bd*) &tx_bd[next_tx_buf_num];
   
   // If it's in use - wait
   while ((tx_bd->len_status & OETH_TX_BD_IRQ));

   /* Clear all of the status flags.
   */
   tx_bd->len_status &= ~OETH_TX_BD_STATS;
  
  /* If the frame is short, tell CPM to pad it.
   */
  #define ETH_ZLEN        60   /* Min. octets in frame sans FCS */
  if (length <= ETH_ZLEN)
    tx_bd->len_status |= OETH_TX_BD_PAD;
  else
    tx_bd->len_status &= ~OETH_TX_BD_PAD;
  
  if (data){
    //Copy the data into the transmit buffer, byte at a time 
    char* data_p = (char*) data;
    char* data_b = (char*) tx_bd->addr;
    for(i=0;i<length;i++)
      {
	data_b[i] = data_p[i];
      }
  }

  /* Set the length of the packet's data in the buffer descriptor */
  tx_bd->len_status = (tx_bd->len_status & 0x0000ffff) | 
    ((length&0xffff) << 16);

  /* Send it on its way.  Tell controller its ready, interrupt when sent
  * and to put the CRC on the end.
  */
  tx_bd->len_status |= (OETH_TX_BD_READY  | OETH_TX_BD_CRC | OETH_TX_BD_IRQ);
  
  next_tx_buf_num = (next_tx_buf_num + 1) & OETH_TXBD_NUM_MASK;

  return;

}

/* The interrupt handler.
 */
void
oeth_interrupt(void)
{

  volatile oeth_regs *regs;
  regs = (oeth_regs *)(OETH_REG_BASE);

  uint	int_events;
  int serviced;
  
	serviced = 0;

	/* Get the interrupt events that caused us to be here.
	 */
	int_events = regs->int_src;
	regs->int_src = int_events;


	/* Handle receive event in its own function.
	 */
	if (int_events & (OETH_INT_RXF | OETH_INT_RXE)) {
		serviced |= 0x1; 
		oeth_rx();
	}

	/* Handle transmit event in its own function.
	 */
	if (int_events & (OETH_INT_TXB | OETH_INT_TXE)) {
		serviced |= 0x2;
		oeth_tx();
		serviced |= 0x2;
		
	}

	/* Check for receive busy, i.e. packets coming but no place to
	 * put them. 
	 */
	if (int_events & OETH_INT_BUSY) {
		serviced |= 0x4;
		if (!(int_events & (OETH_INT_RXF | OETH_INT_RXE)))
		  oeth_rx();
	}

	return;
}



static void
oeth_rx(void)
{
  volatile oeth_regs *regs;
  regs = (oeth_regs *)(OETH_REG_BASE);

  volatile oeth_bd *rx_bdp;
  int	pkt_len, i;
  int	bad = 0;
  
  rx_bdp = ((oeth_bd *)OETH_BD_BASE) + OETH_TXBD_NUM;
  

  /* Find RX buffers marked as having received data */
  for(i = 0; i < OETH_RXBD_NUM; i++)
    {
      bad=0;
      if(!(rx_bdp[i].len_status & OETH_RX_BD_EMPTY)){ /* Looking for NOT empty buffers desc. */
	/* Check status for errors.
	 */
	if (rx_bdp[i].len_status & (OETH_RX_BD_TOOLONG | OETH_RX_BD_SHORT)) {
	  bad = 1;
	  report(0xbaad0001);
	}
	if (rx_bdp[i].len_status & OETH_RX_BD_DRIBBLE) {
	  bad = 1;
	  report(0xbaad0002);
	}
	if (rx_bdp[i].len_status & OETH_RX_BD_CRCERR) {
	  bad = 1;
	  report(0xbaad0003);
	}
	if (rx_bdp[i].len_status & OETH_RX_BD_OVERRUN) {
	  bad = 1;
	  report(0xbaad0004);
	}
	if (rx_bdp[i].len_status & OETH_RX_BD_MISS) {
	  report(0xbaad0005);
	}
	if (rx_bdp[i].len_status & OETH_RX_BD_LATECOL) {
	  bad = 1;
	  report(0xbaad0006);
	}
	if (bad) {
	  rx_bdp[i].len_status &= ~OETH_RX_BD_STATS;
	  rx_bdp[i].len_status |= OETH_RX_BD_EMPTY;
	  exit(0xbaaaaaad);
	  
	  continue;
	}
	else {
	  /* Process the incoming frame.
	   */
	  pkt_len = rx_bdp[i].len_status >> 16;
	  
	  /* Do something here with the data - copy it into userspace, perhaps*/
 
	  /* finish up */
	  rx_bdp[i].len_status &= ~OETH_RX_BD_STATS; /* Clear stats */
	  rx_bdp[i].len_status |= OETH_RX_BD_EMPTY; /* Mark RX BD as empty */
	  rx_done++;	  
	}	
      }
    }
}



static void
oeth_tx(void)
{
  volatile oeth_bd *tx_bd;
  int i;
  
  tx_bd = (volatile oeth_bd *)OETH_BD_BASE; /* Search from beginning*/
  
  /* Go through the TX buffs, search for one that was just sent */
  for(i = 0; i < OETH_TXBD_NUM; i++)
    {
      /* Looking for buffer NOT ready for transmit. and IRQ enabled */
      if( (!(tx_bd[i].len_status & (OETH_TX_BD_READY))) && (tx_bd[i].len_status & (OETH_TX_BD_IRQ)) )
	{
	  /* Single threaded so no chance we have detected a buffer that has had its IRQ bit set but not its BD_READ flag. Maybe this won't work in linux */
	  tx_bd[i].len_status &= ~OETH_TX_BD_IRQ;

	  /* Probably good to check for TX errors here */
	  
	  /* set our test variable */
	  tx_done++;

	}
    }
  return;  
}

// A function and defines to fill and transmit a packet
#define MAX_TX_BUFFER 1532
static char tx_buffer[MAX_TX_BUFFER];

void
fill_and_tx_packet(int size)
{
  int i;
  char tx_byte;

  volatile oeth_regs *regs;
  regs = (oeth_regs *)(OETH_REG_BASE);
  
  volatile oeth_bd *tx_bd;
  
  tx_bd = (volatile oeth_bd *)OETH_BD_BASE;
  tx_bd = (struct oeth_bd*) &tx_bd[next_tx_buf_num];

  // If it's in use - wait
  while ((tx_bd->len_status & OETH_TX_BD_IRQ));

  // Use rand() function to generate data for transmission
  // Assumption: ethernet buffer descriptors are 4byte aligned
  char* data_b = (char*) tx_bd->addr;
  // We will fill with words until there' less than a word to go
  int words_to_fill = size/ sizeof(unsigned int);
  unsigned int* data_w = (unsigned int*) data_b;

  for(i=0;i<words_to_fill;i++)
    data_w[i] = rand();

  // Point data_b to offset wher word fills ended
  data_b += (words_to_fill * sizeof(unsigned int));

  int leftover_size = size - (words_to_fill * sizeof(unsigned int));

  for(i=0;i<leftover_size;i++)
    {
      data_b[i] = rand()&0xff;
    }

   tx_packet((void*)0, size);
}

int 
main ()
{
  int i;

  /* Initialise handler vector */
  int_init();

  /* Install ethernet interrupt handler, it is enabled here too */
  int_add(ETH0_IRQ, oeth_interrupt, 0);

  /* Enable interrupts in supervisor register */
  cpu_enable_user_interrupts();
    
  ethmac_setup(); /* Configure MAC, TX/RX BDs and enable RX and TX in MODER */

  /* clear tx_done, the tx interrupt handler will set it when it's been transmitted */
  tx_done = 0;
  rx_done = 0;

  ethphy_set_100mbit(0);

#ifndef ETH_TX_TEST_LENGTH
# define ETH_TX_START_LENGTH  40
# define ETH_TX_TEST_LENGTH  1024
# define ETH_TX_TEST_LENGTH_INCREMENT  21
  //# define ETH_TX_TEST_LENGTH  OETH_TX_BUFF_SIZE
#endif

  for(i=ETH_TX_START_LENGTH;i<ETH_TX_TEST_LENGTH;
      i+=ETH_TX_TEST_LENGTH_INCREMENT)
    fill_and_tx_packet(i);
  
  ethphy_set_10mbit(0);

  for(i=ETH_TX_START_LENGTH;i<ETH_TX_TEST_LENGTH;
      i+=ETH_TX_TEST_LENGTH_INCREMENT)
    fill_and_tx_packet(i);
 
  exit(0x8000000d);

  
}
