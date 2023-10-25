SBC encoder module usage guide

Brief:          This module is the SBC encoder implementation, the module is reserved for future usage.

Usage:          GCC:  For SBC encoder, make sure to include the following:
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/middleware/airoha/dspalg/sbc/module.mk
                      2) module.mk provides different options to enable or disable according to the profiles.
                         Please configure the related options on the specified XT-XCC/feature.mk.
                         AIR_BT_A2DP_SBC_ENCODER_ENABLE
                      3) Add the header file path:
                         INC += $(MIDDLEWARE_PROPRIETARY)/dspalg/sbc/inc
                      4) Add CODEC_ENCODER_SBC into the feature table in dsp_sdk.c
                         to apply SBC encoder in the specified scenario, like stream_feature_list_bt_audio_dongle_usb_in_broadcast_0[].

Dependency:     SBC encoder module is only effective when the SBC encoder prebuilt module also exists.

Notice:         1) AIR_BT_A2DP_SBC_ENCODER_ENABLE must be set as "y" on the specified XT-XCC/feature.mk.

Relative doc:   None

Example project:None