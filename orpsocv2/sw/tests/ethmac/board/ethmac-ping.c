//////////////////////////////////////////////////////////////////////
////                                                              ////
////  OpenCores 10/100 Ethernet MAC test and diagnosis program    ////
////                                                              ////
////  Description                                                 ////
////  Controllable ping program - also responds to ARP requests   ////
////                                                              ////
////  Author(s):                                                  ////
////      - Julius Baxter, julius@opencores.org                   ////
////        Parts from old Linux open_eth driver.                 ////
////                                                              ////
////                                                              ////
//////////////////////////////////////////////////////////////////////
////                                                              ////
//// Copyright (C) 2009,2011 Authors and OPENCORES.ORG            ////
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
#include "uart.h"
#include "ethmac.h"
#include "printf.h"
#include "eth-phy-mii.h"


volatile unsigned tx_done;
static int next_tx_buf_num;

#define OUR_IP_BYTES 0xc0,0xa8,0x64,0x9b // 192.168.100.155
#define OUR_IP_LONG 0xc0a8649b

//#define OUR_IP_BYTES 0xc0,0xa8,0x0,0x5a // 192.168.0.90
//#define OUR_IP_LONG 0xc0a8005a

//#define OUR_IP_BYTES 0xc0,0xa8,0x1,0x22 // 192.168.1.34
//#define OUR_IP_LONG 0xc0a80122

//#define OUR_IP_BYTES 0xc0,0xa8,0x1,0x2 // 192.168.1.2
//#define OUR_IP_LONG 0xc0a80102

//#define OUR_IP_BYTES 0xac,0x1e,0x0,0x2 // 172.30.0.2
//#define OUR_IP_LONG 0xac1e0002

//#define OUR_IP_BYTES 0xa,0x0,0x0,0x14 // 10.0.0.20
//#define OUR_IP_LONG 0x0a000014


static char our_ip[4] = {OUR_IP_BYTES};

#define DEST_IP_BYTES 0xc0,0xa8,0x64,0x69 // 192 .168.100.105
//#define DEST_IP_BYTES 0xc0,0xa8,0x01,0x08 // 192 .168.1.8
//#define DEST_IP_BYTES 0xc0,0xa8,0x00,0x0f // 192 .168.0.15
//#define DEST_IP_BYTES 0xac,0x1e,0x0,0x01 // 172.30.0.1
//#define DEST_IP_BYTES 0xa,0x0,0x0,0x1 // 10.0.0.1

#define BCAST_DEST_IP_BYTES 0xc0,0xa8,0x64,0xff // 192.168.100.255
//#define BCAST_DEST_IP_BYTES 0xc0,0xa8,0x01,0xff // 192.168.1.255
//#define BCAST_DEST_IP_BYTES 0xc0,0xa8,0x00,0xff // 192.168.0.255
//#define BCAST_DEST_IP_BYTES 0xa,0x0,0x0,0xff // 10.0.0.255

/* Functions in this file */
void ethmac_setup(void);
void oeth_printregs(void);
void ethphy_init(void);
void oeth_dump_bds();
/* Interrupt functions */
void oeth_interrupt(void);
static void oeth_rx(void);
static void oeth_tx(void);
/* Function to calculate checksum of ping responses we send */
unsigned short calculate_checksum(char* dats, unsigned int len) ;

// Global used to control whether we print out packets as we receive them
int print_packet_contents;
int packet_inspect_debug;
int print_ethmac_debug_reg;

/* Let the ethernet packets use a space beginning here for buffering */
#define ETH_BUFF_BASE 0x01000000


#define RXBUFF_PREALLOC	1
#define TXBUFF_PREALLOC	1
//#undef RXBUFF_PREALLOC
//#undef TXBUFF_PREALLOC

/* The transmitter timeout
*/
#define TX_TIMEOUT	(2*HZ)

/* Buffer number (must be 2^n) 
*/
#define OETH_RXBD_NUM		124
#define OETH_TXBD_NUM		4
#define OETH_RXBD_NUM_MASK	(OETH_RXBD_NUM-1)
#define OETH_TXBD_NUM_MASK	(OETH_TXBD_NUM-1)

/* Buffer size 
*/
#define OETH_RX_BUFF_SIZE	2048
#define OETH_TX_BUFF_SIZE	2048

/* Buffer size  (if not XXBUF_PREALLOC 
*/
#define MAX_FRAME_SIZE		0x600
//#define MAX_FRAME_SIZE		2500

/* The buffer descriptors track the ring buffers.   
*/
struct oeth_private {
  //struct	sk_buff* rx_skbuff[OETH_RXBD_NUM];
  //struct	sk_buff* tx_skbuff[OETH_TXBD_NUM];

  unsigned short	tx_next;/* Next buffer to be sent */
  unsigned short	tx_last;/* Next buffer to be checked if packet sent */
  unsigned short	tx_full;/* Buffer ring fuul indicator */
  unsigned short	rx_cur;	/* Next buffer to be checked if packet received */
  
  oeth_regs	*regs;			/* Address of controller registers. */
  oeth_bd		*rx_bd_base;		/* Address of Rx BDs. */
  oeth_bd		*tx_bd_base;		/* Address of Tx BDs. */
  
  //	struct net_device_stats stats;
};

int ethphy_found = -1;

#define PRINT_BIT_NAME(bit,name) printf("%02d:"name" %d\n",bit,!!(reg&(1<<bit)))
void oeth_print_moder(unsigned long reg)
{
	PRINT_BIT_NAME(16,"RECSMALL");
	PRINT_BIT_NAME(15,"PAD");
	PRINT_BIT_NAME(14,"HUGEN");
	PRINT_BIT_NAME(13,"CRCEN");
	PRINT_BIT_NAME(12,"DLYCRCEN");
	PRINT_BIT_NAME(10,"FULLD");
	PRINT_BIT_NAME(9,"EXDFREN");
	PRINT_BIT_NAME(8,"NOBCKOF");
	PRINT_BIT_NAME(7,"LOOPBCK");
	PRINT_BIT_NAME(6,"IFG");
	PRINT_BIT_NAME(5,"PRO");
	PRINT_BIT_NAME(4,"IAM");
	PRINT_BIT_NAME(3,"BRO");
	PRINT_BIT_NAME(2,"NOPRE");
	PRINT_BIT_NAME(1,"TXEN");
	PRINT_BIT_NAME(0,"RXEN");
}

void oeth_print_intsource(unsigned long reg)
{
	PRINT_BIT_NAME(6,"RXCtrlFrame");
	PRINT_BIT_NAME(5,"TXCtrlFrame");
	PRINT_BIT_NAME(4,"BUSY");
	PRINT_BIT_NAME(3,"RXE");
	PRINT_BIT_NAME(2,"RXB");
	PRINT_BIT_NAME(1,"TXE");
	PRINT_BIT_NAME(0,"TXB");
}

void oeth_print_ctrlmoder(unsigned long reg)
{
	PRINT_BIT_NAME(2,"TXFLOW");
	PRINT_BIT_NAME(1,"RXFLOW");
	PRINT_BIT_NAME(0,"PASSALL");
}

void oeth_print_txbuf(unsigned long reg)
{
	printf("RD%d ",!!(reg&(1<<15)));
	printf("IQ%d ",!!(reg&(1<<14)));
	printf("WP%d ",!!(reg&(1<<13)));
	printf("PD%d ",!!(reg&(1<<12)));
	printf("CC%d ",!!(reg&(1<<11)));
	printf("UN%d ",!!(reg&(1<<8)));
	printf("RY%d ",!!(reg&(1<<3)));
	printf("LC%d ",!!(reg&(1<<2)));
	printf("DF%d ",!!(reg&(1<<1)));
	printf("CS%d ",!!(reg&(1<<0)));
	printf("\n");

}


void oeth_print_rxbuf(unsigned long reg)
{
	printf("EM%d ",!!(reg&(1<<15)));
	printf("IQ%d ",!!(reg&(1<<14)));
	printf("WP%d ",!!(reg&(1<<13)));
	printf("PD%d ",!!(reg&(1<<12)));
	printf("CF%d ",!!(reg&(1<<8)));
	//printf("MS%d ",!!(reg&(1<<7)));
	printf("OR%d ",!!(reg&(1<<6)));
	printf("IS%d ",!!(reg&(1<<5)));
	printf("DN%d ",!!(reg&(1<<4)));
	printf("TL%d ",!!(reg&(1<<3)));
	printf("SF%d ",!!(reg&(1<<2)));
	printf("CE%d ",!!(reg&(1<<1)));
	printf("LC%d ",!!(reg&(1<<0)));
	printf("\n");

}

