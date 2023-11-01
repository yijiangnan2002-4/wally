Bluetooth role handover service module usage guide

Brief:           This module is to manage all Bluetooth role handover users. User can register and deregister their
                 callback functions to complete the role handover procedure.
Usage:           GCC:
                      1) Add the following module.mk for include path and source file:
                         include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_role_handover/module.mk
                         in your GCC project Makefile.

Dependency:      This module has the dependency with Bluetooth, AVM direct feature and AWS MCE profile. Please also make sure to include Bluetooth.
Notice:          None.
Relative doc:    None.
Example project: Please find the project earbuds_ref_design under the project folder.
