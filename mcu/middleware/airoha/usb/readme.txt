USB module usage guide

Brief:          This module is the USB class interface.
Usage:          GCC:  Include the module with "include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/usb/module.mk" in your GCC project Makefile.
                      Please add "AIR_USB_ENABLE = y" in feature.mk under the project GCC folder if USB support is needed.
                      Please add "include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/usb/module.mk" and wrap them with "ifeq ($(AIR_USB_ENABLE), y)" and "endif" in Makefile under the project GCC folder.
Dependency:     Please define HAL_USB_MODULE_ENABLED in the hal_feature_config.h under the project inc folder.
Notice:         USB-CDC class was enabled in default. you can add other class in module.mk, for example AIR_USB_AUDIO_VERSION
Relative doc:   Please refer to the API reference under the doc folder for more detail.
Example project:USB-CDC/USB-Audio class:
                      Please find the project dongle_ref_design under project folder

