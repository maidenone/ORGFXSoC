/*
 *
 * USB usbhostslave core slave functions, very basic
 *
 * Julius Baxter, julius@opencores.org
 *
 */

#include "cpu-utils.h"
#include "board.h"
#include "usbhostslave-slave.h"

const int USBHOSTSLAVE_SLAVE_CORE_ADR[2] = { USB0_BASE, USB1_BASE };

void usb_slave_set_addr(int core, char addr)
{
	REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[core] + USBSLAVE_SC_ADDRESS) = addr;
}

void usb_slave_global_enable_endpoints(int core)
{
	REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[core] + USBSLAVE_SC_CONTROL_REG) |=
	    USBSLAVE_SC_CONTROL_REG_GLOBAL_ENABLE;
}

void usb_slave_global_disable_endpoints(int core)
{
	REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[core] + USBSLAVE_SC_CONTROL_REG) &=
	    ~USBSLAVE_SC_CONTROL_REG_GLOBAL_ENABLE;
}

void usb_slave_endpoint_enable(int core, int ep)
{
	REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[core] + USBSLAVE_EP0_CONTROL_REG +
	     (ep << 2)) |= USBSLAVE_CONTROL_REG_ENDPOINT_ENABLE;
}

void usb_slave_endpoint_disable(int core, int ep)
{
	REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[core] + USBSLAVE_EP0_CONTROL_REG +
	     (ep << 2)) &= ~USBSLAVE_CONTROL_REG_ENDPOINT_ENABLE;
}

void usb_slave_endpoint_ready(int core, int ep)
{
	REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[core] + USBSLAVE_EP0_CONTROL_REG +
	     (ep << 2)) |= USBSLAVE_CONTROL_REG_ENDPOINT_READY;
}

void usb_slave_endpoint_unready(int core, int ep)
{
	REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[core] + USBSLAVE_EP0_CONTROL_REG +
	     (ep << 2)) &= ~USBSLAVE_CONTROL_REG_ENDPOINT_READY;
}

void usb_slave_endpoint_outdataseqset(int core, int ep, int outdata)
{
	if (outdata)
		REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[core] +
		     USBSLAVE_EP0_CONTROL_REG + (ep << 2)) |=
		    USBSLAVE_CONTROL_REG_ENDPOINT_OUTDATA_SEQ;
	else
		REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[core] +
		     USBSLAVE_EP0_CONTROL_REG + (ep << 2)) &=
		    ~USBSLAVE_CONTROL_REG_ENDPOINT_OUTDATA_SEQ;
}

void usb_slave_endpoint_sendstallset(int core, int ep, int sendstall)
{
	if (sendstall)
		REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[core] +
		     USBSLAVE_EP0_CONTROL_REG + (ep << 2)) |=
		    USBSLAVE_CONTROL_REG_ENDPOINT_SEND_STALL;
	else
		REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[core] +
		     USBSLAVE_EP0_CONTROL_REG + (ep << 2)) &=
		    ~USBSLAVE_CONTROL_REG_ENDPOINT_SEND_STALL;
}

void usb_slave_endpoint_isoset(int core, int ep, int iso)
{
	if (iso)
		REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[core] +
		     USBSLAVE_EP0_CONTROL_REG + (ep << 2)) |=
		    USBSLAVE_CONTROL_REG_ENDPOINT_ISO_ENABLE;
	else
		REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[core] +
		     USBSLAVE_EP0_CONTROL_REG + (ep << 2)) &=
		    ~USBSLAVE_CONTROL_REG_ENDPOINT_ISO_ENABLE;
}

int usb_slave_get_frame_num(int core)
{
	int framenum = (int)REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[core] +
				 USBSLAVE_SC_FRAME_NUM_LSP);
	framenum |= (int)(REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[core] +
			       USBSLAVE_SC_FRAME_NUM_MSP) << 8);
	return framenum;
}

void usb_slave_endpoint_tx_fifo_write(int core, int ep, char data)
{
	REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[core] + USBSLAVE_EP0_TX_FIFO_DATA +
	     (ep << 5)) = data;
}

char usb_slave_endpoint_rx_fifo_read_data(int core, int ep)
{
	return REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[core] +
		    USBSLAVE_EP0_RX_FIFO_DATA + (ep << 5));
}

int usb_slave_endpoint_rx_fifo_read_count(int core, int ep)
{
	int count;
	count = (int)REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[core] +
			  USBSLAVE_EP0_RX_FIFO_DATA_CNT_LSB + (ep << 5));
	count |= (int)(REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[core] +
			    USBSLAVE_EP0_RX_FIFO_DATA_CNT_MSB +
			    (ep << 5)) << 8);
	return count;
}

void usb_slave_endpoint_rx_fifo_clear(int core, int ep)
{
	REG8(USBHOSTSLAVE_SLAVE_CORE_ADR[core] +
	     USBSLAVE_EP3_RX_FIFO_CONTROL_REG + (ep << 5)) =
	    USBSLAVE_FIFO_CONTROL_REG_FORCE_EMPTY;
}
