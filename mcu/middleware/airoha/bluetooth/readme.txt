Bluetooth module usage guide

Brief:          This module is the bluetooth stack implementation. It supports EDR & BLE stack.
Usage:          GCC:  For EDR/BLE stack, include the module with 
                      1) Add the following module.mk for libs and source file:
                         include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bluetooth/module.mk
                      2) Module.mk provide different options to enable or disable according profiles, please configure these options on specified GCC/feature.mk:
                         AIR_BT_HFP_ENABLE
                      3) Add the header file path:
                         CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bluetooth/inc 
                         CFLAGS += -I$(SOURCE_DIR)/prebuilt/$(MIDDLEWARE_PROPRIETARY)/bluetooth/inc
                      4) copy the hci_log.h to the [project]/inc folder and hci_log.c to the [project]/src folder from the project 
                         under project folder with bt_ or ble_ prefix. Add the source file:
                            APP_FILES += $(APP_PATH_SRC)/hci_log.c
                      in your GCC project Makefile.

Dependency:     Please define HAL_DVFS_MODULE_ENABLED and HAL_AES_MODULE_ENABLED in the hal_feature_config.h under the project inc folder. 
Notice:         1) The UART port which HCI log use is assigned in the file hci_log.c. Please make sure you have been initialized the UART port in your project main function 
                   or return directly in the function hci_log() in the file hci_log.c if no need to catcher HCI log.
                2) BT trace maybe turn off defaultly afer bootup, please try "log set BT on info" command to turn it on.
Relative doc: Please refer to the bluetooth developer's guide for more detail.
Example project: Please find the project earbuds_ref_design or headset_ref_design under the project folder.
