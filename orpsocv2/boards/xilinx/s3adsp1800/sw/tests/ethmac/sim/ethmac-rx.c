//////////////////////////////////////////////////////////////////////
////                                                              ////
////  Interrupt-driven Ethernet MAC test code                     ////
////                                                              ////
////  Description                                                 ////
////  Do ethernet receive path testing                            ////
////  Relies on testbench to provide simulus - expects at least   ////
////  256 packets to be sent.                                     ////
////                                                              ////
////  Author(s):                                                  ////
////      - Julius Baxter, julius@opencores.org                   ////
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

/* Functions in this file */
void ethmac_setup(void);
void oeth_dump_bds();
/* Interrupt functions */
void oeth_interrupt(void);
static void oeth_rx(void);
static void oeth_tx(void);
/* Function to calculate checksum of ping responses we send */
unsigned short calculate_checksum(char* dats, unsigned int len) ;

/* Let the ethernet packets use a space beginning here for buffering */
#define ETH_BUFF_BASE 0x200000;

#define RXBUFF_PREALLOC	1
#define TXBUFF_PREALLOC	1

/* The transmitter timeout
 */
#define TX_TIMEOUT	(2*HZ)

/* Buffer number (must be 2^n) 
 * Note: if changing these, must also change settings in eth_stim.v testbench
 * file!
 */
#define OETH_RXBD_NUM		16
#define OETH_TXBD_NUM		16
#define OETH_RXBD_NUM_MASK	(OETH_RXBD_NUM-1)
#define OETH_TXBD_NUM_MASK	(OETH_TXBD_NUM-1)

/* Buffer size 
 */
#define OETH_RX_BUFF_SIZE	0x600
#define OETH_TX_BUFF_SIZE	0x600

/* Buffer size  (if not XXBUF_PREALLOC 
 */
#define MAX_FRAME_SIZE		1518

/* The buffer descriptors track the ring buffers.   
 */
struct oeth_private {
  //struct	sk_buff* rx_skbuff[OETH_RXBD_NUM];
  //struct	sk_buff* tx_skbuff[OETH_TXBD_NUM];

  unsigned short	tx_next;			/* Next buffer to be sent */
  unsigned short	tx_last;			/* Next buffer to be checked if packet sent */
  unsigned short	tx_full;			/* Buffer ring fuul indicator */
  unsigned short	rx_cur;				/* Next buffer to be checked if packet received */
  
  oeth_regs	*regs;			/* Address of controller registers. */
  oeth_bd		*rx_bd_base;		/* Address of Rx BDs. */
  oeth_bd		*tx_bd_base;		/* Address of Tx BDs. */
  
  //	struct net_device_stats stats;
};


char CHECKSUM_BUFFER[OETH_RX_BUFF_SIZE]; // Big enough to hold a packet

#define PHYNUM 7

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

  /* Enable just the receiver
   */
  regs->moder &= ~(OETH_MODER_RXEN | OETH_MODER_TXEN);
  regs->moder |= OETH_MODER_RXEN /* | OETH_MODER_TXEN*/;

  return;
}

void 
ethmac_halt(void)
{
  volatile oeth_regs *regs;
    
  regs = (oeth_regs *)(OETH_REG_BASE);

  // Disable receive and transmit
  regs->moder &= ~(OETH_MODER_RXEN | OETH_MODER_TXEN);

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
      /* Looking for buffer descriptors marked not empty */
      if(!(rx_bdp[i].len_status & OETH_RX_BD_EMPTY)){ 
	/* Check status for errors.
	 */
	report(i);
	report(rx_bdp[i].len_status);
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
	  //exit(0xbaaaaaad);
	  
	  continue;
	}
	else {
	  /* 
	   * Process the incoming frame.
	   */
	  pkt_len = rx_bdp[i].len_status >> 16;

	  // Do a bit of work - ie. copy it, process it
	  memcpy(CHECKSUM_BUFFER, rx_bdp[i].addr, pkt_len);
	  report(0xc4eccccc);
	  report(calculate_checksum(CHECKSUM_BUFFER, pkt_len));
	  
	  /* finish up */
	  rx_bdp[i].len_status &= ~OETH_RX_BD_STATS; /* Clear stats */
	  rx_bdp[i].len_status |= OETH_RX_BD_EMPTY; /* Mark RX BD as empty */
	  rx_done++;	
	  report(rx_done);
	}	
      }
    }
}

// Calculate checksum on received data.
// From http://lkml.indiana.edu/hypermail/linux/kernel/9612.3/0060.html
unsigned short calculate_checksum(char* dats, unsigned int len) 
{
  unsigned int itr;
  unsigned long accum = 0;
  unsigned long longsum;
  
  // Sum all pairs of data
  for(itr=0;itr<(len & ~0x1);itr+=2)
    accum += (unsigned long)(((dats[itr]<<8)&0xff00)|(dats[itr+1]&0x00ff));
  
  if (len & 0x1) // Do leftover
    accum += (unsigned long) ((dats[itr-1]<<8)&0xff00);

  longsum = (unsigned long) (accum & 0xffff); 
  longsum += (unsigned long) (accum >> 16); // Sum the carries
  longsum += (longsum >> 16);
  return (unsigned short)((longsum ^ 0xffff) & 0xffff);
  
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
	  tx_done = 1;

	}
    }
  return;  
}

// Loop to check if a number is prime by doing mod divide of the number
// to test by every number less than it
int 
is_prime_number(unsigned long n)
{
  unsigned long c;
  if (n < 2) return 0;
  for(c=2;c<n;c++)
    if ((n % c) == 0) 
      return 0;
  return 1;
}

int
main ()
{
  
  /* Initialise handler vector */
  int_init();

  /* Install ethernet interrupt handler, it is enabled here too */
  int_add(ETH0_IRQ, oeth_interrupt, 0);

  /* Enable interrupts in supervisor register */
  cpu_enable_user_interrupts();

  /* Enable CPU timer */
  cpu_enable_timer();
  
  rx_done = 0;
  
  ethmac_setup(); /* Configure MAC, TX/RX BDs and enable RX in MODER */
  
#define NUM_PRIMES_TO_CHECK 1000
#define RX_TEST_LENGTH_PACKETS 12

  char prime_check_results[NUM_PRIMES_TO_CHECK];
  unsigned long num_to_check;

  for(num_to_check=2;num_to_check<NUM_PRIMES_TO_CHECK;num_to_check++)
    {
      prime_check_results[num_to_check-2] 
	= (char) is_prime_number(num_to_check);
      report(num_to_check | (0x1e<<24));
      report(prime_check_results[num_to_check-2] | (0x2e<<24));
      // Check number of packets received, testbench will hopefully send at
      // least this many packets
      if (rx_done >= (RX_TEST_LENGTH_PACKETS - 1))
	exit(0x8000000d);
    }

  ethmac_halt();
  
  exit(0x8000000d);

  return 0;
}
