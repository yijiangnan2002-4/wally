WWE prebuilt module usage guide

Brief:          This module is the WWE prebuilt library.

Usage:          XT-XCC:  For WWE, make sure to include the following:
                      1) Add the following module.mk for libs and source file:
                         include $(ROOTDIR)/middleware/airoha/dspalg/wwe/module.mk
                      2) module.mk provides different options to enable or disable according to the profiles, please configure these options on specified XT-XCC/feature.mk:
                         AIR_AMA_HOTWORD_ENABLE
                         AIR_AMA_HOTWORD_USE_PIC_ENABLE
                      3) Add the header file path:
                         INC += $(WWE_FUNC_PATH)/inc
                         INC += $(WWE_FUNC_PATH)/portable

Dependency:     None

Notice:         None

Relative doc:   None

Example project:None