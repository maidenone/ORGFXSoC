# This makefile allows you generate a program which will program an SPI flash
# part with another program.

# It requires that the PROGRAMMINGFILE variable to set to a binary copy of a
# program to program the SPI flash with. The file must already exist, the script
# will not automagically compile it for you.
# eg. 
# $ make spiflash-program.elf PROGRAMMINGFILE=../gpio/gpio-board.bin

# This binary file is then converted to a version that will be loaded into the
# SPI flash (the size of the image is set in the first word), and then converted
# to an object file with the data in a section called .spiprogram

# A linker script is then used to put this section into the resulting ELF file
# that will do the programming of the SPI flash

# Simply running this program on the processor should program the SPI flash 
# memory.

#
# author: julius.baxter@orsoc.se
#

SW_ROOT=../..

#PROGRAMMINGFILE=
PROGRAMMINGFILE_SWBIN=progfile.swbin # Binary file with sizeword
PROGRAMMINGFILE_DATA=progfile.o

ELF_DEPENDS +=$(PROGRAMMINGFILE_DATA)

include $(SW_ROOT)/Makefile.inc

OR32_LDFLAGS = -L$(SW_ROOT)/lib -lorpsoc -lgcc -Tspiflash-program.ld -e 256

$(SW_ROOT)/utils/bin2binsizeword:
	$(Q)$(MAKE) -C $(SW_ROOT)/utils bin2binsizeword

$(PROGRAMMINGFILE_SWBIN): $(PROGRAMMINGFILE) $(SW_ROOT)/utils/bin2binsizeword
	$(Q)$(SW_ROOT)/utils/bin2binsizeword $< $@

$(PROGRAMMINGFILE_DATA): $(PROGRAMMINGFILE_SWBIN) 
	$(Q)$(OR32_LD) -r -b binary -o $@ $<
	$(Q)$(OR32_OBJCOPY) --rename-section .data=.spiprogram $@

%.dis: %.elf
	$(Q)$(OR32_OBJDUMP) -d $< > $@

%.bin: %.elf
	$(Q)$(OR32_OBJCOPY) -O binary $< $@

clean:
	$(Q)rm -f *.elf *.bin *.vmem *.flashin *.dis $(PROGRAMMINGFILE_SWBIN) $(PROGRAMMINGFILE_DATA)

#distclean: clean
