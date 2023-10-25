AGC module usage guide

Brief:          This module is the AGC implementation.

Usage:          GCC:  For AGC, make sure to include the following:
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/dspalg/agc/module.mk
                      2) module.mk provides different options to enable or disable according to the profiles.
                         Please configure the related options on the specified XT-XCC/feature.mk.
                         AIR_VOICE_AGC_ENABLE
                      3) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/dspalg/agc/inc
                      4) Add FUNC_RX_WB_AGC (for hfp downlink) / FUNC_TX_AGC (for uplink) 
                         into the feature table in dsp_sdk.c to apply agc in the specified scenario, like stream_feature_list_hfp_downlink[], stream_feature_list_hfp_uplink[].

Dependency:     None

Notice:         1) AIR_VOICE_AGC_ENABLE must be set as "y" on the specified XT-XCC/feature.mk.

Relative doc:   None

Example project:None