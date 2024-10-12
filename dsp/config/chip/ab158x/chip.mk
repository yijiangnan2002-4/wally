XTENSA_CORE               = AIR_PREMIUM_G3_HIFI5
MIDDLEWARE_PROPRIETARY    = middleware/airoha
MTK_SYSLOG_VERSION_2     ?= y
MTK_SYSLOG_SUB_FEATURE_STRING_LOG_SUPPORT = y
MTK_SYSLOG_SUB_FEATURE_BINARY_LOG_SUPPORT = y
MTK_SYSLOG_SUB_FEATURE_MSGID_TO_STRING_LOG_SUPPORT = n
MTK_SYSLOG_SUB_FEATURE_USB_ACTIVE_MODE = y
MTK_SYSLOG_SUB_FEATURE_OFFLINE_DUMP_ACTIVE_MODE = y
MTK_DEBUG_PLAIN_LOG_ENABLE            = n
AIR_SYSLOG_STACK_MONITOR_ENABLE      := y
MTK_SAVE_LOG_AND_CONTEXT_DUMP_ENABLE  ?= n
MTK_CPU_NUMBER_1                      ?= y
MTK_MUX_ENABLE                        ?= y
FPGA_ENV                              ?= n
CCCI_ENABLE                            = n

AIR_HEAP_BEST_FIT                     ?= y

AIR_CPU_MCPS_PRIORING_ENABLE          ?= y

AIR_LE_AUDIO_DONGLE_ENABLE ?= n
AIR_ULL_GAMING_DONGLE_ENABLE ?= n
AIR_ULL_AUDIO_V2_DONGLE_ENABLE ?= n
AIR_BT_AUDIO_DONGLE_ENABLE ?= n
AIR_AUDIO_HARDWARE_ENABLE ?= y

ifneq ($(AIR_LE_AUDIO_DONGLE_ENABLE)_$(AIR_ULL_GAMING_DONGLE_ENABLE)_$(AIR_ULL_AUDIO_V2_DONGLE_ENABLE)_$(AIR_BT_AUDIO_DONGLE_ENABLE), n_n_n_n)
else
ifeq ($(AIR_AUDIO_HARDWARE_ENABLE),y)
AIR_NLE_ENABLE                        := y
AIR_RESET_SDM_ENABLE                  := y
ifeq ($(AIR_NLE_ENABLE),y)
AIR_AUDIO_SILENCE_DETECTION_ENABLE          := y
endif
ifeq ($(AIR_RESET_SDM_ENABLE),y)
AIR_AUDIO_SILENCE_DETECTION_ENABLE          := y
endif
endif
endif

IOT_SDK_XTENSA_VERSION                := 9018
AIR_BT_A2DP_ENABLE ?= y
#MTK_BT_A2DP_AAC_ENABLE ?= y
AIR_BT_A2DP_SBC_ENABLE ?= y
AIR_BT_HFP_MSBC_ENABLE ?= y
AIR_COMPONENT_CALIBRATION_ENABLE ?= y
AIR_ECHO_MEMIF_IN_ORDER_ENABLE := y
AIR_ECHO_PATH_STEREO_ENABLE    ?= n
AIR_AUDIO_NONREALTIME_RX_ENABLE ?= y

ifeq ($(AIR_ECNR_CONFIG_TYPE),ECNR_1_OR_2_MIC)
AIR_VOICE_NR_ENABLE        	:= y
AIR_VOICE_NR_USE_PIC_ENABLE	:= y
AIR_ECNR_1_OR_2_MIC_ENABLE 	:= y
endif

ifeq ($(AIR_ECNR_CONFIG_TYPE),ECNR_1MIC_INEAR)
AIR_VOICE_NR_ENABLE        	:= y
AIR_VOICE_NR_USE_PIC_ENABLE	:= y
AIR_ECNR_1MIC_INEAR_ENABLE 	:= y
endif

ifeq ($(AIR_ECNR_CONFIG_TYPE),ECNR_2MIC_INEAR)
AIR_VOICE_NR_ENABLE        	:= y
AIR_VOICE_NR_USE_PIC_ENABLE	:= y
AIR_ECNR_2MIC_INEAR_ENABLE 	:= y
endif

ifeq ($(AIR_ECNR_CONFIG_TYPE),3RD_PARTY_AI_NR)
AIR_VOICE_NR_ENABLE        	:= y
AIR_VOICE_NR_USE_PIC_ENABLE	:= y
AIR_3RD_PARTY_NR_ENABLE		:= y
ifneq ($(wildcard $(strip $(ROOTDIR))/prebuilt/middleware/third_party/dspalg/igo_nr/),)
AIR_AI_NR_PREMIUM_ENABLE 	:= y
AIR_AI_NR_PREMIUM_270K_VARIANT_1_ENABLE = y
endif
endif

ifeq ($(AIR_ECNR_CONFIG_TYPE),3RD_PARTY_AI_NR_OFFLOAD)
AIR_VOICE_NR_ENABLE             = y
AIR_VOICE_NR_USE_PIC_ENABLE     = y
AIR_3RD_PARTY_NR_ENABLE         = y
ifneq ($(wildcard $(strip $(ROOTDIR))/prebuilt/middleware/third_party/dspalg/igo_nr/),)
AIR_AI_NR_PREMIUM_ENABLE        = y
AIR_AI_NR_PREMIUM_270K_VARIANT_1_ENABLE = y
endif
AIR_ECNR_PREV_PART_ENABLE       = y
AIR_ECNR_PREV_PART_USE_PIC_ENABLE = y
endif

ifeq ($(AIR_ECNR_CONFIG_TYPE),3RD_PARTY_AI_NR_OFFLOAD_POST_EC)
AIR_VOICE_NR_ENABLE             = y
AIR_VOICE_NR_USE_PIC_ENABLE     = y
AIR_3RD_PARTY_NR_ENABLE         = y
ifneq ($(wildcard $(strip $(ROOTDIR))/prebuilt/middleware/third_party/dspalg/igo_nr/),)
AIR_AI_NR_PREMIUM_ENABLE        = y
AIR_AI_NR_PREMIUM_270K_VARIANT_1_ENABLE = y
endif
AIR_ECNR_POST_PART_ENABLE       = y
AIR_ECNR_POST_PART_USE_PIC_ENABLE = y
endif

ifeq ($(AIR_ECNR_CONFIG_TYPE),3RD_PARTY_AI_NR_PRO_BROADSIDE_SEPARATE_MODE)
AIR_VOICE_NR_ENABLE             = y
AIR_VOICE_NR_USE_PIC_ENABLE     = y
AIR_3RD_PARTY_NR_ENABLE         = y
ifneq ($(wildcard $(strip $(ROOTDIR))/prebuilt/middleware/third_party/dspalg/igo_nr_pro_broadside/),)
AIR_AI_NR_PREMIUM_ENABLE        = y
AIR_AI_NR_PREMIUM_INEAR_500K_BROADSIDE_ENABLE = y
endif
AIR_ECNR_PREV_PART_ENABLE       = y
AIR_ECNR_PREV_PART_USE_PIC_ENABLE = y
AIR_ECNR_POST_PART_ENABLE       = y
AIR_ECNR_POST_PART_USE_PIC_ENABLE = y
AIR_ECNR_SEPARATE_MODE_ENABLE   = y
endif

ifeq ($(AIR_ECNR_CONFIG_TYPE),3RD_PARTY_AI_NR_SEPARATE_MODE_EC)
AIR_VOICE_NR_ENABLE             = y
AIR_VOICE_NR_USE_PIC_ENABLE     = y
AIR_3RD_PARTY_NR_ENABLE         = y
AIR_ECNR_PREV_PART_ENABLE       = y
AIR_ECNR_PREV_PART_USE_PIC_ENABLE = y
AIR_ECNR_SEPARATE_MODE_ENABLE   = y
endif

ifeq ($(AIR_ECNR_CONFIG_TYPE),3RD_PARTY_AI_NR_INEAR)
AIR_VOICE_NR_ENABLE        	:= y
AIR_VOICE_NR_USE_PIC_ENABLE	:= y
AIR_3RD_PARTY_NR_ENABLE 	:= y
ifneq ($(wildcard $(strip $(ROOTDIR))/prebuilt/middleware/third_party/dspalg/igo_nr/),)
AIR_AI_NR_PREMIUM_INEAR_ENABLE := y
AIR_AI_NR_PREMIUM_INEAR_270K_VARIANT_1_ENABLE := y
endif
endif

ifeq ($(AIR_ECNR_CONFIG_TYPE),3RD_PARTY_AI_NR_PRO_DISTRACTOR)
AIR_VOICE_NR_ENABLE        	:= y
AIR_VOICE_NR_USE_PIC_ENABLE	:= y
AIR_3RD_PARTY_NR_ENABLE 	:= y
ifneq ($(wildcard $(strip $(ROOTDIR))/prebuilt/middleware/third_party/dspalg/igo_nr_pro_distractor/),)
AIR_AI_NR_PREMIUM_INEAR_ENABLE := y
AIR_AI_NR_PREMIUM_INEAR_500K_PRO_DISTRACTOR_ENABLE := y
endif
endif

ifeq ($(AIR_ECNR_CONFIG_TYPE),3RD_PARTY_AI_NR_PRO_TWS_OO)
AIR_VOICE_NR_ENABLE        	:= y
AIR_VOICE_NR_USE_PIC_ENABLE	:= y
AIR_3RD_PARTY_NR_ENABLE 	:= y
ifneq ($(wildcard $(strip $(ROOTDIR))/prebuilt/middleware/third_party/dspalg/igo_nr_pro_tws_oo/),)
AIR_AI_NR_PREMIUM_INEAR_ENABLE := y
AIR_AI_NR_PREMIUM_INEAR_500K_PRO_TWS_OO_ENABLE := y
endif
endif

ifeq ($(AIR_ECNR_CONFIG_TYPE),3RD_PARTY_CUSTOMIZED)
AIR_VOICE_NR_ENABLE        	:= y
AIR_VOICE_NR_USE_PIC_ENABLE	:= y
AIR_3RD_PARTY_NR_ENABLE 	:= y
endif

AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE_ENABLE ?= y
AIR_UPLINK_RATE                       ?= none
AIR_UPLINK_RESOLUTION                 ?= none
AIR_DOWNLINK_RATE                     ?= none
AIR_FIX_HFP_DL_STREAM_RATE            ?= none

#CCFLAG += -DDSP_MIPS_FEATURE_PROFILE
#CCFLAG += -DDSP_MIPS_STREAM_PROFILE
#CCFLAG += -DDSP_MIPS_AUD_SYS_ISR_PROFILE

AIR_HEARTHROUGH_MAIN_ENABLE                ?= n
AIR_CUSTOMIZED_LLF_ENABLE             ?= n

ifeq ($(AIR_DUAL_CHIP_AUDIO_INTERFACE),uart)
#AIR_3RD_PARTY_AUDIO_PLATFORM_ENABLE = y
endif

# code run in S or NS world #s or ns
# This feature must be set for both DSP and MCU, otherwise, it will not work.
AIR_CPU_SECURITY_MODE                 ?= s

CCFLAG += -DAIR_BTA_IC_PREMIUM_G3
AIR_BTA_IC_PREMIUM_G3 = y

PRODUCT_VERSION           = 0
CCFLAG += -DPRODUCT_VERSION=$(PRODUCT_VERSION)
CCFLAG += -DCORE_DSP0
ASFLAG += -DCORE_DSP0
CCFLAG += -D$(TARGET)_BOOTING
ifeq ($(FPGA_ENV),y)
CCFLAG += -DFPGA_ENV
endif

ifeq ($(AIR_HEAVY_LOAD_SETTING_ENABLE), y)
CCFLAG += -DAIR_HEAVY_LOAD_SETTING_ENABLE
endif
# Link and Compile Configuration

#CCFLAG      :=
CCFLAG      +=  -g
CCFLAG      +=  -W
CCFLAG      +=  -Wall
ifeq ($(AIR_HEAVY_LOAD_SETTING_ENABLE), y)
CCFLAG      +=  -O2
else
CCFLAG      +=  -Os
endif
CCFLAG      +=  -INLINE:requested
CCFLAG      +=  -mlongcalls
CCFLAG      +=  -mlongcalls
ifeq ($(IOT_SDK_XTENSA_VERSION),9018)
CCFLAG      +=  -std=c11
CCFLAG      +=  -fno-jump-tables
CCFLAG      +=  -fgnu-keywords
else ifeq ($(IOT_SDK_XTENSA_VERSION),8013)
CCFLAG      +=  -std=gnu99
endif
CCFLAG      +=  -ffunction-sections -fdata-sections -mtext-section-literals
CCFLAG      +=  -OPT:space_flix=1
CCFLAG      +=  -Werror
#ASFLAG      :=
ASFLAG      +=  -W
ASFLAG      +=  -Wall
ASFLAG      +=  -g
ASFLAG      +=  -mlongcalls --text-section-literals

#LDFLAG      :=
LDFLAG      +=  --gc-sections
LDFLAG      +=  --no-relax
LDFLAG      +=  -wrap=printf
LDFLAG      +=  -wrap=puts
LDFLAG	    += -u _UserExceptionVector
LDFLAG	    += -u _KernelExceptionVector
LDFLAG	    += -u _DoubleExceptionVector

##
## AIR_SYSLOG_STACK_MONITOR_ENABLE
## Brief:       This option is used to monitor syslog call stack.
## Usage:       Enable the feature by configuring it as y.
##              y : enable syslog stack monitor
##              n : disable syslog stack monitor
## Path:        kernel/service/syslog
## Dependency:  None
## Notice:      None
## Related doc :None
##
ifeq ($(AIR_SYSLOG_STACK_MONITOR_ENABLE),y)
CCFLAG += -DAIR_SYSLOG_STACK_MONITOR_ENABLE
endif

##
## MTK_DEBUG_PLAIN_LOG_ENABLE
## Brief:       This option is used to force log display with plain style.
## Usage:       Enable the feature by configuring it as y.
##              y : log display with plain style
##              n : log display with race style, need pc logging tool support
## Path:        kernel/service/syslog
## Dependency:  None
## Notice:      None
## Related doc :None
##
ifeq ($(MTK_DEBUG_PLAIN_LOG_ENABLE),y)
CCFLAG += -DMTK_DEBUG_PLAIN_LOG_ENABLE
endif


