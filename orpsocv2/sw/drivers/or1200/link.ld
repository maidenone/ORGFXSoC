OUTPUT_ARCH(or32)
SEARCH_DIR(.)
__DYNAMIC  =  0;

MEMORY
{
	vectors : ORIGIN = 0, LENGTH = 0x1000
	ram     : ORIGIN = 0x1000, LENGTH = (8M - 0x1000)
}


/* Stack information variables */
_min_stack      = 0x2000;   /* 8K - minimum stack space to reserve */


SECTIONS
{	
	.vectors :
	{
		*(.vectors)
	} > vectors
                              
	.text : {
		_stext = .;
		*(.text)
		_etext  =  .;
		__CTOR_LIST__ = .;
		LONG((__CTOR_END__ - __CTOR_LIST__) / 4 - 2)
		*(.ctors)
		LONG(0)
		__CTOR_END__ = .;
		__DTOR_LIST__ = .;
		LONG((__DTOR_END__ - __DTOR_LIST__) / 4 - 2)
		*(.dtors)
		LONG(0)
		__DTOR_END__ = .;
		*(.lit)
		*(.shdata)
		_endtext = .;
	}  > ram

	.rodata : {
		*(.rodata);
		*(.rodata.*)
	} > ram
	
	.shbss :
	{
		*(.shbss)
	} > ram
	
	.talias :
	{ 
	}  > ram
	
	.data : {
		sdata  =  .;
		_sdata  =  .;
		*(.data)
		edata  =  .;
		_edata  =  .;
	} > ram
	
	.bss :
	{
		_bss_start = .;
		*(.bss)
		*(COMMON)
		_bss_end = .;
	} > ram
	
	/* ensure there is enough room for stack */
        .stack (NOLOAD): {
                . = ALIGN(4);
		. = . + _min_stack ;
                . = ALIGN(4);
                stack = . ;
                _stack = . ;
        } > ram
	
	.stab  0 (NOLOAD) : 
	{
		[ .stab ]
	}
	
	.stabstr  0 (NOLOAD) :
	{
		[ .stabstr ]
	}
}
