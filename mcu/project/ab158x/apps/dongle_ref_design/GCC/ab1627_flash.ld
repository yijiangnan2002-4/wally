OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

/* Memory Spaces Definitions */
MEMORY
{
    ROM_PARTITION_TABLE(rx)        : ORIGIN = 0x08000000, LENGTH = 4K        /* DOWNLOAD, name: partition_table.bin, display: PartitionTable */
    ROM_SECURITY_HEAD1(rx)         : ORIGIN = 0x08001000, LENGTH = 4K
    ROM_SECURITY_HEAD2(rx)         : ORIGIN = 0x08002000, LENGTH = 4K
    ROM_BL(rx)                     : ORIGIN = 0x08003000, LENGTH = 64K       /* DOWNLOAD, name: bootloader.bin, display: BootLoader */
    ROM_ERASE_BACKUP(r)            : ORIGIN = 0x08013000, LENGTH = 4K
    ROM_RTOS(rx)                   : ORIGIN = 0x08014000, LENGTH = 2296K     /* DOWNLOAD, name: freertos_create_thread.bin, display: MCU_FW */
    ROM_NVDM_OTA(rx)               : ORIGIN = 0x08252000, LENGTH = 64K       /* DOWNLOAD, name: nvdm_ou.bin, display: NVDM_OTA */
    ROM_LM(rx)                     : ORIGIN = 0xDEADBEEF, LENGTH = 0K
    ROM_ROFS(rx)                   : ORIGIN = 0xDEADBEEF, LENGTH = 0K
    ROM_FOTA_RESERVED(rx)          : ORIGIN = 0x08262000, LENGTH = 1616K
    ROM_NVDM_RESERVED(rx)          : ORIGIN = 0x083F6000, LENGTH = 40K       /* DOWNLOAD, name: nvdm.bin, display: NVDM */

    TCM (rwx)                      : ORIGIN = 0x04000000, LENGTH = 192K

    SYSRAM (rwx)                   : ORIGIN = 0x04200000, LENGTH = 572K
    VSYSRAM (rwx)                  : ORIGIN = 0x14200000, LENGTH = 572K
    SYSRAM_PRIVATE (rwx)           : ORIGIN = 0x0428F000, LENGTH = 4K
    /* Special design for Bluetooth on hardware. Non-Cachable */
    SYSRAM_BT (rwx)                : ORIGIN = 0x04290000, LENGTH = 64K
    ROM_LOG_STRING(rx)             : ORIGIN = 0x06000000, LENGTH = 32M
}

/* Flash base address and its capacity. */
_rom_base           = 0x08000000;
_rom_length         = 0x00400000;

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
_sysram_private_base    = ORIGIN(SYSRAM_PRIVATE);
_sysram_private_length  = LENGTH(SYSRAM_PRIVATE);

/* swla start and size */
_swla_size = 0;       /* default swla will use free ram as the working memory, and also can reserve a region for swla by modify the value of _swla_size */
_swla_start = ORIGIN(VSYSRAM) + LENGTH(VSYSRAM) - _swla_size; /* at the tail of VRAM */

/* Entry Point */
ENTRY(Reset_Handler)

