FOTA module usage guide

Brief:          This module is the firmware update implementation.
Usage:          Include the module with "include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/fota/module.mk" in your GCC project Makefile and set AIR_FOTA_ENABLE to "y" in feature.mk.
Dependency:     HAL_FLASH_MODULE_ENABLED and HAL_WDT_MODULE_ENABLED must also defined in the hal_feature_config.h  under the project inc folder.
Notice:         None.
Relative doc:   Please refer to the firmware update developer guide under the doc folder for more detail.
Example project:Please find the earbuds_ref_design project under the project folder.