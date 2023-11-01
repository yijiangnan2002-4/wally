WWE module usage guide

Brief:          This module is the WWE implementation for audio. It provides support for AMA/GSOUND wake word detection.

Usage:          XT-XCC:  For WWE, make sure to include the following:
                      1) Add the following module.mk for libs and source file:
                         include $(ROOTDIR)/middleware/airoha/dspalg/wwe/module.mk
                      2) Module.mk provides different options to enable or disable according to the profiles.
                         Please configure the related options on the specified XT-XCC/feature.mk.
                         AIR_AMA_HOTWORD_ENABLE
                         AIR_AMA_HOTWORD_USE_PIC_ENABLE
                      3) Add the header file path:
                         INC += $(WWE_FUNC_PATH)/inc
                         INC += $(WWE_FUNC_PATH)/portable
                      4) Add FUNC_WWE_PREPROC/FUNC_WWE_PROC into the feature table in dsp_sdk.c to apply WWE in the specified scenario, like stream_feature_list_wwe_mic_record[].

Dependency:     None
                for changing WWE coefficient synchronization in earbuds project.

Notice:         None

Relative doc:   None

Example project: None
