ATCI module usage guide

Brief:          This module is the AT command interface.
Usage:          GCC: Include the module with "include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/atci/module.mk" in your GCC project Makefile.
Dependency:     Please define HAL_UART_MODULE_ENABLED in the hal_feature_config.h under the project inc folder.  
Notice:         To disable the Airoha default AT command provided under $(MIDDLEWARE_PROPRIETARY)/atci/at_command folder, please define MTK_AT_CMD_DISABLE.  
Relative doc:   Please refer to the API reference under the doc folder for more detail.
Example project:Please find the earbuds_ref_design project under project folder.
