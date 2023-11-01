ifeq ($(MTK_BT_DUO_ENABLE), y)
ifeq ($(findstring y,$(AIR_BT_A2DP_VENDOR_ENABLE) $(AIR_BT_A2DP_VENDOR_1_ENABLE)),y)
CFLAGS   += -DAIR_BT_A2DP_VENDOR_ENABLE
CFLAGS   += -DAIR_BT_A2DP_VENDOR_CODEC_SUPPORT
ifeq ($(findstring y,$(AIR_BT_A2DP_VENDOR_CODEC_BC_ENABLE) $(AIR_BT_A2DP_VENDOR_1_BC_ENABLE)),y)
CFLAGS   += -DMTK_BT_A2DP_VENDOR_CODEC_BC_ENABLE
CFLAGS   += -DMTK_BT_A2DP_VENDOR_1_CODEC_BC_ENABLE
endif
endif

ifeq ($(MTK_INITIAL_SYNC_BY_SAMPLE_INDEX_SUPPORT), y)
CFLAGS += -DMTK_INITIAL_SYNC_BY_SAMPLE_INDEX_SUPPORT
endif

LHDC_LIB=$(SOURCE_DIR)/prebuilt/middleware/third_party/lhdc_decoder/$(IC_CONFIG)/liblhdc_parser.a
ifeq ($(LHDC_LIB), $(wildcard $(LHDC_LIB)))
ifeq ($(AIR_BT_A2DP_LHDC_ENABLE), y)
CFLAGS += -DAIR_BT_A2DP_VENDOR_2_ENABLE
CFLAGS += -DAIR_BT_A2DP_VENDOR_CODEC_SUPPORT
LIBS += $(SOURCE_DIR)/prebuilt/middleware/third_party/lhdc_decoder/$(IC_CONFIG)/liblhdc_parser.a
include $(SOURCE_DIR)/middleware/third_party/lhdc_decoder/module.mk
endif
endif

ifeq ($(MTK_AUDIO_SYNC_ENABLE), y)
CFLAGS += -DMTK_AUDIO_SYNC_ENABLE
endif

ifeq ($(AIR_FEATURE_SINK_AUDIO_SWITCH_SUPPORT), y)
CFLAGS += -DAIR_FEATURE_SINK_AUDIO_SWITCH_SUPPORT
endif

ifeq ($(AIR_BT_MHDT_ENABLE), y)
CFLAGS += -DAIR_FEATURE_SINK_MHDT_SUPPORT
endif

# BT sink source files
BT_SINK_SRV_SRC = $(MIDDLEWARE_PROPRIETARY)/sink/src
BT_SINK_SRV_FILES = $(BT_SINK_SRV_SRC)/bt_sink_srv.c \
                    $(BT_SINK_SRV_SRC)/bt_sink_srv_atci_cmd.c \
                    $(BT_SINK_SRV_SRC)/bt_sink_srv_common.c \
                    $(BT_SINK_SRV_SRC)/bt_sink_srv_state_notify.c \
                    $(BT_SINK_SRV_SRC)/bt_sink_srv_utils.c \

# Sink call related
BT_SINK_SRV_CALL_SRC = $(MIDDLEWARE_PROPRIETARY)/sink/src/call
BT_SINK_SRV_FILES += $(BT_SINK_SRV_CALL_SRC)/bt_sink_srv_call_audio.c \
                     $(BT_SINK_SRV_CALL_SRC)/bt_sink_srv_call.c \
                     $(BT_SINK_SRV_CALL_SRC)/bt_sink_srv_call_pseudo_dev.c \
                     $(BT_SINK_SRV_CALL_SRC)/bt_sink_srv_call_pseudo_dev_mgr.c \
                     $(BT_SINK_SRV_CALL_SRC)/bt_sink_srv_hf.c \
                     $(BT_SINK_SRV_CALL_SRC)/bt_sink_srv_hf_call_manager.c \
                     $(BT_SINK_SRV_CALL_SRC)/bt_sink_srv_hf_multipoint.c \
                     $(BT_SINK_SRV_CALL_SRC)/bt_sink_srv_pbapc.c
ifeq ($(MTK_AWS_MCE_ENABLE), y)
BT_SINK_SRV_FILES += $(BT_SINK_SRV_CALL_SRC)/bt_sink_srv_aws_mce_call.c \
		             $(BT_SINK_SRV_CALL_SRC)/bt_sink_srv_call_rho.c
