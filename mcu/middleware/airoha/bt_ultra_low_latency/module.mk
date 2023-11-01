ifeq ($(MTK_BT_ENABLE), y)
###################################################
# Sources
###################################################

####for ull spp
ifeq ($(AIR_BT_ULTRA_LOW_LATENCY_ENABLE), y)
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_common.c
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_service.c
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_audio_manager.c
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_sdp.c
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_utility.c
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_air_pairing.c
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_atci_cmd.c
ifeq ($(MTK_AWS_MCE_ENABLE), y)
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_aws_mce.c
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_aws_rho.c
endif

CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/inc
CFLAGS += -DAIR_BT_ULTRA_LOW_LATENCY_ENABLE

ifeq ($(AIR_BT_ULTRA_LOW_LATENCY_IDLE_SNIFF_ENABLE), y)
CFLAGS += -DAIR_BT_ULTRA_LOW_LATENCY_IDLE_SNIFF_ENABLE
endif
ifeq ($(AIR_BT_ULTRA_LOW_LATENCY_A2DP_STANDBY_ENABLE),y)
CFLAGS += -DAIR_BT_ULTRA_LOW_LATENCY_A2DP_STANDBY_ENABLE
endif

ifeq ($(AIR_GAMING_MODE_DONGLE_LINE_IN_ENABLE),y)
CFLAGS += -DAIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_IN_ENABLE
endif

ifeq ($(AIR_GAMING_MODE_DONGLE_LINE_OUT_ENABLE),y)
CFLAGS += -DAIR_BT_ULTRA_LOW_LATENCY_DONGLE_LINE_OUT_ENABLE
endif

ifeq ($(AIR_GAMING_MODE_DONGLE_I2S_IN_ENABLE),y)
CFLAGS += -DAIR_BT_ULTRA_LOW_LATENCY_DONGLE_I2S_IN_ENABLE
endif

CFLAGS += -DAIR_ULL_COMPATIBLE_ENABLE

endif

####for ull le
ifeq ($(AIR_BLE_ULTRA_LOW_LATENCY_ENABLE), y)
CFLAGS += -DAIR_BLE_ULTRA_LOW_LATENCY_ENABLE
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/inc

C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_common.c
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_atci_cmd.c
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_le_conn_service.c
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_le_service.c
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_le_utility.c
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_le_audio_manager.c
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_le_audio_transmitter.c
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_le_call_service.c
###################################################
# include path
###################################################
CFLAGS    +=    -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bluetooth/inc

###################################################
# Libs
###################################################
# The library name
ULL_LIB = libbt_ull.a

ULL_LIB_PATH = bluetooth

# check the bt_ultra_low_latency fodler exist or not.
# If the folder eixst, make the lib with source code
# otherwise, use the library directly.
#ifneq ($(wildcard $(strip $(SOURCE_DIR))/$(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/),)
#LIBS += $(OUTPATH)/$(ULL_LIB_PATH)
#MODULE_PATH += $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency
#else
LIBS += $(SOURCE_DIR)/prebuilt/$(MIDDLEWARE_PROPRIETARY)/$(ULL_LIB_PATH)/$(IC_CONFIG)/lib/$(ULL_LIB)
#endif

endif

####for ull le hid
ifeq ($(AIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE), y)
CFLAGS += -DAIR_BLE_ULTRA_LOW_LATENCY_WITH_HID_ENABLE
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/inc

C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_common.c
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_atci_cmd.c
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_le_hid_conn_service.c
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_le_hid_device_manager.c
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_le_hid_service.c
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_le_utility.c
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_le_audio_manager.c
C_FILES += $(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/src/bt_ull_le_audio_transmitter.c

###################################################
# include path
###################################################
CFLAGS    +=    -I$(SOURCE_DIR)/prebuilt/$(MIDDLEWARE_PROPRIETARY)/bluetooth/$(IC_CONFIG)/inc

###################################################
# Libs
###################################################
# The library name
ULL_LIB = libbt_ull.a

ULL_LIB_PATH = bluetooth

# check the bt_ultra_low_latency fodler exist or not.
# If the folder eixst, make the lib with source code
# otherwise, use the library directly.
#ifneq ($(wildcard $(strip $(SOURCE_DIR))/$(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency/),)
#LIBS += $(OUTPATH)/$(ULL_LIB_PATH)
#MODULE_PATH += $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_ultra_low_latency
#else
LIBS += $(SOURCE_DIR)/prebuilt/$(MIDDLEWARE_PROPRIETARY)/$(ULL_LIB_PATH)/$(IC_CONFIG)/lib/$(ULL_LIB)
#endif

endif
endif
