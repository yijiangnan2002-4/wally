Low latency framework module usage guide
Brief:          This module is the example code for Low latency framework (LLF) implementation.
Usage:          GCC:  For LLF, make sure to include the following:
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/middleware/airoha/dspalg/low_latency_framework_example/module.mk
                      2) module.mk provides different options to enable or disable according to the profiles.
                         AIR_CUSTOMIZED_LLF_ENABLE
                      3) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/middleware/airoha/dspalg/low_latency_framework_example/inc
                      4) Add FUNC_LLF_SAMPLE into the feature table in dsp_sdk.c to apply LLF.
Dependency:     None
Notice:         None
Relative doc:   None
Example project:None