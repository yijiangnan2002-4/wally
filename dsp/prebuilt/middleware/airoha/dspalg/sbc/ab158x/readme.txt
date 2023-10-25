SBC encoder prebuilt module usage guide

Brief:          This module contains the library for SBC encoder hence the module is reserved and can only be to enabled in certain projects.

Usage:          GCC:  For SBC encoder, make sure to include the following:
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/middleware/airoha/dspalg/sbc/module.mk
                      2) Add the header file path:
                         INC += $(MIDDLEWARE_PROPRIETARY)/dspalg/sbc/inc
                      3) Add CODEC_ENCODER_SBC into the feature table in dsp_sdk.c
                         to apply SBC encoder in the specified scenario, like stream_feature_list_bt_audio_dongle_usb_in_broadcast_0[].

Dependency:     SBC encoder prebuilt module is only effective when the SBC encoder module also exists.

Notice:         None

Relative doc:   None

Example project: dongle_ref_design