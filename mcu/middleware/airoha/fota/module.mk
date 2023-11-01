
###############################################################################
# feature option dependency
###############################################################################

## MTK_FOTA_ENABLE
## Brief:       This option is to enable and disable FOTA.
## Usage:       If the value is "y", the MTK_FOTA_ENABLE compile option will be defined. You must also include the gva3\middleware\airoha\fota\module.mk in your Makefile before setting the option to "y". 
## Path:        $(MIDDLEWARE_PROPRIETARY)/fota
## Dependency:  HAL_FLASH_MODULE_ENABLED and HAL_WDT_MODULE_ENABLED must also defined in the hal_feature_config.h  under the project inc folder. If MTK_FOTA_CLI_ENABLE defined in the module.mk is "y", please also include the $(MIDDLEWARE_PROPRIETARY)/tftp/module.mk. 
## Notice:      Please use the driver/CMSIS/Device/airoha/mt2523/linkerscript/gcc/full_bin_fota/flash.ld for gcc build environment. 
##              Please use the driver/CMSIS/Device/airoha/mt2523/linkerscript/rvct/full_bin_fota/flash.sct for KEIL build environment. 


ifeq ($(AIR_FOTA_ENABLE),y)
MTK_FOTA_ENABLE = y
endif

ifeq ($(AIR_FOTA_VIA_RACE_CMD_ENABLE),y)
MTK_FOTA_VIA_RACE_CMD = y
endif

ifeq ($(MTK_FOTA_ENABLE),y)
CFLAGS += -DMTK_FOTA_ENABLE
ifeq ($(MTK_FOTA_VIA_RACE_CMD),y)
CFLAGS += -DMTK_FOTA_VIA_RACE_CMD

ifeq ($(BSP_EXTERNAL_SERIAL_FLASH_ENABLED),y)
CFLAGS += -DMTK_FOTA_EXTERNAL_FLASH_SUPPORT
endif

ifeq ($(MTK_FOTA_STORE_IN_EXTERNAL_FLASH),y)
ifneq ($(BSP_EXTERNAL_SERIAL_FLASH_ENABLED),y)
$(error MTK_FOTA_EXTERNAL_FLASH_SUPPORT is not defined! Check if the chip does not support the external flash. )
endif
CFLAGS += -DMTK_FOTA_STORE_IN_EXTERNAL_FLASH
endif

ifeq ($(AIR_FOTA_SUPPORT_MEMORY_LAYOUT_MODIFICATION_ENABLE),y)
CFLAGS += -DMTK_FOTA_SUPPORT_MEMORY_LAYOUT_MODIFICATION_ENABLE
endif

ifeq ($(AIR_FOTA_MEM_LAYOUT_CHANGE_ENABLE),y)
CFLAGS += -DMTK_FOTA_SUPPORT_MEMORY_LAYOUT_MODIFICATION_ENABLE
endif

endif
endif

ifeq ($(MTK_FOTA_CMD_ENABLE),y)
CFLAGS += -DMTK_FOTA_CMD_ENABLE
endif

ifeq ($(MTK_BOOTLOADER_SUPPORT_PARTITION_FOTA), y)
CFLAGS += -DMTK_BOOTLOADER_SUPPORT_PARTITION_FOTA
endif

#################################################################################
# source files
#################################################################################

FOTA_SRC = $(MIDDLEWARE_PROPRIETARY)/fota

C_FILES  += $(FOTA_SRC)/src/fota.c
C_FILES  += $(FOTA_SRC)/src/common/fota_platform.c

C_FILES  += $(FOTA_SRC)/src/race/fota_util.c
C_FILES  += $(FOTA_SRC)/src/race/fota_multi_info_util.c
C_FILES  += $(FOTA_SRC)/src/race/fota_multi_info.c
C_FILES  += $(FOTA_SRC)/src/race/fota_flash.c
C_FILES  += $(FOTA_SRC)/src/race/fota_flash_config.c
C_FILES  += $(FOTA_SRC)/src/fota_flash_for_gsound.c

ifeq ($(AIR_BL_DFU_ENABLE), y)
C_FILES  += $(FOTA_SRC)/src/race/dfu_util.c
endif

#################################################################################
# include path
#################################################################################
CFLAGS  += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/fota/inc/race
CFLAGS  += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/fota/inc/common
CFLAGS  += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/fota/inc/
CFLAGS  += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/race_cmd/inc
CFLAGS  += -I$(SOURCE_DIR)/driver/chip/inc
