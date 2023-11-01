###################################################
ifeq ($(AIR_BT_A2DP_SBC_ENCODER_ENABLE), y)
    # C_SRC += $(MIDDLEWARE_PROPRIETARY)/dspalg/sbc/src/sbc_interface.c
    # C_SRC += $(MIDDLEWARE_PROPRIETARY)/dspalg/sbc/src/sbc_header_parse.c
    C_SRC += $(MIDDLEWARE_PROPRIETARY)/dspalg/sbc/src/sbc_encoder_interface.c
#default usb pic lib
	PIC += $(strip $(LIBDIR2))/sbc/$(IC_CONFIG)/pisplit/pisplit_sbc_enc.o
    INC += $(MIDDLEWARE_PROPRIETARY)/dspalg/sbc/inc
	INC += $(MIDDLEWARE_PROPRIETARY)/dspalg/sbc/portable
    C_SRC += $(MIDDLEWARE_PROPRIETARY)/dspalg/sbc/portable/sbc_encoder_portable.c
endif