SECTIONS
{
    /* partition table */
    .partition_table :
    {
        /* Security_Header1 */
        LONG(0x00000000);/* BinaryId */
        LONG(0x00000000);/* PartitionId */
        LONG(0x00000000);/* LoadAddressHigh */
        LONG(ORIGIN(ROM_SECURITY_HEAD1));/* LoadAddressLow */
        LONG(0x00000000);/* BinaryMaxLengthHigh */
        LONG(LENGTH(ROM_SECURITY_HEAD1));/* BinaryMaxLengthLow */
        LONG(0x00000000);/* ExecutionAddress */
        LONG(0x00000000);/* ReservedItem0 */
        LONG(0x00000000);/* ReservedItem1 */
        LONG(0x00000000);/* ReservedItem2 */
        LONG(0x00000000);/* ReservedItem3 */
        LONG(0x00000000);/* ReservedItem4 */

        /* Security_Header2 */
        LONG(0x00000008);/* BinaryId */
        LONG(0x00000000);/* PartitionId */
        LONG(0x00000000);/* LoadAddressHigh */
        LONG(ORIGIN(ROM_SECURITY_HEAD2));/* LoadAddressLow */
        LONG(0x00000000);/* BinaryMaxLengthHigh */
        LONG(LENGTH(ROM_SECURITY_HEAD2));/* BinaryMaxLengthLow */
        LONG(0x00000000);/* ExecutionAddress */
        LONG(0x00000000);/* ReservedItem0 */
        LONG(0x00000000);/* ReservedItem1 */
        LONG(0x00000000);/* ReservedItem2 */
        LONG(0x00000000);/* ReservedItem3 */
        LONG(0x00000000);/* ReservedItem4 */

        /* Bootloader */
        LONG(0x00000001);/* BinaryId */
        LONG(0x00000000);/* PartitionId */
        LONG(0x00000000);/* LoadAddressHigh */
        LONG(ORIGIN(ROM_BL));/* LoadAddressLow */
        LONG(0x00000000);/* BinaryMaxLengthHigh */
        LONG(LENGTH(ROM_BL));/* BinaryMaxLengthLow */
        LONG(0x00000000);/* ExecutionAddress */
        LONG(0x00000000);/* ReservedItem0 */
        LONG(0x00000000);/* ReservedItem1 */
        LONG(0x00000000);/* ReservedItem2 */
        LONG(0x00000000);/* ReservedItem3 */
        LONG(0x00000000);/* ReservedItem4 */

        /* CM4 */
        LONG(0x00000003);/* BinaryId */
        LONG(0x00000000);/* PartitionId */
        LONG(0x00000000);/* LoadAddressHigh */
        LONG(ORIGIN(ROM_RTOS));/* LoadAddressLow */
        LONG(0x00000000);/* BinaryMaxLengthHigh */
        LONG(LENGTH(ROM_RTOS));/* BinaryMaxLengthLow */
        LONG(0x00000000);/* ExecutionAddress */
        LONG(0x00000000);/* ReservedItem0 */
        LONG(0x00000000);/* ReservedItem1 */
        LONG(0x00000000);/* ReservedItem2 */
        LONG(0x00000000);/* ReservedItem3 */
        LONG(0x00000000);/* ReservedItem4 */

        /* Note: BootROM need this 0x4 entry for DSP0 */
        LONG(0x00000004);/* BinaryId */
        LONG(0x00000000);/* PartitionId */
        LONG(0x00000000);/* LoadAddressHigh */
        LONG(0x00000000);/* LoadAddressLow */
        LONG(0x00000000);/* BinaryMaxLengthHigh */
        LONG(0x00000000);/* BinaryMaxLengthLow */
        LONG(0x00000000);/* ExecutionAddress */
        LONG(0x00000000);/* ReservedItem0 */
        LONG(0x00000000);/* ReservedItem1 */
        LONG(0x00000000);/* ReservedItem2 */
        LONG(0x00000000);/* ReservedItem3 */
        LONG(0x00000000);/* ReservedItem4 */

        /* FOTA */
        LONG(0x00000006);/* BinaryId */
        LONG(0x00000000);/* PartitionId */
        LONG(0x00000000);/* LoadAddressHigh */
        LONG(ORIGIN(ROM_FOTA_RESERVED));/* LoadAddressLow */
        LONG(0x00000000);/* BinaryMaxLengthHigh */
        LONG(LENGTH(ROM_FOTA_RESERVED));/* BinaryMaxLengthLow */
        LONG(0x00000000);/* ExecutionAddress */
        LONG(0x00000000);/* ReservedItem0 */
        LONG(0x00000000);/* ReservedItem1 */
        LONG(0x00000000);/* ReservedItem2 */
        LONG(0x00000000);/* ReservedItem3 */
        LONG(0x00000000);/* ReservedItem4 */

        /* NVDM */
        LONG(0x00000007);/* BinaryId */
        LONG(0x00000000);/* PartitionId */
        LONG(0x00000000);/* LoadAddressHigh */
        LONG(ORIGIN(ROM_NVDM_RESERVED));/* LoadAddressLow */
        LONG(0x00000000);/* BinaryMaxLengthHigh */
        LONG(LENGTH(ROM_NVDM_RESERVED));/* BinaryMaxLengthLow */
        LONG(0x00000000);/* ExecutionAddress */
        LONG(0x00000000);/* ReservedItem0 */
        LONG(0x00000000);/* ReservedItem1 */
        LONG(0x00000000);/* ReservedItem2 */
        LONG(0x00000000);/* ReservedItem3 */
        LONG(0x00000000);/* ReservedItem4 */

        /* NVDM_OTA */
        LONG(0x0000000F);/* BinaryId */
        LONG(0x00000000);/* PartitionId */
        LONG(0x00000000);/* LoadAddressHigh */
        LONG(ORIGIN(ROM_NVDM_OTA));/* LoadAddressLow */
        LONG(0x00000000);/* BinaryMaxLengthHigh */
        LONG(LENGTH(ROM_NVDM_OTA));/* BinaryMaxLengthLow */
        LONG(0x00000000);/* ExecutionAddress */
        LONG(0x00000000);/* ReservedItem0 */
        LONG(0x00000000);/* ReservedItem1 */
        LONG(0x00000000);/* ReservedItem2 */
        LONG(0x00000000);/* ReservedItem3 */
        LONG(0x00000000);/* ReservedItem4 */

        /* ROFS */
        LONG(0x00000009);/* BinaryId */
        LONG(0x00000000);/* PartitionId */
        LONG(0x00000000);/* LoadAddressHigh */
        LONG(ORIGIN(ROM_ROFS));/* LoadAddressLow */
        LONG(0x00000000);/* BinaryMaxLengthHigh */
        LONG(LENGTH(ROM_ROFS));/* BinaryMaxLengthLow */
        LONG(0x00000000);/* ExecutionAddress */
        LONG(0x00000000);/* ReservedItem0 */
        LONG(0x00000000);/* ReservedItem1 */
        LONG(0x00000000);/* ReservedItem2 */
        LONG(0x00000000);/* ReservedItem3 */
        LONG(0x00000000);/* ReservedItem4 */

        /* LM */
        LONG(0x0000000A);/* BinaryId */
        LONG(0x00000000);/* PartitionId */
        LONG(0x00000000);/* LoadAddressHigh */
        LONG(ORIGIN(ROM_LM));/* LoadAddressLow */
        LONG(0x00000000);/* BinaryMaxLengthHigh */
        LONG(LENGTH(ROM_LM));/* BinaryMaxLengthLow */
        LONG(0x00000000);/* ExecutionAddress */
        LONG(0x00000000);/* ReservedItem0 */
        LONG(0x00000000);/* ReservedItem1 */
        LONG(0x00000000);/* ReservedItem2 */
        LONG(0x00000000);/* ReservedItem3 */
        LONG(0x00000000);/* ReservedItem4 */

        /* ROM Erase Backup */
        LONG(0x00000010);/* BinaryId */
        LONG(0x00000000);/* PartitionId */
        LONG(0x00000000);/* LoadAddressHigh */
        LONG(ORIGIN(ROM_ERASE_BACKUP));/* LoadAddressLow */
        LONG(0x00000000);/* BinaryMaxLengthHigh */
        LONG(LENGTH(ROM_ERASE_BACKUP));/* BinaryMaxLengthLow */
        LONG(0x00000000);/* ExecutionAddress */
        LONG(0x00000000);/* ReservedItem0 */
        LONG(0x00000000);/* ReservedItem1 */
        LONG(0x00000000);/* ReservedItem2 */
        LONG(0x00000000);/* ReservedItem3 */
        LONG(0x00000000);/* ReservedItem4 */

        /* DUMMY_END */
        LONG(0x4D4D5544);
        LONG(0x444E4559);
        LONG(0x00000000);
        LONG(0x00000000);
        LONG(0x00000000);
        LONG(0x00000000);
        LONG(0x00000000);
        LONG(0x00000000);
        LONG(0x00000000);
        LONG(0x00000000);
        LONG(0x00000000);
        LONG(0x00000000);
    } > ROM_PARTITION_TABLE

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

    /* Flash code & rodata region */
    . = ORIGIN(ROM_RTOS);
    . = ALIGN(4);
    .text :ALIGN(4)
    {
        _text_start = .;
        Image$$TEXT$$Base = .;

        KEEP(*(.reset_handler*))
        KEEP(*(.init))
        KEEP(*(.fini))
        *(EXCLUDE_FILE (*FreeRTOS*) *.text)
        *(EXCLUDE_FILE (*FreeRTOS*) .text*)
        *(.rodata)
        *(.rodata*)

        . = ALIGN(4);
        _log_filter_start = .;
        KEEP(*(SORT_BY_NAME(.log_filter.*)))
        _log_filter_end = .;

        . = ALIGN(4);
        Image$$TEXT$$Limit = .;
        _text_end = .;

    } > ROM_RTOS  AT> ROM_RTOS

    /* GNU region for unwinding the stack */
    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > ROM_RTOS  AT> ROM_RTOS

    /* GNU region for unwinding the stack */
    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > ROM_RTOS  AT> ROM_RTOS

    /* Sysram code & rodata region */
    . = ALIGN(32);
    _sysram_code_load = LOADADDR(.sysram_text);
    .sysram_text :
    {
        Image$$CACHED_SYSRAM_TEXT$$Base = .;
        _sysram_code_start = .;

        *(.ram_code)
        *(.ram_code*)
        *(.sysram_code)
        *(.sysram_code*)
        *(.ram_rodata)
        *(.ram_rodata*)
        *(.sysram_rodata)
        *(.sysram_rodata*)
        *FreeRTOS*(*.text)
        *FreeRTOS*(.text*)
		
        . = ALIGN(4);
        Image$$CACHED_SYSRAM_TEXT$$Limit = .;
        _sysram_code_end = .;
    } > VSYSRAM   AT> ROM_RTOS

    /* Sysram cacheable rwdata region */
    . = ALIGN(4);
    _cached_sysram_data_load = LOADADDR(.data);
    .data :
    {
        _cached_sysram_data_start = .;
        Image$$CACHED_SYSRAM_DATA$$RW$$Base = .;

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

        . = ALIGN(4);
        _cached_sysram_data_end = .;
        Image$$CACHED_SYSRAM_DATA$$RW$$Limit = .;
    } > VSYSRAM  AT> ROM_RTOS

    /* Sysram cacheable zidata region */
    . = ALIGN(4);
    .bss (NOLOAD) :
    {
        _bss_start = .;
        _cached_sysram_bss_start = .;
        Image$$CACHED_SYSRAM_DATA$$ZI$$Base = .;

        *(.bss)
        *(.bss*)
        *(COMMON)
        *(.cached_sysram_zidata)
        *(.cached_sysram_zidata*)

        . = ALIGN(4);
        _bss_end = .;
        _cached_sysram_bss_end = .;
        Image$$CACHED_SYSRAM_DATA$$ZI$$Limit = .;
    } > VSYSRAM  AT> ROM_RTOS

    /* Sysram non-cacheable rwdata region */
    . = ALIGN(32);
    _noncached_sysram_rw_load = LOADADDR(.noncached_sysram_data);
    . = . - ORIGIN(VSYSRAM) + ORIGIN(SYSRAM);
    .noncached_sysram_data . :
    {
        Image$$NONCACHED_SYSRAM_DATA$$Base = .;

        _noncached_sysram_rw_start = .;

        *(.noncached_ram_rwdata)
        *(.noncached_ram_rwdata*)
        *(.noncached_sysram_rwdata)
        *(.noncached_sysram_rwdata*)

        . = ALIGN(4);
        Image$$NONCACHED_SYSRAM_DATA$$Limit = .;
        _noncached_sysram_rw_end = .;
    } > SYSRAM  AT> ROM_RTOS

    /* Sysram non-cacheable zidata region */
    . = ALIGN(4);
    .noncached_sysram_bss . (NOLOAD) :
    {
        Image$$NONCACHED_SYSRAM_ZI$$Base = .;
        _noncached_sysram_zi_start = .;

        *(.noncached_ram_zidata)
        *(.noncached_ram_zidata*)
        *(.noncached_sysram_zidata)
        *(.noncached_sysram_zidata*)

        . = ALIGN(4);
        Image$$NONCACHED_SYSRAM_ZI$$Limit = .;
        _noncached_sysram_zi_end = .;
    } > SYSRAM  AT> ROM_RTOS

    /* Sysram shareable rwdata region */
    . = ALIGN(4);
    _share_data_load = LOADADDR(.share_data);
    .share_data . :
    {
        Image$$SHARE_DATA$$Base = .;
        _share_data_start = .;

        *(.share_rwdata)
        *(.share_rwdata*)

        . = ALIGN(4);
        Image$$SHARE_DATA$$Limit = .;
        _share_data_end = .;
    } > SYSRAM  AT> ROM_RTOS

    /* Sysram shareable zidata region */
    . = ALIGN(4);
    .share_bss . (NOLOAD) :
    {
        Image$$SHARE_ZI$$Base = .;
        _share_bss_start = .;

        *(.share_zidata)
        *(.share_zidata*)

        . = ALIGN(4);
        Image$$SHARE_ZI$$Limit = .;
        _share_bss_end = .;
    } > SYSRAM  AT> ROM_RTOS

    /* Do NOT modify the order of calculation and
     * make the expressions in ASSERT as simple as possible
     */
    __swla_start = _swla_start - ORIGIN(VSYSRAM) + ORIGIN(SYSRAM);
    ASSERT(_share_bss_end <= __swla_start, "SYSRAM expired")

    /* Sysram swla region */
    . = _swla_start;
    . = ALIGN(32);
    .swla . (NOLOAD) :
    {
        Image$$SWLA$$Base = .;
        . = ALIGN(4);

        . = . + (_swla_size) ;

        . = ALIGN(4);
        Image$$SWLA$$Limit = .;
    } > VSYSRAM

    /* bt shareable rwdata region */
    . = ALIGN(4);
    _bt_sysram_rw_data_load = LOADADDR(.bt_sysram_rw_data);
    .bt_sysram_rw_data :ALIGN(4)
    {
        _bt_sysram_rw_data_start = .;
        Image$$BT_SYSRAM_RW_DATA$$RW$$Base = .;

        *(.share_rwbt)
        *(.share_rwbt*)

        . = ALIGN(4);
        Image$$BT_SYSRAM_RW_DATA$$RW$$Limit = .;
        _bt_sysram_rw_data_end = .;
    }> SYSRAM_BT  AT> ROM_RTOS

    /* bt shareable zidata region */
    . = ALIGN(4);
    _bt_sysram_zi_data_load = LOADADDR(.bt_sysram_zi_data);
    .bt_sysram_zi_data (NOLOAD) :ALIGN(4)
    {
        _bt_sysram_zi_data_start = .;
        Image$$BT_SYSRAM_ZI_DATA$$RW$$Base = .;

        *(.share_zibt)
        *(.share_zibt*)

        . = ALIGN(4);
        Image$$BT_SYSRAM_ZI_DATA$$RW$$Limit = .;
        _bt_sysram_zi_data_end = .;
    }> SYSRAM_BT  AT> ROM_RTOS

    /* TCM code & rodata & rwdata region */
    . = ALIGN(4);
    _tcm_text_load = LOADADDR(.tcm_code);
    .tcm_code :
    {
        _tcm_text_start = .;
        Image$$TCM$$RO$$Base = .;
        Image$$VECTOR$$TABLE$$Base = .;

        KEEP(*(.isr_vector))
        *(.exception_code)
        *(.exception_code*)
        *(.exception_rodata)
        *(.exception_rodata*)
        *(.tcm_code)
        *(.tcm_code*)
        *(.tcm_rodata)
        *(.tcm_rodata*)

        Image$$TCM$$RO$$Limit = .;
        . = ALIGN(4);
        _tcm_text_end = .;
    }> TCM  AT> ROM_RTOS

    /* TCM code & rodata & rwdata region */
    . = ALIGN(4);
    _tcm_rw_data_load = LOADADDR(.tcm_rw_data);
    .tcm_rw_data :
    {
        Image$$TCM$$RW$$Base = .;
        _tcm_rw_start = .;

        *(.tcm_rwdata)
        *(.tcm_rwdata*)

        . = ALIGN(4);
        Image$$TCM$$RW$$Limit = .;
        _tcm_rw_end = .;
    }> TCM  AT> ROM_RTOS

    /* TCM zidata region */
    . = ALIGN(4);
    .tcm_bss (NOLOAD) :
    {
        _tcm_zi_start = .;
        Image$$TCM$$ZI$$Base = .;

        *(.tcm_zidata)
        *(.tcm_zidata*)

        . = ALIGN(4);
        _tcm_zi_end = .;
        Image$$TCM$$ZI$$Limit = .;
    }> TCM
    _BIN_END_ = LOADADDR(.tcm_bss);

    /* use to check if the stack exceeds the total TCM size*/
    .stack :
    {
        . = ALIGN(4);
        PROVIDE ( end = . );
        PROVIDE ( _end = . );
        . = . + _stack_size;
        . = ALIGN(4);
    } > TCM

    Image$$STACK$$ZI$$Base = _stack_end - _stack_size;
    Image$$STACK$$ZI$$Limit = _stack_end;

    /* provide nvdm start and length for NVDM management */
    Image$$NVDM$$ZI$$Base = ORIGIN(ROM_NVDM_RESERVED);
    Image$$NVDM$$ZI$$Length = LENGTH(ROM_NVDM_RESERVED);
    Image$$NVDM$$ZI$$Limit = ORIGIN(ROM_NVDM_RESERVED) + LENGTH(ROM_NVDM_RESERVED);

    _nvdm_start = ORIGIN(ROM_NVDM_RESERVED);
    _nvdm_end =  ORIGIN(ROM_NVDM_RESERVED) + LENGTH(ROM_NVDM_RESERVED);
    _nvdm_ou_start = ORIGIN(ROM_NVDM_OTA);
    _nvdm_ou_end = ORIGIN(ROM_NVDM_OTA) + LENGTH(ROM_NVDM_OTA);

    _nvdm_ota_start = ORIGIN(ROM_NVDM_OTA);
    _nvdm_length = LENGTH(ROM_NVDM_OTA);
    _nvdm_ota_end = _nvdm_ota_start + _nvdm_length;

    /* Add some symbols to facilitate the calculation of
     * the amount and remaining amount of various memory.
     */
    __rom_rtos_limit = LENGTH(ROM_RTOS);
    __rom_rtos_used = _BIN_END_ - ORIGIN(ROM_RTOS);
    __rom_rtos_free = __rom_rtos_limit - __rom_rtos_used;

    __rom_pt_limit = LENGTH(ROM_PARTITION_TABLE);
    __rom_pt_used = SIZEOF(.partition_table);
    __rom_pt_free = __rom_pt_limit - __rom_pt_used;

    __tcm_limit = LENGTH(TCM);
    __tcm_used = _tcm_zi_end + _stack_size - ORIGIN(TCM);
    __tcm_free = __tcm_limit - __tcm_used;
    __tcm_free_begin = _tcm_start + __tcm_used;
    __tcm_free_limit = _stack_start;

    __sysram_diff_addr = ORIGIN(SYSRAM) - ORIGIN(VSYSRAM);
    __sysram_limit = LENGTH(SYSRAM);
    __sysram_used = _share_bss_end - ORIGIN(VSYSRAM) - __sysram_diff_addr;
    __sysram_free = __sysram_limit - __sysram_used;
    __sysram_free_begin = _sysram_start + __sysram_used;
    __sysram_free_limit = _sysram_start + _sysram_length;
}
