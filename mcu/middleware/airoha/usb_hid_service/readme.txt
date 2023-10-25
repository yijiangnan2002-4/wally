USB HID Service module usage guide

Brief:          This module is the USB HID Service.
Usage:          GCC:  Include the module with "include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/usb_hid_service/module.mk" in your GCC project Makefile.
                      Please add "AIR_USB_ENABLE = y" in feature.mk under the project GCC folder if USB support is needed.
Dependency:     This module depends on $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/usb/src/hid
Notice:         None
Relative doc:   None
Example project:Please find the project dongle_ref_design under project folder

