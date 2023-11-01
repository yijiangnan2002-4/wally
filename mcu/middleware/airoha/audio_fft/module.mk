
AUDIO_FFT_SRC =  $(MIDDLEWARE_PROPRIETARY)/audio_fft

C_FILES  += $(AUDIO_FFT_SRC)/src/Audio_FFT.c

#################################################################################
#include path
CFLAGS     += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio_fft/inc
