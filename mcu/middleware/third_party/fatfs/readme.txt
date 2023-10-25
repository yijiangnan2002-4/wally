FatFS module usage guide

Brief:          This module is the file system implementation to manage file operation.
Usage:          GCC: Include the module with "include $(SOURCE_DIR)/middleware/third_party/fatfs/module.mk" in your GCC project Makefile.
Dependency:     If the device uses a SD module, please define HAL_SD_MODULE_ENABLED in hal_feature_config.h under the inc folder of your project.
Notice:         middleware/third_party/fatfs/src/diskio.c does not have RTC implementation.
                Please configure the RTC for FatFS under middleware/third_party/fatfs/src/diskio.c if you require time information.
Related doc:   Please refer to the open source user guide under the doc folder for more information.
Example project:None.
