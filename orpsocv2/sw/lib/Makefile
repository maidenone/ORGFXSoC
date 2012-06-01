SW_ROOT=..
include $(SW_ROOT)/Makefile.inc

LIBORPSOC_SRC= lib-utils.c printf.c

%/$(DRIVER_OBJ):
	$(Q)$(MAKE) -C $* $(DRIVER_OBJ)

LIBORPSOC_OBJS=$(LIBORPSOC_SRC:%.c=%.o)

liborpsoc.a: $(LIBORPSOC_OBJS) $(SW_DRIVER_PATHS:%=%/$(DRIVER_OBJ))
	$(Q)$(OR32_AR) cru $@ $^
	$(Q)$(OR32_RANLIB) $@

clean:
	$(Q)rm -f *.a *.o

# Special rule to help simulation Makefile compile VMEMs from ELFs passed via
# the USER_ELF variable
$(USER_ELF_BIN): $(USER_ELF)
	$(Q)$(OR32_OBJCOPY) -O binary $< $@
$(USER_ELF_VMEM): $(USER_ELF_BIN) $(UTILS_BIN2VMEM)
	$(Q)$(UTILS_BIN2VMEM) $< $(USER_ELF_VMEM_GEN_OPTS) > $@
gen-user-elf-vmem: $(USER_ELF_VMEM)
