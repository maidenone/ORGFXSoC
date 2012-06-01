/* cfi_ctrl driver header */

#define CFI_CTRL_UNLOCKBLOCK_OFFSET  0x04000000
#define CFI_CTRL_ERASEBLOCK_OFFSET   0x08000000
#define CFI_CTRL_REGS_OFFSET         0x0c000000
#define CFI_CTRL_DEVICEIDENT_OFFSET  0x0e000000
#define CFI_CTRL_CFIQUERY_OFFSET     0x0e010000

#define CFI_CTRL_SCR_OFFSET (CFI_CTRL_REGS_OFFSET + 0)
#define CFI_CTRL_FSR_OFFSET (CFI_CTRL_REGS_OFFSET + 4)

#define CFI_CTRL_SCR_CONTROLLER_BUSY (1 << 0)
#define CFI_CTRL_SCR_CLEAR_FSR       (1 << 1)
#define CFI_CTRL_SCR_RESET_DEVICE    (1 << 2)

/* Flash status register (FSR) bit meanings - from CFI standard */
#define CFI_FSR_DWS (1<<7) /* Device write status. 0 - busy, 1 - ready */
#define CFI_FSR_ERR (1<<6) /* Erase suspend status - N/A here */
#define CFI_FSR_ES  (1<<5) /* Erase status. 0 - successful, 1 - fail/seq err. */
#define CFI_FSR_PS  (1<<4) /* Program status. 0 - successful, 1 - fail/seq err*/
#define CFI_FSR_VPPS (1<<3) /* VPP status. N/A here */
#define CFI_FSR_PSS (1<<2) /* Program suspend status. N/A here */
#define CFI_FSR_BLS (1<<1) /* Block-locked status */
#define CFI_FSR_BWS (1<<0) /* Buffer-enhanced programming status - N/A here */


/* Driver function prototypes */
void cfi_ctrl_reset_flash(void);
void cfi_ctrl_clear_status(void);
int cfi_ctrl_busy(void);
unsigned char cfi_ctrl_get_status(void);
void cfi_ctrl_unlock_block(unsigned int addr);
int cfi_ctrl_erase_block(unsigned int addr);
void cfi_ctrl_erase_block_no_wait(unsigned int addr);
int cfi_ctrl_write_short(short data, unsigned int addr);
short cfi_ctrl_read_identifier(unsigned int addr);
short cfi_ctrl_query_info(unsigned int addr);
void cfi_ctrl_enable_data_read(void);