//	PRINT_BIT_NAME(,"");
void oeth_printregs(void)
{
  volatile oeth_regs *regs;
  regs = (oeth_regs *)(OETH_REG_BASE);
	
  printf("Oeth regs: Mode Register : 0x%lx\n",
	 (unsigned long) regs->moder);          /* Mode Register */
  oeth_print_moder((unsigned long) regs->moder);
  printf("Oeth regs: Interrupt Source Register 0x%lx\n", 
	 (unsigned long) regs->int_src);        /* Interrupt Source Register */
  oeth_print_intsource((unsigned long) regs->int_src);
  printf("Oeth regs: Interrupt Mask Register 0x%lx\n",
	 (unsigned long) regs->int_mask);       /* Interrupt Mask Register */
  printf("Oeth regs: Back to Bak Inter Packet Gap Register 0x%lx\n",
	 (unsigned long) regs->ipgt);           /* Back to Bak Inter Packet Gap Register */
  printf("Oeth regs: Non Back to Back Inter Packet Gap Register 1 0x%lx\n",
	 (unsigned long) regs->ipgr1);          /* Non Back to Back Inter Packet Gap Register 1 */
  printf("Oeth regs: Non Back to Back Inter Packet Gap Register 2 0x%lx\n",
	 (unsigned long) regs->ipgr2);          /* Non Back to Back Inter Packet Gap Register 2 */
  printf("Oeth regs: Packet Length Register (min. and max.) 0x%lx\n",
	 (unsigned long) regs->packet_len);     /* Packet Length Register (min. and max.) */
  printf("Oeth regs: Collision and Retry Configuration Register 0x%lx\n",
	 (unsigned long) regs->collconf);       /* Collision and Retry Configuration Register */
  printf("Oeth regs: Transmit Buffer Descriptor Number Register 0x%lx\n",
	 (unsigned long) regs->tx_bd_num);      /* Transmit Buffer Descriptor Number Register */
  printf("Oeth regs: Control Module Mode Register 0x%lx\n",
	 (unsigned long) regs->ctrlmoder);      /* Control Module Mode Register */
  oeth_print_ctrlmoder((unsigned long) regs->ctrlmoder);
  printf("Oeth regs: MII Mode Register 0x%lx\n",
	 (unsigned long) regs->miimoder);       /* MII Mode Register */
  printf("Oeth regs: MII Command Register 0x%lx\n",
	 (unsigned long) regs->miicommand);     /* MII Command Register */
  printf("Oeth regs: MII Address Register 0x%lx\n",
	 (unsigned long) regs->miiaddress);     /* MII Address Register */
  printf("Oeth regs: MII Transmit Data Register 0x%lx\n",
	 (unsigned long) regs->miitx_data);     /* MII Transmit Data Register */
  printf("Oeth regs: MII Receive Data Register 0x%lx\n",
	 (unsigned long) regs->miirx_data);     /* MII Receive Data Register */
  printf("Oeth regs: MII Status Register 0x%lx\n",
	 (unsigned long) regs->miistatus);      /* MII Status Register */
  printf("Oeth regs: MAC Individual Address Register 0 0x%lx\n",
	 (unsigned long) regs->mac_addr0);      /* MAC Individual Address Register 0 */
  printf("Oeth regs: MAC Individual Address Register 1 0x%lx\n",
	 (unsigned long) regs->mac_addr1);      /* MAC Individual Address Register 1 */
  printf("Oeth regs: Hash Register 0 0x%lx\n",
	 (unsigned long) regs->hash_addr0);     /* Hash Register 0 */
  printf("Oeth regs: Hash Register 1  0x%lx\n",
	 (unsigned long) regs->hash_addr1);     /* Hash Register 1 */    
  printf("Oeth regs: TXCTRL  0x%lx\n",
	 (unsigned long) regs->txctrl);     /* TX ctrl reg */    
  printf("Oeth regs: RXCTRL  0x%lx\n",
	 (unsigned long) regs->rxctrl);     /* RX ctrl reg */    
  printf("Oeth regs: WBDBG  0x%lx\n",
	 (unsigned long) regs->wbdbg);     /* Wishbone debug reg */    
	
}

void oeth_print_wbdebug(void)
{
	volatile oeth_regs *regs;
	regs = (oeth_regs *)(OETH_REG_BASE);
	printf("Oeth regs: WBDBG  0x%lx\n",
	       (unsigned long) regs->wbdbg);     /* Wishbone debug reg */    
}

static int last_char;

void spin_cursor(void)
{
#ifdef RTLSIM
  return;
#endif
  volatile unsigned int i; // So the loop doesn't get optimised away
  printf("\r");
  if (last_char == 0)
    printf("/");
  else if (last_char == 1)
    printf("-");
  else if (last_char == 2)
    printf("\\");
  else if (last_char == 3)
    printf("|");
  else if (last_char == 4)
    printf("/");
  else if (last_char == 5)
    printf("-");
  else if (last_char == 6)
    printf("\\");
  else if (last_char == 7)
    {
      printf("|");
      last_char=-1;
    }
  printf("\r");  
  last_char++;
  
  for(i=0;i<150000;i++);

}

static inline void ethphy_smi_read(void)
{

	volatile oeth_regs *regs;
	regs = (oeth_regs *)(OETH_REG_BASE);
	
	regs->miicommand = OETH_MIICOMMAND_RSTAT;
	 /* Wait for command to be registered*/
	while(!(regs->miistatus & OETH_MIISTATUS_BUSY));
	
	regs->miicommand = 0;
	
	while(regs->miistatus & OETH_MIISTATUS_BUSY);
}


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


/* Scan the MIIM bus for PHYs */
void scan_ethphys(void)
{
  unsigned int phynum,regnum, i;
  
  volatile oeth_regs *regs;
  regs = (oeth_regs *)(OETH_REG_BASE);
  
  regs->miitx_data = 0;

  printf("Locating Ethernet PHYs on MDIO bus\n");
 
  for(phynum=0;phynum<32;phynum++)
    {
      for (regnum=0;regnum<8;regnum++)
	{
	  
	  
	  /* Now actually perform the read on the MIIM bus*/
	  regs->miiaddress = (regnum << 8) | phynum; /* Basic Control Register */
	  ethphy_smi_read();
	  
	  //printf("%x\n",regs->miirx_data);
	  
	  // Remember first phy found
	  if (regnum==0 && regs->miirx_data!=0xffff)
	  {
		  // Save found phy address in global variable ethphy_found
		  // for use elsewhere.
		  ethphy_found = phynum;
		  printf("PHY detected at address %d\n",phynum);
		  return;
	  }

	}
    }
}


/* Scan the MIIM bus for PHYs */
void scan_ethphy(unsigned int phynum)
{
  unsigned int regnum, i;
  
  volatile oeth_regs *regs;
  regs = (oeth_regs *)(OETH_REG_BASE);
  
  regs->miitx_data = 0;
 
  for (regnum=0;regnum<32;regnum++)
    {
      printf("scan_ethphy%d: r%x ",phynum, regnum);
	  
      /* Now actually perform the read on the MIIM bus*/
      regs->miiaddress = (regnum << 8) | phynum; /* Basic Control Register */

      ethphy_smi_read();
      
      printf("%x\n",regs->miirx_data);
    }

}


void ethphy_toggle_loopback(void)
{
	volatile oeth_regs *regs;
	regs = (oeth_regs *)(OETH_REG_BASE);

	// First read the loopback bit from the basic control reg
	if (eth_mii_read(ethphy_found%0xff, 0) & (1<<14))
	{
		printf("Disabling PHY loopback\n");
		eth_mii_write(ethphy_found, 0, 
			      eth_mii_read(ethphy_found,0) & ~(1<<14));
	}
	else
	{
		printf("Enabling PHY loopback\n");
		eth_mii_write(ethphy_found, 0, 
			      eth_mii_read(ethphy_found,0) | (1<<14));
	}

}

void marvell_phy_toggle_delay(void)
{
	volatile oeth_regs *regs;
	regs = (oeth_regs *)(OETH_REG_BASE);

	// First read the loopback bit from the basic control reg
	if (eth_mii_read(ethphy_found%0xff, 20) & (1<<1))
	{
		printf("Disabling PHY GTX_CLK delay\n");
		eth_mii_write(ethphy_found, 20, 
			      eth_mii_read(ethphy_found,20) & ~(1<<1));
	}
	else
	{
		printf("Enabling PHY GTX_CLK delay\n");
		eth_mii_write(ethphy_found, 20, 
			      eth_mii_read(ethphy_found,20) | (1<<1));
	}

}

void ethphy_toggle_gigadvertise()
{
	volatile oeth_regs *regs;
	regs = (oeth_regs *)(OETH_REG_BASE);
	// Are we advertising gige?
	if (eth_mii_read(ethphy_found%0xff, 9) & (1<<8))
	{
		printf("Disabling 1000base-t advertisement\n");
		eth_mii_write(ethphy_found, 9, 
			      eth_mii_read(ethphy_found,9) & ~((1<<8)|(1<<9)));
	}
	else
	{
		printf("Enabling 1000base-t advertisement\n");
		eth_mii_write(ethphy_found, 9, 
			      eth_mii_read(ethphy_found,9) | ((1<<8)|(1<<9)));
	}

		
	
}
	  