##
## AIR_HEAP_BEST_FIT
## Brief:       This option is used to heap malloc policy
## Usage:       Enable the feature by configuring it as y.
##              y : use best fit policy
##              n : use frist fit policy
## Dependency:  None
## Notice:      None
## Related doc :None
##
ifeq ($(AIR_HEAP_BEST_FIT),y)
CCFLAG += -DAIR_HEAP_BEST_FIT
endif

##
## AIR_CPU_MCPS_PRIORING_ENABLE
## Brief:       This option is used to enable task profile
## Usage:       Enable the feature by configuring it as y.
##              y : enable task profile
##              n : disable task profile
## Dependency:  HAL_TIME_CHECK_ISR_ENABLED
## Notice:      None
## Related doc :None
##
ifeq ($(AIR_CPU_MCPS_PRIORING_ENABLE),y)
CCFLAG += -DAIR_CPU_MCPS_PRIORING_ENABLE
endif

##
## AIR_DEBUG_LEVEL
## Brief:       This option is to configure system log debug level.
## Usage:       The valid values are empty, error, warning, info, debug, and none.
##              The setting will determine whether a debug log will be compiled.
##              However, the setting has no effect on the prebuilt library.
##              empty   : All debug logs are compiled.
##              error   : Only error logs are compiled.
##              warning : Only warning and error logs are compiled.
##              info    : Only info, warning, and error logs are compiled.
##              debug   : All debug logs are compiled.
##              none    : All debugs are disabled.
## Path:        kernel/service
## Dependency:  None
## Notice:      None
## Realted doc: Please refer to doc/LinkIt_for_RTOS_System_Log_Developers_Guide.pdf
##
AIR_DEBUG_LEVEL ?= $(MTK_DEBUG_LEVEL)

ifeq ($(AIR_DEBUG_LEVEL),)
CCFLAG += -DMTK_DEBUG_LEVEL_DEBUG
CCFLAG += -DMTK_DEBUG_LEVEL_INFO
CCFLAG += -DMTK_DEBUG_LEVEL_WARNING
CCFLAG += -DMTK_DEBUG_LEVEL_ERROR
endif

ifeq ($(AIR_DEBUG_LEVEL),error)
CCFLAG += -DMTK_DEBUG_LEVEL_ERROR
endif

ifeq ($(AIR_DEBUG_LEVEL),warning)
CCFLAG += -DMTK_DEBUG_LEVEL_WARNING
CCFLAG += -DMTK_DEBUG_LEVEL_ERROR
endif

ifeq ($(AIR_DEBUG_LEVEL),info)
CCFLAG += -DMTK_DEBUG_LEVEL_INFO
CCFLAG += -DMTK_DEBUG_LEVEL_WARNING
CCFLAG += -DMTK_DEBUG_LEVEL_ERROR
endif

ifeq ($(AIR_DEBUG_LEVEL),debug)
CCFLAG += -DMTK_DEBUG_LEVEL_DEBUG
CCFLAG += -DMTK_DEBUG_LEVEL_INFO
CCFLAG += -DMTK_DEBUG_LEVEL_WARNING
CCFLAG += -DMTK_DEBUG_LEVEL_ERROR
endif

ifeq ($(AIR_DEBUG_LEVEL),printf)
CCFLAG += -DMTK_DEBUG_LEVEL_PRINTF
endif
ifeq ($(AIR_DEBUG_LEVEL),none)
CCFLAG += -DMTK_DEBUG_LEVEL_NONE
endif

# This option is used to enable/disable JTAG DEBUG.
# Dependency: NA
AIR_ICE_DEBUG_ENABLE  ?=y

##
## MTK_SAVE_LOG_AND_CONTEXT_DUMP_ENABLE
## Brief:       This option is to enable runtime log and crash context save in flash feature.
## Usage:       Enable the feature by configuring it as y.
##              y : save runtime logging registers and all memory in coredump format
##              n   : no effect
## Path:        kernel/service/src_core
## Dependency:  flash driver
## Notice:      Reserve flash blocks to store runtime log and dumped data
## Related doc :None
##
ifeq ($(MTK_SAVE_LOG_AND_CONTEXT_DUMP_ENABLE),y)
CCFLAG += -DMTK_SAVE_LOG_AND_CONTEXT_DUMP_ENABLE
endif
##

##
## AIR_DSP_PRODUCT_CATEGORY
## Brief:       This option is to configure DSP different project.
## Usage:       Project category includes none/Headset/Earbuds
## Notice:      None.
##
ifeq ($(AIR_DSP_PRODUCT_CATEGORY), Headset)
CCFLAG += -DAIR_DSP_PRODUCT_CATEGORY_AB158x_HEADSET
CCFLAG += -DAIR_DSP_PRODUCT_CATEGORY_HEADSET
endif

ifeq ($(AIR_DSP_PRODUCT_CATEGORY), Earbuds)
CCFLAG += -DAIR_DSP_PRODUCT_CATEGORY_AB158x_EARBUDS
CCFLAG += -DAIR_DSP_PRODUCT_CATEGORY_EARBUDS
endif

##
## AIR_DSP_TASK_PRINT_PROCESS_WARNING_LOG_ENABLE
## Brief:       This option is to print warning logs of DSP task processing time.
## Usage:       Enable the feature by configuring it as y.
## Notice:      None.
##
ifeq ($(AIR_DSP_TASK_PRINT_PROCESS_WARNING_LOG_ENABLE),y)
CCFLAG += -DAIR_DSP_TASK_PRINT_PROCESS_WARNING_LOG_ENABLE
endif

##
## AIR_3RD_PARTY_AUDIO_PLATFORM_ENABLE
## Brief:       Internal use.
## Notice:      AIR_3RD_PARTY_AUDIO_PLATFORM_ENABLE is a option to enabled 3rd party audio platform.
## Path:        None
## Dependency:  None
## Notice:      Default disable audio post process
##
ifeq ($(AIR_3RD_PARTY_AUDIO_PLATFORM_ENABLE),y)
CCFLAG  += -DAIR_3RD_PARTY_AUDIO_PLATFORM_ENABLE

AIR_PEQ_ENABLE := n
MTK_PEQ_ENABLE := n
AIR_LINE_IN_PEQ_ENABLE := n
MTK_LINEIN_PEQ_ENABLE := n
AIR_VP_PEQ_ENABLE := n
AIR_LINE_IN_INS_ENABLE := n
MTK_LINEIN_INS_ENABLE := n
AIR_DRC_ENABLE := n

AIR_BT_HFP_CVSD_ENABLE := n

AIR_ANC_WIND_NOISE_DETECTION_ENABLE := n
AIR_ANC_ENVIRONMENT_DETECTION_ENABLE := n
MTK_ANC_SURROUND_MONITOR_ENABLE := y
endif

##
## MTK_USB_DEMO_ENABLED
## Brief:       This option is to enable USB device feature.
## Usage:       Enable the feature by configuring it as y.
## Path:        middleware/airoha/usb/
## Dependency:  None
## Notice:      None
## Related doc :None
##
ifeq ($(MTK_USB_DEMO_ENABLED),y)
CCFLAG += -DMTK_USB_DEMO_ENABLED
endif

##
## MTK_PROMPT_SOUND_ENABLE
## Brief:       This option is to enable prompt sound feature.
## Usage:       Enable enable prompt sound feature by configuring it as y.
##              y : enable prompt sound feature.
##              n : not enable prompt sound feature.
## Path:        middleware\airoha\stream
## Dependency:  None.
## Notice:      None.
## Realted doc: None.
##
ifeq ($(MTK_PROMPT_SOUND_ENABLE),y)
CCFLAG += -DMTK_PROMPT_SOUND_ENABLE
endif

##
## MTK_I2S_SLAVE_ENABLE
## Brief:       This option is to open i2s slave driver.
## Usage:       Enable the i2s slave driver by configuring it as y.
##              y : open i2s slave driver.
##              n : not open i2s slave driver.
## Path:        middleware/airoha/dspfw/port/chip/mt2822/src/dsp_lower_layer/dsp_drv
## Dependency:  None
## Notice:      None
## Realted doc: None
##
ifeq ($(MTK_I2S_SLAVE_ENABLE),y)
CCFLAG += -DMTK_I2S_SLAVE_ENABLE
endif




##
## DSP_MEMORY_MANAGEMENT_ENABLE
## Brief:       This option is to open dsp memory management feature.
## Usage:       Enable the dsp memory management by configuring it as y.
##              y : open dsp memory management feature.
##              n : not open dsp memory management feature.
## Path:        kernel/service/DSP_Memory_Management
## Dependency:  None
## Notice:      None
## Realted doc: None
##
ifeq ($(DSP_MEMORY_MANAGEMENT_ENABLE),y)
CCFLAG += -DDSP_MEMORY_MANAGEMENT_ENABLE
CCFLAG += -DAIR_DSP_MEMORY_REGION_ENABLE
endif



##
## PRELOADER_ENABLE
## Brief:       This option is to enable and disable preload pisplit features(dynamic to load PIC libraries)
## Usage:       If the value is "y", the PRELOADER_ENABLE compile option will be defined. You must also include the kernel/service/pre_libloader/dsp0/module.mk in your Makefile before setting the option to "y".
## Path:        kernel/service/pre_libloader
## Dependency:  None
## Notice:      None
## Relative doc:None
##
ifeq ($(PRELOADER_ENABLE),y)
CCFLAG += -DPRELOADER_ENABLE
ASFLAG += -DPRELOADER_ENABLE

##
## DSP0_PISPLIT_DEMO_LIBRARY
## Brief:       This option is to enable and disable the demo of DSP0 PIC library
## Usage:       If the value is "y", the DSP0_PISPLIT_DEMO_LIBRARY compile option will be defined. This is a sub-feature option of PRELOADER_ENABLE.
## Path:        kernel/service/pre_libloader/dsp0/dsp0_pic_demo_portable
## Dependency:  PRELOADER_ENABLE
## Notice:      None
## Relative doc:None
##
ifeq ($(DSP0_PISPLIT_DEMO_LIBRARY),y)
CCFLAG += -DDSP0_PISPLIT_DEMO_LIBRARY
endif
endif

##
## CCCI_ENABLE
## Brief:       This option is to enable and disable CCCI(Cross Core communication Interface)
## Usage:       If the value is "y", the CCCI_ENABLE compile option will be defined.
## Path:        kernel/service/ccci
## Dependency:  None
## Notice:      None
## Relative doc:None
##
ifeq ($(CCCI_ENABLE),y)
CCFLAG += -DCCCI_ENABLE
endif


##
## LINE_IN_PURE_FOR_AMIC_CLASS_G_HQA
## Brief:       This option is to enable for Audio HQA verification.
## Usage:       If the value is "y",  the LINE_IN_PURE_FOR_AMIC_CLASS_G_HQA option will be defined.
## Path:        middleware/airoha/audio/
## Dependency:  None
## Notice:      None
## Relative doc:None
##
ifeq ($(LINE_IN_PURE_FOR_AMIC_CLASS_G_HQA),y)
CCFLAG += -DLINE_IN_PURE_FOR_AMIC_CLASS_G_HQA
endif

##
## ANALOG_OUTPUT_CLASSD_ENABLE
## Brief:       This option is to enable for default setting to class-d.
## Usage:       If the value is "y",  the ANALOG_OUTPUT_CLASSD_ENABLE option will be defined.
## Path:        middleware/airoha/audio/
## Dependency:  None
## Notice:      None
## Relative doc:None
##
ifeq ($(ANALOG_OUTPUT_CLASSD_ENABLE),y)
CCFLAG += -DANALOG_OUTPUT_CLASSD_ENABLE
endif

##
## AIR_VOICE_DRC_ENABLE
## Brief:       This option is to enable voice drc.
## Usage:       If the value is "y",  the AIR_VOICE_DRC_ENABLE option will be defined.
## Path:        middleware/airoha/dspfw/
## Dependency:  None
## Notice:      None
## Relative doc:None
##
ifeq ($(AIR_VOICE_DRC_ENABLE),y)
AIR_BT_A2DP_CPD_USE_PIC_ENABLE 	:= y
CCFLAG += -DMTK_BT_A2DP_CPD_USE_PIC
CCFLAG += -DAIR_VOICE_DRC_ENABLE
endif

##
## AIR_ANC_ENABLE
## Brief:       This option is to enable Active Noise Cancellation (ANC) main function.
## Usage:       Enable ANC feature by configuring it as y.
##              y : enable ANC feature.
##              n : not enable ANC feature.
## Path:        dsp/prebuilt/middleware/airoha/dspfw/anc/$(IC_TYPE)
## Dependency:  None.
## Notice:      None.
## Realted doc: None.
##
ifeq ($(AIR_ANC_ENABLE),y)
CCFLAG += -DMTK_ANC_ENABLE
CCFLAG += -DMTK_ANC_V2
CCFLAG += -DHAL_AUDIO_ANC_ENABLE
ifeq ($(AIR_BTA_IC_PREMIUM_G3),y)
CCFLAG += -DAIR_ANC_V3
endif
endif

##
## AIR_FULL_ADAPTIVE_ANC_ENABLE
## Brief:       This option is to enable full adaptive Active Noise Cancellation (ANC) main function.
## Usage:       Enable full adaptive ANC feature by configuring it as y.
##              y : enable full adaptive ANC feature.
##              n : not enable full adaptive ANC feature.
## Path:        dsp/prebuilt/middleware/airoha/dspalg/full_adaptive_anc/$(IC_TYPE)
## Dependency:  None.
## Notice:      None.
## Realted doc: None.
##
ifeq ($(AIR_FULL_ADAPTIVE_ANC_ENABLE),y)
CCFLAG += -DAIR_FULL_ADAPTIVE_ANC_ENABLE
CCFLAG += -DMTK_AUDIO_DUMP_SPDIF_TRUNCATE_24BIT
CCFLAG += -DAIR_ANC_ADAPTIVE_CLOCK_CONTROL_ENABLE
ifeq ($(AIR_FULL_ADAPTIVE_ANC_STEREO_ENABLE),y)
CCFLAG += -DAIR_FULL_ADAPTIVE_ANC_STEREO_ENABLE
else
CCFLAG += -DAIR_ANC_ADAPTIVE_STATUS_SYNC_ENABLE
endif
endif

