###################################################
LLF_PATH = middleware/airoha/llf


C_SRC += $(LLF_PATH)/stream/src/stream_llf.c
INC += $(LLF_PATH)/stream/inc

ifneq ($(wildcard $(strip $(ROOTDIR))/middleware/airoha/dspfw/psap_protected/$(IC_CONFIG)/),)
C_SRC += middleware/airoha/dspfw/psap_protected/$(IC_CONFIG)/src/hal_LLF_driver.c
else
LIBS += $(ROOTDIR)/prebuilt/middleware/airoha/dspfw/psap/$(IC_CONFIG)/libllf_driver.a
endif

ifneq ($(AIR_HEARING_AID_ENABLE)_$(AIR_HEARTHROUGH_PSAP_ENABLE), n_n)
    AIR_PSAP_CODE = $(ROOTDIR)/$(LLF_PATH)/psap/
	ifneq ($(wildcard $(AIR_PSAP_CODE)),)
	  C_SRC += $(LLF_PATH)/psap/src/llf_psap.c
	  INC += $(LLF_PATH)/psap/inc
	  ifeq ($(AIR_HEARING_AID_ENABLE),y)
        AIR_HA_CODE = $(strip $(ROOTDIR))/middleware/airoha/dspalg/hearthrough_hearing_aid/
	AIR_HA_LIB = $(strip $(ROOTDIR))/prebuilt/middleware/airoha/dspalg/hearing_aid/$(IC_CONFIG)/
            ifneq ($(wildcard $(AIR_HA_CODE)),)
              include $(ROOTDIR)/middleware/airoha/dspalg/hearthrough_hearing_aid/module.mk
            else ifneq ($(wildcard $(AIR_HA_LIB)),)
		include $(ROOTDIR)/prebuilt/middleware/airoha/dspalg/hearing_aid/$(IC_CONFIG)/module.mk
	    endif
	  endif
	  ifeq ($(AIR_HEARTHROUGH_PSAP_ENABLE),y)
		  AIR_HEARTHROUGH_PSAP_CODE = $(strip $(ROOTDIR))/middleware/airoha/dspalg/hearthrough_psap/$(IC_CONFIG)/
		  AIR_HEARTHROUGH_PSAP_LIB = $(strip $(ROOTDIR))/prebuilt/middleware/airoha/dspalg/hearthrough_psap/$(IC_CONFIG)/
		  ifneq ($(wildcard $(AIR_HEARTHROUGH_PSAP_CODE)),)
			  include $(ROOTDIR)/middleware/airoha/dspalg/hearthrough_psap/$(IC_CONFIG)/module.mk
		  else ifneq ($(wildcard $(AIR_HEARTHROUGH_PSAP_LIB)),)
			  include $(ROOTDIR)/prebuilt/middleware/airoha/dspalg/hearthrough_psap/$(IC_CONFIG)/module.mk
		  endif
	  endif
	endif
endif

ifeq ($(AIR_HEARTHROUGH_VIVID_PT_ENABLE),y)
AIR_VIVID_CODE = $(ROOTDIR)/$(LLF_PATH)/vivid_pt/
ifneq ($(wildcard $(AIR_VIVID_CODE)),)
  C_SRC += $(LLF_PATH)/vivid_pt/src/llf_vivid_pt.c
  INC += $(LLF_PATH)/vivid_pt/inc
  AIR_VIVID_ALG_CODE = $(strip $(ROOTDIR))/middleware/airoha/dspalg/hearthrough_vivid_passthru/
  AIR_VIVID_ALG_LIB = $(strip $(ROOTDIR))/prebuilt/middleware/airoha/dspalg/vivid_pt/$(IC_CONFIG)/
  ifneq ($(wildcard $(AIR_VIVID_ALG_CODE)),)
	include $(ROOTDIR)/middleware/airoha/dspalg/hearthrough_vivid_passthru/module.mk
  else ifneq ($(wildcard $(AIR_VIVID_ALG_LIB)),)
	include $(ROOTDIR)/prebuilt/middleware/airoha/dspalg/vivid_pt/$(IC_CONFIG)/module.mk
  endif
endif
endif

ifeq ($(AIR_CUSTOMIZED_LLF_ENABLE), y)
include $(ROOTDIR)/middleware/airoha/dspalg/low_latency_framework_example/module.mk
endif
###################################################
# include path
