ifeq ($(MTK_BT_ENABLE), y)
###################################################
# Libs
###################################################

ifeq ($(AIR_BT_FAST_PAIR_ENABLE), y)
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_fast_pair/src/bt_fast_pair_utility.c
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_fast_pair/src/bt_fast_pair_spp.c
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_fast_pair/src/bt_fast_pair_log.c
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_fast_pair/src/bt_fast_pair_l2cap_le.c
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_fast_pair/inc
CFLAGS += -DAIR_BT_FAST_PAIR_ENABLE
ifeq ($(AIR_BT_FAST_PAIR_LE_AUDIO_ENABLE), y)
CFLAGS += -DAIR_BT_FAST_PAIR_LE_AUDIO_ENABLE
endif

ifeq ($(AIR_SPOT_ENABLE), y)
FAST_PAIR_LIB = libbt_fast_pair_spot.a
else
FAST_PAIR_LIB = libbt_fast_pair.a
endif

ifeq ($(AIR_BT_FAST_PAIR_SASS_ENABLE), y)
CFLAGS += -DAIR_BT_FAST_PAIR_SASS_ENABLE
endif

export AIR_SPOT_ENABLE

ifneq ($(wildcard $(strip $(SOURCE_DIR))/$(MIDDLEWARE_PROPRIETARY)/bt_fast_pair_protected/),)
LIBS += $(OUTPATH)/$(FAST_PAIR_LIB)
MODULE_PATH += $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_fast_pair_protected/GCC
else
LIBS += $(SOURCE_DIR)/prebuilt/$(MIDDLEWARE_PROPRIETARY)/bluetooth/$(IC_CONFIG)/lib/$(FAST_PAIR_LIB)
endif

# AIR_BT_FAST_PAIR_ENABLE
endif

endif