##
## AIR_FULL_ADAPTIVE_ANC_USE_PIC
## Brief:       Internal use.
## Notice:      AIR_FULL_ADAPTIVE_ANC_USE_PIC is a option to use full adaptive Active Noise Cancellation (ANC) PIC.
##
ifeq ($(AIR_FULL_ADAPTIVE_ANC_ENABLE),y)
ifeq ($(AIR_FULL_ADAPTIVE_ANC_USE_PIC),y)
CCFLAG += -DAIR_FULL_ADAPTIVE_ANC_USE_PIC
endif
endif

##
## AIR_HYBRID_PT_ENABLE
## Brief:       This option is to enable Hybrid Passthru.
## Usage:       Enable Hybrid Passthru feature by configuring it as y.
##              y : enable Hybrid Passthru feature.
##              n : not enable Hybrid Passthru feature.
## Path:        dsp/prebuilt/middleware/airoha/dspfw/anc/$(IC_TYPE)
## Dependency:  None.
## Notice:      None.
## Realted doc: None.
##
ifeq ($(AIR_HYBRID_PT_ENABLE),y)
CCFLAG += -DAIR_HYBRID_PT_ENABLE
endif

##
## AIR_ANC_ADAP_PT_ENABLE
## Brief:       This option is to enable Adaptive Passthru.
## Usage:       Enable Adaptive Passthru feature by configuring it as y.
##              y : enable Adaptive Passthru feature.
##              n : not enable Adaptive Passthru feature.
## Path:        dsp/prebuilt/middleware/airoha/dspfw/anc/$(IC_TYPE)
## Dependency:  AIR_ANC_ENABLE.
## Notice:      None.
## Realted doc: None.
##
ifeq ($(AIR_ANC_ENABLE),y)
ifeq ($(AIR_ANC_ADAP_PT_ENABLE),y)
CCFLAG += -DAIR_ANC_ADAP_PT_ENABLE
CCFLAG += -DAIR_HYBRID_PT_ENABLE
endif
endif

ifeq ($(AIR_ANC_ENABLE),y)
ifeq ($(AIR_ANC_SCENARIO_CONTROL_GAIN_ENABLE),y)
CCFLAG += -DAIR_ANC_SCENARIO_CONTROL_GAIN_ENABLE
endif
endif

##
## AIR_UPLINK_RATE
## Brief:       This option is to choose the uplink rate.
## Usage:       The valid values are none, 32k, 48k.
##              The setting will determine which rate of uplink rate will be used.
##              none    : uplink rate will be handled by scenario itself.
##              32k     : uplink rate will be fixed in 32k Hz.
##              48k     : uplink rate will be fixed in 48k Hz.
## Path:        dsp/driver/chip/$(IC_TYPE)
## Dependency:  None
## Notice:      None
##
ifeq ($(AIR_UPLINK_RATE),32k)
CCFLAG += -DAIR_UL_FIX_SAMPLING_RATE_32K
CCFLAG += -DAIR_FIXED_RATIO_SRC
endif

ifeq ($(AIR_UPLINK_RATE),48k)
CCFLAG += -DAIR_UL_FIX_SAMPLING_RATE_48K
CCFLAG += -DAIR_FIXED_RATIO_SRC
endif

##
## AIR_UPLINK_RESOLUTION
## Brief:       This option is to fix the uplink DMA resolution.
## Usage:       The valid values are none, 32bit.
##              The setting will determine which resolution of uplink DMA will be used.
##              none    : uplink resolution will be handled by scenario itself.
##              32bit   : uplink resolution will be fixed at 32-bit.
## Path:        dsp/driver/chip/$(IC_TYPE)
## Dependency:  None
## Notice:      None
##
ifeq ($(AIR_UPLINK_RESOLUTION),32bit)
CCFLAG += -DAIR_UL_FIX_RESOLUTION_32BIT
endif

##
## AIR_DOWNLINK_RATE
## Brief:       This option is to choose the downlink rate.
## Usage:       The valid values are none, 48k, 96k.
##              The setting will determine which rate of downlink rate will be used.
##              none    : downlink rate will be handled by scenario itself.
##              48k     : downlink rate will be fixed in 48k Hz.
##              96k     : downlink rate will be fixed in 96k Hz.
## Path:        dsp/driver/chip/$(IC_TYPE)
## Dependency:  None
## Notice:      None
##
ifeq ($(AIR_DOWNLINK_RATE),48k)
CCFLAG += -DFIXED_SAMPLING_RATE_TO_48KHZ
endif

ifeq ($(AIR_DOWNLINK_RATE),96k)
CCFLAG += -DAIR_FIXED_DL_SAMPLING_RATE_TO_96KHZ
endif

##
## AIR_HFP_DL_STREAM_RATE
## Brief:       This option is to choose the HFP stream rate.
## Usage:       The valid values are dynamic, 48k, 96k.
##              The setting will determine which rate of processing stream will be used.
##              dynamic : HFP stream rate will be handled by scenario itself.
##              48k     : HFP stream rate will be fixed in 48k Hz.
##              96k     : HFP stream rate will be fixed in 96k Hz.
## Path:        dsp/driver/chip/$(IC_TYPE)
## Dependency:  None
## Notice:      None
##
ifeq ($(AIR_HFP_DL_STREAM_RATE),48k)
CCFLAG += -DAIR_HFP_DL_STREAM_RATE_FIX_TO_48KHZ
CCFLAG += -DAIR_FIXED_RATIO_SRC
endif

ifeq ($(AIR_HFP_DL_STREAM_RATE),96k)
CCFLAG += -DAIR_HFP_DL_STREAM_RATE_FIX_TO_96KHZ
CCFLAG += -DAIR_FIXED_RATIO_SRC
endif

##
## AIR_A2DP_DL_STREAM_RAT
## Brief:       This option is to choose the A2DP stream rate.
## Usage:       The valid values are dynamic, 48k, 96k.
##              The setting will determine which rate of processing stream will be used.
##              dynamic : A2DP stream rate will be handled by scenario itself.
##              48k     : A2DP stream rate will be fixed in 48k Hz.
##              96k     : A2DP stream rate will be fixed in 96k Hz.
## Path:        dsp/driver/chip/$(IC_TYPE)
## Dependency:  None
## Notice:      None
##
ifeq ($(AIR_A2DP_DL_STREAM_RATE),48k)
CCFLAG += -DAIR_A2DP_DL_STREAM_RATE_FIX_TO_48KHZ
CCFLAG += -DMTK_HWSRC_IN_STREAM
endif

ifeq ($(AIR_A2DP_DL_STREAM_RATE),96k)
CCFLAG += -DAIR_A2DP_DL_STREAM_RATE_FIX_TO_96KHZ
CCFLAG += -DMTK_HWSRC_IN_STREAM
endif

##
## AIR_LE_CALL_DL_STREAM_RATE
## Brief:       This option is to choose the LE Call stream rate.
## Usage:       The valid values are none, 48k, 96k.
##              The setting will determine which rate of processing stream will be used.
##              dynamic : LE Call stream rate will be handled by scenario itself.
##              48k     : LE Call stream rate will be fixed in 48k Hz.
##              96k     : LE Call stream rate will be fixed in 96k Hz.
## Path:        dsp/driver/chip/$(IC_TYPE)
## Dependency:  None
## Notice:      None
##
ifeq ($(AIR_LE_CALL_DL_STREAM_RATE),48k)
CCFLAG += -DAIR_LE_CALL_DL_STREAM_RATE_FIX_TO_48k
CCFLAG += -DAIR_FIXED_RATIO_SRC
endif

ifeq ($(AIR_LE_CALL_DL_STREAM_RATE),96k)
CCFLAG += -DAIR_LE_CALL_DL_STREAM_RATE_FIX_TO_96k
CCFLAG += -DAIR_FIXED_RATIO_SRC
endif

##
## AIR_PEQ_ENABLE
## Brief:       This option is to enable PEQ feature.
## Usage:       If the value is "y",  the AIR_PEQ_ENABLE option will be defined.
## Path:        middleware/airoha/dspalg/
## Dependency:  AIR_BT_PEQ_USE_PIC_ENABLE
## Notice:      None
## Relative doc:None
##
AIR_PEQ_ENABLE ?= $(MTK_PEQ_ENABLE)
ifeq ($(AIR_PEQ_ENABLE),y)
AIR_BT_PEQ_USE_PIC_ENABLE := y
CCFLAG += -DMTK_PEQ_ENABLE
CCFLAG += -DMTK_BT_PEQ_USE_PIC
endif

##
## AIR_ADAPTIVE_EQ_ENABLE
## Brief:       This option is to enable adaptive eq feature.
## Usage:       If the value is "y",  the AIR_ADAPTIVE_EQ_ENABLE option will be defined.
## Path:        middleware/airoha/dspalg/
## Dependency:  AIR_BT_PEQ_USE_PIC_ENABLE
## Notice:      None
## Relative doc:None
##
ifeq ($(AIR_ADAPTIVE_EQ_ENABLE),y)
CCFLAG += -DAIR_ADAPTIVE_EQ_ENABLE
CCFLAG += -DAIR_ADAPITVE_EQ_DETECTION_USE_PIC_ENABLE
endif

##
## AIR_DRC_ENABLE
## Brief:       This option is to enable DRC feature.
## Usage:       If the value is "y",  the AIR_DRC_ENABLE option will be defined.
## Path:        middleware/airoha/dspalg/
## Dependency:  None
## Notice:      None
## Relative doc:None
##
ifeq ($(AIR_DRC_ENABLE),y)
AIR_BT_A2DP_CPD_USE_PIC_ENABLE 	:= y
CCFLAG += -DMTK_BT_A2DP_CPD_USE_PIC
CCFLAG += -DAIR_DRC_ENABLE
endif

##
## AIR_VP_PEQ_ENABLE
## Brief:       This option is to enable VP PEQ feature.
## Usage:       If the value is "y",  the AIR_VP_PEQ_ENABLE option will be defined.
## Path:        middleware/airoha/dspalg/
## Dependency:  AIR_PEQ_ENABLE
## Notice:      None
## Relative doc:None
##
ifeq ($(AIR_VP_PEQ_ENABLE),y)
CCFLAG += -DAIR_VP_PEQ_ENABLE
CCFLAG += -DAIR_VP_PEQ_USE_PIC
endif

##
## AIR_LINE_IN_PEQ_ENABLE
## Brief:       This option is to enable LINE_IN PEQ feature.
## Usage:       If the value is "y",  the AIR_LINE_IN_PEQ_ENABLE option will be defined.
## Path:        middleware/airoha/dspalg/
## Dependency:  AIR_BT_PEQ_USE_PIC_ENABLE
## Notice:      None
## Relative doc:None
##
AIR_LINE_IN_PEQ_ENABLE ?= $(MTK_LINEIN_PEQ_ENABLE)
ifeq ($(AIR_LINE_IN_PEQ_ENABLE),y)
AIR_BT_PEQ_USE_PIC_ENABLE := y
CCFLAG += -DMTK_LINEIN_PEQ_ENABLE
CCFLAG += -DMTK_BT_PEQ_USE_PIC
endif

##
## AIR_LINE_IN_INS_ENABLE
## Brief:       This option is to enable LINE_IN INS feature.
## Usage:       If the value is "y",  the AIR_LINE_IN_INS_ENABLE option will be defined.
## Path:        middleware/airoha/dspalg/
## Dependency:  None
## Notice:      None
## Relative doc:None
##
AIR_LINE_IN_INS_ENABLE ?= $(MTK_LINEIN_INS_ENABLE)
ifeq ($(AIR_LINE_IN_INS_ENABLE),y)
CCFLAG += -DMTK_LINEIN_INS_ENABLE
endif

##
## AIR_LINE_IN_MIX_ENABLE
## Brief:       Internal use.
## Notice:      AIR_LINE_IN_MIX_ENABLE is a option to support line-in audio mix with A2DP/HFP.
##
ifeq ($(AIR_LINE_IN_MIX_ENABLE),y)
CCFLAG += -DAIR_LINE_IN_MIX_ENABLE
endif

##
## AIR_USB_AUDIO_IN_MIX_ENABLE
## Brief:       Internal use.
## Notice:      AIR_USB_AUDIO_IN_MIX_ENABLE is a option to support usb-in audio mix with A2DP/HFP.
##
ifeq ($(AIR_USB_AUDIO_IN_MIX_ENABLE),y)
CCFLAG += -DAIR_USB_AUDIO_IN_MIX_ENABLE
endif

##
## AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE
## Brief:       Internal use.
## Notice:      AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE is a option to support usb-out audio mix with usb-in audio, and output with usb-in.
##
ifeq ($(AIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE),y)
CCFLAG += -DAIR_USB_AUDIO_IN_AND_OUT_MIX_ENABLE
endif

##
## AIR_AUDIO_DETACHABLE_MIC_ENABLE
## Brief:       Internal use. This option is for voice detachable mic.
## Usage:       If the value is "n",  the AIR_AUDIO_DETACHABLE_MIC_ENABLE option will not be defined.
## Path:        middleware/airoha/audio_manager/
## Dependency:  None
## Notice:      None
## Relative doc:None
##
ifeq ($(AIR_AUDIO_DETACHABLE_MIC_ENABLE),y)
CCFLAG += -DAIR_AUDIO_DETACHABLE_MIC_ENABLE
endif

##
## AIR_WIRED_AUDIO_ENABLE
## Brief:       Internal use.
## Notice:      AIR_WIRED_AUDIO_ENABLE is a option to enable wired audio,inlucde line-in/line-out and usb-in/usb-out.
##
ifeq ($(AIR_WIRED_AUDIO_ENABLE),y)
CCFLAG += -DAIR_WIRED_AUDIO_ENABLE
ifeq ($(AIR_WIRED_AUDIO_SUB_STREAM_ENABLE),y)
CCFLAG += -DAIR_WIRED_AUDIO_SUB_STREAM_ENABLE
endif
endif

##
## AIR_CELT_ENC_V2_ENABLE
## Brief:       Internal use.
## Notice:      AIR_CELT_ENC_V2_ENABLE is a option to support CELT encoder v2 function.
##
ifeq ($(AIR_CELT_ENC_V2_ENABLE),y)
CCFLAG += -DAIR_CELT_ENC_V2_ENABLE
ifeq ($(MTK_BT_CELT_USE_PIC),y)
CCFLAG += -DAIR_BT_CELT_USE_PIC
CCFLAG += -DMTK_BT_CELT_USE_PIC
endif
ifeq ($(AIR_BT_CELT_USE_PIC),y)
CCFLAG += -DAIR_BT_CELT_USE_PIC
CCFLAG += -DMTK_BT_CELT_USE_PIC
endif
endif

