bluetooth_service module usage guide

Brief:          This module is the bluetooth common services implementation. It includes 3 services, one is Device Manager Service, which manages bonded peer 
                device's security information, and the other is GATT Server Service, which supports GAP service and characteristics configuration 
                feature, and the last is BT GAP LE Service, which manages the LE advertising operation and the device's connection information and connection parameters.
Usage:          GCC: 1. Include the module with "include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bluetooth_service/module.mk" in the GCC project Makefile
Dependency: None
Notice:         None.
Relative doc:   None.
Example project:Please find the project earbuds_ref_design or headset_ref_design under the project folder.


