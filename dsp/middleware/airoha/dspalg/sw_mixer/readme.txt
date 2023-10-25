Software mxier module usage guide

Brief:          This module is the Software mixer (sw_mxier) implementation.

Usage:          GCC:  For sw_mxier, make sure to include the following:
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/middleware/airoha/dspalg/sw_mxier/module.mk
                      2) Module.mk provides different options to enable or disable according to the profiles.
                         Please configure the related options on the specified XT-XCC/feature.mk.
                         AIR_SOFTWARE_MIXER_ENABLE
                      3) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/middleware/airoha/dspalg/sw_mxier/inc
                      4) Add FUNC_SW_MXIER into the feature table in dsp_sdk.c to apply sw_mxier in the specified scenario and then call sw_mxier APIs to configure port settings.

Dependency:     None

Notice:         AIR_SOFTWARE_MIXER_ENABLE must be set as "y" on the specified XT-XCC/feature.mk.

Relative doc:   None

Example project:None