-include $(SOURCE_DIR)/middleware/airoha/ble_ancs/module.mk
CFLAGS 	+= -I$(SOURCE_DIR)/prebuilt/middleware/airoha/ble_ancs/inc
LIBS += $(SOURCE_DIR)/prebuilt/middleware/airoha/ble_ancs/lib/libble_ancs.a
