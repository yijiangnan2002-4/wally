
###################################################
SURROUND_AUDIO = middleware/third_party/dspalg/surround_audio
C_SRC += $(SURROUND_AUDIO)/src/surround_audio.c
# LIBS += $(strip $(LIBDIR3))/surround_audio/libsurround_audio.a

###################################################
# include path

INC += $(SURROUND_AUDIO)/inc