void ethmac_scanstatus(void)
{
  volatile oeth_regs *regs;
  regs = (oeth_regs *)(OETH_REG_BASE);

  
  printf("Oeth: regs->miistatus %x regs->miirx_data %x\n",regs->miistatus, regs->miirx_data);
  regs->miiaddress = 0;
  regs->miitx_data = 0;
  regs->miicommand = OETH_MIICOMMAND_SCANSTAT;
  printf("Oeth: regs->miiaddress %x regs->miicommand %x\n",regs->miiaddress, regs->miicommand);  
  //regs->miicommand = 0; 
  volatile int i; for(i=0;i<1000;i++);
  while(regs->miistatus & OETH_MIISTATUS_BUSY) ;
  //spin_cursor(); 
  //printf("\r"); 
  //or32_exit(0);
}
	

void
ethphy_reset(int phynum)
{
  eth_mii_write(phynum, MII_BMCR, 
		(eth_mii_read(phynum,MII_BMCR)|BMCR_RESET));
}
 

void
ethphy_reneg(int phynum)
{
  eth_mii_write(phynum, MII_BMCR, 
		(eth_mii_read(phynum,MII_BMCR)|BMCR_ANRESTART));
}

void 
ethphy_set_10mbit(int phynum)
{
  // Hardset PHY to just use 10Mbit mode
  short cr = eth_mii_read(phynum, MII_BMCR);
  cr &= ~BMCR_ANENABLE; // Clear auto negotiate bit
  cr &= ~BMCR_SPEED100; // Clear fast eth. bit
  eth_mii_write(phynum, MII_BMCR, cr);
}

void 
ethphy_set_100mbit(int phynum)
{
  // Hardset PHY to just use 10Mbit mode
  short cr = eth_mii_read(phynum, MII_BMCR);
  cr &= ~(1<<6);
  cr |= (1<<13);
  eth_mii_write(phynum, MII_BMCR, cr);
}

void 
ethphy_toggle_autoneg(int phynum)
{

  short cr = eth_mii_read(phynum, MII_BMCR);
  if (cr & BMCR_ANENABLE)
	  printf("Disabling PHY autonegotiation\n");
  else
	  printf("Enabling PHY autonegotiation\n");

  cr ^= BMCR_ANENABLE; // Toggle auto negotiate bit
  eth_mii_write(phynum, MII_BMCR, cr);
}


void ethphy_print_status(int phyaddr)
{
  short regnum, value;
  int bitnum;
  int bitset;
  short reg2;
  printf("phyaddr %d\n",phyaddr);
  for  (regnum = 0;regnum<16; regnum++)
    {
      value = eth_mii_read(phyaddr, regnum);
      printf("\treg 0x%x: ", regnum);
      switch(regnum)
	{
	case 0:
	  printf("basic control\n");
	  for(bitnum = 0; bitnum<16;bitnum++)
	    {
	      bitset = !!(value & (1<<bitnum));
	      switch(bitnum)
		{
		case 0:
		  printf("\t\tbit%d:\t%d \t(disable transmitter)\n",bitnum,bitset);
		  break;
		case 6:
		  printf("\t\tbit%d:\t%d \t(msb speed (1000))\n",bitnum,bitset);
		  break;
		case 7:
		  printf("\t\tbit%d:\t%d \t(collision test)\n",bitnum,bitset);
		  break;
		case 8:
		  printf("\t\tbit%d:\t%d \t(duplex mode)\n",bitnum,bitset);
		  break;
		case 9:
		  printf("\t\tbit%d:\t%d \t(restart autoneg)\n",bitnum,bitset);
		  break;
		case 10:
		  printf("\t\tbit%d:\t%d \t(isloate)\n",bitnum,bitset);
		  break;
		case 11:
		  printf("\t\tbit%d:\t%d \t(power down)\n",bitnum,bitset);
		  break;
		case 12:
		  printf("\t\tbit%d:\t%d \t(autoneg enable)\n",bitnum,bitset);
		  break;
		case 13:
		  printf("\t\tbit%d:\t%d \t(speed select)\n",bitnum,bitset);
		  break;
		case 14:
		  printf("\t\tbit%d:\t%d \t(loop back)\n",bitnum,bitset);
		  break;
		case 15:
		  printf("\t\tbit%d:\t%d \t(reset)\n",bitnum,bitset);
		  break;
		default:
		  break;
		}
	    }
	  break;
	case 1:
	  printf("basic status\n");
	  for(bitnum = 0; bitnum<16;bitnum++)
	    {
	      bitset = !!(value & (1<<bitnum));
	      switch(bitnum)
		{
		case 0:
		  printf("\t\tbit%d:\t%d \t(extend capability)\n",bitnum,bitset);
		  break;
		case 1:
		  printf("\t\tbit%d:\t%d \t(jabber detect)\n",bitnum,bitset);
		  break;
		case 2:
		  printf("\t\tbit%d:\t%d \t(link status)\n",bitnum,bitset);
		  break;
		case 3:
		  printf("\t\tbit%d:\t%d \t(autoneg capability)\n",bitnum,bitset);
		  break;
		case 4:
		  printf("\t\tbit%d:\t%d \t(remote fault)\n",bitnum,bitset);
		  break;
		case 5:
		  printf("\t\tbit%d:\t%d \t(autoneg complete)\n",bitnum,bitset);
		  break;
		case 6:
		  printf("\t\tbit%d:\t%d \t(no preamble)\n",bitnum,bitset);
		  break;
		case 11:
		  printf("\t\tbit%d:\t%d \t(10base-t half dup.)\n",bitnum,bitset);
		  break;
		case 12:
		  printf("\t\tbit%d:\t%d \t(10base-t full dup.)\n",bitnum,bitset);
		  break;
		case 13:
		  printf("\t\tbit%d:\t%d \t(100base-t half dup.)\n",bitnum,bitset);
		  break;
		case 14:
		  printf("\t\tbit%d:\t%d \t(100base-t full dup.)\n",bitnum,bitset);
		  break;
		case 15:
		  printf("\t\tbit%d:\t%d \t(100base-t4)\n",bitnum,bitset);
		  break;
		default:
		  break;
		  
		}
	    }
	  break;
	case 2:
		// We'll do presentation of phy ID in reg 3
		reg2 = value;
		printf("\n");
		break;
	case 3:
		printf("\t\tPHY Identifier (regs 2,3)\n");
		printf("\t\tOrganizationally Unique Identifier (OUI): 0x%06x\n",
		       ((reg2<<6) | ((value>>10)&0x3f)));
		printf("\t\tManufacturer's Model: 0x%02x\n",(value>>4)&0x3f);
		printf("\t\tRevision number: 0x%01x\n",value&0xf);
		break;
	case 4:
	  printf("autoneg advertise reg\n");
	  for(bitnum = 0; bitnum<16;bitnum++)
	    {
	      bitset = !!(value & (1<<bitnum));
	      switch(bitnum)
		{
		case 5:
		  printf("\t\tbit%d:\t%d \t(10mbps cap.)\n",bitnum,bitset);
		  break;
		case 6:
		  printf("\t\tbit%d:\t%d \t(10base-5 full dup. cap.)\n",bitnum,bitset);
		  break;
		case 7:
		  printf("\t\tbit%d:\t%d \t(100base-tx cap.)\n",bitnum,bitset);
		  break;
		case 8:
		  printf("\t\tbit%d:\t%d \t(100base-tx full dup. cap.)\n",bitnum,bitset);
		  break;
		case 9:
		  printf("\t\tbit%d:\t%d \t(100base-t4 cap.)\n",bitnum,bitset);
		  break;
		case 10:
		  printf("\t\tbit%d:\t%d \t(pause cap.)\n",bitnum,bitset);
		  break;
		case 13:
		  printf("\t\tbit%d:\t%d \t(remote fault sup.)\n",bitnum,bitset);
		  break;
		case 15:
		  printf("\t\tbit%d:\t%d \t(next page cap.)\n",bitnum,bitset);
		  break;
	
		default:
		  break;
		}
	    }
	  break;
	case 5:
	  printf("autoneg link partner ability\n");
	  for(bitnum = 0; bitnum<16;bitnum++)
	    {
	      bitset = !!(value & (1<<bitnum));
	      switch(bitnum)
		{
		case 5:
		  printf("\t\tbit%d:\t%d \t(10mbps cap.)\n",bitnum,bitset);
		  break;
		case 6:
		  printf("\t\tbit%d:\t%d \t(10base-5 full dup. cap.)\n",bitnum,bitset);
		  break;
		case 7:
		  printf("\t\tbit%d:\t%d \t(100base-tx cap.)\n",bitnum,bitset);
		  break;
		case 8:
		  printf("\t\tbit%d:\t%d \t(100base-tx full dup. cap.)\n",bitnum,bitset);
		  break;
		case 9:
		  printf("\t\tbit%d:\t%d \t(100base-t4 cap.)\n",bitnum,bitset);
		  break;
		case 10:		  
		  printf("\t\tbit%d:\t%d \t(pause cap bit0)\n",bitnum,bitset);
		  break;
		case 11:
		  printf("\t\tbit%d:\t%d \t(pause cap bit1)\n",bitnum,bitset);
		  break;

		case 13:
		  printf("\t\tbit%d:\t%d \t(remote fault sup.)\n",bitnum,bitset);
		  break;
		case 14:
		  printf("\t\tbit%d:\t%d \t(acknowledge)\n",bitnum,bitset);
		  break;
		case 15:
		  printf("\t\tbit%d:\t%d \t(next page cap.)\n",bitnum,bitset);
		  break;
	
		default:
		  break;
		}
	    }
	  break;
	case 9:
	  printf("1000mbit advertise\n");
	  for(bitnum = 0; bitnum<16;bitnum++)
	    {
	      bitset = !!(value & (1<<bitnum));
	      switch(bitnum)
		{
		case 8:
		  printf("\t\tbit%d:\t%d \t(1000base-t half dup)\n",bitnum,bitset);
		  break;
		case 9:
		  printf("\t\tbit%d:\t%d \t(1000base-t full dup)\n",bitnum,bitset);
		  break;
		default:
		  break;
		}
	    }
	  break;	   
	case 0xf:
	  printf("extended status\n");
	  for(bitnum = 0; bitnum<16;bitnum++)
	    {
	      bitset = !!(value & (1<<bitnum));
	      switch(bitnum)
		{
		case 12:
		  printf("\t\tbit%d:\t%d \t(1000mb half dup.)\n",bitnum,bitset);
		  break;		  
		case 13:
		  printf("\t\tbit%d:\t%d \t(1000mb full dup.)\n",bitnum,bitset);
		  break;
		default:
		  break;
		}
	    }
	  break;
	  /*	case 1:
	  for(bitnum = 0; bitnum<16;bitnum++)
	  {
	  bitset = !!(value & (1<<bitnum));
	  switch(bitnum)
	  {
	  case 0:
	  printf("\t\tbit%d:\t%d \t()\n",bitnum,bitset);
	  break;
	  default:
	  break;
	  }
	  }
	  break;
	  */
	default:
	  printf("ignored\n");
	  break;
	}
    }	  


		  
}
 
