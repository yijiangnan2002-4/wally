inc module usage guide

Brief:          This module is used to support some audio output decoration. It provides channel selection & mute smoother.

Usage:          XT-XCC:  For inc, make sure to include the following:
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/dspalg/src/module.mk
                      2) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/dspalg/inc
                      3) Add FUNC_CH_SEL into the feature table in dsp_sdk.c to apply channel selection in the specified scenario, like stream_feature_list_a2dp[].
                         Add FUNC_MUTE_SMOOTHER into the feature table in dsp_sdk.c to apply mute smoother in the specified scenario, like stream_feature_list_a2dp[].

Dependency:     None.

Notice:         None.

Relative doc:   None

Example project: None
