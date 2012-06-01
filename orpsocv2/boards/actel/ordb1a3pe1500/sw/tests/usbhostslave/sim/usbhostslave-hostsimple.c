/*
 *
 * USB simple host test
 *
 * Adam Edvardsson, adam.edvardsson@orsoc.se
 * Julius Baxter, julius.baxter@orsoc.se
 *
 */

#include "cpu-utils.h"
#include "spr-defs.h"
#include "board.h"
#include "usbhostslave-host.h"
#include "int.h"
#include "printf.h"

// Ensure we have USB in the design, or we cannot run this test
#include "orpsoc-defines.h"

#ifndef USB0
# ifndef USB1
#  error
#  error No USB module in design!
#  error
# else
// Using USB1, check that it's a HOST
#  ifdef USB1_ONLY_SLAVE
#   error
#   error Module USB1 is a slave only, cannot run host tests
#   error
#  else
#   define USBHOSTCORE 1
#  endif
# endif
#else
# define USBHOSTCORE 0
#endif

#ifndef USBHOSTCORE
# ifndef USB1
#  ifndef USB0
#   error
#   error No USB module in design!
#   error
#  else
#   define USBHOSTCORE 0
#  endif
# endif
#endif

volatile unsigned char int_status;

#if 1
#    define debug printf
#else
#    define debug
#endif

//Interupt handler just read status register and return, simply to test board connectivity
void usb_int_handler(void *corenum);
int send_packet(char usb_core, char USBAddress, char USBEndPoint,
		char transType, char dSize);
void usb_int_handler(void *corenum)
{
	char status;
	int core = USBHOSTCORE;	//*((int*)corenum); // For some reason this doesn't work!
	status =
	    REG8(USBHOSTSLAVE_HOST_CORE_ADR[core] + RA_HC_INTERRUPT_STATUS_REG);

	//clear interrupt
	REG8(USBHOSTSLAVE_HOST_CORE_ADR[core] + RA_HC_INTERRUPT_STATUS_REG) =
	    0xff;

	int_status = status;

	return;

}

/*T0, Check for correct values in registers
  Return failed register
  Print value of failed register
*/
int T1(usb_core)
{
	int ret = 0;
	int i;
	debug("T1 Register Init Val \n");
	volatile unsigned char usb_data_rw;
	//Initilisation test, read all register       
	REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] + 0xE0) = 0x02;	//RESET
	for (i = HCREG_BASE; i < (HCREG_BASE + 15); i++) {
		usb_data_rw = (REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] + i));
		if (usb_data_rw != 0) {
			ret = i;
			report(usb_data_rw);
		}

	}

	for (i = HOST_RX_FIFO_BASE; i < (HOST_RX_FIFO_BASE + 3); i++) {
		usb_data_rw = (REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] + i));
		if (usb_data_rw != 0) {
			//ret=i;
			report(usb_data_rw);
		}

	}
	for (i = HOST_TX_FIFO_BASE; i < (HOST_TX_FIFO_BASE + 4); i++) {
		usb_data_rw = (REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] + i));
		if (usb_data_rw != 0) {
			ret = i;
			report(usb_data_rw);
		}

	}

	return 0;
}

/*T2, Write to a register and readback. Reset core and read register
  Return failed part
  Print value of failed register
*/
int T2(usb_core)
{
	volatile unsigned char usb_data_rw;
	int ret = 0;

	//HOST MODE    
	REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] + 0xE0) = 0x01;

	debug("T2 Register Write/Read\n");
	REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] + TX_LINE_CONTROL_REG) = 0x18;
	usb_data_rw =
	    REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] +
		 RA_HC_TX_LINE_CONTROL_REG);
	if (usb_data_rw != 0x18) {
		report(usb_data_rw);
		return 1;

	}

	REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] + RA_HOST_SLAVE_MODE) = 0x02;
	usb_data_rw =
	    REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] +
		 RA_HC_TX_LINE_CONTROL_REG);
	if (usb_data_rw != 0x00) {
		report(usb_data_rw);
		return 2;
	}

	return ret;
}

/*T3, Setup fullspeed and check so the connection interupt is generated
  Return failed part (1 = timeout, 2 = wrong connectivity)
  If fail: Print value of connect state register
*/

int T3(usb_core)
{
	int k = 0;
	volatile unsigned char usb_data_rw;

	debug("T3 Enable core %d and setup fullspeed\n", usb_core);

	//debug("Enable core and setup fullspeed  ");
	REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] + RA_HOST_SLAVE_MODE) = 0x01;
	REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] + RA_HC_TX_LINE_CONTROL_REG) =
	    0x18;
	REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] + RA_HC_INTERRUPT_MASK_REG) =
	    0xff;

	//Wait for interupt
	while (k < 100) {
		k++;
	}

	int_status = 0;

	usb_data_rw
	    =
	    REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] +
		 RA_HC_RX_CONNECT_STATE_REG);

	usb_data_rw
	    =
	    REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] +
		 RA_HC_RX_CONNECT_STATE_REG);

	if (usb_data_rw != 0x02) {
		report(usb_data_rw);
		return 1;
	}

	return 0;
}

/*T4, Transfer test 1, send data
 * Device address = 0x00, 2 byte SETUP transaction to Endpoint 0. 
 * Return failed (1 = timeout,)
 * If fail: Print value of connect state register
 */