void ethphy_init(void)
{
	short ctl;
	scan_ethphys();

	// Restart autoneg
	printf("Resetting phy...\n");  
	ctl = eth_mii_read(ethphy_found, MII_BMCR);
	ctl |= (BMCR_ANENABLE | BMCR_ANRESTART);
	eth_mii_write(ethphy_found, MII_BMCR, ctl);

	printf("\nOeth: PHY control reg: 0x%.4x\n",
	       eth_mii_read(ethphy_found, MII_BMCR));
	printf("Oeth: PHY control reg: 0x%.4x\n",
	       eth_mii_read(ethphy_found, MII_BMSR));
	printf("Oeth: PHY id0: 0x%.4x\n",
	       eth_mii_read(ethphy_found, MII_PHYSID1));
	printf("Oeth: PHY id1: 0x%.4x\n",
	       eth_mii_read(ethphy_found, MII_PHYSID2));
	printf("Oeth: PHY adv: 0x%.4x\n",
	       eth_mii_read(ethphy_found, MII_ADVERTISE));
	printf("Oeth: PHY lpa: 0x%.4x\n",
	       eth_mii_read(ethphy_found, MII_LPA));
	printf("Oeth: PHY physpec: 0x%.4x\n",
	       eth_mii_read(ethphy_found, MII_M1011_PHY_SPEC_CONTROL));
	printf("Oeth: PHY expansion: 0x%.4x\n",
	       eth_mii_read(ethphy_found, MII_EXPANSION ));
	printf("Oeth: PHY ctrl1000: 0x%.4x\n",
	       eth_mii_read(ethphy_found, MII_CTRL1000));
	printf("Oeth: PHY stat1000: 0x%.4x\n",
	       eth_mii_read(ethphy_found, MII_STAT1000));
	printf("Oeth: PHY estatus: 0x%.4x\n",
	       eth_mii_read(ethphy_found, MII_ESTATUS));
 
  
}


void ethmac_setup(void)
{
  // from arch/or32/drivers/open_eth.c
  volatile oeth_regs *regs;
  
  regs = (oeth_regs *)(OETH_REG_BASE);
  
  /*printf("\nbefore reset\n\n");
  oeth_printregs();*/

  /* Reset MII mode module */
  regs->miimoder = OETH_MIIMODER_RST; /* MII Reset ON */
  regs->miimoder &= ~OETH_MIIMODER_RST; /* MII Reset OFF */
  regs->miimoder = 0x64; /* Clock divider for MII Management interface */
  
  /* Reset the controller.
  */
  regs->moder = OETH_MODER_RST;	/* Reset ON */
  regs->moder &= ~OETH_MODER_RST;	/* Reset OFF */
  
  //printf("\nafter reset\n\n");
  //oeth_printregs();
  
  /* Setting TXBD base to OETH_TXBD_NUM.
  */
  regs->tx_bd_num = OETH_TXBD_NUM;
  
  
  /* Set min/max packet length 
  */
  //regs->packet_len = 0x00400600;
  regs->packet_len = (0x0040 << 16) | (MAX_FRAME_SIZE & 0xffff);
  
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
  regs->mac_addr0 = ETH_MACADDR2 << 24 | ETH_MACADDR3 << 16 | 
    ETH_MACADDR4 << 8 | ETH_MACADDR5;
  
  /* Clear all pending interrupts 
  */
  regs->int_src = 0xffffffff;
  
  /* Promisc, IFG, CRCEn
  */
  regs->moder |= OETH_MODER_PRO | OETH_MODER_PAD | OETH_MODER_IFG | 
    OETH_MODER_CRCEN | OETH_MODER_FULLD;
  
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

  /* Enable receiver and transmiter 
  */
  regs->moder |= OETH_MODER_RXEN | OETH_MODER_TXEN;

  next_tx_buf_num = 0; // init tx buffer pointer

  return;
}

void oeth_ctrlmode_switch(void)
{
	volatile oeth_regs *regs;
	
	regs = (oeth_regs *)(OETH_REG_BASE);

	if (regs->ctrlmoder & (OETH_CTRLMODER_TXFLOW | OETH_CTRLMODER_RXFLOW))
	{
		printf("Disabling TX/RX flow control");

		regs->ctrlmoder = 0;
	}
	else
	{
		printf("Enabling TX/RX flow control");

		regs->ctrlmoder = (OETH_CTRLMODER_TXFLOW | 
				   OETH_CTRLMODER_RXFLOW);
		
	}
		
}

void
oeth_toggle_promiscuous(void)
{
	// from arch/or32/drivers/open_eth.c
	volatile oeth_regs *regs;
  	regs = (oeth_regs *)(OETH_REG_BASE);
	
	if (  regs->moder & OETH_MODER_PRO )
	{
		printf("Disabling ");
		regs->moder &= ~OETH_MODER_PRO;

	}
	else
	{
		printf("Enabling ");
		regs->moder |= OETH_MODER_PRO;

	}
	printf("promisucous mode\n");
}

void oeth_transmit_pause(void)
{
	
	volatile oeth_regs *regs;
	regs = (oeth_regs *)(OETH_REG_BASE);
	regs->txctrl = 0x1fffe;
}


void
ethmac_togglehugen(void)
{
  
  volatile oeth_regs *regs;
  regs = (oeth_regs *)(OETH_REG_BASE);
  
  regs->moder ^= OETH_MODER_HUGEN; // Toggle huge packet enable
  
  if (regs->moder & OETH_MODER_HUGEN) // If we just enabled huge packets
    regs->packet_len = (0x0040 << 16) | (((64*1024)-4) & 0xffff);
  else
    // back to normal
    regs->packet_len = (0x0040 << 16) | (MAX_FRAME_SIZE & 0xffff);

  return;
  
}

void 
oeth_reset_tx_bd_pointer(void)
{
  printf("Resetting TX BD pointer\n");
  // from arch/or32/drivers/open_eth.c
  volatile oeth_regs *regs;
  regs = (oeth_regs *)(OETH_REG_BASE);

  // Toggle TXEN bit, resetting TX BD number
  regs->moder &= OETH_MODER_TXEN;
  regs->moder |= OETH_MODER_TXEN;

  next_tx_buf_num = 0; // init tx buffer pointer
  
}



/* Find the next available transmit buffer */
struct oeth_bd* get_next_tx_bd()
{
  
  int i;
  volatile oeth_bd *tx_bd;
  tx_bd = (volatile oeth_bd *)OETH_BD_BASE; /* Search from beginning*/
  
  /* Go through the TX buffs, search for unused one */
  for(i = next_tx_buf_num; i < OETH_TXBD_NUM; i++) {
    
    if(!(tx_bd[i].len_status & OETH_TX_BD_READY)) /* Looking for buffer NOT ready for transmit. ie we can manipulate it */
      {
	if (print_packet_contents)
	  printf("Oeth: Using TX_bd at 0x%lx\n",(unsigned long)&tx_bd[i]);

	if (next_tx_buf_num == OETH_TXBD_NUM-1) next_tx_buf_num = 0;
	else next_tx_buf_num++;
	
	return (struct oeth_bd*) &tx_bd[i];
      }
    
    if ((i == OETH_TXBD_NUM-1) && (next_tx_buf_num != 0)) 
      i = 0;
    
  }

