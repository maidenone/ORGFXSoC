
common_testfloat_sources = fail.c		\
			random.c	\
			writeHex.c	\
			softfloat.c	\
			testCases.c	\
			testLoops.c

testsoftfloat_SOURCES    = slowfloat.c	\
			testsoftfloat.c

testfloat_SOURCES    = 	systmodes.c	\
			systflags.c	\
			systfloat.S	\
			testFunction.c	\
			testfloat.c

ELF_DEPENDS= $(common_testfloat_sources) $(testfloat_SOURCES)

SW_ROOT=../..

include $(SW_ROOT)/Makefile.inc


#testfloat.elf: 
#	$(Q)$(OR32_CC) $^ -o $@


%.dis: %.elf
	$(Q)$(OR32_OBJDUMP) -d $< > $@

%.bin: %.elf
	$(Q)$(OR32_OBJCOPY) -O binary $< $@

clean:
	$(Q)rm -f *.bin *.vmem *.flashin *.dis