int T4(usb_core)
{
	int k = 0;
	volatile unsigned char usb_data_rw;
	char USBAddress = 0x00;
	char USBEndPoint = 0x00;
	char transType = 0x00;
	char dSize = 0;

	debug("T4 Transfer test 1\n");
	usb_data_rw =
	    REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] +
		 RA_HC_INTERRUPT_STATUS_REG);
	send_packet(usb_core, USBAddress, USBEndPoint, transType, dSize);

	while ((int_status & 0x01) != 0x01) {
		k++;
		if (k > 1000) {	//timeout    
			report(k);
			return 1;
		}
	}
	int_status = 0;

	return 0;
}

/*T5, Transfer test 2, send data
 * Device address = 0x5a, 64 byte OUT DATA0 transaction to Endpoint 1. "); 
 * Return failed (1 = timeout )
 * If fail: Print value of connect state register
 */

int T5(usb_core)
{
	int k = 0;
	volatile unsigned char usb_data_rw;

	char USBAddress = 0x5a;
	char USBEndPoint = 0x01;
	char transType = 2;
	char dSize = 64;

	debug("T5 Transfer test 2\n");
	usb_data_rw =
	    REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] +
		 RA_HC_INTERRUPT_STATUS_REG);
	send_packet(usb_core, USBAddress, USBEndPoint, transType, dSize);

	while ((int_status & 0x01) != 0x01) {
		k++;
		if (k > 1000) {	//timeout    
			report(k);
			return 1;
		}
	}
	int_status = 0;

	return 0;
}

/*T6, Transfer test 3, recive data,
 * Device address = 0x5a, 64 byte OUT DATA0 transaction to Endpoint 1. "); 
 * Return failed (1 = timeout, 2= To litle data recived)
 * If fail: Print value of connect state register
 */

int T6(usb_core)
{
	int i, k = 0;
	volatile unsigned char usb_data_rw;

	char USBAddress = 0x01;
	char USBEndPoint = 0x02;
	char transType = 1;
	char dSize = 64;

	debug("T6 Transfer test 3\n");
	usb_data_rw =
	    REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] +
		 RA_HC_INTERRUPT_STATUS_REG);
	send_packet(usb_core, USBAddress, USBEndPoint, transType, dSize);

	while ((int_status & 0x01) != 0x01) {
		k++;
		if (k > 1000) {	//timeout    
			report(k);
			return 1;
		}
	}
	int_status = 0;

	usb_data_rw = REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] + 0x23);
	if (usb_data_rw != 64)
		return 2;

	for (i = dSize; i > 0; i = i - 1) {
		usb_data_rw =
		    REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] +
			 RA_HC_RX_FIFO_DATA_REG);
		report(usb_data_rw);

	}
	return 0;

	return 0;
}

int send_packet(char usb_core, char USBAddress, char USBEndPoint,
		char transType, char dSize)
{
	int i;
	int data = 0;
	REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] + RA_HC_TX_ADDR_REG) =
	    USBAddress;
	REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] + RA_HC_TX_ENDP_REG) =
	    USBEndPoint;
	REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] + RA_HC_TX_TRANS_TYPE_REG) =
	    transType;

	for (i = 0; i < dSize; i = i + 1) {
		REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] +
		     RA_HC_TX_FIFO_DATA_REG) = data;
		data = data + 1;
	}

	REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] + RA_HC_TX_CONTROL_REG) =
	    0x01;

}

#define EXPECTED_VERSION 0x20
int main()
{
	char USBAddress = 0x00;
	char USBEndPoint = 0x00;
	char transType = 0x00;
	int dataSize = 4;

	int i;
	char data = 0;
	volatile int k = 0;

	int usb_core = USBHOSTCORE;
	int test_result;
	volatile unsigned char usb_data_rw;

	// init user interrupt handler
	int_init();

#if USBHOSTCORE==0
	/* Install USB host core 0 interrupt handler */
	int_add(USB0_HOST_IRQ, usb_int_handler, (void *)&usb_core);
#endif
#if USBHOSTCORE==1
	/* Install USB host core 1 interrupt handler */
	int_add(USB1_HOST_IRQ, usb_int_handler, (void *)&usb_core);
#endif

	/* Enable interrupts in supervisor register */
	mtspr(SPR_SR, mfspr(SPR_SR) | SPR_SR_IEE);

	debug("\nStarting USB host test\n");

	usb_data_rw =
	    REG8(USBHOSTSLAVE_HOST_CORE_ADR[usb_core] + RA_HOST_SLAVE_VERSION);

	if (usb_data_rw != EXPECTED_VERSION) {
		debug("Wrong core_verison");
		exit(usb_data_rw);
	}
	//Start testing

	test_result = T1(usb_core);
	if (test_result > 0)
		exit(test_result);
	report(0x00000001);

	test_result = T2(usb_core);
	if (test_result > 1)
		exit(test_result);
	report(0x00000002);

	test_result = T3(usb_core);
	if (test_result > 1)
		exit(test_result);
	report(0x00000003);

	test_result = T4(usb_core);
	if (test_result > 1)
		exit(test_result);
	report(0x00000004);

	test_result = T5(usb_core);
	if (test_result > 1)
		exit(test_result);
	report(0x00000005);

	test_result = T6(usb_core);
	if (test_result > 1)
		exit(test_result);
	report(0x00000006);

	exit(0x8000000d);
}
