
###################################################
# Sources
FATFS_SRC = middleware/third_party/fatfs

C_FILES  += $(FATFS_SRC)/src/ff.c
C_FILES  += $(FATFS_SRC)/src/option/unicode.c
C_FILES  += $(FATFS_SRC)/src/option/syscall.c
CFLAGS += -fwide-exec-charset=UTF-16LE


ifeq ($(PRODUCT_VERSION),2523)
ifeq ($(MTK_FOTA_FS_ENABLE),y)
C_FILES += 	$(FATFS_SRC)/src/portable/mt2523/bootloader/src/diskio.c
else
C_FILES += 	$(FATFS_SRC)/src/portable/mt2523/src/diskio.c
C_FILES += 	$(FATFS_SRC)/src/portable/mt2523/src/diskio_snand.c
C_FILES += 	$(FATFS_SRC)/src/portable/mt2523/src/diskio_sd.c
endif
endif


ifeq ($(PRODUCT_VERSION),2533)
ifeq ($(MTK_FOTA_FS_ENABLE),y)
C_FILES += 	$(FATFS_SRC)/src/portable/mt2533/bootloader/src/diskio.c
else
C_FILES += 	$(FATFS_SRC)/src/portable/mt2533/src/diskio.c
C_FILES += 	$(FATFS_SRC)/src/portable/mt2533/src/diskio_sd.c
C_FILES += 	$(FATFS_SRC)/src/portable/mt2533/src/diskio_snand.c
endif
endif


ifeq ($(PRODUCT_VERSION),7687)
C_FILES += 	$(FATFS_SRC)/src/portable/mt7687/src/diskio.c
C_FILES += 	$(FATFS_SRC)/src/portable/mt7687/src/diskio_sd.c
endif


ifeq ($(PRODUCT_VERSION),7697)
C_FILES += 	$(FATFS_SRC)/src/portable/mt7697/src/diskio.c
C_FILES += 	$(FATFS_SRC)/src/portable/mt7697/src/diskio_sd.c
endif

ifeq ($(PRODUCT_VERSION),7686)
C_FILES += 	$(FATFS_SRC)/src/portable/mt7686/src/diskio.c
C_FILES += 	$(FATFS_SRC)/src/portable/mt7686/src/diskio_sd.c
C_FILES += 	$(FATFS_SRC)/src/portable/mt7686/src/diskio_snor.c
endif

ifeq ($(PRODUCT_VERSION),7682)
C_FILES += 	$(FATFS_SRC)/src/portable/mt7682/src/diskio.c
C_FILES += 	$(FATFS_SRC)/src/portable/mt7682/src/diskio_sd.c
endif

ifeq ($(PRODUCT_VERSION),5932)
C_FILES += 	$(FATFS_SRC)/src/portable/mt5932/src/diskio.c
C_FILES += 	$(FATFS_SRC)/src/portable/mt5932/src/diskio_sd.c
endif

ifeq ($(PRODUCT_VERSION),1552)
C_FILES += 	$(FATFS_SRC)/src/portable/ab155x/src/diskio.c
C_FILES += 	$(FATFS_SRC)/src/portable/ab155x/src/diskio_sd.c
endif

ifeq ($(IC_CONFIG),am255x)
C_FILES +=      $(FATFS_SRC)/src/portable/am255x/src/diskio.c
C_FILES +=      $(FATFS_SRC)/src/portable/am255x/src/diskio_sd.c
endif

ifeq ($(PRODUCT_VERSION),7698)
C_FILES += 	$(FATFS_SRC)/src/portable/aw7698/src/diskio.c
C_FILES += 	$(FATFS_SRC)/src/portable/aw7698/src/diskio_sd.c
C_FILES += 	$(FATFS_SRC)/src/portable/aw7698/src/diskio_snor.c
endif

ifeq ($(PRODUCT_VERSION),2822)
C_FILES += 	$(FATFS_SRC)/src/portable/mt2822/src/diskio.c
C_FILES += 	$(FATFS_SRC)/src/portable/mt2822/src/diskio_sd.c
endif

#################################################################################
# include path

CFLAGS 	+= -I$(SOURCE_DIR)/middleware/util/include
CFLAGS 	+= -I$(SOURCE_DIR)/middleware/airoha/minicli/inc
CFLAGS  += -I$(SOURCE_DIR)/kernel/rtos/$(IC_CONFIG)/FreeRTOS/Source/include
#CFLAGS  += -I$(SOURCE_DIR)/kernel/rtos/FreeRTOS/Source/portable/GCC/ARM_CM4F
CFLAGS  += -I$(SOURCE_DIR)/kernel/service/inc
CFLAGS  += -I$(SOURCE_DIR)/driver/chip/inc
CFLAGS  += -I$(SOURCE_DIR)/middleware/third_party/fatfs/src


ifeq ($(PRODUCT_VERSION),2523)
CFLAGS  += -I$(SOURCE_DIR)/$(FATFS_SRC)/src/portable/mt2523/inc
ifeq ($(MTK_FOTA_FS_ENABLE),y)
CFLAGS  += -I$(SOURCE_DIR)/driver/board/mt25x3_hdk/bootloader/core/inc
endif
endif


ifeq ($(PRODUCT_VERSION),2533)
CFLAGS  += -I$(SOURCE_DIR)/$(FATFS_SRC)/src/portable/mt2533/inc
ifeq ($(MTK_FOTA_FS_ENABLE),y)
CFLAGS  += -I$(SOURCE_DIR)/driver/board/mt25x3_hdk/bootloader/core/inc
endif
endif


ifeq ($(PRODUCT_VERSION),7687)
CFLAGS  += -I$(SOURCE_DIR)/$(FATFS_SRC)/src/portable/mt7687/inc
endif


ifeq ($(PRODUCT_VERSION),7697)
CFLAGS  += -I$(SOURCE_DIR)/$(FATFS_SRC)/src/portable/mt7697/inc
endif

ifeq ($(PRODUCT_VERSION),7686)
CFLAGS  += -I$(SOURCE_DIR)/$(FATFS_SRC)/src/portable/mt7686/inc
endif


ifeq ($(PRODUCT_VERSION),7682)
CFLAGS  += -I$(SOURCE_DIR)/$(FATFS_SRC)/src/portable/mt7682/inc
endif


ifeq ($(PRODUCT_VERSION),5932)
CFLAGS  += -I$(SOURCE_DIR)/$(FATFS_SRC)/src/portable/mt5932/inc
endif


ifeq ($(PRODUCT_VERSION),1552)
CFLAGS  += -I$(SOURCE_DIR)/$(FATFS_SRC)/src/portable/ab155x/inc
endif

ifeq ($(IC_CONFIG),am255x)
CFLAGS  += -I$(SOURCE_DIR)/$(FATFS_SRC)/src/portable/am255x/inc
endif

ifeq ($(PRODUCT_VERSION),7698)
CFLAGS  += -I$(SOURCE_DIR)/$(FATFS_SRC)/src/portable/aw7698/inc
endif

ifeq ($(PRODUCT_VERSION),2822)
CFLAGS  += -I$(SOURCE_DIR)/$(FATFS_SRC)/src/portable/mt2822/inc
endif
