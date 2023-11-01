AAC decoder prebuilt module usage guide

Brief:          This module is the library for AAC decoder implementation.

Usage:          For AAC decoder, make sure to include the following:
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/middleware/airoha/dspalg/aac_decoder/module.mk
                      2) Set AIR_BT_A2DP_AAC_ENABLE as "y" in specified XT-XCC/feature.mk.
                      3) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/middleware/airoha/dspalg/aac_decoder/inc
                      4) Add CODEC_DECODER_AAC into the feature table in dsp_sdk.c to apply AAC decoder in the specified scenario.

Dependency:     None
Notice:         None
Relative doc:   None
Example project:None
