Packet Lost Compensation module usage guide

Brief:          This module is the Packet Lost Compensation (PLC) implementation.

Usage:          GCC:  For PLC, make sure to include the following:
                      1) Add the following module.mk for libs and source file:
                         include $(ROOTDIR)/middleware/airoha/dspalg/voice_plc/module.mk
                      2) Module.mk provides different options to enable or disable according to the profiles.
                         Please configure the related options on the specified XT-XCC/feature.mk.
                         AIR_VOICE_PLC_ENABLE
                      3) Add FUNC_PLC into the feature table in dsp_sdk.c to apply PLC in the specified scenario, like stream_feature_list_hfp_downlink[].

Dependency:     None

Notice:         AIR_VOICE_PLC_ENABLE must be set as "y" on the specified XT-XCC/feature.mk.

Relative doc:   None

Example project:None