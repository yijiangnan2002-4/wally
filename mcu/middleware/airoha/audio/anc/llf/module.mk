###################################################
AUDIO_SRC = $(MIDDLEWARE_PROPRIETARY)/audio
LLF_SRC = anc/llf


CFLAGS  += -I$(SOURCE_DIR)/$(AUDIO_SRC)/$(LLF_SRC)/inc
AUDIO_FILES += $(AUDIO_SRC)/$(LLF_SRC)/src/audio_anc_llf_control.c

ifeq ($(AIR_CUSTOMIZED_LLF_ENABLE),y)
AUDIO_FILES += $(AUDIO_SRC)/$(LLF_SRC)/src/audio_anc_llf_example.c
endif
