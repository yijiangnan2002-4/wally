###################################################
# Sources
###################################################

BLE_IAS_SOURCE = $(MIDDLEWARE_PROPRIETARY)/ble_ias/src

C_FILES  += $(BLE_IAS_SOURCE)/ble_ias.c

###################################################
# include path
###################################################
CFLAGS  += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/ble_ias/inc
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_callback_manager/inc
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bluetooth/inc

CFLAGS += -DMTK_BLE_IAS