/*
 *
 * USB usbhostslave core slave register defines
 *
 * Julius Baxter, julius@opencores.org
 *
 */

#ifndef _USBHOSTSLAVE_SLAVE_H_
#define _USBHOSTSLAVE_SLAVE_H_

extern const int USBHOSTSLAVE_SLAVE_CORE_ADR[2];

void usb_slave_set_addr(int, char);
void usb_slave_global_enable_endpoints(int);
void usb_slave_global_disable_endpoints(int);
void usb_slave_endpoint_enable(int, int);
void usb_slave_endpoint_disable(int, int);
void usb_slave_endpoint_ready(int, int);
void usb_slave_endpoint_unready(int, int);
void usb_slave_endpoint_outdataseqset(int, int, int);
void usb_slave_endpoint_sendstallset(int, int, int);
void usb_slave_endpoint_isoset(int, int, int);
int  usb_slave_get_frame_num(int);
void usb_slave_endpoint_tx_fifo_write(int, int, char);
char usb_slave_endpoint_rx_fifo_read_data(int, int);
int  usb_slave_endpoint_rx_fifo_read_count(int, int);
void usb_slave_endpoint_rx_fifo_clear(int, int);

#define USBSLAVE_EP0_CONTROL_REG              0x40
#define USBSLAVE_EP0_STATUS_REG               0x41
#define USBSLAVE_EP0_TRANSTYPE_STATUS_REG     0x42
#define USBSLAVE_EP0_NAK_TRANSTYPE_STATUS_REG 0x43

#define USBSLAVE_EP1_CONTROL_REG              0x44
#define USBSLAVE_EP1_STATUS_REG               0x45
#define USBSLAVE_EP1_TRANSTYPE_STATUS_REG     0x46
#define USBSLAVE_EP1_NAK_TRANSTYPE_STATUS_REG 0x47

#define USBSLAVE_EP2_CONTROL_REG              0x48
#define USBSLAVE_EP2_STATUS_REG               0x49
#define USBSLAVE_EP2_TRANSTYPE_STATUS_REG     0x4a
#define USBSLAVE_EP2_NAK_TRANSTYPE_STATUS_REG 0x4b

#define USBSLAVE_EP3_CONTROL_REG              0x4c
#define USBSLAVE_EP3_STATUS_REG               0x4d
#define USBSLAVE_EP3_TRANSTYPE_STATUS_REG     0x4e
#define USBSLAVE_EP3_NAK_TRANSTYPE_STATUS_REG 0x4f

#define USBSLAVE_SC_CONTROL_REG               0x50
#define USBSLAVE_SC_LINE_STATUS_REG           0x51
#define USBSLAVE_SC_INTERRUPT_STATUS_REG      0x52
#define USBSLAVE_SC_INTERRUPT_MASK_REG        0x53
#define USBSLAVE_SC_ADDRESS                   0x54
#define USBSLAVE_SC_FRAME_NUM_MSP             0x55
#define USBSLAVE_SC_FRAME_NUM_LSP             0x56

#define USBSLAVE_EP0_RX_FIFO_DATA             0x60
#define USBSLAVE_EP0_RX_FIFO_DATA_CNT_MSB     0x62
#define USBSLAVE_EP0_RX_FIFO_DATA_CNT_LSB     0x63
#define USBSLAVE_EP0_RX_FIFO_CONTROL_REG      0x64

#define USBSLAVE_EP0_TX_FIFO_DATA             0x70
#define USBSLAVE_EP0_TX_FIFO_CONTROL_REG      0x74

#define USBSLAVE_EP1_RX_FIFO_DATA             0x80
#define USBSLAVE_EP1_RX_FIFO_DATA_CNT_MSB     0x82
#define USBSLAVE_EP1_RX_FIFO_DATA_CNT_LSB     0x83
#define USBSLAVE_EP1_RX_FIFO_CONTROL_REG      0x84

#define USBSLAVE_EP1_TX_FIFO_DATA             0x90
#define USBSLAVE_EP1_TX_FIFO_CONTROL_REG      0x94

#define USBSLAVE_EP2_RX_FIFO_DATA             0xa0
#define USBSLAVE_EP2_RX_FIFO_DATA_CNT_MSB     0xa2
#define USBSLAVE_EP2_RX_FIFO_DATA_CNT_LSB     0xa3
#define USBSLAVE_EP2_RX_FIFO_CONTROL_REG      0xa4

#define USBSLAVE_EP2_TX_FIFO_DATA             0xb0
#define USBSLAVE_EP2_TX_FIFO_CONTROL_REG      0xb4