##
## AIR_CELT_DEC_V2_ENABLE
## Brief:       Internal use.
## Notice:      AIR_CELT_DEC_V2_ENABLE is a option to support CELT decoder v2 function.
##
ifeq ($(AIR_CELT_DEC_V2_ENABLE),y)
CCFLAG += -DAIR_CELT_DEC_V2_ENABLE
ifeq ($(MTK_BT_CELT_USE_PIC),y)
CCFLAG += -DAIR_BT_CELT_USE_PIC
CCFLAG += -DMTK_BT_CELT_USE_PIC
endif
ifeq ($(AIR_BT_CELT_USE_PIC),y)
CCFLAG += -DAIR_BT_CELT_USE_PIC
CCFLAG += -DMTK_BT_CELT_USE_PIC
endif
endif

##
## AIR_MIXER_STREAM_ENABLE
## Brief:       This option is to enable Mixer Stream.
## Usage:       If the value is "n",  the AIR_MIXER_STREAM_ENABLE option will not be defined.
## Path:        middleware/airoha/stream/src/stream_interface
## Dependency:  None
## Notice:      None
## Relative doc:None
##
ifeq ($(AIR_MIXER_STREAM_ENABLE),y)
CCFLAG += -DAIR_MIXER_STREAM_ENABLE
AIR_SOFTWARE_MIXER_ENABLE := y
AIR_SOFTWARE_GAIN_ENABLE  := y
endif

###############################################################################
##
## The following makefile options are not configurable or only for internal user. They may be removed in the future.

##
## MTK_CPU_NUMBER_1
## Brief:       Internal use.
##
ifeq ($(MTK_CPU_NUMBER_1),y)
CCFLAG += -DMTK_CPU_NUMBER_1
CCFLAG += -DMTK_MAX_CPU_NUMBER_2
endif

##
## MTK_SENSOR_SOURCE_ENABLE
## Brief:       Internal use.
## Notice:      MTK_SENSOR_SOURCE_ENABLE is a option to support Sensor Source.
##
ifeq ($(MTK_SENSOR_SOURCE_ENABLE),y)
CCFLAG += -DMTK_SENSOR_SOURCE_ENABLE
endif

##
## MTK_SUPPORT_HEAP_DEBUG
## Brief:       Internal use.
## MTK_SUPPORT_HEAP_DEBUG is a option to show heap status (alocatted or free),
## It's for RD internal development and debug. Default should be disabled.
##
ifeq ($(MTK_SUPPORT_HEAP_DEBUG),y)
CCFLAG += -DMTK_SUPPORT_HEAP_DEBUG
endif

##
## MTK_HEAP_SIZE_GUARD_ENABLE
## Brief:       Internal use.
## MTK_HEAP_SIZE_GUARD_ENABLE is a option to profiling heap usage,
## It's for RD internal development and debug. Default should be disabled.
##
ifeq ($(MTK_HEAP_SIZE_GUARD_ENABLE),y)
LDFLAG  += -wrap=pvPortMalloc -wrap=vPortFree
CCFLAG  += -DMTK_HEAP_SIZE_GUARD_ENABLE
endif

##
## MTK_SWLA_ENABLE
## Brief:       Internal use. This option is to enable and disable the Software Logical Analyzer service, Each event(task/isr activity) is recorded while CPU context switching, also support customization tag
## Usage:       If the value is "y", the MTK_SWLA_ENABLE compile option will be defined. You must also include the gva\kernel\service\module.mk in your Makefile before setting the option to "y".
## Path:        kernel/service
## Dependency:  None
## Notice:      None
## Relative doc:None
##
ifeq ($(MTK_SWLA_ENABLE),y)
CCFLAG += -DMTK_SWLA_ENABLE
CCFLAG += -DPRODUCT_VERSION_STR=\"$(PRODUCT_VERSION)\"
endif
##

##
## AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE_ENABLE
## Brief:       Internal use.
## Notice:      AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE_ENABLE is a option to to support multiple microphone.
##
ifeq ($(AIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE_ENABLE),y)
CCFLAG += -DHAL_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
CCFLAG += -DAIR_AUDIO_SUPPORT_MULTIPLE_MICROPHONE
endif

##
## AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
## Brief:       Internal use.
## Notice:      AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE is a option to enable multiple stream out.
##
ifeq ($(AIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE),y)
CCFLAG += -DAIR_AUDIO_MULTIPLE_STREAM_OUT_ENABLE
endif

##
## AIR_ECHO_PATH_FIRST_ENABLE
## Brief:       Internal use.
## Notice:      AIR_ECHO_PATH_FIRST_ENABLE is a option to enable multiple stream out first path to be echo path.
##
ifeq ($(AIR_ECHO_PATH_FIRST_ENABLE),y)
CCFLAG += -DAIR_ECHO_PATH_FIRST_ENABLE
endif

##
## AIR_ECHO_PATH_SECOND_ENABLE
## Brief:       Internal use.
## Notice:      AIR_ECHO_PATH_SECOND_ENABLE is a option to enable multiple stream out second path to be echo path..
##
ifeq ($(AIR_ECHO_PATH_SECOND_ENABLE),y)
CCFLAG += -DAIR_ECHO_PATH_SECOND_ENABLE
endif

##
## AIR_FIXED_RATIO_SRC
## Brief:       Internal use. This option is to enable fixed ratio SRC.
## Usage:       If the value is "y",  the AIR_FIXED_RATIO_SRC option will be defined.
## Path:        driver/chip/
## Dependency:  None
## Notice:      None
## Relative doc:None
##
ifeq ($(AIR_FIXED_RATIO_SRC),y)
CCFLAG += -DAIR_FIXED_RATIO_SRC
ifeq ($(AIR_FIXED_RATIO_SRC_USE_PIC),y)
CCFLAG += -DAIR_FIXED_RATIO_SRC_USE_PIC
endif
endif

##
## AIR_AUDIO_DUMP_ENABLE
## Brief:       Internal use.
## Notice:      AIR_AUDIO_DUMP_ENABLE is a option to choose that whether the dump path will be selected by Config Tool(y) or by Coding(n). Default should be Config Tool(y).
##
ifeq ($(AIR_AUDIO_DUMP_ENABLE),y)
CCFLAG += -DAIR_AUDIO_DUMP_ENABLE
endif

##
## MTK_BT_A2DP_MSBC_USE_PIC
## Brief:       Internal use.
## Notice:      MTK_BT_A2DP_MSBC_USE_PIC is a option to use mSBC PIC. Default should be enabled.
##
ifeq ($(MTK_BT_A2DP_MSBC_USE_PIC),y)
CCFLAG += -DMTK_BT_A2DP_MSBC_USE_PIC
endif

##
## MTK_BT_A2DP_CVSD_USE_PIC
## Brief:       Internal use.
## Notice:      MTK_BT_A2DP_CVSD_USE_PIC is a option to use CVSD PIC. Default should be enabled.
##
ifeq ($(MTK_BT_A2DP_CVSD_USE_PIC),y)
CCFLAG += -DMTK_BT_A2DP_CVSD_USE_PIC
endif

##
## MTK_BT_CLK_SKEW_USE_PIC
## Brief:       Internal use.
## Notice:      MTK_BT_CLK_SKEW_USE_PIC is a option to use clock skew PIC. Default should be enabled.
##
ifeq ($(MTK_BT_CLK_SKEW_USE_PIC),y)
CCFLAG += -DMTK_BT_CLK_SKEW_USE_PIC
endif

##
## MTK_VOICE_AGC_ENABLE
## Brief:       Internal use.
## Notice:      MTK_VOICE_AGC_ENABLE is a option to enable Voice AGC. Default should be enabled.
##
AIR_VOICE_AGC_ENABLE ?= $(MTK_VOICE_AGC_ENABLE)
ifeq ($(AIR_VOICE_AGC_ENABLE),y)
CCFLAG += -DMTK_VOICE_AGC_ENABLE
endif


##
## MTK_BT_AGC_USE_PIC
## Brief:       Internal use.
## Notice:      MTK_BT_AGC_USE_PIC is a option to use AGC PIC. If MTK_VOICE_AGC_ENABLE is enabled, this compile option will be enabled.
##
ifeq ($(MTK_BT_AGC_USE_PIC),y)
CCFLAG += -DMTK_BT_AGC_USE_PIC
endif



##
## MTK_PLC_USE_PIC
## Brief:       Internal use.
## Notice:      MTK_PLC_USE_PIC is a option to use Packet Lost Compensation(PLC) PIC. Default should be enabled.
##
ifeq ($(MTK_PLC_USE_PIC),y)
CCFLAG += -DMTK_PLC_USE_PIC
endif

##
## MTK_BT_HFP_SPE_ALG_V2
## Brief:       Internal use.
## Notice:      MTK_BT_HFP_SPE_ALG_V2 is a option for mt2822 which use different algorithm interface (CPD, clk skew). Default should be enabled.
##
ifeq ($(MTK_BT_HFP_SPE_ALG_V2),y)
CCFLAG += -DMTK_BT_HFP_SPE_ALG_V2
endif

##
## AIR_HWSRC_ON_MAIN_STREAM_ENABLE
## Brief:       Internal use.
## Notice:      AIR_HWSRC_ON_MAIN_STREAM_ENABLE is a option support HWSRC dor DL1.
##
AIR_HWSRC_ON_MAIN_STREAM_ENABLE ?= $(ENABLE_HWSRC_ON_MAIN_STREAM)
ifeq ($(AIR_HWSRC_ON_MAIN_STREAM_ENABLE),y)
CCFLAG += -DENABLE_HWSRC_ON_MAIN_STREAM
CCFLAG += -DENABLE_HWSRC_CLKSKEW
endif

##
## AIR_HWSRC_IN_STREAM_ENABLE
## Brief:       Internal use.
## Notice:      AIR_HWSRC_IN_STREAM_ENABLE is a option for speaker project hwsrc clk skew. Default should not be enabled.
##
AIR_HWSRC_IN_STREAM_ENABLE ?= $(MTK_HWSRC_IN_STREAM)
ifeq ($(AIR_HWSRC_IN_STREAM_ENABLE),y)
CCFLAG += -DMTK_HWSRC_IN_STREAM
endif

##
## AIR_BT_HFP_ENABLE
## Brief:       Internal use.
## Notice:      AIR_BT_HFP_ENABLE is a option for HFP feature. Default should be enabled.
##
ifeq ($(AIR_BT_HFP_ENABLE),y)
CCFLAG += -DAIR_BT_HFP_ENABLE
endif

##
## MTK_BT_HFP_FORWARDER_ENABLE
## Brief:       Internal use.
## Notice:      MTK_BT_HFP_FORWARDER_ENABLE is a option for mt2822 HFP which have audio forwarder. Default should be enabled.
##
ifeq ($(MTK_BT_HFP_FORWARDER_ENABLE),y)
CCFLAG += -DMTK_BT_HFP_FORWARDER_ENABLE
endif

##
## AIR_LE_AUDIO_ENABLE
## Brief:       Internal use.
## Notice:      AIR_LE_AUDIO_ENABLE is a option for BLE feature. Default should be enabled.
##
ifeq ($(AIR_LE_AUDIO_ENABLE),y)
CCFLAG += -DAIR_LE_AUDIO_ENABLE
endif

##
## AIR_LE_AUDIO_DONGLE_ENABLE
## Brief:       Internal use.
## Notice:      AIR_LE_AUDIO_DONGLE_ENABLE is a option for BLE dongle feature. Default should be enabled.
##
ifeq ($(AIR_LE_AUDIO_DONGLE_ENABLE),y)
CCFLAG += -DAIR_LE_AUDIO_DONGLE_ENABLE
CCFLAG += -DAIR_BLE_AUDIO_DONGLE_ENABLE
endif

##
## AIR_BT_CODEC_BLE_ENABLED
## Brief:       Internal use.
## Notice:      AIR_BT_CODEC_BLE_ENABLED is a option for BLE feature. Default should be enabled.
##
ifeq ($(AIR_BT_CODEC_BLE_ENABLED),y)
CCFLAG += -DAIR_BT_CODEC_BLE_ENABLED
endif



##
## AIR_BT_LE_LC3_ENABLE
## Brief:       Internal use.
## Notice:      AIR_BT_LE_LC3_ENABLE is a option for LC3 feature. Default should be enabled.
##
ifeq ($(AIR_BT_LE_LC3_ENABLE),y)
CCFLAG += -DAIR_BT_LE_LC3_ENABLE
endif

##
## AIR_BT_CODEC_BLE_V2_ENABLED
## Brief:       Internal use.
## Notice:      AIR_BT_CODEC_BLE_V2_ENABLED is a option for BLE feature. Default should be enabled.
##
ifeq ($(AIR_BT_CODEC_BLE_V2_ENABLED),y)
CCFLAG += -DAIR_BT_CODEC_BLE_V2_ENABLED
endif

##
## AIR_BT_A2DP_LC3_USE_PIC
## Brief:       Internal use.
## Notice:      AIR_BT_A2DP_LC3_USE_PIC is a option to use LC3 PIC. Default should be enabled.
##
ifeq ($(MTK_BT_A2DP_LC3_USE_PIC),y)
CCFLAG += -DMTK_BT_A2DP_LC3_USE_PIC
endif

##
## AIR_BT_A2DP_LC3_USE_LIGHT_PIC
## Brief:       Internal use.
## Notice:      AIR_BT_A2DP_LC3_USE_LIGHT_PIC is a option to use LC3 light PIC. Default should be enabled.
##
ifeq ($(MTK_BT_A2DP_LC3_USE_LIGHT_PIC),y)
CCFLAG += -DMTK_BT_A2DP_LC3_USE_LIGHT_PIC
endif

##
## AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE
## Brief:       Internal use.
## Notice:      AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE is a option support LC3 use LC3PLUS PLC. Default should be disabled.
##
ifeq ($(AIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE),y)
CCFLAG += -DAIR_LC3_USE_LC3PLUS_PLC_CUSTOMIZE
endif

