/* Generated by LinkerScriptGenerator [http://visualgdb.com/tools/LinkerScriptGenerator]
 * Target: STM32F407VG
 * The file is provided under the BSD license.
 */

ENTRY(Reset_Handler)

MEMORY
{
	FLASH_BOOT (RX)   : ORIGIN = 0x08000000, LENGTH = 16K
	STORAGE0 (RW) : ORIGIN = 0x08004000, LENGTH = 16K
	STORAGE1 (RW) : ORIGIN = 0x08008000, LENGTH = 16K
	STORAGE2 (RW) : ORIGIN = 0x0800C000, LENGTH = 16K
	FLASH (RX)   : ORIGIN = 0x08010000, LENGTH = 960K
	SRAM (RWX)   : ORIGIN = 0x20000000, LENGTH = 128K
	CCMRAM (RWX) : ORIGIN = 0x10000000, LENGTH = 64K
}

_estack = 0x20020000;

SECTIONS
{
	.isr_vector :
	{
		. = ALIGN(4);
		KEEP(*(.isr_vector))
		. = ALIGN(4);
	} > FLASH_BOOT

	.storage0  (NOLOAD):
	{
		*(.Storage0*);
	} > STORAGE0

	.storage1  (NOLOAD):
	{
		*(.Storage1*);
	} > STORAGE1

	.storage2  (NOLOAD):
	{
		*(.Storage2*);
	} > STORAGE2


	.text :
	{
		. = ALIGN(4);
		_stext = .;

		*(.text)
		*(.text*)
		*(.rodata)
		*(.rodata*)
		*(.glue_7)
		*(.glue_7t)
		KEEP(*(.init))
		KEEP(*(.fini))
		. = ALIGN(4);
		_etext = .;

	} > FLASH

	.ARM.extab :
	{
		. = ALIGN(4);
		*(.ARM.extab)
		*(.gnu.linkonce.armextab.*)
		. = ALIGN(4);
	} > FLASH

	.exidx :
	{
		. = ALIGN(4);
		PROVIDE(__exidx_start = .);
		*(.ARM.exidx*)
		. = ALIGN(4);
		PROVIDE(__exidx_end = .);
	} > FLASH

	.ARM.attributes :
	{
		*(.ARM.attributes)
	} > FLASH

	.preinit_array :
	{
		PROVIDE(__preinit_array_start = .);
		KEEP(*(.preinit_array*))
		PROVIDE(__preinit_array_end = .);
	} > FLASH

	.init_array :
	{
		PROVIDE(__init_array_start = .);
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array*))
		PROVIDE(__init_array_end = .);
	} > FLASH

	.fini_array :
	{
		PROVIDE(__fini_array_start = .);
		KEEP(*(.fini_array*))
		KEEP(*(SORT(.fini_array.*)))
		PROVIDE(__fini_array_end = .);
	} > FLASH

	.data :
	{
		. = ALIGN(4);
		_sdata = .;

		PROVIDE(__data_start__ = _sdata);
		*(.data)
		*(.data*)
		. = ALIGN(4);
		_edata = .;

		PROVIDE(__data_end__ = _edata);
	} > SRAM AT >FLASH

	_sidata = LOADADDR(.data);

	.bss :
	{
		. = ALIGN(4);
		_sbss = .;

		PROVIDE(__bss_start__ = _sbss);
		*(.bss)
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		_ebss = .;

		PROVIDE(__bss_end__ = _ebss);
	} > SRAM

	PROVIDE(end = .);

	.heap (NOLOAD) :
	{
		. = ALIGN(4);
		PROVIDE(__heap_start__ = .);
		KEEP(*(.heap))
		. = ALIGN(4);
		PROVIDE(__heap_end__ = .);
	} > SRAM

	.reserved_for_stack (NOLOAD) :
	{
		. = ALIGN(4);
		PROVIDE(__reserved_for_stack_start__ = .);
		KEEP(*(.reserved_for_stack))
		. = ALIGN(4);
		PROVIDE(__reserved_for_stack_end__ = .);
	} > SRAM

}

