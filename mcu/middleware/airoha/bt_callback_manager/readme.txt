Bluetooth callback manager module usage guide

Brief:           This module is to manage the callback function of the Bluetooth stack. User can register and deregister the 
                 EDR/BLE callback function to handle different callback functions. 
Usage:           GCC: 
                      1) Add the following module.mk for include path and source file:
                         include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_callback_manager/module.mk
                         in your GCC project Makefile.


Dependency:      This module has the dependency with Bluetooth. Please also make sure to include Bluetooth.
                 
Notice:          None.
Relative doc:    None.
Example project: Please find the projects earbuds_ref_design or headset_ref_design under the project folder.