##
## AIR_VOICE_BAND_CONFIG_TYPE
## Brief:       This option is to choose the BAND.
## Usage:       The valid values are WB, SWB, FB.
##              The setting will determine which the maximum bandwidth uplink will be used.
##              WB    : The maximum bandwidth achievable is WB
##              SWB   : The maximum bandwidth achievable is SWB
##              FB    : The maximum bandwidth achievable is FB
## Path:        driver/chip/
## Dependency:  None
## Notice:      None
## Relative doc:None
##
ifeq ($(AIR_VOICE_BAND_CONFIG_TYPE),WB)
else ifeq ($(AIR_VOICE_BAND_CONFIG_TYPE),SWB)
CCFLAG += -DAIR_VOICE_BAND_CONFIG_TYPE_SWB
AIR_BT_BLE_SWB_ENABLE = y
ifneq ($(AIR_AUDIO_VEND_CODEC_ENABLE),y)
AIR_BT_ULL_SWB_ENABLE = y
endif
else ifeq ($(AIR_VOICE_BAND_CONFIG_TYPE),FB)
CCFLAG += -DAIR_VOICE_BAND_CONFIG_TYPE_FB
AIR_BT_BLE_SWB_ENABLE = y
ifneq ($(AIR_AUDIO_VEND_CODEC_ENABLE),y)
AIR_BT_ULL_SWB_ENABLE = y
AIR_BT_ULL_FB_ENABLE = y
endif
endif

ifeq ($(AIR_BT_BLE_SWB_ENABLE),y)
CCFLAG += -DAIR_BT_BLE_SWB_ENABLE
endif
ifeq ($(AIR_BT_ULL_SWB_ENABLE),y)
CCFLAG += -DAIR_BT_ULL_SWB_ENABLE
endif
ifeq ($(AIR_BT_ULL_FB_ENABLE),y)
CCFLAG += -DAIR_BT_ULL_FB_ENABLE
endif

##
## AIR_BLE_FIXED_RATIO_SRC_ENABLE
## Brief:       This option is to enable BLE SWB
## Usage:       If the value is "y",  the AIR_BLE_FIXED_RATIO_SRC_ENABLE option will be defined.
## Path:        driver/chip/
## Dependency:  None
## Notice:      None
## Relative doc:None
##
ifeq ($(AIR_BLE_FIXED_RATIO_SRC_ENABLE),y)
CCFLAG += -DAIR_BLE_FIXED_RATIO_SRC_ENABLE
CCFLAG += -DAIR_FIXED_RATIO_SRC
endif

##
## AIR_BLE_FEATURE_MODE_ENABLE
## Brief:       Internal use.
## Notice:      AIR_BLE_FEATURE_MODE_ENABLE is a option to use SWB SPE. Default should be enabled.
##
ifeq ($(AIR_BLE_FEATURE_MODE_ENABLE),y)
CCFLAG += -DAIR_BLE_FEATURE_MODE_ENABLE
endif

##
## MTK_DSP_AUDIO_MESSAGE_ENABLE
## Brief:       Internal use.
## Notice:      MTK_DSP_AUDIO_MESSAGE_ENABLE is a option to enable audio message ISR handler. Default must be enabled.
##
ifeq ($(MTK_DSP_AUDIO_MESSAGE_ENABLE),y)
CCFLAG += -DMTK_DSP_AUDIO_MESSAGE_ENABLE
endif

##
## AIR_USB_AUDIO_IN_ENABLE
## Brief:       Internal use.
## Notice:      AIR_USB_AUDIO_IN_ENABLE is a option to support usb-in audio.
##
ifeq ($(AIR_USB_AUDIO_IN_ENABLE),y)
CCFLAG += -DAIR_USB_AUDIO_IN_ENABLE
MTK_CM4_PLAYBACK_ENABLE = y
endif
##
## MTK_CM4_PLAYBACK_ENABLE
## Brief:       Internal use.
## Notice:      MTK_CM4_PLAYBACK_ENABLE is a option to support CM4 playback function.
##
ifeq ($(MTK_CM4_PLAYBACK_ENABLE),y)
CCFLAG += -DMTK_CM4_PLAYBACK_ENABLE
endif

##
## AIR_MIC_RECORD_ENABLE
## Brief:       Internal use.
## Notice:      AIR_MIC_RECORD_ENABLE is a option to support CM4 record function.
##
AIR_MIC_RECORD_ENABLE ?= $(MTK_CM4_RECORD_ENABLE)
ifeq ($(AIR_MIC_RECORD_ENABLE),y)
CCFLAG += -DMTK_CM4_RECORD_ENABLE
endif

##
## AIR_ANC_WIND_NOISE_DETECTION_ENABLE
## Brief:       Internal use.
## Notice:      AIR_ANC_WIND_NOISE_DETECTION_ENABLE is a option to support wind detection to suppress noise when ANC on.
##
ifeq ($(AIR_ANC_ENABLE),y)
ifeq ($(AIR_ANC_WIND_NOISE_DETECTION_ENABLE),y)
ifeq ($(AIR_WIND_DETECTION_USE_PIC),y)
CCFLAG += -DAIR_WIND_DETECTION_USE_PIC
endif
MTK_ANC_SURROUND_MONITOR_ENABLE = y
CCFLAG += -DAIR_ANC_WIND_DETECTION_ENABLE
endif
endif

## AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
## Brief:       Internal use.
## Notice:      AIR_ANC_ENVIRONMENT_DETECTION_ENABLE
##
ifeq ($(AIR_ANC_ENABLE),y)
ifeq ($(AIR_ANC_ENVIRONMENT_DETECTION_ENABLE),y)
MTK_ANC_SURROUND_MONITOR_ENABLE = y
CCFLAG += -DAIR_ANC_ENVIRONMENT_DETECTION_ENABLE
endif
endif

##
## MTK_ANC_SURROUND_MONITOR_ENABLE
## Brief:       Internal use.
## Notice:      MTK_ANC_SURROUND_MONITOR_ENABLE is a option to support ANC surround monitor functions.
##
ifeq ($(AIR_ANC_ENABLE),y)
ifeq ($(MTK_ANC_SURROUND_MONITOR_ENABLE),y)
CCFLAG += -DMTK_ANC_SURROUND_MONITOR_ENABLE
CCFLAG += -DAIR_AUDIO_TRANSMITTER_ENABLE
endif
endif

## ANC_DEBUG_PATH_EANBLE
## Brief:       Internal use.
## Notice:      ANC_DEBUG_PATH_EANBLE
##
ifeq ($(AIR_ANC_ENABLE),y)
ifeq ($(AIR_ANC_DEBUG_PATH_EANBLE),y)
CCFLAG += -DANC_DEBUG_PATH_EANBLE
endif
endif

##
## AIR_SIDETONE_ENABLE
## Brief:       Internal use.
## Notice:      AIR_SIDETONE_ENABLE is a option to support sidetone function.
##
ifeq ($(AIR_SIDETONE_ENABLE),y)
CCFLAG += -DAIR_SIDETONE_ENABLE
endif

##
## AIR_SIDETONE_VERIFY_ENABLE
## Brief:       Internal use.
## Notice:      AIR_SIDETONE_VERIFY_ENABLE is a option to support sidetone AT CMD.
##
ifeq ($(AIR_SIDETONE_VERIFY_ENABLE),y)
CCFLAG += -DAIR_SIDETONE_VERIFY_ENABLE
endif

##
## AIR_BT_A2DP_ENABLE
## Brief:       Internal use.
## Notice:      AIR_BT_A2DP_ENABLE is a option to support AUDIO LOOPBACK TEST function.
##
AIR_BT_A2DP_ENABLE ?= $(MTK_BT_A2DP_ENABLE)
ifeq ($(AIR_BT_A2DP_ENABLE),y)
CCFLAG += -DMTK_BT_A2DP_ENABLE
endif

##
## MTK_BT_A2DP_AAC_ENABLE
## Brief:       Internal use.
## Notice:      MTK_BT_A2DP_AAC_ENABLE is a option to use AAC codec.
##
ifeq ($(AIR_BT_A2DP_AAC_ENABLE),y)
CCFLAG += -DMTK_BT_A2DP_AAC_ENABLE
endif

##
## AIR_BT_A2DP_SBC_ENABLE
## Brief:       Internal use.
## Notice:      AIR_BT_A2DP_SBC_ENABLE is a option to use SBC codec.
##
AIR_BT_A2DP_SBC_ENABLE ?= $(MTK_BT_A2DP_SBC_ENABLE)
ifeq ($(AIR_BT_A2DP_SBC_ENABLE),y)
CCFLAG += -DMTK_BT_A2DP_SBC_ENABLE
endif

##
## MTK_BT_A2DP_AAC_USE_PIC
## Brief:       Internal use.
## Notice:      MTK_BT_A2DP_AAC_USE_PIC is a option to use AAC PIC. If MTK_BT_A2DP_AAC_ENABLE is enabled, this one should be enabled too.
##
ifeq ($(MTK_BT_A2DP_AAC_USE_PIC),y)
CCFLAG += -DMTK_BT_A2DP_AAC_USE_PIC
endif

##
## MTK_BT_A2DP_SBC_USE_PIC
## Brief:       Internal use.
## Notice:      MTK_BT_A2DP_SBC_USE_PIC is a option to use SBC PIC. Default should be enabled.
##
ifeq ($(MTK_BT_A2DP_SBC_USE_PIC),y)
CCFLAG += -DMTK_BT_A2DP_SBC_USE_PIC
endif

##
## AIR_ADAPITVE_EQ_DETECTION_USE_PIC_ENABLE
## Brief:       Internal use.
## Notice:      AIR_ADAPITVE_EQ_DETECTION_USE_PIC_ENABLE is a option to use adaptive eq PIC. Default should be enabled.
##
ifeq ($(AIR_ADAPITVE_EQ_DETECTION_USE_PIC_ENABLE),y)
CCFLAG += -DAIR_ADAPITVE_EQ_DETECTION_USE_PIC_ENABLE
endif

##
## MTK_BT_A2DP_VENDOR_ENABLE
## Brief:       Internal use.
## Notice:      MTK_BT_A2DP_VENDOR_ENABLE is an option to use Vendor codec & Vendor codec PIC. Default should be enabled.
##
VENDOR_LIB = $(strip $(ROOTDIR))/middleware/third_party/dspalg/vendor_decoder/module.mk
ifeq ($(VENDOR_LIB), $(wildcard $(VENDOR_LIB)))
ifeq ($(AIR_BT_A2DP_VENDOR_ENABLE),y)
CCFLAG += -DMTK_BT_A2DP_VENDOR_ENABLE
##
## MTK_BT_A2DP_VENDOR_USE_PIC
## Brief:       Internal use.
## Notice:      MTK_BT_A2DP_VENDOR_USE_PIC is an option to use Vendor codec PIC. Default should be enabled.
##
ifeq ($(MTK_BT_A2DP_VENDOR_USE_PIC),y)
CCFLAG += -DMTK_BT_A2DP_VENDOR_USE_PIC
endif
##
## MTK_BT_A2DP_VENDOR_BC_ENABLE
## Brief:       Internal use.
## Notice:      MTK_BT_A2DP_VENDOR_BC_ENABLE is an option to apply buffer control to the Vendor codec. Default should be enabled.
##
ifeq ($(MTK_BT_A2DP_VENDOR_BC_ENABLE),y)
CCFLAG += -DMTK_BT_A2DP_VENDOR_BC_ENABLE
endif
endif
else
AIR_BT_A2DP_VENDOR_ENABLE = n
endif

VENDOR_1_LIB = $(strip $(ROOTDIR))/middleware/third_party/dspalg/vendor_1_decoder/module.mk
ifeq ($(VENDOR_1_LIB), $(wildcard $(VENDOR_1_LIB)))
ifeq ($(AIR_BT_A2DP_VENDOR_1_ENABLE),y)
CCFLAG += -DMTK_BT_A2DP_VENDOR_1_ENABLE
##
## MTK_BT_A2DP_VENDOR_USE_PIC
## Brief:       Internal use.
## Notice:      MTK_BT_A2DP_VENDOR_USE_PIC is an option to use Vendor codec PIC. Default should be enabled.
##
ifeq ($(MTK_BT_A2DP_VENDOR_1_USE_PIC),y)
CCFLAG += -DMTK_BT_A2DP_VENDOR_1_USE_PIC
endif
##
## MTK_BT_A2DP_VENDOR_BC_ENABLE
## Brief:       Internal use.
## Notice:      MTK_BT_A2DP_VENDOR_BC_ENABLE is an option to apply buffer control to the Vendor codec. Default should be enabled.
##
ifeq ($(AIR_BT_A2DP_VENDOR_1_BC_ENABLE),y)
CCFLAG += -DMTK_BT_A2DP_VENDOR_1_BC_ENABLE
endif
endif
else
AIR_BT_A2DP_VENDOR_1_ENABLE = n
endif

##
## AIR_BT_A2DP_LC3PLUS_ENABLE
## Brief:       Internal use.
## Notice:      AIR_BT_A2DP_LC3PLUS_ENABLE is an option to use LC3PLUS codec.
##
LC3PLUS_LIB = $(strip $(ROOTDIR))/prebuilt/middleware/third_party/dspalg/lc3plus_codec/$(IC_CONFIG)/pisplit/codec_lib/pisplit_lc3plusi_codec.o
ifeq ($(LC3PLUS_LIB), $(wildcard $(LC3PLUS_LIB)))
ifeq ($(AIR_BT_A2DP_LC3PLUS_ENABLE),y)
CCFLAG += -DAIR_BT_A2DP_LC3PLUS_ENABLE
endif
else
AIR_BT_A2DP_LC3PLUS_ENABLE = n
endif

##
## AIR_MUTE_SMOOTHER_ENABLE
## Brief:       Internal use.
## Notice:      AIR_MUTE_SMOOTHER_ENABLE is a option to global turn on/off mute smoother function.
##
ifeq ($(AIR_MUTE_SMOOTHER_ENABLE),y)
CCFLAG += -DAIR_MUTE_SMOOTHER_ENABLE
endif

##
## AIR_VOICE_PLC_ENABLE
## Brief:       Internal use.
## Notice:      AIR_VOICE_PLC_ENABLE is a option to global turn on/off voice PLC function.
##
ifeq ($(AIR_VOICE_PLC_ENABLE),y)
CCFLAG += -DAIR_VOICE_PLC_ENABLE
endif

