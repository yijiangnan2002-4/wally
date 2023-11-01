
OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

/* Memory Spaces Definitions */
MEMORY
{
    TCM (rwx)                      : ORIGIN = 0x04000000, LENGTH = 240K

    SYSRAM_BT (rwx)                : ORIGIN = 0x04200000, LENGTH = 32K        /* Special design for Bluetooth on hardware. Non-Cachable */
    VSYSRAM (rwx)                  : ORIGIN = 0x04208000, LENGTH = 988K       /* Cachable */
    SYSRAM (rwx)                   : ORIGIN = 0x24208000, LENGTH = 988K       /* Non-Cacheable */
    SYSRAM_PRIVATE (rwx)           : ORIGIN = 0x242FF000, LENGTH = 4K         /* Non-Cacheable */

    ROM_LOG_STRING(rx)             : ORIGIN = 0x06000000, LENGTH = 32M
}

/* Highest address of the stack */
_stack_end = ORIGIN(TCM) + LENGTH(TCM);    /* end of TCM */

/* Generate a link error if stack don't fit into TCM */
_stack_size = 0x1000; /* required amount of stack 4KB*/

/* stack start */
_stack_start = _stack_end - _stack_size;

_image_stack_zi_base = _stack_end - _stack_size;
_image_stack_zi_limit = _stack_end;

/* Linker symbol provided for C/Assembly Code */
_sysram_start       = ORIGIN(SYSRAM);
_sysram_length      = LENGTH(SYSRAM);
_vsysram_start      = ORIGIN(VSYSRAM);
_vsysram_length     = LENGTH(VSYSRAM);
_sysram_bt_start    = ORIGIN(SYSRAM_BT);
_sysram_bt_length   = LENGTH(SYSRAM_BT);
_sysram_bt_limit    = _sysram_bt_start + _sysram_bt_length;
_tcm_start          = ORIGIN(TCM);
_tcm_length         = LENGTH(TCM);

/* swla start and size */
_swla_size = 0;       /* default swla will use free ram as the working memory, and also can reserve a region for swla by modify the value of _swla_size */
_swla_start = ORIGIN(SYSRAM) + LENGTH(SYSRAM) - _swla_size; /* at the tail of SYSRAM */

/* Entry Point */
ENTRY(Reset_Handler)

