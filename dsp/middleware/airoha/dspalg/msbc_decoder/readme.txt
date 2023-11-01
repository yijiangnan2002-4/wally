mSBC Decoder/Encoder module usage guide

Brief:          This module is the mSBC Decoder/Encoder implementation.

Usage:          GCC:  For mSBC Decoder/Encoder, make sure to include the following:
                      1) Add the following module.mk for libs and source file:
                         include $(ROOTDIR)/middleware/airoha/dspalg/msbc_decoder/module.mk
                      2) Module.mk provides different options to enable or disable according to the profiles.
                         Please configure the related options on the specified XT-XCC/feature.mk.
                         AIR_BT_HFP_MSBC_ENABLE
                      3) Add CODEC_DECODER_MSBC (for hfp downlink) / CODEC_ENCODER_MSBC (for hfp uplink) into the feature table in dsp_sdk.c 
                         to apply mSBC decoder / encoder in the specified scenario, like stream_feature_list_hfp_downlink[].

Dependency:     None

Notice:         1) AIR_BT_HFP_MSBC_ENABLE must be set as "y" on the specified XT-XCC/feature.mk.

Relative doc:   None

Example project:None