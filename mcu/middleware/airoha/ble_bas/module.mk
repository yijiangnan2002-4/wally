###################################################
# Sources
###################################################

BLE_BAS_SOURCE = $(MIDDLEWARE_PROPRIETARY)/ble_bas/src

C_FILES  += $(BLE_BAS_SOURCE)/ble_bas.c

###################################################
# include path
###################################################
CFLAGS  += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/ble_bas/inc
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_callback_manager/inc
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bluetooth/inc

CFLAGS += -DMTK_BLE_BAS