#define USBSLAVE_EP3_RX_FIFO_DATA             0xc0
#define USBSLAVE_EP3_RX_FIFO_DATA_CNT_MSB     0xc2
#define USBSLAVE_EP3_RX_FIFO_DATA_CNT_LSB     0xc3
#define USBSLAVE_EP3_RX_FIFO_CONTROL_REG      0xc4

#define USBSLAVE_EP3_TX_FIFO_DATA             0xd0
#define USBSLAVE_EP3_TX_FIFO_CONTROL_REG      0xd4

#define RX_FIFO_DATA 0x20
#define TX_CONTROL_REG  0x00
#define TX_TRANS_TYPE_REG 0x01 
#define TX_ADDR_REG  0x04
#define TX_ENDP_REG  0x05
#define TX_FIFO_DATA 0x30 
#define INTERRUPT_MASK_REG 0x09 
#define RX_CONNECT_STATE_REG 0x0e
#define INTERRUPT_STATUS_REG 0x08
#define HOST_SLAVE_CONTROL_REG 0xe0
#define HOST_SLAVE_VERSION_REG      0xe1
#define TX_LINE_CONTROL_REG 0x02
// Bit masks for registers

#define USBSLAVE_CONTROL_REG_ENDPOINT_ENABLE          0x01
#define USBSLAVE_CONTROL_REG_ENDPOINT_READY           0x02
#define USBSLAVE_CONTROL_REG_ENDPOINT_OUTDATA_SEQ     0x04
#define USBSLAVE_CONTROL_REG_ENDPOINT_SEND_STALL      0x08
#define USBSLAVE_CONTROL_REG_ENDPOINT_ISO_ENABLE      0x10

#define USBSLAVE_STATUS_REG_SC_CRC_ERROR              0x01
#define USBSLAVE_STATUS_REG_SC_BIT_STUFF_ERROR        0x02
#define USBSLAVE_STATUS_REG_SC_RX_OVERFLOW            0x04
#define USBSLAVE_STATUS_REG_SC_RX_TIME_OUT            0x08
#define USBSLAVE_STATUS_REG_SC_NAK_SENT               0x10
#define USBSLAVE_STATUS_REG_SC_STALL_SENT             0x20
#define USBSLAVE_STATUS_REG_SC_ACK_RXED               0x40
#define USBSLAVE_STATUS_REG_SC_DATA_SEQ               0x80

#define USBSLAVE_TRANSTYPE_STATUS_REG_TRANS_TYPE_MASK    0x03
#define USBSLAVE_TRANSTYPE_STATUS_REG_TRANS_TYPE_SETUP   0x00
#define USBSLAVE_TRANSTYPE_STATUS_REG_TRANS_TYPE_IN      0x01
#define USBSLAVE_TRANSTYPE_STATUS_REG_TRANS_TYPE_OUTDATA 0x02

#define USBSLAVE_SC_CONTROL_REG_GLOBAL_ENABLE            0x01
#define USBSLAVE_SC_CONTROL_REG_TX_LINE_STATE            0x06
#define USBSLAVE_SC_CONTROL_REG_DIRECT_CONTROL           0x08
#define USBSLAVE_SC_CONTROL_REG_FULL_SPEED_LINE_POLARITY 0x10
#define USBSLAVE_SC_CONTROL_REG_FULL_SPEED_LINE_BITRATE  0x20
#define USBSLAVE_SC_CONTROL_REG_CONNECT_TO_HOST          0x40

#define USBSLAVE_SC_LINE_STATUS_REG_RX_LINE_STATE        0x3
#define USBSLAVE_SC_LINE_STATUS_REG_VBUS_STATE           0x4

#define USBSLAVE_SC_LINE_STATUS_REG_RX_LINE_STATE_RESET  0x0
#define USBSLAVE_SC_LINE_STATUS_REG_RX_LINE_STATE_LSPEED 0x1
#define USBSLAVE_SC_LINE_STATUS_REG_RX_LINE_STATE_FSPEED 0x2

#define USBSLAVE_SC_INTERRUPT_STATUS_REG_TRANS_DONE      0x01
#define USBSLAVE_SC_INTERRUPT_STATUS_REG_RESUME_INT      0x02
#define USBSLAVE_SC_INTERRUPT_STATUS_REG_RESET_EVENT     0x04
#define USBSLAVE_SC_INTERRUPT_STATUS_REG_SOF_RECEIVED    0x08
#define USBSLAVE_SC_INTERRUPT_STATUS_REG_NAK_SENT        0x10
#define USBSLAVE_SC_INTERRUPT_STATUS_REG_VBUS_DETECT     0x20

#define TRANS_DONE_MASK 0x01
#define RESUME_INT_MASK 0x02
#define CONNECTION_EVENT 0x04
#define SOF_SENT_BIT 0x08

#define USBSLAVE_FIFO_CONTROL_REG_FORCE_EMPTY            0x1

#endif

