SBC decoder prebuilt module usage guide

Brief:          This module is the library for SBC decoder implementation.

Usage:          For SBC decoder, make sure to include the following:
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/middleware/airoha/dspalg/sbc_decoder/module.mk
                      2) Set AIR_BT_A2DP_SBC_ENABLE as "y" in specified XT-XCC/feature.mk.
                      3) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/middleware/airoha/dspalg/sbc_decoder/inc
                      4) Add CODEC_DECODER_SBC into the feature table in dsp_sdk.c to apply SBC decoder in the specified scenario.

Dependency:     None
Notice:         None
Relative doc:   None
Example project:None