endif
BT_SINK_SRV_FILES += $(BT_SINK_SRV_CALL_SRC)/bt_sink_srv_hsp.c

# Sink bt_music related
BT_SINK_SRV_BT_MUSIC_SRC = $(MIDDLEWARE_PROPRIETARY)/sink/src/bt_music
BT_SINK_SRV_FILES += $(BT_SINK_SRV_BT_MUSIC_SRC)/bt_sink_srv_a2dp.c \
                     $(BT_SINK_SRV_BT_MUSIC_SRC)/bt_sink_srv_a2dp_callback.c \
                     $(BT_SINK_SRV_BT_MUSIC_SRC)/bt_sink_srv_a2dp_state_machine.c \
                     $(BT_SINK_SRV_BT_MUSIC_SRC)/bt_sink_srv_avrcp.c \
                     $(BT_SINK_SRV_BT_MUSIC_SRC)/bt_sink_srv_music.c \
                     $(BT_SINK_SRV_BT_MUSIC_SRC)/bt_sink_srv_music_iot_device.c


ifeq ($(MTK_BT_SPEAKER_FEC_ENABLE), y)
CFLAGS += -DMTK_BT_SPEAKER_FEC_ENABLE
BT_SINK_SRV_FILES += $(BT_SINK_SRV_BT_MUSIC_SRC)/speaker_fec.c
endif

# Sink le audio related
ifeq ($(AIR_LE_AUDIO_ENABLE), y)
# LE Audio
LE_AUDIO_LIB = $(strip $(SOURCE_DIR))/middleware/airoha/bt_le_audio/module.mk
ifeq  ($(LE_AUDIO_LIB), $(wildcard $(LE_AUDIO_LIB)))
BT_SINK_SRV_LE_AUDIO_SRC = $(MIDDLEWARE_PROPRIETARY)/sink/src/le_audio
BT_SINK_SRV_FILES += $(BT_SINK_SRV_LE_AUDIO_SRC)/bt_sink_srv_le_cap.c\
                     $(BT_SINK_SRV_LE_AUDIO_SRC)/bt_sink_srv_le_cap_audio_manager.c\
                     $(BT_SINK_SRV_LE_AUDIO_SRC)/bt_sink_srv_le_cap_stream.c\
                     $(BT_SINK_SRV_LE_AUDIO_SRC)/bt_sink_srv_le.c\
                     $(BT_SINK_SRV_LE_AUDIO_SRC)/bt_sink_srv_le_music.c\
                     $(BT_SINK_SRV_LE_AUDIO_SRC)/bt_sink_srv_le_call.c\
                     $(BT_SINK_SRV_LE_AUDIO_SRC)/bt_sink_srv_le_volume.c
endif
endif

# Sink State Manager related
ifeq ($(AIR_BT_SINK_SRV_STATE_MANAGER_ENABLE), y)
CFLAGS += -DAIR_BT_SINK_SRV_STATE_MANAGER_ENABLE
CFLAGS += -DAIR_BT_SINK_SRV_STATE_MANAGER_DUMMY_DEIVCE_ENABLE
ifeq ($(AIR_BT_INTEL_EVO_ENABLE), y)
CFLAGS += -DAIR_BT_SINK_SRV_CUSTOMIZED_ENABLE
endif
BT_SINK_SRV_STATE_MANAGER_SRC = $(MIDDLEWARE_PROPRIETARY)/sink/src/state_manager
BT_SINK_SRV_FILES += $(BT_SINK_SRV_STATE_MANAGER_SRC)/bt_sink_srv_state_manager.c \
                     $(BT_SINK_SRV_STATE_MANAGER_SRC)/bt_sink_srv_state_manager_action.c \
                     $(BT_SINK_SRV_STATE_MANAGER_SRC)/bt_sink_srv_state_manager_am.c \
                     $(BT_SINK_SRV_STATE_MANAGER_SRC)/bt_sink_srv_state_manager_config.c \
                     $(BT_SINK_SRV_STATE_MANAGER_SRC)/bt_sink_srv_state_manager_rho.c \
                     $(BT_SINK_SRV_STATE_MANAGER_SRC)/bt_sink_srv_state_manager_ring.c \
                     $(BT_SINK_SRV_STATE_MANAGER_SRC)/bt_sink_srv_state_manager_state.c \
                     $(BT_SINK_SRV_STATE_MANAGER_SRC)/bt_sink_srv_state_manager_psedev.c
endif

