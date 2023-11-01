ble_hrs module usage guide

Brief:          This module is the Heart Rate Service (HRS) implementation. HRS exposes the PLX Features and Record Access Control Point to the peer
				device by a Bluetooth low-energy link. This module supports PLX Spot-check Measurement indication and PLX Continuous Measurement notification
				to the peer device and execute a desired procedure from the peer device.
Usage:          GCC: 1. Include the module with "include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/ble_hrs/module.mk" in the GCC project Makefile
                KEIL: 1. Drag the $(MIDDLEWARE_PROPRIETARY)/ble_hrs folder to the project.
                      2. Add $(MIDDLEWARE_PROPRIETARY)/ble_hrs/inc to include paths.
                IAR: 1. Drag the $(MIDDLEWARE_PROPRIETARY)/ble_hrs folder to the project.
                     2. Add $(MIDDLEWARE_PROPRIETARY)/ble_hrs/inc to include paths.
Dependency:     Please set AIR_BLE_HRS_ENABLE to be "y" in feature.mk.
Notice:         None.
Relative doc:   None.
Example project:project/mt2833/apps/earbuds_ref_design/src/apps/app_hrs/ble_app_hrs.


