###################################################
# Sources
###################################################

BT_CMGR_SOURCE = $(MIDDLEWARE_PROPRIETARY)/bt_callback_manager/src
BT_CMGR_FILES =  $(BT_CMGR_SOURCE)/bt_callback_manager.c


C_FILES += $(BT_CMGR_FILES)
###################################################
# include path
###################################################

CFLAGS  += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_callback_manager/inc
