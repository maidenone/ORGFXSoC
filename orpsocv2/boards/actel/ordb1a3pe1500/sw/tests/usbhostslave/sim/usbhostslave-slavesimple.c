/*
 *
 * Simple USB slave test
 *
 * Adam Edvardsson, adam.edvardsson@orsoc.se
 * Julius Baxter, julius.baxter@orsoc.se
 *
 */

#include "cpu-utils.h"
#include "spr-defs.h"
#include "board.h"
#include "usbhostslave-slave.h"
#include "int.h"
#include "printf.h"
volatile int i;

// We rely on USB1 being a slave, check it's in the design.
#include "orpsoc-defines.h"

# ifndef USB1
#  error
#  error USB1 module not in design - enable it to run USB slave tests
#  error
# else
// Using USB1, check that it's a HOST
#  ifdef USB1_ONLY_HOST
#   error
#   error Module USB1 is a host only - must have slave capability to run tests
#   error
#  else
#   define USBSLAVECORE 1
#  endif
# endif

int main()
{

	int usb_core = USBSLAVECORE;

	volatile unsigned char interrupt_status;

	// disable all endpoints first
	usb_slave_global_disable_endpoints(usb_core);

	printf("\nUSB Slave test for usb core %d\n", USBSLAVECORE);

	//Enable slave Setup slave
	//Enable fullspeed, 
	//Connect to host (D+ pullup)
	REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] + USBSLAVE_SC_CONTROL_REG) =
	    0x71;
	//Set Slave addres
	usb_slave_set_addr(usb_core, 0x63);
	//Enable endpont
	REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] + USBSLAVE_EP0_CONTROL_REG) =
	    0x03;
	//Enable interupt
	REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
	     USBSLAVE_SC_INTERRUPT_MASK_REG) = 0xff;

	//Test 1, check for VBUS connection
	//Poll for VBUS interupt.  
	interrupt_status = REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
				USBSLAVE_SC_INTERRUPT_STATUS_REG);

	while ((interrupt_status & USBSLAVE_SC_INTERRUPT_STATUS_REG_VBUS_DETECT)
	       != USBSLAVE_SC_INTERRUPT_STATUS_REG_VBUS_DETECT) {

		interrupt_status = REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
					USBSLAVE_SC_INTERRUPT_STATUS_REG);

		interrupt_status = REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
					USBSLAVE_SC_INTERRUPT_STATUS_REG);

	}

	//VBUS change detected 

	// Clear interrupt
	REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
	     USBSLAVE_SC_INTERRUPT_STATUS_REG)
	    = USBSLAVE_SC_INTERRUPT_STATUS_REG_VBUS_DETECT;

	// Check so VBUS is present
	if ((REG8
	     (USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
	      USBSLAVE_SC_LINE_STATUS_REG) &
	     USBSLAVE_SC_LINE_STATUS_REG_VBUS_STATE)
	    == USBSLAVE_SC_LINE_STATUS_REG_VBUS_STATE)
	{
		report(0x00000001);
	} else {
		report(0x00000001);
		exit(0xBAAAAAAD);
	}

	//Test 2, check for bus-reset and fullspeed present  
	interrupt_status = REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
				USBSLAVE_SC_INTERRUPT_STATUS_REG);
	while ((interrupt_status & USBSLAVE_SC_INTERRUPT_STATUS_REG_RESET_EVENT)
	       != USBSLAVE_SC_INTERRUPT_STATUS_REG_RESET_EVENT) {
		interrupt_status = REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
					USBSLAVE_SC_INTERRUPT_STATUS_REG);
		interrupt_status = REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
					USBSLAVE_SC_INTERRUPT_STATUS_REG);

	}

	//Bus reset 
	// Clear interrupt
	REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
	     USBSLAVE_SC_INTERRUPT_STATUS_REG)
	    = USBSLAVE_SC_INTERRUPT_STATUS_REG_RESET_EVENT;

	//Check so Fullspeed is activated
	if ((REG8
	     (USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
	      USBSLAVE_SC_LINE_STATUS_REG) &
	     USBSLAVE_SC_LINE_STATUS_REG_RX_LINE_STATE_FSPEED)
	    == USBSLAVE_SC_LINE_STATUS_REG_RX_LINE_STATE_FSPEED) {
		report(0x00000002);
	} else {
		report(0x00000002);
		exit(0xBAAAAAAD);
	}

	// Test 3, check for first transfer completion and check for correct data, 
	// transfertype and nr byte sent.
	//
	interrupt_status = REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
				USBSLAVE_SC_INTERRUPT_STATUS_REG);
	while ((interrupt_status & USBSLAVE_SC_INTERRUPT_STATUS_REG_TRANS_DONE)
	       != USBSLAVE_SC_INTERRUPT_STATUS_REG_TRANS_DONE) {
		interrupt_status = REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
					USBSLAVE_SC_INTERRUPT_STATUS_REG);
		interrupt_status = REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
					USBSLAVE_SC_INTERRUPT_STATUS_REG);

	}

	//Transfer complete, check for correct typ of transfer (SETUP_TRANSFER)
	if (REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
		 USBSLAVE_EP0_TRANSTYPE_STATUS_REG) != 0) {
		report(0x10000003);
		exit(0xBAAAAAAD);
	}
	//Check nr byte sent
	if (REG8
	    (USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
	     USBSLAVE_EP0_RX_FIFO_DATA_CNT_LSB) != 2) {
		report(0x20000003);
		exit(0xBAAAAAAD);
	}
	//Check recived data 
	if ((REG8
	     (USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
	      USBSLAVE_EP0_RX_FIFO_DATA) == 0)
	    &&
	    (REG8
	     (USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
	      USBSLAVE_EP0_RX_FIFO_DATA) == 1)) {
		report(0x00000003);
	} else {
		report(0x00000003);
		report(0xBAAAAAAD);
		exit(0);
	}

	//Enable endpoint again
	//Enable fullspeed, 

	//Connect to host (D+ pullup)
	REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] + USBSLAVE_SC_CONTROL_REG) =
	    0x71;

	//Enable endpont
	REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] + USBSLAVE_EP0_CONTROL_REG) =
	    0x03;

	// Clear interrupt
	REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
	     USBSLAVE_SC_INTERRUPT_STATUS_REG)
	    = USBSLAVE_SC_INTERRUPT_STATUS_REG_TRANS_DONE;

	// Test 4, check for first transfer completion and check for correct data, 
	// transfertype and nr byte sent.
	//
	interrupt_status = REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
				USBSLAVE_SC_INTERRUPT_STATUS_REG);
	while ((interrupt_status & USBSLAVE_SC_INTERRUPT_STATUS_REG_TRANS_DONE)
	       != USBSLAVE_SC_INTERRUPT_STATUS_REG_TRANS_DONE) {
		interrupt_status =
		    REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
			 USBSLAVE_SC_INTERRUPT_STATUS_REG);
		interrupt_status =
		    REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
			 USBSLAVE_SC_INTERRUPT_STATUS_REG);

	}

	//Transfer complete, check for correct typ of transfer (OUT_TRANSFER)
	if (REG8
	    (USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
	     USBSLAVE_EP0_TRANSTYPE_STATUS_REG)
	    != USBSLAVE_TRANSTYPE_STATUS_REG_TRANS_TYPE_OUTDATA) {
		report(0x10000004);
		exit(0xBAAAAAAD);
	}
	//Check nr byte sent
	if (REG8
	    (USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
	     USBSLAVE_EP0_RX_FIFO_DATA_CNT_LSB) != 20) {
		report(0x20000004);
		exit(0xBAAAAAAD);
	}
	//Check recived data 
	for (i = 0; i < 20; i++) {

		if (REG8
		    (USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
		     USBSLAVE_EP0_RX_FIFO_DATA) != i) {
			report(0x00000004);
			report(0xBAAAAAAD);
			exit(0);
		}
	}
	report(0x00000004);

	//Test 5, check for NAK interupt, and type of transfer.
	//
	interrupt_status = REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
				USBSLAVE_SC_INTERRUPT_STATUS_REG);
	while ((interrupt_status & USBSLAVE_SC_INTERRUPT_STATUS_REG_NAK_SENT)
	       != USBSLAVE_SC_INTERRUPT_STATUS_REG_NAK_SENT) {
		interrupt_status = REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
					USBSLAVE_SC_INTERRUPT_STATUS_REG);
		interrupt_status = REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
					USBSLAVE_SC_INTERRUPT_STATUS_REG);

	}

	//Transfer NAK, check for correct typ of transfer (OUT_TRANSFER)     
	if (REG8
	    (USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
	     USBSLAVE_EP0_NAK_TRANSTYPE_STATUS_REG)
	    != USBSLAVE_TRANSTYPE_STATUS_REG_TRANS_TYPE_OUTDATA) {
		report(0x10000005);
		exit(0xBAAAAAAD);
	}

	report(0x00000005);

	// Clear all interrupt
	REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
	     USBSLAVE_SC_INTERRUPT_STATUS_REG) = 0x3f;

	//Test 6, IN-transfer to endpoint 2
	//Connect to host (D+ pullup)
	REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] + USBSLAVE_SC_CONTROL_REG) =
	    0x71;
	usb_slave_set_addr(usb_core, 0x63);
	//Enable endpoint
	REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] + USBSLAVE_EP2_CONTROL_REG) =
	    0x03;

	//Fill TX_FIFO
	for (i = 0; i <= 20; i++) {
		REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
		     USBSLAVE_EP2_TX_FIFO_DATA) = i;
	}

	interrupt_status = REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
				USBSLAVE_SC_INTERRUPT_STATUS_REG);

	while ((interrupt_status & USBSLAVE_SC_INTERRUPT_STATUS_REG_TRANS_DONE)
	       != USBSLAVE_SC_INTERRUPT_STATUS_REG_TRANS_DONE) {
		interrupt_status = REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
					USBSLAVE_SC_INTERRUPT_STATUS_REG);
		interrupt_status = REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
					USBSLAVE_SC_INTERRUPT_STATUS_REG);

	}

	//Transfer complete, check for correct typ of transfer (IN_TRANSFER)

	if ((REG8
	     (USBHOSTSLAVE_SLAVE_CORE_ADR[usb_core] +
	      USBSLAVE_EP2_TRANSTYPE_STATUS_REG)
	     & USBSLAVE_TRANSTYPE_STATUS_REG_TRANS_TYPE_IN) !=
	    USBSLAVE_TRANSTYPE_STATUS_REG_TRANS_TYPE_IN) {
		report(0x10000006);
		exit(0xBAAAAAAD);
	}

	//Fill TX_FIFO
	//while(1){}

	//When transfer request arrive, fill TX-FIFO 0-20, and start transfer
	// data = 8'h00;
	//for (i=0; i<dataSize; i=i+1) begin
	//   testHarness.u_wb_master_model.wb_write(1, `SIM_SLAVE_BASE_ADDR + `EP2_TX_FIFO_BASE + `FIFO_DATA_REG , data);
	//  data = data + 1'b1;
	//  end
	// testHarness.u_wb_master_model.wb_write(1, `SIM_HOST_BASE_ADDR + `HCREG_BASE+`TX_CONTROL_REG , 8'h01);

	// Finish simulation
	exit(0x8000000d);
}
