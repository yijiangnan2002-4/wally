LC3 module usage guide

Brief:          This module is the LC3 codec implementation for audio. It provides support for LC3 encode and decode.

Usage:          XT-XCC:  For LC3, make sure to include the following:
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/middleware/third_party/dspalg/lc3_codec/module.mk
                      2) module.mk provides different options to enable or disable according to the profiles.
                         Please configure the related options on the specified GCC/feature.mk(MCU side) & XT-XCC/feature.mk.(DSP side) with _leaudio (postfix)
                         AIR_LE_AUDIO_ENABLE
                      3) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/dspalg/lc3_codec/inc
                      4) Add CODEC_ENCODER_LC3/CODEC_DECODER_LC3 into the feature table in dsp_sdk.c to apply LC3 codec in the specified scenario, like AudioFeatureList_BLE_Call_UL[].

Dependency:     None.

Notice:         1) AIR_LE_AUDIO_ENABLE must be set as "y" on both MCU & DSP side

Relative doc:   None

Example project: None