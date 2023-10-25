###################################################
# Sources
###################################################

BT_COMP_SOURCE = $(MIDDLEWARE_PROPRIETARY)/bluetooth_service/src

CFLAGS += -DAIR_GATT_SERVICE_CHANGE_ENABLE
C_FILES  += $(BT_COMP_SOURCE)/bt_device_manager_le.c \
            $(BT_COMP_SOURCE)/bt_gatts_service.c

ifeq ($(AIR_GATTC_DISCOVERY_ENHANCE_ENABLE), y)
C_FILES += $(BT_COMP_SOURCE)/bt_gattc_discovery.c
CFLAGS += -DAIR_GATTC_DISCOVERY_ENHANCE_ENABLE
endif

ifeq ($(AIR_GATT_SRV_CLIENT_ENABLE), y)
C_FILES += $(BT_COMP_SOURCE)/bt_gatt_service_client.c
CFLAGS += -DAIR_GATT_SRV_CLIENT_ENABLE
endif

ifeq ($(MTK_BLE_GAP_SRV_ENABLE), y)
C_FILES  += $(BT_COMP_SOURCE)/bt_gap_le_service.c \
            $(BT_COMP_SOURCE)/bt_gap_le_service_utils.c \
            $(BT_COMP_SOURCE)/bt_utils.c

CFLAGS += -DMTK_BLE_GAP_SRV_ENABLE
endif
###################################################
# include path
###################################################
CFLAGS  += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bluetooth_service/inc
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_callback_manager/inc
ifeq ($(MTK_NVDM_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/nvdm/inc
endif
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bluetooth/inc

 