  printf("No free tx buffers\n");
  /* Set to null our returned buffer */
  tx_bd = (volatile oeth_bd *) 0;
  return (struct oeth_bd*) tx_bd;
  
}

/* print packet contents */
static void
oeth_print_packet(int bd, unsigned long add, int len)
{

	int truncate = (len > 256);
  int length_to_print = truncate ? 256 : len;

  int i;
  printf("\nbd%03d packet: add = %lx len = %d\n", bd,add, len);
  for(i = 0; i < length_to_print; i++) {
    if(!(i % 8))
      printf(" ");
    if(!(i % 16))
      printf("\n");
    printf(" %.2x", *(((unsigned char *)add) + i));

  }
  printf("\n");

  if (truncate)
    printf("\ttruncated....\n");
	
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

  /*if((tx_bd = (volatile oeth_bd *) get_next_tx_bd()) == NULL)
  {
  printf("No more TX buffers available\n");
  return;
  }
  */
  if (print_packet_contents)
    printf("Oeth: Using TX_bd buffer address: 0x%lx\n",(unsigned long) tx_bd->addr);

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
  
  //printf("Oeth: Copying %d bytes from 0x%lx to TX_bd buffer at 0x%lx\n",length,(unsigned long) data,(unsigned long) tx_bd->addr);
  
  /* Copy the data into the transmit buffer, byte at a time */
  char* data_p = (char*) data;
  char* data_b = (char*) tx_bd->addr;
  for(i=0;i<length;i++)
    {
      data_b[i] = data_p[i];
    }
  //printf("Oeth: Data copied to buffer\n");
  
  /* Set the length of the packet's data in the buffer descriptor */
  tx_bd->len_status = (tx_bd->len_status & 0x0000ffff) | 
    ((length&0xffff) << 16);

  //oeth_print_packet(tx_bd->addr, (tx_bd->len_status >> 16));

  /* Send it on its way.  Tell controller its ready, interrupt when sent
  * and to put the CRC on the end.
  */
  tx_bd->len_status |= (OETH_TX_BD_READY  | OETH_TX_BD_CRC | OETH_TX_BD_IRQ);

  //next_tx_buf_num = (next_tx_buf_num + 1) & OETH_TXBD_NUM_MASK;
  next_tx_buf_num++;
  if (next_tx_buf_num == OETH_TXBD_NUM)
  {
	  next_tx_buf_num = 0;
  }
	  

  return;


}  

/* enable RX, loop waiting for arrived packets and print them out */
void oeth_monitor_rx(void)
{
  volatile oeth_regs *regs;
  regs = (oeth_regs *)(OETH_REG_BASE);

  /* Set RXEN in MAC MODER */
  regs->moder = OETH_MODER_RXEN | regs->moder;  

  
  volatile oeth_bd *rx_bd; 
  rx_bd = ((volatile oeth_bd *)OETH_BD_BASE) + OETH_TXBD_NUM;

  volatile int i;
  
  for(i=0;i<OETH_RXBD_NUM;i++)
    {
      if (!(rx_bd[i].len_status & OETH_RX_BD_EMPTY)) /* Not empty */
	{
	  // Something in this buffer!
	  printf("Oeth: RX in buf %d - len_status: 0x%lx\n",i, rx_bd[i].len_status);
	  oeth_print_packet(i,rx_bd[i].addr, rx_bd[i].len_status >> 16);
	  /* Clear recieved bit */
	  rx_bd[i].len_status |=  OETH_RX_BD_EMPTY;	      
	  printf("\t end of packet\n\n");
	}
    }
}

#include "spr-defs.h"

/* Print out all buffer descriptors */
void oeth_dump_bds()
{
	// Disable interrupts
	mtspr (SPR_SR, mfspr (SPR_SR) & ~SPR_SR_IEE);

	unsigned long* bd_base = (unsigned long*) OETH_BD_BASE;

	int i;
	for(i=0;i<OETH_TXBD_NUM;i++)
	{
		printf("TXBD%03d len_status %08lx ",i,*bd_base);
		oeth_print_txbuf(*bd_base++);
		//printf("addr: %lx\n", *bd_base++);
		*bd_base++;
	}

	for(i=0;i<OETH_RXBD_NUM;i++)
	{
		printf("RXBD%03d len_status %08lx ",i,*bd_base);
		oeth_print_rxbuf(*bd_base++);
		*bd_base++;
      
		//printf("addr: %lx\n", *bd_base++);
	}

	// Enable interrupts
	mtspr (SPR_SR, mfspr (SPR_SR) | SPR_SR_IEE);
  
}



char broadcast_ping_packet[] =  {
  0xff,0xff,0xff,0xff,0xff,0xff, /*SRC MAC*/
  0x00, 0x12, 0x34, 0x56, 0x78, 0x9a, /*SRC MAC*/
  0x08,0x00,
  0x45,
  0x00,
  //  0x00,0x54, /* length */
  0x01,0x1c, /* length */
  0x00,0x00,
  0x40,
  0x00,
  0x40,
  0x01,
  0xb5,0x8f,
  OUR_IP_BYTES, /* Source IP */
  BCAST_DEST_IP_BYTES, /* Dest. IP */
  /* ICMP Message body */
  0x08,0x00,0x7d,0x65,0xa7,0x20,0x00,0x01,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,
  15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,
  40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,
  65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,
  90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,
  111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,
  130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,
  149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,
  168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,
  187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,
  206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,
  225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,
  244,245,246,247,248,249,250,251,252,253,254,255};


char big_ping_packet[] =  {
  0x00, 0x24, 0xe8, 0x91, 0x7c, 0x0d, /*DST MAC*/
  //0xff,0xff,0xff,0xff,0xff,0xff, /*SRC MAC*/
  0x00, 0x12, 0x34, 0x56, 0x78, 0x9a, /*SRC MAC*/
  0x08,0x00,
  0x45,
  0x00,
  0x05,0xdc, /* length */
  0x00,0x00,
  0x40,
  0x00,
  0x40,
  0x01,
  0xea,0xcb,   /* Header checksum */
  OUR_IP_BYTES, /* Source IP */
  DEST_IP_BYTES, /* Dest. IP */
  /* ICMP Message body */
  0x08,0x00, /* Type */
  0x04,0x48, /* ICMP checksum */
  0x02,0x00, /* Identifier */
  0x3a,0x00, /* sequence number */ 
  0x61,0x62,0x63,0x64,0x65,0x66,0x67,
  0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,
  0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,
  0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,
  0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,
  0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,
  0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,
  0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,
  0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,
  0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,
  0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,
  0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,
  0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,
  0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,
  0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,
  0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,
  0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,
  0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,
  0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,
  0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,
  0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,
  0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
  0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
  0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
  0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,
  0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,
  0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,
  0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,
  0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,
  0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,
  0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,
  0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,
  0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,
  0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,
  0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,
  0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,
  0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,
  0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,
  0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,
  0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,
  0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,
  0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,
  0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,
  0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,
  0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
  0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
  0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
  0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,
  0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,
  0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,
  0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,
  0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,
  0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,
  0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,
  0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,
  0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,
  0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,
  0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,
  0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,
  0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,
  0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,
  0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,
  0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,
  0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,
  0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,
  0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,
  0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,
  0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
  0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
  0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
  0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,
  0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,
  0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,
  0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,
  0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,
  0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,
  0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,
  0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,
  0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,
  0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,
  0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,
  0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,
  0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,
  0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,
  0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,
  0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,
  0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,
  0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,
  0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,
  0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,
  0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
  0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
  0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
  0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,
  0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,
  0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,
  0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,
  0x76,0x77,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,
  0x6e,0x6f,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77};


/* This should be 98 bytes big */
char ping_packet[] = {
  0x00, 0x24, 0xe8, 0x91, 0x7c, 0x0d, /*DST MAC*/
  //0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /*DST MAC*/
  0x00, 0x12, 0x34, 0x56, 0x78, 0x9a, /*SRC MAC*/
  0x08, 0x00, /*TYPE*/
  /* IP */
  0x45, /* Version, header length*/
  0x00, /* Differentiated services field */  
  0x00, 0x54, /* Total length */
  0x00, 0x00, /* Identification */
  0x40, /* Flags */
  0x00, /* Fragment offset */
  0x40, /* Time to live */
  0x01, /* Protocol (0x01 = ICMP */
  0xf0, 0x53, /* Header checksum */
  //0xc0, 0xa8, 0x64, 0xDE, /* Source IP */
  OUR_IP_BYTES, /* Source IP */
  DEST_IP_BYTES, /* Dest. IP */
  /* ICMP Message body */
  0x08, 0x00, 0x9a, 0xd4, 0xc8, 0x18, 0x00, 0x01, 0xd9, 0x8c, 0x54, 
  0x4a, 0x7b, 0x37, 0x01, 0x00, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 
  0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 
  0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 
  0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 
  0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
};

void send_test_packet()
{

  /*Send packet */
  //tx_packet((void*) ping_packet, 102);
  tx_packet((void*) broadcast_ping_packet, 102);

}


