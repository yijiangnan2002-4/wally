###################################################
# Sources
###################################################

BT_HOGP_SOURCE = $(MIDDLEWARE_PROPRIETARY)/bt_hogp/src


ifeq ($(AIR_HID_BT_HOGP_ENABLE),y)
C_FILES  += $(BT_HOGP_SOURCE)/bt_hogp_client.c
else ifeq ($(AIR_LE_AUDIO_WITH_HID_ENABLE),y)
C_FILES  += $(BT_HOGP_SOURCE)/bt_hogp_client.c
endif

ifeq ($(AIR_HOGP_ENABLE),y)
C_FILES  += $(BT_HOGP_SOURCE)/bt_hid_service.c  \
            $(BT_HOGP_SOURCE)/bt_hogp.c
CFLAGS += -DAIR_HOGP_ENABLE
endif

###################################################
# include path
###################################################
CFLAGS  += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_hogp/inc