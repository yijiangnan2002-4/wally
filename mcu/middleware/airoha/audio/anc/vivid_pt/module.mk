###################################################
AUDIO_SRC = $(MIDDLEWARE_PROPRIETARY)/audio
VIVID_SRC = anc/vivid_pt

include $(SOURCE_DIR)/$(AUDIO_SRC)/anc/llf/module.mk

CFLAGS  += -I$(SOURCE_DIR)/$(AUDIO_SRC)/$(VIVID_SRC)/inc
AUDIO_FILES += $(AUDIO_SRC)/$(VIVID_SRC)/src/audio_anc_vivid_pt.c
