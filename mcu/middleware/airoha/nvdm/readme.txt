NVDM module usage guide

Brief:          This module is the non-volatile data management implementation.
Usage:          GCC: Please add "include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/nvdm/module.mk" in your GCC project Makefile, and configure "AIR_NVDM_ENABLE = y" on specified GCC/feature.mk.
Dependency:     Please define HAL_FLASH_MODULE_ENABLED in hal_feature_config.h under the project inc folder.
Notice:         To customize the nvdm setting, please configure nvdm_port.c in $(MIDDLEWARE_PROPRIETARY)/nvdm/src folder.
                The users also should include their code with option "AIR_NVDM_ENABLE".
Relative doc:   Please refer to the section of NVDM in Airoha IoT SDK for xxxx API Reference Manual.html under the doc folder for more detail.
Example project:none.
