# Program to control the cfi_ctrl module to manage contents of flash memory.
#
# author: julius.baxter@orsoc.se
#

SW_ROOT=../..

#PROGRAMMINGFILE=
PROGRAMMINGFILE_SWBIN=progfile.swbin # Binary file with sizeword
PROGRAMMINGFILE_DATA=progfile.o

ELF_DEPENDS +=$(PROGRAMMINGFILE_DATA)

include $(SW_ROOT)/Makefile.inc

OR32_LDFLAGS = -L$(SW_ROOT)/lib -lorpsoc -lgcc -Tcfi_ctrl_programmer.ld -e 256

$(PROGRAMMINGFILE_DATA): $(PROGRAMMINGFILE)
	$(Q)$(OR32_LD) -r -b binary -o $@ $<
	$(Q)$(OR32_OBJCOPY) --rename-section .data=.userprogram $@

%.dis: %.elf
	$(Q)$(OR32_OBJDUMP) -d $< > $@

%.bin: %.elf
	$(Q)$(OR32_OBJCOPY) -O binary $< $@

clean:
	$(Q)rm -f *.elf *.bin *.vmem *.flashin *.dis $(PROGRAMMINGFILE_SWBIN) $(PROGRAMMINGFILE_DATA)

#distclean: clean
