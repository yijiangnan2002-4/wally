bt_air module usage guide

Brief:          This module is the BT AIR Service implementation. It includes 4 modules (BLE AIR, SPP AIR, AirUpdate, GATT OVER BREDR AIR). It can help the AIR application communicate with peer device
                by low-energy link or Bluetooth SPP profile or AirUpdate Fixed channel or GATT OVER BREDR. This module manages all user's registered callbacks and notify
                all users when RX data event occurs from the peer device.
Usage:          Include the module with "include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_air/module.mk" in the GCC project Makefile.
Dependency:     This module depends on Bluetooth. Please include Bluetooth module.mk in your project.
Notice:         None.
Relative doc:   None.
Example project:Please find the earbuds_ref_design project under the project folder.