##
## AIR_BT_HFP_CVSD_ENABLE
## Brief:       Internal use.
## Notice:      AIR_BT_HFP_CVSD_ENABLE is a option to global turn on/off CVSD encode/decode function.
##
ifeq ($(AIR_BT_HFP_CVSD_ENABLE),y)
CCFLAG += -DAIR_BT_HFP_CVSD_ENABLE
endif

##
## AIR_BT_A2DP_CVSD_USE_PIC_ENABLE
## Brief:       Internal use.
## Notice:      AIR_BT_A2DP_CVSD_USE_PIC_ENABLE is a option to global turn on/off CVSD encode/decode function.
##
ifeq ($(AIR_BT_A2DP_CVSD_USE_PIC_ENABLE),y)
CCFLAG += -DAIR_BT_A2DP_CVSD_USE_PIC_ENABLE
endif

##
## AIR_BT_HFP_MSBC_ENABLE
## Brief:       Internal use.
## Notice:      AIR_BT_HFP_MSBC_ENABLE is a option to global turn on/off mSBC encode/decode function.
##
ifeq ($(AIR_BT_HFP_MSBC_ENABLE),y)
CCFLAG += -DAIR_BT_HFP_MSBC_ENABLE
endif

##
## AIR_VOICE_NR_ENABLE
## Brief:       Internal use.
## Notice:      AIR_VOICE_NR_ENABLE is a option to global turn on/off voice NR function.
##
ifeq ($(AIR_VOICE_NR_ENABLE),y)
CCFLAG += -DAIR_VOICE_NR_ENABLE
endif

##
## AIR_VOICE_NR_USE_PIC_ENABLE
## Brief:       Internal use.
## Notice:      AIR_VOICE_NR_USE_PIC_ENABLE is a option to use ECNR(Echo Cancellation / Noice Reduction) PIC. Default should be enabled.
##
ifeq ($(AIR_VOICE_NR_USE_PIC_ENABLE),y)
CCFLAG += -DAIR_VOICE_NR_USE_PIC_ENABLE
endif

##
## AIR_ECNR_1_OR_2_MIC_ENABLE
## Brief:       Internal use.
## Notice:      AIR_ECNR_1_OR_2_MIC_ENABLE is a option for 1 or 2. Default should not be enabled.
##
ifeq ($(AIR_ECNR_1_OR_2_MIC_ENABLE),y)
CCFLAG += -DAIR_ECNR_1_OR_2_MIC_ENABLE
endif

##
## AIR_ECNR_1MIC_INEAR_ENABLE
## Brief:       Internal use.
## Notice:      AIR_ECNR_1MIC_INEAR_ENABLE is a option to support INEAR function.
##
ifeq ($(AIR_ECNR_1MIC_INEAR_ENABLE),y)
CCFLAG += -DAIR_ECNR_1MIC_INEAR_ENABLE
endif

##
## AIR_ECNR_2MIC_INEAR_ENABLE
## Brief:       Internal use.
## Notice:      AIR_ECNR_2MIC_INEAR_ENABLE is a option to support 2+1NR function.
##
ifeq ($(AIR_ECNR_2MIC_INEAR_ENABLE),y)
CCFLAG += -DAIR_ECNR_2MIC_INEAR_ENABLE
endif

##
## AIR_AI_NR_PREMIUM_ENABLE
## Brief:       Internal use.
## Notice:      AIR_AI_NR_PREMIUM_ENABLE is a option to support AINR premium function.
##
ifeq ($(AIR_AI_NR_PREMIUM_ENABLE),y)
CCFLAG += -DAIR_AI_NR_PREMIUM_ENABLE
endif

##
## AIR_AI_NR_PREMIUM_270K_VARIANT_1_ENABLE
## Brief:       Internal use.
## Notice:      AIR_AI_NR_PREMIUM_270K_VARIANT_1_ENABLE is a option to support AINR premium with FB function.
##
ifeq ($(AIR_AI_NR_PREMIUM_270K_VARIANT_1_ENABLE),y)
CCFLAG += -DAIR_AI_NR_PREMIUM_270K_VARIANT_1_ENABLE
endif

##
## AIR_AI_NR_PREMIUM_INEAR_ENABLE
## Brief:       Internal use.
## Notice:      AIR_AI_NR_PREMIUM_INEAR_ENABLE is a option to support AINR premium with FB function.
##
ifeq ($(AIR_AI_NR_PREMIUM_INEAR_ENABLE),y)
CCFLAG += -DAIR_AI_NR_PREMIUM_INEAR_ENABLE
endif

##
## AIR_AI_NR_PREMIUM_INEAR_270K_VARIANT_1_ENABLE
## Brief:       Internal use.
## Notice:      AIR_AI_NR_PREMIUM_INEAR_270K_VARIANT_1_ENABLE is a option to support AINR premium with FB function.
##
ifeq ($(AIR_AI_NR_PREMIUM_INEAR_270K_VARIANT_1_ENABLE),y)
CCFLAG += -DAIR_AI_NR_PREMIUM_INEAR_270K_VARIANT_1_ENABLE
endif

##
## AIR_AI_NR_PREMIUM_INEAR_500K_BROADSIDE_ENABLE
## Brief:       Internal use.
## Notice:      AIR_AI_NR_PREMIUM_INEAR_500K_BROADSIDE_ENABLE is a option to support AINR premium with FB function.
##
ifeq ($(AIR_AI_NR_PREMIUM_INEAR_500K_BROADSIDE_ENABLE),y)
CCFLAG += -DAIR_AI_NR_PREMIUM_INEAR_500K_BROADSIDE_ENABLE
endif

##
## AIR_AI_NR_PREMIUM_INEAR_500K_PRO_DISTRACTOR_ENABLE
## Brief:       Internal use.
## Notice:      AIR_AI_NR_PREMIUM_INEAR_500K_PRO_DISTRACTOR_ENABLE is a option to support AINR premium with FB function.
##
ifeq ($(AIR_AI_NR_PREMIUM_INEAR_500K_PRO_DISTRACTOR_ENABLE),y)
CCFLAG += -DAIR_AI_NR_PREMIUM_INEAR_500K_PRO_DISTRACTOR_ENABLE
endif

##
## AIR_AI_NR_PREMIUM_INEAR_500K_PRO_TWS_OO_ENABLE
## Brief:       Internal use.
## Notice:      AIR_AI_NR_PREMIUM_INEAR_500K_PRO_TWS_OO_ENABLE is a option to support AINR premium with FB function.
##
ifeq ($(AIR_AI_NR_PREMIUM_INEAR_500K_PRO_TWS_OO_ENABLE),y)
CCFLAG += -DAIR_AI_NR_PREMIUM_INEAR_500K_PRO_TWS_OO_ENABLE
endif

##
## AIR_3RD_PARTY_NR_ENABLE
## Brief:       Internal use.
## Notice:      AIR_3RD_PARTY_NR_ENABLE is a option to support 3rd party ECNR function.
##
ifeq ($(AIR_3RD_PARTY_NR_ENABLE),y)
CCFLAG += -DAIR_3RD_PARTY_NR_ENABLE
endif

##
## AIR_ECNR_PREV_PART_ENABLE
## Brief:       Internal use.
## Notice:      AIR_ECNR_PREV_PART_ENABLE is a option to support split ECNR function.
##
ifeq ($(AIR_ECNR_PREV_PART_ENABLE),y)
CCFLAG += -DAIR_ECNR_PREV_PART_ENABLE
endif

##
## AIR_ECNR_POST_PART_ENABLE
## Brief:       Internal use.
## Notice:      AIR_ECNR_POST_PART_ENABLE is a option to support split ECNR function.
##
ifeq ($(AIR_ECNR_POST_PART_ENABLE),y)
CCFLAG += -DAIR_ECNR_POST_PART_ENABLE
endif

##
## AIR_ECNR_SEPARATE_MODE_ENABLE
## Brief:       Internal use.
## Notice:      AIR_ECNR_SEPARATE_MODE_ENABLE is a option to support split ECNR function.
##
ifeq ($(AIR_ECNR_SEPARATE_MODE_ENABLE),y)
CCFLAG += -DAIR_ECNR_SEPARATE_MODE_ENABLE
endif

##
## AIR_ECNR_PREV_PART_USE_PIC_ENABLE
## Brief:       Internal use.
## Notice:      AIR_ECNR_PREV_PART_USE_PIC_ENABLE is a option to enable split ECNR PIC.
##
ifeq ($(AIR_ECNR_PREV_PART_USE_PIC_ENABLE),y)
CCFLAG += -DAIR_ECNR_PREV_PART_USE_PIC_ENABLE
endif

##
## AIR_ECNR_POST_PART_USE_PIC_ENABLE
## Brief:       Internal use.
## Notice:      AIR_ECNR_POST_PART_USE_PIC_ENABLE is a option to enable split ECNR PIC.
##
ifeq ($(AIR_ECNR_POST_PART_USE_PIC_ENABLE),y)
CCFLAG += -DAIR_ECNR_POST_PART_USE_PIC_ENABLE
endif

## AIR_MULTI_MIC_STREAM_ENABLE
## Brief:       Internal use.
## Notice:      AIR_MULTI_MIC_STREAM_ENABLE is a option to concurrently use AFE source.
##
ifeq ($(AIR_MULTI_MIC_STREAM_ENABLE),y)
CCFLAG += -DAIR_MULTI_MIC_STREAM_ENABLE
endif

##
## AIR_AIRDUMP_ENABLE
## Brief:       Internal use.
## Notice:      AIR_AIRDUMP_ENABLE is a option to to support AirDump function.
ifeq ($(AIR_AIRDUMP_ENABLE),y)
CCFLAG += -DAIR_AIRDUMP_ENABLE
endif

##
## AIR_AUDIO_DUMP_BY_SPDIF_ENABLE
## Brief:       Internal use.
## Notice:      AIR_AUDIO_DUMP_BY_SPDIF_ENABLE is an option to support audio dump over SPDIF interface.
ifeq ($(AIR_AUDIO_DUMP_BY_SPDIF_ENABLE),y)
CCFLAG += -DAIR_AUDIO_DUMP_BY_SPDIF_ENABLE
endif

##
## AIR_AUDIO_TRANSMITTER_ENABLE
## Brief:       Internal use.
## Notice:      AIR_AUDIO_TRANSMITTER_ENABLE is a option for audio transmitter
ifeq ($(AIR_AUDIO_TRANSMITTER_ENABLE),y)
CCFLAG += -DAIR_AUDIO_TRANSMITTER_ENABLE
endif

##
## AIR_AUDIO_BT_COMMON_ENABLE
## Brief:       Internal use.
## Notice:      AIR_AUDIO_BT_COMMON_ENABLE is a option to to support audio bt common architecture.
ifeq ($(AIR_AUDIO_BT_COMMON_ENABLE), y)
CCFLAG += -DAIR_AUDIO_BT_COMMON_ENABLE
endif

##
## AIR_CPU_SECURITY_MODE
## Brief:       Internal use. This option is determine build in security world or none security world.
## Usage:       The valid values are s,ns.
## Dependency:  None
## Notice:      None
##
ifeq ($(AIR_CPU_SECURITY_MODE),s)
CCFLAG += -DAIR_CPU_IN_SECURITY_MODE
else
#CCFLAG += -DAIR_CPU_IN_NON_SECURITY_MODE
endif

##
## AIR_AUDIO_HARDWARE_ENABLE
## Brief:       Internal use.
## Notice:      AIR_AUDIO_HARDWARE_ENABLE is a option to to enable audio hardware.
ifeq ($(AIR_AUDIO_HARDWARE_ENABLE), y)
CCFLAG += -DAIR_AUDIO_HARDWARE_ENABLE
endif

##
## AIR_HWSRC_CLKSKEW_ENABLE
## Brief:       Internal use.
## Notice:      AIR_HWSRC_CLKSKEW_ENABLE is a option to to enable HWSRC CLK SKEW.
ifeq ($(AIR_HWSRC_CLKSKEW_ENABLE), y)
CCFLAG += -DENABLE_HWSRC_CLKSKEW
endif

##
## AIR_BT_CLK_SKEW_ENABLE
## Brief:       Internal use.
## Notice:      AIR_HWSRC_CLKSKEW_ENABLE is a option to to enable Airoha CLK Skew.
ifeq ($(AIR_BT_CLK_SKEW_ENABLE), y)
CCFLAG += -DAIR_BT_CLK_SKEW_ENABLE
endif

##
## AIR_SOFTWARE_SRC_ENABLE
## Brief:       Internal use.
## Notice:      AIR_SOFTWARE_SRC_ENABLE is a option to enable software src.
##
ifeq ($(AIR_SOFTWARE_SRC_ENABLE),y)
CCFLAG += -DAIR_SOFTWARE_SRC_ENABLE
endif

##
## AIR_COMPONENT_CALIBRATION_ENABLE
## Brief:       Internal use.
## Notice:      AIR_COMPONENT_CALIBRATION_ENABLE is a option for enable calibration of microphone and speaker
ifeq ($(AIR_COMPONENT_CALIBRATION_ENABLE),y)
CCFLAG += -DAIR_COMPONENT_CALIBRATION_ENABLE
endif

##
## AIR_SOFTWARE_CLK_SKEW_ENABLE
## Brief:       Internal use.
## Notice:      AIR_SOFTWARE_CLK_SKEW_ENABLE is a option to enable software clk skew.
##
ifeq ($(AIR_SOFTWARE_CLK_SKEW_ENABLE),y)
CCFLAG += -DAIR_SOFTWARE_CLK_SKEW_ENABLE
endif

