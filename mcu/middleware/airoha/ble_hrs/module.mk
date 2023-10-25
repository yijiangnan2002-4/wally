###################################################
# Sources
###################################################

BLE_HRS_SOURCE = $(MIDDLEWARE_PROPRIETARY)/ble_hrs/src

C_FILES  += $(BLE_HRS_SOURCE)/ble_hrs.c

###################################################
# include path
###################################################
CFLAGS  += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/ble_hrs/inc
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_callback_manager/inc
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bluetooth/inc