SECTIONS
{
    /* log string */
    .log_str :
    {
        KEEP(*(.log_version))
        KEEP(*(.log_timestamp))
        KEEP(*(SORT_BY_NAME(.log_filter_mirror*)))
        KEEP(*(.log_filter_end))
        KEEP(*(.log_chip_option))
        *(.log_string)
        *(.assert_string*)
    } > ROM_LOG_STRING

    /* Code & rodata region */
    . = ORIGIN(SYSRAM);
    _text_load = LOADADDR(.text);
    .text :ALIGN(4)
    {
        _text_start = .;

        KEEP(*(.reset_handler*))
        KEEP(*(.init))
        KEEP(*(.fini))
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)

        . = ALIGN(4);
        _log_filter_start = .;
        KEEP(*(SORT_BY_NAME(.log_filter.*)))
        _log_filter_end = .;

        . = ALIGN(4);
        _text_end = .;
    } > SYSRAM  AT> SYSRAM

    /* GNU region for unwinding the stack */
    .ARM.extab :
    {
        __extab_start = .;
        *(.ARM.extab* .gnu.linkonce.armextab.*)
        __extab_end = .;
    } > SYSRAM  AT> SYSRAM

    /* GNU region for unwinding the stack */
    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > SYSRAM  AT> SYSRAM

    . = ALIGN(32);
    _sysram_code_load = LOADADDR(.cached_ram_text);
    .cached_ram_text . :ALIGN(32)
    {
        _sysram_code_start = .;

        *(.ram_code)
        *(.ram_code*)
        *(.sysram_code)
        *(.sysram_code*)
        *(.ram_rodata)
        *(.ram_rodata*)
        *(.sysram_rodata)
        *(.sysram_rodata*)

        _sysram_code_end = .;
    } > SYSRAM  AT> SYSRAM

    /* sysram cacheable rwdata region */
    . = ALIGN(32);
    _cached_sysram_data_load = LOADADDR(.cached_ram_data);
    .cached_ram_data . :ALIGN(32)
    {
        _cached_sysram_data_start = .;

        *(.data)
        *(.data*)
        *(.cached_sysram_rwdata)
        *(.cached_sysram_rwdata*)

        . = ALIGN(4);
        /* preinit data */
        PROVIDE (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE (__preinit_array_end = .);

        . = ALIGN(4);
        /* init data */
        PROVIDE (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE (__init_array_end = .);

        . = ALIGN(4);
        /* finit data */
        PROVIDE (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE (__fini_array_end = .);

        _cached_sysram_data_end = .;
    } > SYSRAM  AT> SYSRAM

    /* sysram cacheable zidata region */
    . = ALIGN(4);
    .bss (NOLOAD):
    {
        _bss_start = .;

        *(.bss)
        *(.bss*)
        *(COMMON)
        *(.cached_sysram_zidata)
        *(.cached_sysram_zidata*)

        . = ALIGN(4);
        _bss_end = .;
    } > SYSRAM


    . = . - ORIGIN(SYSRAM) + ORIGIN(VSYSRAM);
    . = ALIGN(4);
    /* non-cacheable rwdata & shareable rwdata region */
    _noncached_sysram_rw_load = LOADADDR(.noncached_data);
    .noncached_data . :ALIGN(4)
    {
        . = ALIGN(4);
        _noncached_sysram_rw_start = .;

        *(.noncached_ram_rwdata)
        *(.noncached_ram_rwdata*)
        *(.noncached_sysram_rwdata)
        *(.noncached_sysram_rwdata*)

        _noncached_sysram_rw_end = .;
    } > VSYSRAM  AT> SYSRAM

    /* non-cacheable zidata region */
    . = ALIGN(4);
    .noncached_bss . (NOLOAD) :
    {
        _noncached_sysram_zi_start = .;

        *(.noncached_ram_zidata)
        *(.noncached_ram_zidata*)
        *(.noncached_sysram_zidata)
        *(.noncached_sysram_zidata*)

        _noncached_sysram_zi_end = .;
    } > VSYSRAM

    /* Sysram shareable rwdata region */
    . = ALIGN(4);
    _share_data_load = LOADADDR(.share_data);
    .share_data . :
    {
        _share_data_start = .;

        *(.share_rwdata)
        *(.share_rwdata*)

        . = ALIGN(4);
        _share_data_end = .;
    } > VSYSRAM  AT> SYSRAM

    /* Sysram shareable zidata region */
    . = ALIGN(4);
    .share_bss . (NOLOAD) :
    {
        _share_bss_start = .;

        *(.share_zidata)
        *(.share_zidata*)

        . = ALIGN(4);
        _share_bss_end = .;
    } > VSYSRAM

    /* bt shareable rwdata region */
    . = ALIGN(4);
    _bt_sysram_rw_data_load = LOADADDR(.bt_sysram_rw_data);
    .bt_sysram_rw_data :ALIGN(4)
    {
        _bt_sysram_rw_data_start = .;

        *(.share_rwbt)
        *(.share_rwbt*)

        . = ALIGN(4);
        _bt_sysram_rw_data_end = .;
    }> SYSRAM_BT  AT> SYSRAM

    /* bt shareable zidata region */
    . = ALIGN(4);
    _bt_sysram_zi_data_load = LOADADDR(.bt_sysram_zi_data);
    .bt_sysram_zi_data (NOLOAD) :ALIGN(4)
    {
        _bt_sysram_zi_data_start = .;

        *(.share_zibt)
        *(.share_zibt*)

        . = ALIGN(4);
        _bt_sysram_zi_data_end = .;
    }> SYSRAM_BT

    /* TCM code & rodata & rwdata region */
    . = ALIGN(4);
    _tcm_text_load = LOADADDR(.tcm);
    .tcm :ALIGN(4)
    {
        _tcm_text_start = .;

        _tcm_ro_start = .;
        KEEP(*(.isr_vector))
        *(.exception_code)
        *(.exception_code*)
        *(.exception_rodata)
        *(.exception_rodata*)
        *(.tcm_code)
        *(.tcm_code*)
        *(.tcm_rodata)
        *(.tcm_rodata*)
        _tcm_ro_end = .;

        . = ALIGN(4);
        _tcm_text_end = .;
    }> TCM  AT> SYSRAM

    /* TCM rwdata region */
    . = ALIGN(4);
    _tcm_rw_data_load = LOADADDR(.tcm_rw_sec);
    .tcm_rw_sec :
    {
        _tcm_rw_start = .;
        *(.tcm_rwdata)
        *(.tcm_rwdata*)
        . = ALIGN(4);
        _tcm_rw_end = .;
    }> TCM  AT> SYSRAM

    /* TCM zidata region */
    . = ALIGN(4);
    .tcm_bss (NOLOAD) :
    {
        _tcm_zi_start = .;

        *(.tcm_zidata)
        *(.tcm_zidata*)

        _tcm_zi_end = .;
    }> TCM

    /* use to check if the stack exceeds the total TCM size*/
    .stack (NOLOAD):
    {
        . = ALIGN(4);
        PROVIDE ( end = . );
        PROVIDE ( _end = . );
        . = . + _stack_size;
        . = ALIGN(4);
    } > TCM
}