##
## AIR_HEARTHROUGH_MAIN_ENABLE
## Brief:       Internal use.
## Notice:      AIR_HEARTHROUGH_MAIN_ENABLE is a option to support series of Hear Through.
##
ifeq ($(AIR_HEARTHROUGH_MAIN_ENABLE),y)
ifeq ($(AIR_ANC_ENABLE),y)
AIR_PSAP_CODE = $(strip $(ROOTDIR))/middleware/airoha/llf/stream/
ifneq ($(wildcard $(AIR_PSAP_CODE)),)
    CCFLAG += -DAIR_HEARTHROUGH_MAIN_ENABLE
    ifeq ($(AIR_HEARTHROUGH_VIVID_PT_ENABLE),y)
        AIR_VIVID_LIB = $(strip $(ROOTDIR))/prebuilt/middleware/airoha/dspalg/vivid_pt/$(IC_CONFIG)/
        ifneq ($(wildcard $(AIR_VIVID_LIB)),)
            CCFLAG += -DAIR_HEARTHROUGH_VIVID_PT_ENABLE
            ifeq ($(AIR_HEARTHROUGH_VIVID_PT_USE_PIC),y)
                CCFLAG += -DAIR_HEARTHROUGH_VIVID_PT_USE_PIC
            endif
        endif
    endif
    ifeq ($(AIR_HEARTHROUGH_PSAP_ENABLE),y)
        AIR_HEARTHROUGH_PSAP_LIB = $(strip $(ROOTDIR))/prebuilt/middleware/airoha/dspalg/hearthrough_psap/$(IC_CONFIG)/
        ifneq ($(wildcard $(AIR_HEARTHROUGH_PSAP_LIB)),)
            CCFLAG += -DAIR_HEARTHROUGH_PSAP_ENABLE
            CCFLAG += -DAIR_DAC_MODE_RUNTIME_CHANGE
            ifeq ($(AIR_HEARTHROUGH_PSAP_USE_PIC),y)
                CCFLAG += -DAIR_HEARTHROUGH_PSAP_USE_PIC
            endif
        endif
        AIR_CUSTOMIZED_LLF_ENABLE := y
    endif
    ifeq ($(AIR_HEARING_AID_ENABLE),y)
        AIR_HA_LIB = $(strip $(ROOTDIR))/prebuilt/middleware/airoha/dspalg/hearing_aid/$(IC_CONFIG)/
        ifneq ($(wildcard $(AIR_HA_LIB)),)
            CCFLAG += -DAIR_HEARING_AID_ENABLE
            CCFLAG += -DAIR_HEARTHROUGH_HA_ENABLE
            CCFLAG += -DAIR_DAC_MODE_RUNTIME_CHANGE
            AIR_SOFTWARE_GAIN_ENABLE = y
            ifeq ($(AIR_HEARTHROUGH_HA_USE_PIC),y)
                CCFLAG += -DAIR_HEARTHROUGH_HA_USE_PIC
            endif
        endif
    endif
endif
endif
endif

##
## AIR_CUSTOMIZED_LLF_ENABLE
## Brief:       This option is to open Low Latency Framework.
## Usage:       Enable the feature by configuring it as y.
## Dependency:  None
## Notice:      None
## Relative doc:None
##
ifeq ($(AIR_CUSTOMIZED_LLF_ENABLE), y)
ifeq ($(AIR_ANC_ENABLE), y)
AIR_LLF_LIB = $(strip $(ROOTDIR))/middleware/airoha/llf/stream/
AIR_LLF_SAMPLE = $(strip $(ROOTDIR))/middleware/airoha/dspalg/low_latency_framework_example/module.mk
ifneq ($(wildcard $(AIR_LLF_LIB)),)
ifneq ($(wildcard $(AIR_LLF_SAMPLE)),)
CCFLAG += -DAIR_CUSTOMIZED_LLF_ENABLE
else
AIR_CUSTOMIZED_LLF_ENABLE := n
endif
endif
endif
endif
##
## AIR_SOFTWARE_GAIN_ENABLE
## Brief:       Internal use.
## Notice:      AIR_SOFTWARE_GAIN_ENABLE is a option to enable software gain.
##
ifeq ($(AIR_SOFTWARE_GAIN_ENABLE),y)
CCFLAG += -DAIR_SOFTWARE_GAIN_ENABLE
endif

##
## AIR_DUAL_CHIP_MIXING_MODE
## Brief:       This option is used to set the type of dual chip mixing mode.
## Usage:       If this value is "master", the AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE will be defiend. If the value is "slave", the AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE will be defiend
##              If the value is "dongle", the AIR_DCHS_MODE_DONGLE_ENABLE will be defiend
## Path:        None
## Dependency:  None
## Notice:      None
## Relative doc:None
##
ifeq ($(AIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE),y)
CCFLAG  += -DAIR_DUAL_CHIP_MIXING_MODE_ROLE_MASTER_ENABLE
endif

ifeq ($(AIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE),y)
CCFLAG  += -DAIR_DUAL_CHIP_MIXING_MODE_ROLE_SLAVE_ENABLE
endif

ifeq ($(AIR_DUAL_CHIP_MIXING_MODE),master)
ifeq ($(AIR_DUAL_CHIP_AUDIO_INTERFACE),uart)
CCFLAG  += -DAIR_DCHS_MODE_ENABLE
CCFLAG  += -DAIR_DCHS_MODE_MASTER_ENABLE
CCFLAG  += -DAIR_MIXER_STREAM_ENABLE
AIR_LOW_LATENCY_MUX_ENABLE = y
AIR_SOFTWARE_MIXER_ENABLE = y
AIR_SURROUND_AUDIO_ENABLE = y
AIR_MIXER_STREAM_ENABLE = y
AIR_NLE_ENABLE:= n
else
endif

endif
ifeq ($(AIR_DUAL_CHIP_MIXING_MODE),slave)
ifeq ($(AIR_DUAL_CHIP_AUDIO_INTERFACE),uart)
CCFLAG  += -DAIR_DCHS_MODE_ENABLE
CCFLAG  += -DAIR_DCHS_MODE_SLAVE_ENABLE
CCFLAG  += -DAIR_MIXER_STREAM_ENABLE
AIR_LOW_LATENCY_MUX_ENABLE = y
AIR_SOFTWARE_MIXER_ENABLE = y
AIR_MIXER_STREAM_ENABLE = y
AIR_NLE_ENABLE:= n
else
endif
endif

ifeq ($(AIR_DUAL_CHIP_MIXING_MODE),dongle)
CCFLAG  += -DAIR_DCHS_MODE_DONGLE_ENABLE
AIR_SURROUND_AUDIO_ENABLE = y
endif

##
## AIR_DCHS_OFF_AINR_ENABLE
## Brief:       Internal use.
## Notice:      AIR_DCHS_OFF_AINR_ENABLE is a option to to disable dchs ainr.
ifeq ($(AIR_DCHS_OFF_AINR_ENABLE), y)
CCFLAG  += -DAIR_DCHS_OFF_AINR_ENABLE
endif

##
## AIR_SOFTWARE_MIXER_ENABLE
## Brief:       Internal use.
## Notice:      AIR_SOFTWARE_MIXER_ENABLE is a option to enable software mixer.
##
ifeq ($(AIR_SOFTWARE_MIXER_ENABLE),y)
CCFLAG += -DAIR_SOFTWARE_MIXER_ENABLE
endif

##
## AIR_SOFTWARE_BUFFER_ENABLE
## Brief:       Internal use.
## Notice:      AIR_SOFTWARE_BUFFER_ENABLE is a option to enable software buffer.
##
ifeq ($(AIR_SOFTWARE_BUFFER_ENABLE),y)
CCFLAG += -DAIR_SOFTWARE_BUFFER_ENABLE
endif

##
## AIR_FRAMESIZE_ADJUSTOR_ENABLE
## Brief:       Internal use.
## Notice:      AIR_FRAMESIZE_ADJUSTOR_ENABLE is a option to enable framesize adjustor
##
ifeq ($(AIR_FRAMESIZE_ADJUSTOR_ENABLE),y)
CCFLAG += -DAIR_FRAMESIZE_ADJUSTOR_ENABLE
endif

##
## AIR_ANC_FIT_DETECTION_ENABLE
## Brief:       Internal use.
## Notice:      AIR_ANC_FIT_DETECTION_ENABLE is a option to enable fit detection.
##
MTK_LEAKAGE_DETECTION_ENABLE ?= $(AIR_ANC_FIT_DETECTION_ENABLE)
ifeq ($(MTK_LEAKAGE_DETECTION_ENABLE),y)
CCFLAG += -DMTK_LEAKAGE_DETECTION_ENABLE
endif

##
## AIR_AUDIO_NONREALTIME_RX_ENABLE
## Brief:       Internal use.
## Notice:      AIR_AUDIO_NONREALTIME_RX_ENABLE is a option to enable low priority RX task mechanism.
ifeq ($(AIR_AUDIO_NONREALTIME_RX_ENABLE), y)
CCFLAG += -DAIR_AUDIO_NONREALTIME_RX_ENABLE
#CCFLAG += -DAIR_AUDIO_IRQ_LEVEL_RX_ENABLE
endif

##
## AIR_ULL_AUDIO_V2_DONGLE_ENABLE
## Brief:       Internal use.
## Notice:      AIR_ULL_AUDIO_V2_DONGLE_ENABLE is a option for ULL2.0 dongle feature.
##
ifeq ($(AIR_ULL_AUDIO_V2_DONGLE_ENABLE),y)
CCFLAG += -DAIR_ULL_AUDIO_V2_DONGLE_ENABLE
endif

##
## AIR_ULL_BLE_HEADSET_ENABLE
## Brief:       Internal use.
## Notice:      AIR_ULL_BLE_HEADSET_ENABLE is a option for ULL2.0 headset feature.
##
ifeq ($(AIR_ULL_BLE_HEADSET_ENABLE),y)
CCFLAG += -DAIR_ULL_BLE_HEADSET_ENABLE
endif

##
## AIR_BT_LE_LC3PLUS_ENABLE
## Brief:       Internal use.
## Notice:      AIR_BT_LE_LC3PLUS_ENABLE is a option for LC3plus codec.
##
ifeq ($(AIR_BT_LE_LC3PLUS_ENABLE),y)
CCFLAG += -DAIR_BT_LE_LC3PLUS_ENABLE
endif

##
## AIR_BT_A2DP_LC3PLUS_USE_PIC
## Brief:       Internal use.
## Notice:      AIRBT_A2DP_LC3PLUS_USE_PIC is a option to use LC3 PIC. Default should be enabled.
##
ifeq ($(AIR_BT_LE_LC3PLUS_USE_PIC),y)
CCFLAG += -DAIR_BT_LE_LC3PLUS_USE_PIC
endif
##

##
## AIR_BT_LE_LC3PLUS_USE_ALL_MODE
## Brief:       Internal use.
## Notice:      AIR_BT_LE_LC3PLUS_USE_ALL_MODE is a option to use LC3 performance mode. Default should be disabled.
##
ifeq ($(AIR_BT_LE_LC3PLUS_USE_ALL_MODE),y)
CCFLAG += -DAIR_BT_LE_LC3PLUS_USE_ALL_MODE
endif

##
## AIR_WIRELESS_MIC_ENABLE
## Brief:       Internal use.
## Notice:      This option is for wireless mic.
##
ifeq ($(AIR_WIRELESS_MIC_ENABLE),y)
CCFLAG += -DAIR_WIRELESS_MIC_ENABLE
endif

##
## AIR_VOLUME_CONTROL_ON_DRC_ENABLE
## Brief:       Internal use.
## Notice:      AIR_VOLUME_CONTROL_ON_DRC_ENABLE is a option to enable volume control on DRC(Dynamic Range Compression)
##
ifeq ($(AIR_VOLUME_CONTROL_ON_DRC_ENABLE),y)
CCFLAG += -DAIR_VOLUME_CONTROL_ON_DRC_ENABLE
endif

##
## AIR_I2S_SLAVE_ENABLE
## Brief:       Internal use.
## Notice:      AIR_I2S_SLAVE_ENABLE is a option to enabled I2S Slave DMA Mode.
##
ifeq ($(AIR_I2S_SLAVE_ENABLE),y)
CCFLAG += -DAIR_I2S_SLAVE_ENABLE
endif

# This option is to choose the type of dongle afe in type. Default setting is line in + i2s slv in.
# It must be set to the same value for both DSP and MCU, otherwise, it will not work.
#                  NONE                : not support afe in.
#                  LINE_IN             : Only support line in.
#                  I2S_MST_IN          : Only support i2s master in.
#                  I2S_SLV_IN          : Only support i2s slave in.
#                  LINE_IN_I2S_MST_IN  : Support line in and i2s master in, but cann't playback at the same time.
#                  LINE_IN_I2S_SLV_IN  : Support line in and i2s slave in, but cann't playback at the same time.
ifeq ($(AIR_ULL_AUDIO_V2_DONGLE_ENABLE),y)

ifeq ($(AIR_DONGLE_AFE_IN_TYPE), NONE)
endif

ifeq ($(AIR_DONGLE_AFE_IN_TYPE), LINE_IN)
AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE := y
CCFLAG += -DAIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
endif

ifeq ($(AIR_DONGLE_AFE_IN_TYPE), I2S_MST_IN)
AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE := y
CCFLAG += -DAIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE
endif

ifeq ($(AIR_DONGLE_AFE_IN_TYPE), I2S_SLV_IN)
AIR_I2S_SLAVE_ENABLE := y
AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE := y
CCFLAG += -DAIR_I2S_SLAVE_ENABLE
CCFLAG += -DAIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE
endif

ifeq ($(AIR_DONGLE_AFE_IN_TYPE), LINE_IN_I2S_MST_IN)
AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE := y
AIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE := y
CCFLAG += -DAIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
CCFLAG += -DAIR_GAMING_MODE_DONGLE_V2_I2S_MST_IN_ENABLE
endif

ifeq ($(AIR_DONGLE_AFE_IN_TYPE), LINE_IN_I2S_SLV_IN)
AIR_I2S_SLAVE_ENABLE := y
AIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE := y
AIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE := y
CCFLAG += -DAIR_GAMING_MODE_DONGLE_V2_LINE_IN_ENABLE
CCFLAG += -DAIR_I2S_SLAVE_ENABLE
CCFLAG += -DAIR_GAMING_MODE_DONGLE_V2_I2S_SLV_IN_ENABLE
endif

endif

ifeq ($(AIR_LE_AUDIO_DONGLE_ENABLE),y)
ifeq ($(AIR_DONGLE_AFE_IN_TYPE), NONE)
endif

ifeq ($(AIR_DONGLE_AFE_IN_TYPE), LINE_IN)
AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE := y
CCFLAG += -DAIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
endif

ifeq ($(AIR_DONGLE_AFE_IN_TYPE), I2S_MST_IN)
AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE := y
CCFLAG += -DAIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
endif

