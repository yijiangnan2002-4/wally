###################################################
AUDIO_SRC = $(MIDDLEWARE_PROPRIETARY)/audio
TRANSMITTER_SRC = $(MIDDLEWARE_PROPRIETARY)/audio/audio_transmitter

CFLAGS  += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/audio_transmitter/inc
AUDIO_FILES += $(AUDIO_SRC)/audio_transmitter/src/audio_transmitter_playback.c
AUDIO_FILES += $(AUDIO_SRC)/audio_transmitter/src/audio_transmitter_control.c
AUDIO_FILES += $(AUDIO_SRC)/audio_transmitter/src/audio_transmitter_playback_port.c

CFLAGS  += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/audio_transmitter/inc/audio_transmitter_scenario_port


####################PORT###########################
AUDIO_FILES += $(TRANSMITTER_SRC)/src/audio_transmitter_scenario_port/scenario_gsensor.c
AUDIO_FILES += $(TRANSMITTER_SRC)/src/audio_transmitter_scenario_port/scenario_multi_mic_stream.c
AUDIO_FILES += $(TRANSMITTER_SRC)/src/audio_transmitter_scenario_port/scenario_gaming_mode.c
AUDIO_FILES += $(TRANSMITTER_SRC)/src/audio_transmitter_scenario_port/scenario_anc_monitor_stream.c
AUDIO_FILES += $(TRANSMITTER_SRC)/src/audio_transmitter_scenario_port/scenario_test.c
AUDIO_FILES += $(TRANSMITTER_SRC)/src/audio_transmitter_scenario_port/scenario_tdm.c
AUDIO_FILES += $(TRANSMITTER_SRC)/src/audio_transmitter_scenario_port/scenario_wired_audio.c
AUDIO_FILES += $(TRANSMITTER_SRC)/src/audio_transmitter_scenario_port/scenario_ble_audio.c
AUDIO_FILES += $(TRANSMITTER_SRC)/src/audio_transmitter_scenario_port/scenario_audio_hw_loopback.c
AUDIO_FILES += $(TRANSMITTER_SRC)/src/audio_transmitter_scenario_port/scenario_adaptive_eq_monitor_stream.c
AUDIO_FILES += $(TRANSMITTER_SRC)/src/audio_transmitter_scenario_port/scenario_ull_audio_v2.c
AUDIO_FILES += $(TRANSMITTER_SRC)/src/audio_transmitter_scenario_port/scenario_dchs.c
AUDIO_FILES += $(TRANSMITTER_SRC)/src/audio_transmitter_scenario_port/scenario_wireless_mic_rx.c
AUDIO_FILES += $(TRANSMITTER_SRC)/src/audio_transmitter_scenario_port/scenario_bt_audio.c

AUDIO_FILES += $(TRANSMITTER_SRC)/src/audio_transmitter_scenario_port/scenario_dongle_common.c
AUDIO_FILES += $(TRANSMITTER_SRC)/src/audio_transmitter_scenario_port/scenario_advanced_record.c

ifeq ($(AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_TRACKING_VDMA_MODE_ENABLE), y)
CFLAGS  += -DAIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_TRACKING_VDMA_MODE_ENABLE
endif

ifeq ($(AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_TRACKING_MEMIF_MODE_ENABLE), y)
CFLAGS  += -DAIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_TRACKING_MEMIF_MODE_ENABLE
endif

