			 

#################################################################################

SMART_HOME_SOURCE = $(MIDDLEWARE_PROPRIETARY)/ble_ancs/src
C_FILES  += $(SMART_HOME_SOURCE)/ble_ancs_porting.c  
C_FILES  += $(SMART_HOME_SOURCE)/ble_ancs_log.c  
#include path
CFLAGS 	+= -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/ble_ancs/inc

#LIBS += $(SOURCE_DIR)/prebuilt/$(MIDDLEWARE_PROPRIETARY)/ble_ancs/lib/libble_ancs.a

#################################################################################
