clock skew prebuilt module usage guide

Brief:          This module is the library for clock skew implementation.

Usage:          For clock skew, make sure to include the following:
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/prebuilt/middleware/airoha/dspalg/clk_skew/module.mk
                      2) Set AIR_HWSRC_CLKSKEW_ENABLE as "y" in specified XT-XCC/feature.mk.
                      3) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/prebuilt/middleware/airoha/dspalg/clk_skew/inc
                      4) Add FUNC_CLK_SKEW_A2DP_DL into the feature table in dsp_sdk.c to apply clock skew in the specified scenario.

Dependency:     Please set AIR_HWSRC_ON_MAIN_STREAM_ENABLE as "y".
Notice:         None
Relative doc:   None
Example project:None