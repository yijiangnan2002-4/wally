Race cmd module usage guide

Brief:          This module is the Race command interface.
Usage:          Include the module with "include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/race_cmd/module.mk" in your GCC project Makefile.
Dependency:     Please define HAL_UART_MODULE_ENABLED in the hal_feature_config.h under the project inc folder.
Notice:         To enable the Race command provided under $(MIDDLEWARE_PROPRIETARY)/race_cmd folder, please set AIR_RACE_CMD_ENABLE to "y" in feature.mk
Relative doc:   Please refer to the API reference under the doc folder for more detail.
Example project:Please find the earbuds_ref_design project under the project folder.