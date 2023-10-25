# BT sink source files
NVDM_DSP0_SRC = $(MIDDLEWARE_PROPRIETARY)/audio/port/$(IC_CONFIG)

C_FILES += $(NVDM_DSP0_SRC)/src/nvdm/audio_dsp0_nvdm.c
C_FILES += $(NVDM_DSP0_SRC)/src/nvdm/sysram_nvdm.c
C_FILES += $(NVDM_DSP0_SRC)/src/linked_syslog/linked_syslog.c

# Include bt sink path
CFLAGS += -I$(SOURCE_DIR)/$(NVDM_DSP0_SRC)/inc/nvdm

#Remove from new generation
#CFLAGS += -mno-unaligned-access
