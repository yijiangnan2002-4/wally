###################################################
# Sources
###################################################

BT_HOGP_SOURCE = $(MIDDLEWARE_PROPRIETARY)/bt_hogp/src

C_FILES  += $(BT_HOGP_SOURCE)/bt_hid_service.c  \
            $(BT_HOGP_SOURCE)/bt_hogp.c

ifeq ($(AIR_HOGP_ENABLE),y)
CFLAGS += -DAIR_HOGP_ENABLE
endif

###################################################
# include path
###################################################
CFLAGS  += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_hogp/inc