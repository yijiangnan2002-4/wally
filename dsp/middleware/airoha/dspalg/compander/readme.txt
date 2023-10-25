Compander module usage guide

Brief:          This module is the Compander implementation.

Usage:          GCC:  For Compander, make sure to include the following:
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/dspalg/compander/module.mk
                      2) module.mk provides different options to enable or disable according to the profiles.
                         Please configure the related options on the specified XT-XCC/feature.mk.
                         AIR_DRC_ENABLE
                      3) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/dspalg/compander/inc
                      4) Add FUNC_DRC (for a2dp) / FUNC_RX_WB_DRC (for hfp downlink) / FUNC_TX_WB_DRC (for uplink) 
                         into the feature table in dsp_sdk.c to apply compander in the specified scenario, like stream_feature_list_a2dp[].

Dependency:     None

Relative doc:   None

Example project:None