/* The interrupt handler.
*/
void
oeth_interrupt(void)
{

	volatile oeth_regs *regs;
	regs = (oeth_regs *)(OETH_REG_BASE);

	uint int_events;
	int  serviced;
  
	serviced = 0;

	/* Get the interrupt events that caused us to be here.
	 */
	int_events = regs->int_src;
	regs->int_src = int_events;


	/* Indicate busy */
	if (int_events & OETH_INT_BUSY)
	{
		printf("\tBusy flag\n");
		/*
		  printf("\n=tx_ | %x | %x | %x | %x | %x | %x | %x | %x\n",
		  ((oeth_bd *)(OETH_BD_BASE))->len_status,
		  ((oeth_bd *)(OETH_BD_BASE+8))->len_status,
		  ((oeth_bd *)(OETH_BD_BASE+16))->len_status,
		  ((oeth_bd *)(OETH_BD_BASE+24))->len_status,
		  ((oeth_bd *)(OETH_BD_BASE+32))->len_status,
		  ((oeth_bd *)(OETH_BD_BASE+40))->len_status,
		  ((oeth_bd *)(OETH_BD_BASE+48))->len_status,
		  ((oeth_bd *)(OETH_BD_BASE+56))->len_status);
		*/	
		printf("=rx_ | %x | %x | %x | %x | %x | %x | %x | %x\n",
		       ((oeth_bd *)(OETH_BD_BASE+64))->len_status,
		       ((oeth_bd *)(OETH_BD_BASE+64+8))->len_status,
		       ((oeth_bd *)(OETH_BD_BASE+64+16))->len_status,
		       ((oeth_bd *)(OETH_BD_BASE+64+24))->len_status,
		       ((oeth_bd *)(OETH_BD_BASE+64+32))->len_status,
		       ((oeth_bd *)(OETH_BD_BASE+64+40))->len_status,
		       ((oeth_bd *)(OETH_BD_BASE+64+48))->len_status,
		       ((oeth_bd *)(OETH_BD_BASE+64+56))->len_status);
      
		printf("=int | rxc %d | txc %d | txb %d | txe %d | rxb %d | rxe %d | busy %d\n",
		       (int_events & OETH_INT_RXC) > 0,
		       (int_events & OETH_INT_TXC) > 0,
		       (int_events & OETH_INT_TXB) > 0,
		       (int_events & OETH_INT_TXE) > 0,
		       (int_events & OETH_INT_RXF) > 0,
		       (int_events & OETH_INT_RXE) > 0,
		       (int_events & OETH_INT_BUSY) > 0);
	}/*
	else 
		printf("=int | rxc %d | txc %d | txb %d | txe %d | rxb %d | rxe %d | busy %d\n",
		       (int_events & OETH_INT_RXC) > 0,
		       (int_events & OETH_INT_TXC) > 0,
		       (int_events & OETH_INT_TXB) > 0,
		       (int_events & OETH_INT_TXE) > 0,
		       (int_events & OETH_INT_RXF) > 0,
		       (int_events & OETH_INT_RXE) > 0,
		       (int_events & OETH_INT_BUSY) > 0);
	     */
	/* Handle receive event in its own function.
	 */
	if (int_events & (OETH_INT_RXF | OETH_INT_RXE)) {
		serviced |= 0x1; 
		if (print_ethmac_debug_reg)
			oeth_print_wbdebug();
		oeth_rx();
		if (print_ethmac_debug_reg)
			oeth_print_wbdebug();
	}

	/* Handle transmit event in its own function.
	 */
	if (int_events & (OETH_INT_TXB | OETH_INT_TXE)) {
		if (print_ethmac_debug_reg)
			oeth_print_wbdebug();
		serviced |= 0x2;
		oeth_tx();
		serviced |= 0x2;
		if (print_ethmac_debug_reg)
			oeth_print_wbdebug();
		
	}
	
	return;
}

// ARP stuff

typedef unsigned long		IPaddr_t;

/*
*	Ethernet header
*/
typedef struct {
  unsigned char		et_dest[6];	/* Destination node		*/
  unsigned char		et_src[6];	/* Source node			*/
  unsigned short		et_protlen;	/* Protocol or length	*/
  unsigned char		et_dsap;	/* 802 DSAP			*/
  unsigned char		et_ssap;	/* 802 SSAP			*/
  unsigned char		et_ctl;		/* 802 control			*/
  unsigned char		et_snap1;	/* SNAP				*/
  unsigned char		et_snap2;
  unsigned char		et_snap3;
  unsigned short		et_prot;	/* 802 protocol		*/
} Ethernet_t;

#define ETHER_HDR_SIZE	14		/* Ethernet header size		*/
#define E802_HDR_SIZE	22		/* 802 ethernet header size	*/
#define PROT_IP		0x0800		/* IP protocol			*/
#define PROT_ARP	0x0806		/* IP ARP protocol		*/
#define PROT_RARP	0x8035		/* IP ARP protocol		*/


/*
 *	Internet Protocol (IP) header.
 */
typedef struct {
	unsigned char		ip_hl_v;        /* header length and version*/
	unsigned char		ip_tos;		/* type of service	    */
	unsigned short		ip_len;		/* total length		    */
	unsigned short		ip_id;		/* identification	    */
	unsigned short		ip_off;		/* fragment offset field    */
	unsigned char		ip_ttl;		/* time to live		    */
	unsigned char		ip_p;		/* protocol		    */
	unsigned short		ip_sum;		/* checksum		    */
	IPaddr_t	ip_src;		/* Source IP address		*/
	IPaddr_t	ip_dst;		/* Destination IP address	*/
	unsigned short		udp_src;	/* UDP source port	*/
	unsigned short		udp_dst;	/* UDP destination port	*/
	unsigned short		udp_len;	/* Length of UDP packet	*/
	unsigned short		udp_xsum;	/* Checksum		*/
} IP_t;

#define IP_HDR_SIZE_NO_UDP	(sizeof (IP_t) - 8)
#define IP_HDR_SIZE		(sizeof (IP_t))

#define IPPROTO_ICMP	 1	/* Internet Control Message Protocol	*/
#define IPPROTO_UDP	17	/* User Datagram Protocol		*/


/*
 * ICMP stuff (just enough to handle (host) redirect messages)
 */
#define ICMP_REDIRECT		5	/* Redirect (change route)	*/

/* Codes for REDIRECT. */
#define ICMP_REDIR_NET		0	/* Redirect Net			*/
#define ICMP_REDIR_HOST		1	/* Redirect Host		*/

#define ICMP_TYPE_ECHO_REPLY          0x00
#define ICMP_TYPE_ECHO_REQ          0x08

unsigned char ip_reply_packet[0x600] __attribute__ ((aligned (4))); // Save space for a full ICMP reply packet

typedef struct {
	unsigned char		type;
	unsigned char		code;
	unsigned short		checksum;
	union {
		struct {
			unsigned short	id;
			unsigned short	sequence;
		} echo;
		unsigned long	gateway;
		struct {
			unsigned short	__unused;
			unsigned short	mtu;
		} frag;
	} un;
} ICMP_t;

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

