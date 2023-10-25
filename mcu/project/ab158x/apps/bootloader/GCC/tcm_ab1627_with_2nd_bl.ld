/********************************************************
memory map
    Start Address   Size    CM4 view Devices
    0x0000_0000     64MB    Reserved
    0x0400_0000     224KB   CM4 TCM / cache
    0x0403_8000     800KB   Reserved
    0x0410_0000     40KB    CM4 ROM
    0x0410_A000     984KB   Reserved
    0x0420_0000     604K    SYSRAM
    0x0800_0000     128MB   SFC0
    0x1000_0000     128MB   SW virtual memory management
    0x1800_0000     128MB   SW virtual memory management
*********************************************************/

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0;      /* required amount of heap  */
_Min_Stack_Size = 0x400; /* required amount of stack */

/* Memory Spaces Definitions:4M FLASH,4M PSRAM,384k SYSRAM */
MEMORY
{
    TCM (rwx)             : ORIGIN = 0x04000000, LENGTH = 64K
    TCM_STACK (rwx)       : ORIGIN = 0x04010000, LENGTH = 20K
    TCM_BSS (rwx)         : ORIGIN = 0x04015000, LENGTH = 140K
    TCM_2ND_BL (rwx)      : ORIGIN = 0xDEADBEEF, LENGTH = 0K
    SYSRAM (rwx)          : ORIGIN = 0x04200000, LENGTH = 128K
    SYSRAM_NC (rwx)       : ORIGIN = 0x04220000, LENGTH = 128K
    ROM_BL(rx)            : ORIGIN = 0x08003000, LENGTH = 64K        /* DOWNLOAD, name: bootloader.bin, display: BootLoader */
    ROM_2ND_BL(rx)        : ORIGIN = 0x08013000, LENGTH = 64K
}

/* Flash base address and its capacity. */
_rom_base           = 0x08000000;
_rom_length         = 0x00400000;

 /* Highest address of the stack */
_estack = ORIGIN(TCM_STACK) + LENGTH(TCM_STACK);    /* end of TCM */

/* Generate a link error if stack don't fit into TCM */
_stack_size = 0x1000; /* required amount of stack 4KB*/

/* stack start */
_stack_start = _estack - _stack_size;

/* Entry Point */
ENTRY(Reset_Handler)

/* Define output sections */
SECTIONS
{
    _2nd_bl_tcm_code_load = LOADADDR(.2nd_bl_tcm_code);
    .2nd_bl_tcm_code :
    {
        . = ALIGN(4);
        _2nd_bl_tcm_code_start = .;

        /* Put TCM code at here   */

        . = ALIGN(4);
        _2nd_bl_tcm_code_end = .;

        _2nd_bl_tcm_ro_data_start = .;

        /* Put TCM RO data at here   */

        . = ALIGN(4);
        _2nd_bl_tcm_ro_data_end = .;
    } > TCM_2ND_BL  AT > ROM_2ND_BL

    _2nd_bl_tcm_rw_data_load = LOADADDR(.2nd_bl_tcm_rw_data);
    .2nd_bl_tcm_rw_data :
    {
        . = ALIGN(4);
        _2nd_bl_tcm_rw_data_start = .;


        /* Put TCM RW data at here   */


        . = ALIGN(4);
        _2nd_bl_tcm_rw_data_end = .;
    } > TCM_2ND_BL  AT > ROM_2ND_BL

    .2nd_bl_tcm_zi_data(NOLOAD) :
    {
        . = ALIGN(4);
        _2nd_bl_tcm_zi_data_start = .;

        /* Put TCM ZI data at here   */

        . = ALIGN(4);
        _2nd_bl_tcm_zi_data_end = .;
    } > TCM_2ND_BL

    _2nd_bl_sysram_code_load = LOADADDR(.2nd_bl_sysram_code);
    .2nd_bl_sysram_code :
    {
        . = ALIGN(4);
        _2nd_bl_sysram_code_start = .;


        /* Put SYSRAM code at here   */

	
        . = ALIGN(4);
        _2nd_bl_sysram_code_end = .;

        _2nd_bl_sysram_ro_data_start = .;


        /* Put SYSRAM RO data at here   */

        . = ALIGN(4);
        _2nd_bl_sysram_ro_data_end = .;
    } > SYSRAM  AT > ROM_2ND_BL

    _2nd_bl_sysram_rw_data_load = LOADADDR(.2nd_bl_sysram_rw_data);
    .2nd_bl_sysram_rw_data :
    {
        . = ALIGN(4);
        _2nd_bl_sysram_rw_data_start = .;


        /* Put SYSRAM RO data at here   */


        . = ALIGN(4);
        _2nd_bl_sysram_rw_data_end = .;
    } > SYSRAM  AT > ROM_2ND_BL

    .2nd_bl_sysram_zi_data(NOLOAD) :
    {
        . = ALIGN(4);
        _2nd_bl_sysram_zi_data_start = .;


        /* Put sysram ZI data at here   */

        
        . = ALIGN(4);
        _2nd_bl_sysram_zi_data_end = .;
    } > SYSRAM

    . = ORIGIN(ROM_BL);
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.reset_handler))
    KEEP(*(.isr_vector))
    KEEP(*(.rom_parameter))
    . = ALIGN(4);
  } > TCM AT > ROM_BL

  /* The program code and other data goes into FLASH */
  .text : ALIGN(4)
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    *(.ram_rodata*)
    *(.sysram_rodata*)
    *(.tcm_rodata*)
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)
    *(.ram_code*)
    *(.sysram_code*)
    *(.tcm_code*)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
    _exit = .;
  } >TCM AT > ROM_BL


  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */
    *(.tcm_rwdata*)
    *(.cached_sysram_rwdata*)
    *(.noncached_sysram_rwdata*)
    *(.noncached_ram_rwdata*)
    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >TCM AT > ROM_BL


  . = ALIGN(4);
  .sysram_bss (NOLOAD) :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sysram_sbss = .;         /* define a global symbol at bss start */
    *(.cached_sysram_zidata_sboot)
    . = ALIGN(4);
    _sysram_ebss = .;         /* define a global symbol at bss end */
  } >SYSRAM AT > ROM_BL

   /* Uninitialized data section */
  . = ALIGN(4);
  .bss (NOLOAD) :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    *(.tcm_zidata*)
    *(.cached_sysram_zidata*)
    *(.noncached_sysram_zidata*)
    *(.noncached_ram_zidata*)
    . = ALIGN(4);
    __end__ = .;
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >TCM_BSS AT > ROM_BL
  .gnu.sgstubs : ALIGN(32)
  {
    KEEP(*(.gnu.sgstubs.*))
    . = ALIGN(32);
  } > TCM AT > ROM_BL
  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack :
  {
    . = ALIGN(4);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    __HeapBase = .;
    . = . + _Min_Heap_Size;
    __HeapLimit = .;
    __StackLimit = .;
    . = . + _Min_Stack_Size;
    . = ALIGN(4);
  } >TCM AT > ROM_BL
}
