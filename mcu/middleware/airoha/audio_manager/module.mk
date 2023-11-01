###################################################
# Sources
###################################################

AUDIO_MANAGER_SRC= $(MIDDLEWARE_PROPRIETARY)/audio_manager/src

AUDIO_MANAGER_SRV_FILES  = $(AUDIO_MANAGER_SRC)/audio_src_srv.c \
                           $(AUDIO_MANAGER_SRC)/audio_src_srv_reserve.c \
                           $(AUDIO_MANAGER_SRC)/audio_src_srv_state_mgr.c \
                           $(AUDIO_MANAGER_SRC)/bt_sink_srv_am_task.c \
                           $(AUDIO_MANAGER_SRC)/bt_sink_srv_ami.c \
                           $(AUDIO_MANAGER_SRC)/bt_sink_srv_audio_setting.c \
                           $(AUDIO_MANAGER_SRC)/sidetone_control.c \
                           $(AUDIO_MANAGER_SRC)/sidetone_playback.c \
                           $(AUDIO_MANAGER_SRC)/peq_setting.c \
                           $(AUDIO_MANAGER_SRC)/ecnr_setting.c \
                           $(AUDIO_MANAGER_SRC)/audio_set_driver.c \

C_FILES += $(AUDIO_MANAGER_SRV_FILES)

ifeq ($(MTK_AUDIO_TUNING_ENABLED), y)
AUDIO_TUNNING_SRC = $(MIDDLEWARE_PROPRIETARY)/audio_manager/src
AUDIO_TUNNING_FILES = $(AUDIO_TUNNING_SRC)/bt_sink_srv_audio_tunning.c
C_FILES += $(AUDIO_TUNNING_FILES)
endif

###################################################
# include path
###################################################
CFLAGS  += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio_manager/inc
CFLAGS  += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/audio/port/$(IC_CONFIG)/inc/nvdm