void
packet_check_icmp_header(void * pkt)
{
  Ethernet_t * eth_pkt;
  IP_t * ip;
  ICMP_t * icmp;

  unsigned int zero = 0;
  
  eth_pkt = (Ethernet_t *) pkt;
    
  // Check it's for our MAC
  char* eth_pkt_dst_mac = (char*) eth_pkt;
  if (!(
	((eth_pkt_dst_mac[0] == (char) ETH_MACADDR0) &&  // Either our MAC
	 (eth_pkt_dst_mac[1] == (char) ETH_MACADDR1) &&
	 (eth_pkt_dst_mac[2] == (char) ETH_MACADDR2) &&
	 (eth_pkt_dst_mac[3] == (char) ETH_MACADDR3) &&
	 (eth_pkt_dst_mac[4] == (char) ETH_MACADDR4) &&
	 (eth_pkt_dst_mac[5] == (char) ETH_MACADDR5)) || 
	((eth_pkt_dst_mac[0] == (char) 0xff) &&          // or broadcast MAC
	 (eth_pkt_dst_mac[1] == (char) 0xff) &&
	 (eth_pkt_dst_mac[2] == (char) 0xff) &&
	 (eth_pkt_dst_mac[3] == (char) 0xff) &&
	 (eth_pkt_dst_mac[4] == (char) 0xff) &&
	 (eth_pkt_dst_mac[5] == (char) 0xff)) 
	)
      )
	
    return ;

  // Check it's an IP packet
  if (!(eth_pkt->et_protlen == PROT_IP))
    return ; 

  pkt += ETHER_HDR_SIZE; // Skip eth header stuff
  
  ip = (IP_t*) pkt;
  
  // Check if this is an ICMP packet
  if (!(ip->ip_p == IPPROTO_ICMP))
    return ;
  
  // Check if this is for our IP, get pointer to the DST IP part of IP header
  // which is end of IP section - 4 bytes
  char * internet_protocol_adr = ((unsigned char*)ip + (IP_HDR_SIZE_NO_UDP-4));
  
  if (!((internet_protocol_adr[0] == our_ip[0]) &&
	(internet_protocol_adr[1] == our_ip[1]) &&
	(internet_protocol_adr[2] == our_ip[2]) &&
	(internet_protocol_adr[3] == our_ip[3])))
    return ;
  
  pkt += IP_HDR_SIZE_NO_UDP;
  
  icmp = (ICMP_t*) pkt;
  
  // Currently we only support replying to echo (ping) requests

  // Check for ICMP echo request type
  if (!(icmp->type == ICMP_TYPE_ECHO_REQ))
    return;

  // Skip forward to the target I.P address
  if (packet_inspect_debug)
	  printf("Ping packet\n");
  
  // Go ahead and construct a response packet
  // Setup pointers
  Ethernet_t * reply_pkt  = (Ethernet_t *) &ip_reply_packet[0];
  IP_t       * reply_IP   = (IP_t*)        &ip_reply_packet[ETHER_HDR_SIZE];
  ICMP_t     * reply_ICMP = (ICMP_t*)      &ip_reply_packet[ETHER_HDR_SIZE+
							    IP_HDR_SIZE_NO_UDP];
  // Setup Ethernet header
  // Copy source MAC to destination MAC
  memcpy( (void*)&reply_pkt->et_dest , (void*)&eth_pkt->et_src , 6);
  reply_pkt->et_src[0] = ETH_MACADDR0;
  reply_pkt->et_src[1] = ETH_MACADDR1;
  reply_pkt->et_src[2] = ETH_MACADDR2;
  reply_pkt->et_src[3] = ETH_MACADDR3;
  reply_pkt->et_src[4] = ETH_MACADDR4;
  reply_pkt->et_src[5] = ETH_MACADDR5;
  reply_pkt->et_protlen = PROT_IP;
  
  // IP header
  reply_IP->ip_hl_v = 0x45; // version 4, 20 byte long header, not 100% on this!
  reply_IP->ip_tos = 0x00 ; // ToS (DSCP) - usually used for QoS, set to 0 
  unsigned short ip_indicated_length;
  // First copy total length indicated in received IP header to a variable, we
  // need it again later...
  memcpy ((void*)&ip_indicated_length, (void*)&ip->ip_len, 2);
  // Now copy into reply IP packet
  memcpy ((void*)&reply_IP->ip_len, (void*)&ip_indicated_length, 2);
  memcpy ((void*)&reply_IP->ip_id, (void*)&ip->ip_id, 2); // Copy ID
  reply_IP->ip_ttl = 0x40; // Packet TTL - 64, typical value
  reply_IP->ip_p = IPPROTO_ICMP; // Duh, this is an ICMP echo reply
  // Clear header checksum for now  
  
  memcpy ((void*)&reply_IP->ip_sum, (void*)&zero, 2);
  memcpy ((void*)&reply_IP->ip_src, (void*)&our_ip[0], 4); // Copy in our IP
  // "...return to sender... budupbadah address ....KNOWN ..."
  // WTF this memcpy() fails with alignment error...(?!?!)
  //memcpy (&reply_IP->ip_dst, &ip->ip_src, sizeof(IPaddr_t));
  //...OK then, do it manually.....
  unsigned char *ptr1, *ptr2;
  ptr1 = &reply_IP->ip_dst; ptr2 = &ip->ip_src;
  ptr1[0] = ptr2[0];
  ptr1[1] = ptr2[1];
  ptr1[2] = ptr2[2];
  ptr1[3] = ptr2[3];

  // Now calculate the CRC, probably move this to a fuction....
  unsigned short ip_sum = 0; // Initialise checksum value to 0
  int itr;   
  char* sum_data_ptr = (char*)reply_IP;
  ip_sum = calculate_checksum(sum_data_ptr,IP_HDR_SIZE_NO_UDP);
  /*
  for(itr=0;itr<IP_HDR_SIZE_NO_UDP;itr+=2)
    ip_sum += (((sum_data_ptr[itr]<<8)&0xff00)|(sum_data_ptr[itr+1]&0x00ff));
  while (ip_sum>>16)
    ip_sum = (ip_sum & 0xffff) + (ip_sum>>16);
  ip_sum = ~ip_sum;
  */
  
  memcpy ((void*)&reply_IP->ip_sum, (void*)&ip_sum, 2);
  
  //
  // Now construct ICMP part of packet
  //
  reply_ICMP->type = ICMP_TYPE_ECHO_REPLY; // ping response type
  reply_ICMP->code = icmp->code; // ICMP code same as sender (is this right?)
  
  // Clear ICMP checksum for now  
  memcpy ((void*)&reply_ICMP->checksum, (void*)&zero, 2);
  
  //
  // Simply copy in the data from the received packet
  // Figure out how much there is after the checksum
  // It should be :
  // length_from_received_IP_header - IP_header_length - initial_ICMP_packet_dat
  // 
  unsigned long icmp_data_length = ip_indicated_length - IP_HDR_SIZE_NO_UDP - 4;
  memcpy ((void*)&reply_ICMP->un,(void*)&icmp->un, icmp_data_length);
  
  // Now calculate checksum for ICMP data
  unsigned short icmp_sum = 0;
  sum_data_ptr = (char*)reply_ICMP;
  icmp_sum = calculate_checksum(sum_data_ptr,icmp_data_length+4);
  
  memcpy ((void*)&reply_ICMP->checksum, (void*)&icmp_sum, 2);
  
  // All done!
  
  tx_packet((void*)ip_reply_packet,ETHER_HDR_SIZE+ip_indicated_length);
  
}



/*
*	Address Resolution Protocol (ARP) header.
*/
typedef struct
{
  unsigned short		ar_hrd;	/* Format of hardware address	*/
#   define ARP_ETHER	    1		/* Ethernet  hardware address	*/
  unsigned short		ar_pro;	/* Format of protocol address	*/
  unsigned char		ar_hln;		/* Length of hardware address	*/
  unsigned char		ar_pln;		/* Length of protocol address	*/
  unsigned short		ar_op;	/* Operation			*/
#   define ARPOP_REQUEST    1		/* Request  to resolve  address	*/
#   define ARPOP_REPLY	    2		/* Response to previous request	*/

#   define RARPOP_REQUEST   3		/* Request  to resolve  address	*/
#   define RARPOP_REPLY	    4		/* Response to previous request */

  /*
  * The remaining fields are variable in size, according to
  * the sizes above, and are defined as appropriate for
  * specific hardware/protocol combinations.
  */
  unsigned char		ar_data[0];
#if 0
  unsigned char		ar_sha[];	/* Sender hardware address	*/
  unsigned char		ar_spa[];	/* Sender protocol address	*/
  unsigned char		ar_tha[];	/* Target hardware address	*/
  unsigned char		ar_tpa[];	/* Target protocol address	*/
#endif /* 0 */
} ARP_t;

#define ARP_HDR_SIZE	(8+20)		/* Size assuming ethernet	*/

char arp_reply_packet[(ETHER_HDR_SIZE + ARP_HDR_SIZE)];

