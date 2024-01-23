################################################################################
# Compiler Toolchain Settings
################################################################################
#Xtensa tool chain path & license setting,
#These settings can be configured either in a project's Makefile for the specific
#project or setting here for all the projets.
-include ~/airoha_sdk_toolchain/airoha_sdk_env_157x
-include ~/airoha_sdk_toolchain/airoha_sdk_env_158x

export IOT_SDK_XTENSA_VERSION := $(IOT_SDK_XTENSA_VERSION)

ifeq ($(shell uname), Linux)

    ifeq ($(IOT_SDK_XTENSA_VERSION), 9018)
        XTENSA_ROOT ?= /mtkeda/xtensa/Xplorer-9.0.18
        XTENSA_VER ?= RI-2021.8-linux
    else ifeq ($(IOT_SDK_XTENSA_VERSION), 8013)
        XTENSA_ROOT ?= /mtkeda/xtensa/Xplorer-8.0.13
        XTENSA_VER ?= RI-2020.4-linux
    else ifeq ($(IOT_SDK_XTENSA_VERSION), 809)
        XTENSA_ROOT ?= /mtkeda/xtensa/Xplorer-8.0.9
        XTENSA_VER ?= RG-2019.12-linux
    else
        XTENSA_ROOT ?= /mtkeda/xtensa/Xplorer-7.0.7
        XTENSA_VER ?= RG-2017.7-linux
    endif

    # XTENSA_LICENSE_FILE Server
    ifeq ($(shell domainname), mcdswrd)
      XTENSA_LICENSE_FILE ?= 7400@10.19.50.25
    else
      XTENSA_LICENSE_FILE ?= 7400@mtklc17
    endif

    ifeq ($(SDK_PACKAGE), yes)
        XTENSA_SYSTEM ?= $(XTENSA_ROOT)/xtensa/$(XTENSA_VER)/XtensaTools/config/
        XTENSA_BIN_PATH ?= $(XTENSA_ROOT)/xtensa/$(XTENSA_VER)/XtensaTools/bin
        XTENSA_LIB_PATH ?= $(XTENSA_ROOT)/xtensa/$(XTENSA_VER)/$(XTENSA_CORE)/xtensa-elf
    else
       XTENSA_BIN_PATH ?= $(XTENSA_ROOT)/XtDevTools/install/tools/$(XTENSA_VER)/XtensaTools/bin
       XTENSA_LIB_PATH := $(XTENSA_ROOT)/XtDevTools/install/builds/$(XTENSA_VER)/$(XTENSA_CORE)/xtensa-elf
    endif

else
    XTENSA_ROOT ?= $(HOME)/airoha_sdk_toolchain
    ifeq ($(IOT_SDK_XTENSA_VERSION), 9018)
        XTENSA_VER ?= RI-2021.8-win32
    else ifeq ($(IOT_SDK_XTENSA_VERSION), 8013)
        XTENSA_VER ?= RI-2020.4-win32
    else ifeq ($(IOT_SDK_XTENSA_VERSION), 809)
        XTENSA_VER ?= RG-2019.12-win32
    else
        XTENSA_VER ?= RG-2017.7-win32
    endif
    XTENSA_SYSTEM ?= $(XTENSA_ROOT)/xtensa/$(XTENSA_VER)/XtensaTools/config/
    XTENSA_BIN_PATH ?= $(XTENSA_ROOT)/xtensa/$(XTENSA_VER)/XtensaTools/bin
    XTENSA_LIB_PATH ?= $(XTENSA_ROOT)/xtensa/$(XTENSA_VER)/$(XTENSA_CORE)/xtensa-elf
endif

export PATH := $(XTENSA_BIN_PATH):$(PATH)

$(info XTENSA_ROOT : $(XTENSA_ROOT) )
$(info XTENSA_VER : $(XTENSA_VER) )
$(info XTENSA_BIN_PATH : $(XTENSA_BIN_PATH) )

ifeq ($(shell uname), Linux)
    ifeq ($(wildcard $(XTENSA_BIN_PATH)),)
        $(error Folder $(XTENSA_BIN_PATH) not exist! )
    else
       ifeq ($(IOT_SDK_XTENSA_VERSION), 9018)
        # check xtensa toolchain
        xtensa_status:=$(shell export PATH=$(XTENSA_BIN_PATH):$(PATH); which xt-clang)
       else
        # check xtensa toolchain
        xtensa_status:=$(shell export PATH=$(XTENSA_BIN_PATH):$(PATH); which xt-xcc)
       endif
        ifeq ($(xtensa_status),)
            $(error Error : Missing Xtensa toolchain. The Xtensa toolchain may not be installed correctly or been removed. Please run install.sh to resovle the problem.)
        endif
    endif
endif

