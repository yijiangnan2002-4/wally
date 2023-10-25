Echo cancellation & Noise reduction module usage guide

Brief:          This module is the Echo cancellation & Noise reduction (ECNR) implementation.

Usage:          GCC:  For ECNR, make sure to include the following:
                      1) Add the following module.mk for libs and source file:
                         include $(ROOTDIR)/middleware/airoha/dspalg/ec_nr/module.mk
                      2) Module.mk provides different options to enable or disable according to the profiles.
                         Please configure the related options on the specified XT-XCC/feature.mk.
                         AIR_VOICE_NR_ENABLE
                      4) Add FUNC_RX_NR (hfp downlink) / FUNC_TX_NR (hfp uplink) into the feature table in dsp_sdk.c 
                         to apply ECNR in the specified scenario, like stream_feature_list_hfp_downlink[].

Dependency:     None

Notice:         1) AIR_VOICE_NR_ENABLE must be set as "y" on the specified XT-XCC/feature.mk.

Relative doc:   None

Example project:None