void
packet_check_arp_header(void * pkt)
{
  Ethernet_t * eth_pkt;

  ARP_t * arp;

  //printf("packet_check_arp_header: pkt data at 0x%.8x\n",(unsigned long) pkt);
  eth_pkt = (Ethernet_t *) pkt;

  if (eth_pkt->et_protlen == 0x0806)
    {
      // Is an ARP request
      // Check the IP
      pkt += ETHER_HDR_SIZE; // Skip eth header stuff
      //printf("Is ARP protocol\npkt header now at 0x%.8x\n",pkt);
      
      arp = (ARP_t*) pkt;

      if (arp->ar_hrd == ARP_ETHER && arp->ar_op == ARPOP_REQUEST)
	{
		// Skip forward to the target I.P address
		if (packet_inspect_debug)
			printf("ARP packet\n");
	  
	  char * internet_protocol_adr = (((unsigned long)&arp->ar_data[0]) + 
					  (arp->ar_hln * 2) + (arp->ar_pln));
	  
	  //printf("Is ARP ethernet request\ncheck adr at 0x%.8x\n",
	  // internet_protocol_adr);
	  if ((internet_protocol_adr[0] == our_ip[0]) &&
	      (internet_protocol_adr[1] == our_ip[1]) &&
	      (internet_protocol_adr[2] == our_ip[2]) &&
	      (internet_protocol_adr[3] == our_ip[3]))
	    {
	      //printf("Got valid ARP request\n");
	      // Send ARP reply
	      
	      Ethernet_t * reply_pkt = (Ethernet_t *)&arp_reply_packet[0];
	      ARP_t * reply_arp = (ARP_t*)&arp_reply_packet[ETHER_HDR_SIZE];
	      memcpy( (void*)&reply_pkt->et_dest , (void*)&eth_pkt->et_src , 6);
	      reply_pkt->et_src[0] = ETH_MACADDR0;
	      reply_pkt->et_src[1] = ETH_MACADDR1;
	      reply_pkt->et_src[2] = ETH_MACADDR2;
	      reply_pkt->et_src[3] = ETH_MACADDR3;
	      reply_pkt->et_src[4] = ETH_MACADDR4;
	      reply_pkt->et_src[5] = ETH_MACADDR5;
	      reply_pkt->et_protlen = 0x0806;
	      // ARP part of packet	      
	      reply_arp->ar_hrd = ARP_ETHER;
	      reply_arp->ar_pro = 0x0800; // IP Protocol
	      reply_arp->ar_hln = 0x06;
	      reply_arp->ar_pln = 0x04;
	      reply_arp->ar_op = ARPOP_REPLY;
	      // My MAC
	      memcpy( (void*)&reply_arp->ar_data[0] , 
		      (void*)&reply_pkt->et_src , 6);
	      // My IP
	      memcpy( (void*)&reply_arp->ar_data[6] , (void*)&our_ip , 4);
	      // Their MAC
	      memcpy( (void*)&reply_arp->ar_data[10] , 
		      (void*)&eth_pkt->et_src , 6);
	      // Their IP
	      char * their_internet_protocol_adr = 
		(((unsigned long)&arp->ar_data[0]) + arp->ar_hln );
	      memcpy( (void*)&reply_arp->ar_data[16] , 
		      (void*)&their_internet_protocol_adr[0] , 4);

	      tx_packet((void*)arp_reply_packet,(ETHER_HDR_SIZE+ARP_HDR_SIZE) );
	      
	    }
	}
    }
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
  
	if (print_packet_contents)
		printf("rx ");
  
	/* Find RX buffers marked as having received data */
	for(i = 0; i < OETH_RXBD_NUM; i++)
	{
		/* Looking for NOT empty buffers desc. */
		if(!(rx_bdp[i].len_status & OETH_RX_BD_EMPTY)){ 
			/* Check status for errors.
			 */
			if (rx_bdp[i].len_status & (OETH_RX_BD_TOOLONG | 
						    OETH_RX_BD_SHORT)) {
				bad = 1;
			}
			if (rx_bdp[i].len_status & OETH_RX_BD_DRIBBLE) {
				bad = 1;
			}
			if (rx_bdp[i].len_status & OETH_RX_BD_CRCERR) {
				bad = 1;
			}
			if (rx_bdp[i].len_status & OETH_RX_BD_OVERRUN) {
				bad = 1;
			}
			if (rx_bdp[i].len_status & OETH_RX_BD_MISS) {
	  
			}
			if (rx_bdp[i].len_status & OETH_RX_BD_LATECOL) {
				bad = 1;
			}
	
			if (bad) {
				printf("RXE: 0x%x\n",rx_bdp[i].len_status & 
				       OETH_RX_BD_STATS);
				rx_bdp[i].len_status &= ~OETH_RX_BD_STATS;
				rx_bdp[i].len_status |= OETH_RX_BD_EMPTY;

				if (print_packet_contents)
				{
					oeth_print_packet(i, rx_bdp[i].addr, 
							  rx_bdp[i].len_status 
							  >> 16);
					printf("\t end of packet\n\n");
				}

				bad = 0;
				continue;
			}
			else {
	  
				/* Process the incoming frame.
				 */
				pkt_len = rx_bdp[i].len_status >> 16;
	  
				/* Do something here with the data - copy it 
				   into userspace, perhaps. */
				// See if it's an ARP packet
				packet_check_arp_header((void*)rx_bdp[i].addr);
				// See if it's an ICMP echo request
				packet_check_icmp_header((void*)rx_bdp[i].addr);
				if (print_packet_contents)
				{
					oeth_print_packet(i, rx_bdp[i].addr, 
							  rx_bdp[i].len_status 
							  >> 16);
					printf("\t end of packet\n\n");
				}
				/* finish up */
				/* Clear stats */
				rx_bdp[i].len_status &= ~OETH_RX_BD_STATS;
				 /* Mark RX BD as empty */
				rx_bdp[i].len_status |= OETH_RX_BD_EMPTY;

				oeth_transmit_pause(); //try this!
	  
	  
			}
		}
	}
}



static void
oeth_tx(void)
{
	volatile oeth_bd *tx_bd;
	int i;
	if (print_packet_contents)
		printf("tx");

	tx_bd = (volatile oeth_bd *)OETH_BD_BASE; /* Search from beginning*/
  
	/* Go through the TX buffs, search for one that was just sent */
	for(i = 0; i < OETH_TXBD_NUM; i++)
	{
		/* Looking for buffer NOT ready for transmit. and IRQ enabled */
		if( (!(tx_bd[i].len_status & (OETH_TX_BD_READY))) && 
		    (tx_bd[i].len_status & (OETH_TX_BD_IRQ)) )
		{
			/* Single threaded so no chance we have detected a 
			   buffer that has had its IRQ bit set but not its 
			   BD_READ flag. Maybe this won't work in linux */
			tx_bd[i].len_status &= ~OETH_TX_BD_IRQ;

			/* Probably good to check for TX errors here */
			// Check if either carrier sense lost or colission 
			// indicated
			if (tx_bd[i].len_status & OETH_TX_BD_STATS)
				printf("TXER: 0x%x\n",
				       (tx_bd[i].len_status &OETH_TX_BD_STATS));
	  
			if (print_packet_contents)
				printf("T%d\n",i);
		}
	}
	return;  
}


int main ()
{
  
	print_packet_contents = 0; // Default to not printing packet contents.
	packet_inspect_debug = 0;
	print_ethmac_debug_reg = 0;

	/* Initialise vector handler */
	int_init();

	/* Install ethernet interrupt handler, it is enabled here too */
	int_add(ETH0_IRQ, oeth_interrupt, 0);

	/* Enable interrupts */
	cpu_enable_user_interrupts();
    
	last_char=0; /* Variable init for spin_cursor() */
	next_tx_buf_num = 4; /* init for tx buffer counter */

	uart_init(DEFAULT_UART); // init the UART before we can printf
	printf("\n\teth ping program\n\n");
	printf("\n\tboard IP: %d.%d.%d.%d\n",our_ip[0]&0xff,our_ip[1]&0xff,
	       our_ip[2]&0xff,our_ip[3]&0xff);
  
	ethmac_setup(); /* Configure MAC, TX/RX BDs and enable RX and TX in 
			   MODER */
  
	//scan_ethphys(); /* Scan MIIM bus for PHYs */
	//ethphy_init(); /* Attempt reset and configuration of PHY via MIIM */
	//ethmac_scanstatus(); /* Enable scanning of status register via MIIM */

	/* Loop, monitoring user input from TTY */
	while(1)  
	{
		char c;
      
		while(!uart_check_for_char(DEFAULT_UART))
		{
			
			spin_cursor();
			
			if (print_ethmac_debug_reg)
				oeth_print_wbdebug();
		}
      
		c = uart_getc(DEFAULT_UART);

		if (c == 's')
			tx_packet((void*) ping_packet, 98);
		else if (c == 'S')
			tx_packet((void*)big_ping_packet, 1514);
		else if (c == 'h')
			scan_ethphys();
		else if (c == 'i')
			ethphy_init();
		else if (c == 'c')
			oeth_ctrlmode_switch();
		else if (c == 'P')
		{
			print_packet_contents = print_packet_contents ? 0 : 1;
			if (print_packet_contents)
				printf("Enabling packet dumping\n");
			else
				printf("Packet dumping disabled\n");
		}
		else if (c == 'p')
			oeth_printregs();
		else if (c == 'a')
			ethphy_print_status(ethphy_found);
		else if (c >= '0' && c <= '9')
			scan_ethphy(c - 0x30);
		else if (c == 'r')
		{
			ethphy_reset(ethphy_found);
			printf("PHY reset\n");
		}
		else if (c == 'R')
		{
			//oeth_reset_tx_bd_pointer();
			ethmac_setup();
			printf("MAC reset\n");
		}
		else if (c == 'n')
			ethphy_reneg(ethphy_found);
		else if (c == 'N')
			ethphy_toggle_autoneg(ethphy_found);
		else if (c == 'm')
			ethmac_togglehugen();
		else if (c == 't')
			ethphy_set_10mbit(ethphy_found);
		else if (c == 'H')
			ethphy_set_100mbit(ethphy_found);
		else if ( c == 'b' )
		{
			printf("\n\t---\n");
			oeth_dump_bds();
			printf("\t---\n");
		}
      
		else if ( c == 'B' )
		{
			tx_packet((void*) broadcast_ping_packet, 298);
		}
		else if (c == 'u' )
			oeth_transmit_pause();
		else if (c == 'd' )
		{
			oeth_print_wbdebug();
			print_ethmac_debug_reg = !print_ethmac_debug_reg;
		}
		else if (c == 'D')
		{
			marvell_phy_toggle_delay();
		}
		else if (c == 'v' )
		{
			if (packet_inspect_debug)
				printf("Disabling ");
			else 
				printf("Enabling ");
			
			printf("packet type announcments\n");
			
			packet_inspect_debug = !packet_inspect_debug;
		}
		else if ( c == 'o' )
			oeth_toggle_promiscuous();
		else if (c == 'L')
			ethphy_toggle_loopback();
		else if (c == 'g')
			ethphy_toggle_gigadvertise();

	}

}