export LM_LICENSE_FILE := $(XTENSA_LICENSE_FILE)
export XTENSA_SYSTEM ?= $(XTENSA_ROOT)/XtDevTools/XtensaRegistry/$(XTENSA_VER)
export XTENSA_CORE := $(XTENSA_CORE)
LM_LICENSE_FILE := $(strip $(LM_LICENSE_FILE))
XTENSA_CORE     := $(strip $(XTENSA_CORE))
XTENSA_SYSTEM   := $(strip $(XTENSA_SYSTEM))

Red=\E[1;31m
Green=\E[1;32m
Yellow=\E[1;33m
Blue=\E[1;34m
Pink=\E[1;35m
Color_Off=\E[0m

.PHONY: clean_log

include $(ROOTDIR)/middleware/airoha/verno/module.mk

# Warning to error compile option
WARNING_AS_ERROR_OPTION = -Wno-error

ifneq ($(filter MINGW% MSYS%,$(OS_VERSION)),)
    ifeq ($(AIR_FORCE_WARNING_AS_ERROR),y)
        WARNING_AS_ERROR_OPTION =
    endif
else ifeq ($(OS_VERSION), Darwin)
    ifeq ($(AIR_FORCE_WARNING_AS_ERROR),y)
        WARNING_AS_ERROR_OPTION =
    endif
else
    ACCOUNT_INFO = $(shell whoami)
    ACCOUNT_INFO_PREFIX = $(shell echo $(ACCOUNT_INFO) | head -c 3)
    ifeq ($(AIR_FORCE_WARNING_AS_ERROR),y)
        WARNING_AS_ERROR_OPTION =
    else
        ifeq ($(AIR_FORCE_WARNING_AS_ERROR),)
            ifeq ($(ACCOUNT_INFO_PREFIX),mtk)
                WARNING_AS_ERROR_OPTION =
            endif # ifeq ($(AIR_FORCE_WARNING_AS_ERROR),)
            ifeq ($(ACCOUNT_INFO),srv_airoha_epm)
                WARNING_AS_ERROR_OPTION =
            endif # ifeq ($(ACCOUNT_INFO),srv_airoha_epm)
            ifeq ($(ACCOUNT_INFO_PREFIX),ar7)
                WARNING_AS_ERROR_OPTION =
            endif
        endif # ifeq ($(AIR_FORCE_WARNING_AS_ERROR),), if it's empty need treat build warning as error
    endif # ifeq ($(AIR_FORCE_WARNING_AS_ERROR),y)
endif # build env checking

################################################################################
# Common Rule
################################################################################
clean_log:
ifeq ($(TARGET_PATH),)
	@if [ -e "$(strip $(LOGDIR))" ]; then rm -rf "$(strip $(LOGDIR))"; fi
	@mkdir -p "$(strip $(LOGDIR))"
	@make gen_copy_firmware_opts
else
	@echo "trigger by build.sh, skip clean_log"
	@make gen_copy_firmware_opts
endif

gen_copy_firmware_opts:
ifeq ($(wildcard $(strip $(OUTDIR))/copy_firmware_opts.log),)
    ifeq ($(filter clean prebuilt%,$(MAKECMDGOALS)),)
        MAKE_VARS := $(shell echo '$(.VARIABLES)' | awk -v RS=' ' '/^[a-zA-Z0-9]+[a-zA-Z0-9_]*[\r\n]*$$/' | sort | uniq)
        #MTK_AIR_MAKE_VARS := $(shell echo '$(.VARIABLES)' | awk -v RS=' ' '/^[a-zA-Z0-9]+[a-zA-Z0-9_]*[\r\n]*$$/' | grep  "^MTK_\|AIR_")
        CP_FIRMWARE_OPTS_TMP := $(shell cat $(strip $(ROOTDIR))/tools/scripts/build/copy_firmware_opts.lis | tr -d ' ' | sort | uniq)
        CP_FIRMWARE_OPTS := $(filter $(CP_FIRMWARE_OPTS_TMP), $(MAKE_VARS))
        #$(shell ( rm -f $(strip $(OUTDIR))/dsp_cflags.log && echo $(CCFLAG) |tr " " "\n" |grep "\-D"|tr -d "\-D" > $(strip $(OUTDIR))/dsp_cflags.log) )
        $(shell (test -d $(strip $(OUTDIR)) || mkdir -p $(strip $(OUTDIR))))
        $(shell (test -e $(strip $(OUTDIR))/copy_firmware_opts.log && rm -f $(strip $(OUTDIR))/copy_firmware_opts.log))
        $(foreach v,$(CP_FIRMWARE_OPTS), $(shell echo $(v)='$($(v))' >> $(strip $(OUTDIR))/copy_firmware_opts.log))
        #$(foreach v,$(MTK_AIR_MAKE_VARS), $(shell echo $(v)='$($(v))' >> $(strip $(OUTDIR))/feature_opts_list.log))
    endif
endif

################################################################################
# Pattern Rule
################################################################################
$(OUTDIR)/%.o : $(ROOTDIR)/%.S
	@mkdir -p $(dir $@)
	@echo $(AS) $(ASFLAG) $(INC:%=-I"$(ROOTDIR)/%") -MD -MF $(subst .o,.d,$@) -c -o $@ $< >> $(BUILD_LOG)
	@$(AS) $(ASFLAG) $(INC:%=-I"$(ROOTDIR)/%") -MD -MF $(subst .o,.d,$@) -c -o $@ $< 2>>$(ERR_LOG) ;\
	if [ "$$?" != "0" ]; then \
		echo -e "Build... ${Red}$$(basename $@) FAIL${Color_Off}"; \
		echo "Build... $@ FAIL" >> $(BUILD_LOG); \
	else \
		echo -e "Build... ${Yellow}$$(basename $@) PASS${Color_Off}"; \
		echo "Build... $@ PASS" >> $(BUILD_LOG); \
	fi;

$(OUTDIR)/%.o : $(ROOTDIR)/%.c
	@mkdir -p $(dir $@)
	@echo $(sort $(CCFLAG)) > $(basename $@)_CFLAGS
	@echo $(CC) @$(basename $@)_CFLAGS $(INC:%=-I"$(ROOTDIR)/%") $(DEFINE:%=-D%) -MMD -MF $(subst .o,.d,$@) -c -o $@ $< >> $(BUILD_LOG)
	@if [ -e "$@" ]; then rm -f "$@"; fi ;\
	$(CC) @$(basename $@)_CFLAGS $(WARNING_AS_ERROR_OPTION) $(INC:%=-I"$(ROOTDIR)/%") $(DEFINE:%=-D%) -MMD -MF $(subst .o,.d,$@) -c -o $@ $< 2>>$(ERR_LOG) ;\
	if [ "$$?" != "0" ]; then \
		echo -e "Build... ${Red}$$(basename $@) FAIL${Color_Off}"; \
		echo "Build... $@ FAIL" >> $(BUILD_LOG); \
	else \
    sed -i 's/\([A-Z]\):\//\/\L\1\//g' $(basename $@).d ;\
		echo -e "Build... ${Green}$$(basename $@) PASS${Color_Off}"; \
		echo "Build... $@ PASS" >> $(BUILD_LOG); \
	fi;

$(OUTDIR)/%.o : $(ROOTDIR)/%.cpp
	@echo Compiling... $^
	@mkdir -p $(dir $@)
	@echo $(sort $(CCFLAG)) > $(basename $@)_CFLAGS
	@echo $(CC) $(COM_CPPFLAG) @$(basename $@)_CFLAGS $(INC:%=-I"$(ROOTDIR)/%") $(DEFINE:%=-D%) -MMD -MF $(subst .o,.d,$@) -c -o $@ $< >> $(BUILD_LOG)
	@if [ -e "$@" ]; then rm -f "$@"; fi ;\
	$(CPP) $(COM_CPPFLAG) @$(basename $@)_CFLAGS $(WARNING_AS_ERROR_OPTION) $(INC:%=-I"$(ROOTDIR)/%") $(DEFINE:%=-D%) -MMD -MF $(subst .o,.d,$@) -c -o $@ $< 2>>$(ERR_LOG) ;\
	@if [ "$$?" != "0" ]; then \
		echo "Build... $$(basename $@) FAIL"; \
		echo "Build... $@ FAIL" >> $(BUILD_LOG); \
	else \
    sed -i 's/\([A-Z]\):\//\/\L\1\//g' $(basename $@).d ;\
		echo "Build... $$(basename $@) PASS"; \
		echo "Build... $@ PASS" >> $(BUILD_LOG); \
	fi;

$(TARGET_LIB).a: $(OBJ)
	@echo Gen $(TARGET_LIB).a
	@echo Gen $(TARGET_LIB).a >>$(BUILD_LOG)
	@if [ -e "$@" ]; then rm -f "$@"; fi
	@mkdir -p $(dir $@)
	@$(AR) -r $@ $(OBJ) >>$(BUILD_LOG) 2>>$(ERR_LOG);  \
	if [ "$$?" != "0" ]; then \
		echo -e "MODULE BUILD ${Red}$@ FAIL${Color_Off}" ; \
		echo "MODULE BUILD $@ FAIL" >> $(BUILD_LOG) ; \
		exit 1 ;\
	else \
		echo -e "MODULE BUILD ${Green}$@ PASS${Color_Off}" ; \
		echo "MODULE BUILD $@ PASS" >> $(BUILD_LOG) ; \
	fi;


