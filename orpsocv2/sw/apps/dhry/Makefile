SW_ROOT=../..

# Force it to be regenerated each time, as the NUM_RUNS is likely to be changed


include $(SW_ROOT)/Makefile.inc

NUM_RUNS ?= 10

OR32_CFLAGS+= -DNUM_RUNS=$(NUM_RUNS)

%.dis: %.elf
	$(Q)$(OR32_OBJDUMP) -d $< > $@

%.bin: %.elf
	$(Q)$(OR32_OBJCOPY) -O binary $< $@

clean:
	$(Q)rm -f *.elf *.bin *.vmem *.flashin *.dis

