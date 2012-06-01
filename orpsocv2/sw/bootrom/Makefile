# Makefile for bootROM Verilog
SW_ROOT=..
include $(SW_ROOT)/Makefile.inc

# Whenever PRELOAD_RAM is defined on command line, we probably want the bootrom
# to be jumping to reset instead of doing a full boot preload.
ifeq ($(PRELOAD_RAM),1)
OR32_CFLAGS += -DPRELOAD_RAM
endif

all: bootrom.v

%.v: %.bin $(SW_ROOT)/utils/bin2vlogarray
	$(Q)$(SW_ROOT)/utils/bin2vlogarray < $< > $@

%.dis: %.o
	$(Q)$(OR32_OBJDUMP) -d $< > $@

%.bin: %.o
	$(Q)$(OR32_OBJCOPY) -O binary $< $@

$(SW_ROOT)/utils/bin2vlogarray:
	$(Q)$(MAKE) -C $(SW_ROOT)/utils bin2vlogarray

clean:
	$(Q)rm -f *.o *.bin *.hex *.in *.dis *.v

