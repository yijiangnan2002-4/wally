###################################################
# Sources
###################################################

BT_RHOS_SOURCE = $(MIDDLEWARE_PROPRIETARY)/bt_role_handover/src
BT_RHOS_FILES =  $(BT_RHOS_SOURCE)/bt_role_handover.c
ifeq ($(AIR_MULTI_POINT_ENABLE),y)
BT_RHOS_FILES +=  $(BT_RHOS_SOURCE)/bt_role_handover_multipoint.c
endif


C_FILES += $(BT_RHOS_FILES)
###################################################
# include path
###################################################

CFLAGS  += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_role_handover/inc