# Sink avm_direct related
BT_SINK_SRV_AVM_DIRECT = $(MIDDLEWARE_PROPRIETARY)/sink/src/avm_direct
BT_SINK_SRV_FILES += $(BT_SINK_SRV_AVM_DIRECT)/avm_direct_util.c \

ifeq ($(MTK_AWS_MCE_ENABLE), y)
BT_SINK_SRV_FILES += $(BT_SINK_SRV_BT_MUSIC_SRC)/bt_sink_srv_aws_mce_a2dp.c
ifeq ($(AIR_BT_ROLE_HANDOVER_SERVICE_ENABLE), y)
BT_SINK_SRV_FILES += $(BT_SINK_SRV_BT_MUSIC_SRC)/bt_sink_srv_music_rho.c
endif
endif
# AWS MCE related
ifeq ($(MTK_AWS_MCE_ENABLE), y)
BT_SINK_SRV_FILES += $(BT_SINK_SRV_SRC)/bt_sink_srv_aws_mce.c
ifeq ($(MTK_PROMPT_SOUND_SYNC_ENABLE), y)
BT_SINK_SRV_FILES += $(BT_SINK_SRV_SRC)/bt_sink_srv_aws_mce_vp_sync.c
CFLAGS += -DMTK_PROMPT_SOUND_SYNC_ENABLE
endif
endif

ifneq ($(findstring $(AIR_DCHS_MODE_MASTER_ENABLE)$(AIR_DCHS_MODE_SLAVE_ENABLE),  y  y),)
BT_SINK_SRV_FILES += $(BT_SINK_SRV_SRC)/bt_sink_srv_dual_ant.c
CFLAGS += -DBT_SINK_DUAL_ANT_ENABLE
endif

ifeq ($(AIR_DUAL_CHIP_MIXING_MODE),master)
BT_SINK_SRV_FILES += $(BT_SINK_SRV_SRC)/bt_sink_srv_dual_ant.c
CFLAGS += -DBT_SINK_DUAL_ANT_ENABLE
CFLAGS += -DBT_SINK_DUAL_ANT_ROLE_MASTER
endif

ifeq ($(AIR_DUAL_CHIP_MIXING_MODE),slave)
BT_SINK_SRV_FILES += $(BT_SINK_SRV_SRC)/bt_sink_srv_dual_ant.c
CFLAGS += -DBT_SINK_DUAL_ANT_ENABLE
CFLAGS += -DBT_SINK_DUAL_ANT_ROLE_SLAVE
endif

C_FILES += $(BT_SINK_SRV_FILES)

# BT callback manager module
include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_callback_manager/module.mk

# BT device manager module
include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bluetooth_service/bt_device_manager_module.mk

# Audio manager
AUDIO_MANAGER_MODULE = $(strip $(SOURCE_DIR))/$(MIDDLEWARE_PROPRIETARY)/audio_manager/module.mk
ifeq ($(AUDIO_MANAGER_MODULE), $(wildcard $(AUDIO_MANAGER_MODULE)))
include $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio_manager/module.mk
endif

# Include bt sink path
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bluetooth_service/inc
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/sink/inc
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/sink/inc/call
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/sink/inc/bt_music
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/sink/inc/le_audio
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/sink/inc/state_manager
#CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_sink/inc/audio_command_receiver
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bluetooth/inc
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_le_audio/inc
CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/bt_le_audio/inc/bt_bap
CFLAGS += -I$(SOURCE_DIR)/driver/chip/inc

ifeq ($(MTK_AVM_DIRECT), y)
    CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/bt_codec/inc
else
    CFLAGS += -I$(SOURCE_DIR)/driver/board/mt25x3_hdk/bt_codec/inc
endif

endif

ifeq ($(AIR_WAV_DECODER_ENABLE), y)
CFLAGS += -DAIR_WAV_DECODER_ENABLE
endif

ifeq ($(AIR_MP3_DECODER_ENABLE), y)
CFLAGS += -DAIR_MP3_DECODER_ENABLE
endif

ifeq ($(MTK_AUDIO_GAIN_TABLE_ENABLE), y)
CFLAGS += -DMTK_AUDIO_GAIN_TABLE_ENABLE
endif

ifeq ($(MTK_AM_NOT_SUPPORT_STREAM_IN), y)
CFLAGS += -DMTK_AM_NOT_SUPPORT_STREAM_IN
endif

LIBS += $(SOURCE_DIR)/prebuilt/$(MIDDLEWARE_PROPRIETARY)/bluetooth/$(IC_CONFIG)/lib/libavm_direct.a

