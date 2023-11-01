###################################################
AUDIO_SRC = $(MIDDLEWARE_PROPRIETARY)/audio
PSAP_SRC = anc/psap

include $(SOURCE_DIR)/$(AUDIO_SRC)/anc/llf/module.mk

CFLAGS  += -I$(SOURCE_DIR)/$(AUDIO_SRC)/$(PSAP_SRC)/inc
AUDIO_FILES += $(AUDIO_SRC)/$(PSAP_SRC)/src/audio_anc_psap_control.c