ifeq ($(AIR_DONGLE_AFE_IN_TYPE), I2S_SLV_IN)
AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE := y
AIR_I2S_SLAVE_ENABLE := y
CCFLAG += -DAIR_I2S_SLAVE_ENABLE
CCFLAG += -DAIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
endif

ifeq ($(AIR_DONGLE_AFE_IN_TYPE), LINE_IN_I2S_MST_IN)
AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE := y
AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE  := y
CCFLAG += -DAIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
CCFLAG += -DAIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
endif

ifeq ($(AIR_DONGLE_AFE_IN_TYPE), LINE_IN_I2S_SLV_IN)
AIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE := y
AIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE  := y
AIR_I2S_SLAVE_ENABLE := y
CCFLAG += -DAIR_I2S_SLAVE_ENABLE
CCFLAG += -DAIR_BLE_AUDIO_DONGLE_LINE_IN_ENABLE
CCFLAG += -DAIR_BLE_AUDIO_DONGLE_I2S_IN_ENABLE
endif
endif

##
## AIR_BT_AUDIO_DONGLE_ENABLE
## Brief:       Internal use.
## Notice:      AIR_BT_AUDIO_DONGLE_ENABLE is a option to to enable bt source dongle.
## Dependency:  NONE
ifeq ($(AIR_BT_AUDIO_DONGLE_ENABLE), y)
CCFLAG += -DAIR_BT_AUDIO_DONGLE_ENABLE
CCFLAG += -DAIR_BT_AUDIO_DONGLE_USB_ENABLE
AIR_BT_A2DP_SBC_ENCODER_ENABLE := y
CCFLAG += -DAIR_BT_A2DP_SBC_ENCODER_ENABLE
endif

##
## AIR_SURROUND_AUDIO_ENABLE
## Brief:       Internal use.
## Notice:      AIR_SURROUND_AUDIO_ENABLE is a option to enabled surround audio effect.
##
ifeq ($(AIR_SURROUND_AUDIO_ENABLE),y)
CCFLAG += -DAIR_SURROUND_AUDIO_ENABLE
endif

##
## AIR_VOLUME_ESTIMATOR_ENABLE
## Brief:       Internal use.
## Notice:      AIR_VOLUME_ESTIMATOR_ENABLE is a option to to enable volume estimator.
ifeq ($(AIR_VOLUME_ESTIMATOR_ENABLE), y)
CCFLAG += -DAIR_VOLUME_ESTIMATOR_ENABLE
endif

##
## AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
## Brief:       Internal use.
## Notice:      AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE is a option to to enable game/chat volume smart balance feature.
ifeq ($(AIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE), y)
CCFLAG += -DAIR_VOLUME_ESTIMATOR_ENABLE
CCFLAG += -DAIR_GAME_CHAT_VOLUME_SMART_BALANCE_ENABLE
endif

##
## AIR_AUDIO_VAD_ON_MUTE_ENABLE
## Brief:       Internal use.
## Notice:      AIR_AUDIO_VAD_ON_MUTE_ENABLE is a option to enable volume monitor in mcu side.
ifeq ($(AIR_AUDIO_VAD_ON_MUTE_ENABLE), y)
AIR_AUDIO_VOLUME_MONITOR_ENABLE       := y
CCFLAG += -DAIR_MUTE_MIC_DETECTION_ENABLE
endif

##
## AIR_AUDIO_VOLUME_MONITOR_ENABLE
## Brief:       Internal use.
## Notice:      AIR_AUDIO_VOLUME_MONITOR_ENABLE is a option to enable volume monitor in mcu side.
ifeq ($(AIR_AUDIO_VOLUME_MONITOR_ENABLE), y)
AIR_VOLUME_ESTIMATOR_ENABLE           := y
CCFLAG += -DAIR_VOLUME_ESTIMATOR_ENABLE
CCFLAG += -DAIR_AUDIO_VOLUME_MONITOR_ENABLE
endif

##
## AIR_HEARING_PROTECTION_ENABLE
## Brief:       Internal use.
## Notice:      AIR_HEARING_PROTECTION_ENABLE is a option to to enable hearing protection. AIR_VOICE_DRC_ENABLE must be enabled
ifeq ($(AIR_HEARING_PROTECTION_ENABLE), y)
CCFLAG += -DAIR_HEARING_PROTECTION_ENABLE
endif

##
## AIR_AUDIO_DOWNLINK_SW_GAIN_ENABLE
## Brief:       Internal use.
## Notice:      AIR_AUDIO_DOWNLINK_SW_GAIN_ENABLE is a option to to enable LR volume balance.
ifeq ($(AIR_AUDIO_DOWNLINK_SW_GAIN_ENABLE), y)
CCFLAG += -DAIR_AUDIO_DOWNLINK_SW_GAIN_ENABLE
endif

##
## AIR_BLE_UL_SW_GAIN_CONTROL_ENABLE
## Brief:       Internal use.
## Notice:      AIR_BLE_UL_SW_GAIN_CONTROL_ENABLE is a option to enable SW Gain control for BLE UL.
ifeq ($(AIR_BLE_UL_SW_GAIN_CONTROL_ENABLE), y)
CCFLAG += -DAIR_SOFTWARE_GAIN_ENABLE
CCFLAG += -DAIR_BLE_UL_SW_GAIN_CONTROL_ENABLE
endif

###############################################################################


ifneq ($(MTK_LOWPOWER_LEVEL),)
CCFLAG += -DMTK_LOWPOWER_LEVEL=$(MTK_LOWPOWER_LEVEL)
endif

ifeq ($(ENABLE_SIDETONE_RAMP_TIMER),y)
CCFLAG += -DENABLE_SIDETONE_RAMP_TIMER
endif

ifeq ($(ENABLE_FRAMEWORK_MULTIPLE_CHANNEL),y)
CCFLAG += -DENABLE_FRAMEWORK_MULTIPLE_CHANNEL
endif

ifeq ($(ENABLE_AUDIO_WITH_JOINT_MIC),y)
CCFLAG += -DENABLE_AUDIO_WITH_JOINT_MIC
endif

ifeq ($(MTK_DSP1_DRAM_FOR_DSP0_POOL_ENABLE),y)
CCFLAG += -DMTK_DSP1_DRAM_FOR_DSP0_POOL_ENABLE
endif

ifeq ($(AB1568_BRING_UP_DSP_DEFAULT_HW_LOOPBACK),y)
CCFLAG += -DAB1568_BRING_UP_DSP_DEFAULT_HW_LOOPBACK
endif

ifeq ($(MTK_BT_AVM_SHARE_BUF),y)
CCFLAG += -DMTK_BT_AVM_SHARE_BUF
endif

ifeq ($(AIR_AUDIO_PLC_ENABLE),y)
AUDIO_PLC_LIB = $(strip $(ROOTDIR))/middleware/third_party/dspalg/audio_plc/module.mk
ifeq ($(AUDIO_PLC_LIB), $(wildcard $(AUDIO_PLC_LIB)))
CCFLAG += -DMTK_AUDIO_PLC_ENABLE
else
AIR_AUDIO_PLC_ENABLE = n
endif
endif

ifeq ($(MTK_SLT_AUDIO_HW),y)
CCFLAG += -DMTK_SLT_AUDIO_HW
endif

##
## AIR_KEEP_I2S_ENABLE
## Brief:       Internal use.
## Notice:      AIR_KEEP_I2S_ENABLE is a option to enabled keep I2S Clock and MCLK.
##
ifeq ($(AIR_KEEP_I2S_ENABLE),y)
CCFLAG += -DAIR_KEEP_I2S_ENABLE
endif

##
## AIR_NLE_ENABLE
## Brief:       Internal use.
## Notice:      AIR_NLE_ENABLE is a option to enabled A2DP NLE feature.
##
ifeq ($(AIR_NLE_ENABLE),y)
CCFLAG += -DAIR_NLE_ENABLE
endif

##
## AIR_RESET_SDM_ENABLE
## Brief:       Internal use.
## Notice:      AIR_RESET_SDM_ENABLE is a option to enabled RESET SDM feature.
##
ifeq ($(AIR_RESET_SDM_ENABLE),y)
CCFLAG += -DAIR_RESET_SDM_ENABLE
endif

##
## AIR_AUDIO_SILENCE_DETECTION_ENABLE
## Brief:       Internal use.
## Notice:      AIR_AUDIO_SILENCE_DETECTION_ENABLE is a option to enabled silence detection feature.
##
ifeq ($(AIR_AUDIO_SILENCE_DETECTION_ENABLE),y)
AIR_SILENCE_DETECTION_ENABLE = y
CCFLAG += -DAIR_SILENCE_DETECTION_ENABLE
endif

##
## AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE
## Brief:       Internal use.
## Notice:      AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE is a option to enabled silence detection feature for BT source dongle.
##
ifeq ($(AIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE),y)
CCFLAG += -DAIR_BT_AUDIO_DONGLE_SILENCE_DETECTION_ENABLE
endif

##
## AIR_AUDIO_PATH_CUSTOMIZE_ENABLE
## Brief:       Internal use.
## Notice:      AIR_AUDIO_PATH_CUSTOMIZE_ENABLE is a option to enabled customized project audio path.
##
ifeq ($(AIR_AUDIO_PATH_CUSTOMIZE_ENABLE),y)
CCFLAG += -DAIR_AUDIO_PATH_CUSTOMIZE_ENABLE
endif

##
## AIR_ECHO_MEMIF_IN_ORDER_ENABLE
## Brief:       Internal use.
## Notice:      AIR_ECHO_MEMIF_IN_ORDER_ENABLE is a option to set echo reference memory interface in order, not fix AWB2.
##
ifeq ($(AIR_ECHO_MEMIF_IN_ORDER_ENABLE),y)
CCFLAG += -DAIR_ECHO_MEMIF_IN_ORDER_ENABLE
endif

##
## AIR_ECHO_PATH_STEREO_ENABLE
## Brief:       Internal use.
## Notice:      AIR_ECHO_PATH_STEREO_ENABLE is a option to set stereo echo reference path.
##
ifeq ($(AIR_ECHO_PATH_STEREO_ENABLE),y)
CCFLAG += -DAIR_ECHO_PATH_STEREO_ENABLE
CCFLAG += -DAIR_ECHO_MEMIF_IN_ORDER_ENABLE
endif

##
## AIR_AUDIO_HW_LOOPBACK_ENABLE
## Brief:       Internal use.
## Notice:      AIR_AUDIO_HW_LOOPBACK_ENABLE is a option to enable hordware loopback function.
##
ifeq ($(AIR_AUDIO_HW_LOOPBACK_ENABLE),y)
CCFLAG += -DAIR_AUDIO_HW_LOOPBACK_ENABLE
endif

##
## AIR_HWSRC_TX_TRACKING_ENABLE
## Brief:       Internal use.
## Notice:      AIR_HWSRC_TX_TRACKING_ENABLE is a option to enable i2s slave tx hwsrc tracking mode.
##
ifeq ($(AIR_HWSRC_TX_TRACKING_ENABLE),y)
CCFLAG += -DAIR_HWSRC_TX_TRACKING_ENABLE
endif

##
## AIR_HWSRC_RX_TRACKING_ENABLE
## Brief:       Internal use.
## Notice:      AIR_HWSRC_RX_TRACKING_ENABLE is a option to enable i2s slave rx hwsrc tracking mode.
##
ifeq ($(AIR_HWSRC_RX_TRACKING_ENABLE),y)
CCFLAG += -DAIR_HWSRC_RX_TRACKING_ENABLE
endif

##
## AIR_DUAL_CHIP_I2S_ENABLE
## Brief:       Internal use.
## Notice:      AIR_DUAL_CHIP_I2S_ENABLE is a option to enable i2s of dual chip.
##
ifeq ($(AIR_DUAL_CHIP_I2S_ENABLE),y)
CCFLAG += -DAIR_DUAL_CHIP_I2S_ENABLE
endif

##
## AIR_DUAL_CHIP_MASTER_HWSRC_RX_TRACKING_ENABLE
## Brief:       Internal use.
## Notice:      AIR_DUAL_CHIP_MASTER_HWSRC_RX_TRACKING_ENABLE is a option to to enable dual chip master hwsrc tracking mode feature.
ifeq ($(AIR_DUAL_CHIP_MASTER_HWSRC_RX_TRACKING_ENABLE), y)
CCFLAG += -DAIR_DUAL_CHIP_MASTER_HWSRC_RX_TRACKING_ENABLE
endif

##
## AIR_DUAL_CHIP_NR_ON_MASTER_ENABLE
## Brief:       Internal use.
## Notice:      AIR_DUAL_CHIP_NR_ON_MASTER_ENABLE is a option to enabled MASTER NR.
##
ifeq ($(AIR_DUAL_CHIP_NR_ON_MASTER_ENABLE),y)
CCFLAG += -DAIR_DUAL_CHIP_NR_ON_MASTER_ENABLE
endif

##
## AIR_ICE_DEBUG_ENABLE
## Brief:       This option is to enable and disable ICE debug feature.
## Usage:       Please use AT+DEBUG=? to get detail info for this atcmd
## Path:        mcu\middleware\airoha\atci\at_command\at_command_ice_debug.c
## Dependency:  None
## Notice:      None
## Realted doc: None
##
ifeq ($(AIR_ICE_DEBUG_ENABLE),y)
CCFLAG  += -DAIR_ICE_DEBUG_ENABLE
endif



# Use the library files attached to the Xtensa tool chain to participate in the link.
XTENSA_STANDARD_LIBS  +=  $(strip $(XTENSA_LIB_PATH))/arch/lib/libhal.a
XTENSA_STANDARD_LIBS  +=  $(strip $(XTENSA_LIB_PATH))/lib/libc.a
XTENSA_STANDARD_LIBS  +=  $(strip $(XTENSA_LIB_PATH))/lib/xcc/libgcc.a
XTENSA_STANDARD_LIBS  +=  $(strip $(XTENSA_LIB_PATH))/arch/lib/libhandlers-board.a
XTENSA_STANDARD_LIBS  +=  $(strip $(XTENSA_LIB_PATH))/lib/libgloss.a
XTENSA_STANDARD_LIBS  +=  $(strip $(XTENSA_LIB_PATH))/lib/libm.a
XTENSA_STANDARD_LIBS  +=  $(strip $(XTENSA_LIB_PATH))/lib/xcc/libsupc++.a
