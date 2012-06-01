
/*
 *
 * USB usbhostslave core host functions
 *
 * Julius Baxter, julius@opencores.org
 *
 */

#include "cpu-utils.h"
#include "board.h"
#include "usbhostslave-host.h"

const int USBHOSTSLAVE_HOST_CORE_ADR[2] = { USB0_BASE, USB1_BASE };

// ------------------------ usbInit -----------------------------
char usb_host_init(int core)
{
	volatile int i;

	// Reset the thing
	REG8(USBHOSTSLAVE_HOST_CORE_ADR[core] + RA_HOST_SLAVE_MODE) = 0x2;

	// Wait 10 USB cycles ( this should be plenty)
	for (i = 0; i < 8; i++) ;

	REG8(USBHOSTSLAVE_HOST_CORE_ADR[core] + RA_HC_INTERRUPT_MASK_REG) = 0x00;	// Disable interrupts

	REG8(USBHOSTSLAVE_HOST_CORE_ADR[core] + RA_HC_INTERRUPT_STATUS_REG) = 0xff;	// Clear interrupt statuses

	REG8(USBHOSTSLAVE_HOST_CORE_ADR[core] + RA_HC_TX_LINE_CONTROL_REG) = 0x00;	// low speed normal

	REG8(USBHOSTSLAVE_HOST_CORE_ADR[core] + RA_HC_TX_SOF_ENABLE_REG) = 0x00;	// No SOF

	REG8(USBHOSTSLAVE_HOST_CORE_ADR[core] + RA_HOST_SLAVE_MODE) = 0x01;	// Set core to HOST mode

	// Reset RX FIFO buffer
	REG8(USBHOSTSLAVE_HOST_CORE_ADR[core] + RA_HC_RX_FIFO_CONTROL_REG) =
	    0xff;
	// Reset TX FIFO buffer
	REG8(USBHOSTSLAVE_HOST_CORE_ADR[core] + RA_HC_TX_FIFO_CONTROL_REG) =
	    0xff;

	// Return version number reg
	return REG8(USBHOSTSLAVE_HOST_CORE_ADR[core] + RA_HOST_SLAVE_VERSION);

}
