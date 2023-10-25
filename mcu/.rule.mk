OS_VERSION := $(shell uname)
Q := @

ifneq ($(filter MINGW% MSYS%,$(OS_VERSION)),)
  $(DRV_CHIP_PATH)_EXTRA    := -j1
  $(MID_MBEDTLS_PATH)_EXTRA := -j1
  $(MID_NBIOT_MD_CUSTOM_PATH)_EXTRA := -j1
  IFILEGEN := ifilegen2
else
  IFILEGEN := linux/ifilegen
  # for postbuild
  ifneq ($(wildcard $(strip $(SOURCE_DIR)/tools/gcc4.8.4/linux/gcc-arm-none-eabi/lib64 )),)
        export LD_LIBRARY_PATH := $(SOURCE_DIR)/tools/gcc4.8.4/linux/gcc-arm-none-eabi/lib64:$(LD_LIBRARY_PATH)
  endif

  ifneq ($(wildcard $(strip $(SOURCE_DIR)/tools/gcc9.2.1/linux/gcc-arm-none-eabi/lib64 )),)
        export LD_LIBRARY_PATH := $(SOURCE_DIR)/tools/gcc9.2.1/linux/gcc-arm-none-eabi/lib64:$(LD_LIBRARY_PATH)
  else ifneq ($(wildcard $(strip /mtkeda/airoha/toolchain/gcc9.2.1/linux/gcc-arm-none-eabi/lib64 )),)
        export LD_LIBRARY_PATH := /mtkeda/airoha/toolchain/gcc9.2.1/linux/gcc-arm-none-eabi/lib64:$(LD_LIBRARY_PATH)
  endif
endif

ifeq ($(MAKELEVEL),0)
M :=
else
M := -
endif

Red=\E[1;31m
Green=\E[1;32m
Yellow=\E[1;33m
Blue=\E[1;34m
Pink=\E[1;35m
Color_Off=\E[0m

include $(SOURCE_DIR)/middleware/airoha/verno/$(IC_CONFIG)/module.mk

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
        endif # ifeq ($(AIR_FORCE_WARNING_AS_ERROR),), if it's empty need treat build warning as error
    endif # ifeq ($(AIR_FORCE_WARNING_AS_ERROR),y)
endif # build env checking

.PHONY: merge_lib copy_lib cleanlog genvcxproj gen_copy_firmware_opts

cleanlog:
ifeq ($(TARGET_PATH),)
	rm -f $(OUTPATH)/*.log
	@make gen_copy_firmware_opts
else
	@echo "trigger by build.sh, skip cleanlog"
	@make gen_copy_firmware_opts
endif

gen_copy_firmware_opts:
ifeq ($(wildcard $(strip $(OUTPATH))/copy_firmware_opts.log),)
    ifeq ($(filter clean prebuilt%,$(MAKECMDGOALS)),)
        MAKE_VARS := $(shell echo '$(.VARIABLES)' | awk -v RS=' ' '/^[a-zA-Z0-9]+[a-zA-Z0-9_]*[\r\n]*$$/' | sort | uniq)
        CP_FIRMWARE_OPTS_TMP := $(shell cat $(strip $(SOURCE_DIR))/tools/scripts/build/copy_firmware_opts.lis | tr -d ' ' | sort | uniq)
        CP_FIRMWARE_OPTS := $(filter $(CP_FIRMWARE_OPTS_TMP), $(MAKE_VARS))
        $(shell (test -d $(strip $(OUTPATH)) || mkdir -p $(strip $(OUTPATH))))
        $(shell (test -e $(strip $(OUTPATH))/copy_firmware_opts.log && rm -f $(strip $(OUTPATH))/copy_firmware_opts.log))
        $(foreach v,$(CP_FIRMWARE_OPTS), $(shell echo $(v)='$($(v))' >> $(strip $(OUTPATH))/copy_firmware_opts.log))
    endif
endif


$(TARGET_LIB).a: $(C_OBJS) $(CXX_OBJS) $(S_OBJS)
	@echo Gen $(TARGET_LIB).a
	@echo Gen $(TARGET_LIB).a >>$(BUILD_LOG)
	$(Q)if [ -e "$(OUTPATH)/$@" ]; then rm -f "$(OUTPATH)/$@"; fi
	$(Q)if [ -e "$(OUTPATH)/lib/$@" ]; then rm -f "$(OUTPATH)/lib/$@"; fi
	$(Q)$(M)$(AR) -r $(OUTPATH)/$@ $(C_OBJS) $(CXX_OBJS) $(S_OBJS) >>$(BUILD_LOG) 2>>$(ERR_LOG); \
	if [ "$$?" != "0" ]; then \
		echo -e "MODULE BUILD ${Red}$@ FAIL${Color_Off}"; \
		echo "MODULE BUILD $@ FAIL" >> $(BUILD_LOG); \
		exit 1;\
	else \
		echo -e "MODULE BUILD ${Green}$@ PASS${Color_Off}"; \
		echo "MODULE BUILD $@ PASS" >> $(BUILD_LOG); \
	fi;

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	@if [ -e "$@" ]; then rm -f "$@"; fi
	@if [ -n "$(OVERRIDE_CFLAGS)" ]; then \
		echo $(sort $(OVERRIDE_CFLAGS)) > $(basename $@)_OVERRIDE_CFLAGS ; \
		echo $(CC) @$(basename $@)_OVERRIDE_CFLAGS $(WARNING_AS_ERROR_OPTION) -c $< -o $@ >> $(BUILD_LOG); \
		$(CC) @$(basename $@)_OVERRIDE_CFLAGS $(WARNING_AS_ERROR_OPTION) -c $< -o $@ 1>>$(BUILD_LOG) 2>>$(ERR_LOG); \
	else \
                echo $(sort $(CFLAGS)) > $(basename $@)_CFLAGS; \
                echo $(CC) @$(basename $@)_CFLAGS $(WARNING_AS_ERROR_OPTION) -MMD -MF $(basename $@).d -c $< -o $@ >> $(BUILD_LOG); \
                $(CC) @$(basename $@)_CFLAGS $(WARNING_AS_ERROR_OPTION) -MMD -MF $(basename $@).d -c $< -o $@ 1>>$(BUILD_LOG) 2>>$(ERR_LOG); \
	fi; \
	if [ "$$?" != "0" ]; then \
		echo -e "Build... ${Red}$$(basename $@) FAIL${Color_Off}"; \
		echo "Build... $@ FAIL" >> $(BUILD_LOG); \
	else \
                sed -i 's/\([A-Z]\):\//\/\L\1\//g' $(basename $@).d ;\
		echo -e "Build... ${Green}$$(basename $@) PASS${Color_Off}"; \
		echo "Build... $@ PASS" >> $(BUILD_LOG); \
	fi;

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo Build... $$(basename $@)
	@echo Build... $@ >> $(BUILD_LOG)
	@if [ -e "$@" ]; then rm -f "$@"; fi
	@if [ -n "$(OVERRIDE_CFLAGS)" ]; then \
		echo $(CXX) $(OVERRIDE_CFLAGS) $(WARNING_AS_ERROR_OPTION) $@ >> $(BUILD_LOG); \
		$(CXX) $(OVERRIDE_CFLAGS) $(WARNING_AS_ERROR_OPTION) -c $< -o $@ 2>>$(ERR_LOG); \
	else \
		echo $(CXX) $(CXXFLAGS) $(WARNING_AS_ERROR_OPTION) $@ >> $(BUILD_LOG); \
		$(CXX) $(CXXFLAGS) $(WARNING_AS_ERROR_OPTION) -c $< -o $@ 2>>$(ERR_LOG); \
	fi; \
	if [ "$$?" != "0" ]; then \
		echo "Build... $$(basename $@) FAIL"; \
		echo "Build... $@ FAIL" >> $(BUILD_LOG); \
	else \
		echo "Build... $$(basename $@) PASS"; \
		echo "Build... $@ PASS" >> $(BUILD_LOG); \
	fi;

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.s
	@mkdir -p $(dir $@)
	@echo Build... $@ >> $(BUILD_LOG)
	@if [ -e "$@" ]; then rm -f "$@"; fi
	@if [ -n "$(OVERRIDE_CFLAGS)" ]; then \
		echo $(sort $(OVERRIDE_CFLAGS)) > $(basename $@)_OVERRIDE_CFLAGS ; \
		echo $(CC) @$(basename $@)_OVERRIDE_CFLAGS -c  -x assembler-with-cpp $< -o $@ >> $(BUILD_LOG); \
		$(CC) @$(basename $@)_OVERRIDE_CFLAGS -c  -x assembler-with-cpp $< -o $@ 1>>$(BUILD_LOG) 2>>$(ERR_LOG); \
	else \
                echo $(sort $(CFLAGS)) > $(basename $@)_CFLAGS; \
		echo $(CC) @$(basename $@)_CFLAGS -c -x assembler-with-cpp $< -o $@ >> $(BUILD_LOG);  \
		$(CC) @$(basename $@)_CFLAGS -c -x assembler-with-cpp $< -o $@ 1>>$(BUILD_LOG) 2>>$(ERR_LOG);  \
	fi; \
	if [ "$$?" != "0" ]; then \
		echo -e "Build... ${Yellow}$$(basename $@) FAIL${Color_Off}"; \
		echo "Build... $@ FAIL" >> $(BUILD_LOG); \
	else \
		echo -e "Build... ${Yellow}$$(basename $@) PASS${Color_Off}"; \
		echo "Build... $@ PASS" >> $(BUILD_LOG); \
	fi;

$(BUILD_DIR)/%.d: $(SOURCE_DIR)/%.s
	@echo $@
	@mkdir -p $(dir $@)
	@set -e; rm -f $@; \
	export D_FILE="$@"; \
	export B_NAME=`echo $$D_FILE | sed 's/\.d//g'`; \
	if [ -n "$(OVERRIDE_CFLAGS)" ]; then \
		$(CC) -MM $(OVERRIDE_CFLAGS) -x assembler-with-cpp $< > $@.$$$$; \
	else \
		$(CC) -MM $(CFLAGS) -x assembler-with-cpp $< > $@.$$$$; \
	fi; \
	sed 's@\(.*\)\.o@'"$$B_NAME\.o $$B_NAME\.d"'@g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

genvcxproj:
	@echo "Gen VC Project..."
	@rm -rf $(OUTPATH)/vc_proj
	@mkdir -p $(OUTPATH)/vc_proj
	@echo $(sort $(C_FILES)) > $(OUTPATH)/vc_proj/c_files.txt
	@echo $(filter -D%, $(VC_CFLAGS)) > $(OUTPATH)/vc_proj/c_def.txt
	@echo $(filter -D%, $(SIGDB_CFLAGS)) > $(OUTPATH)/vc_proj/c_def_db.txt
	@echo $(addprefix -I, $(subst $(realpath $(SOURCE_DIR))/,,$(realpath $(subst -I,,$(filter -I%, $(CFLAGS)))))) > $(OUTPATH)/vc_proj/c_inc.txt
	@echo $(sort $(CXX_FILES)) > $(OUTPATH)/vc_proj/cpp_files.txt
	@echo $(filter -D%, $(CPPFLAGS)) > $(OUTPATH)/vc_proj/cpp_def.txt
	@echo $(addprefix -I, $(subst $(realpath $(SOURCE_DIR))/,,$(realpath $(subst -I,,$(filter -I%, $(CPPFLAGS)))))) > $(OUTPATH)/vc_proj/cpp_inc.txt
	@echo C_FILES = '"c_files.txt"' > $(OUTPATH)/vc_proj/vc_proj_gen.cfg
	@echo C_DEFINES = '"c_def.txt"' >> $(OUTPATH)/vc_proj/vc_proj_gen.cfg
	@echo C_INCLUDES = '"c_inc.txt"' >> $(OUTPATH)/vc_proj/vc_proj_gen.cfg
	@echo CPP_FILES = '"cpp_files.txt"' >> $(OUTPATH)/vc_proj/vc_proj_gen.cfg
	@echo CPP_DEFINES = '"cpp_def.txt"' >> $(OUTPATH)/vc_proj/vc_proj_gen.cfg
	@echo CPP_INCLUDES = '"cpp_inc.txt"' >> $(OUTPATH)/vc_proj/vc_proj_gen.cfg
	@echo SIGDB_DEFINES = '"c_def_db.txt"' >> $(OUTPATH)/vc_proj/vc_proj_gen.cfg
	@echo PROJ_NAME = '"$(PROJ_NAME)"' >> $(OUTPATH)/vc_proj/vc_proj_gen.cfg
	@echo EXTRA_DEFINES = '"$(VC_EXTRA_DEFINES)"' >> $(OUTPATH)/vc_proj/vc_proj_gen.cfg
	@echo AVAILABLE_INCLUDE_PATH_PREFIX = '"$(VC_AVAILABLE_INCLUDE_PATH_PREFIX)"' >> $(OUTPATH)/vc_proj/vc_proj_gen.cfg
	@echo ROOT = '"$(VC_ROOT)"' >> $(OUTPATH)/vc_proj/vc_proj_gen.cfg
	@echo PATH_REPLACE_LIST = '"$(VC_PATH_REPLACE_LIST)"' >> $(OUTPATH)/vc_proj/vc_proj_gen.cfg
	@echo UT_PATH_REPLACE_LIST = '"$(UT_PATH_REPLACE_LIST)"' >> $(OUTPATH)/vc_proj/vc_proj_gen.cfg
	@echo ADDITIONAL_LIB_DIR = '"$(VC_ADDITIONAL_LIB_DIR)"' >> $(OUTPATH)/vc_proj/vc_proj_gen.cfg
	@echo ADDITIONAL_DEP = '"$(VC_ADDITIONAL_DEP)"' >> $(OUTPATH)/vc_proj/vc_proj_gen.cfg
	# vc_proj_generator.py is now run as part og the Borg unit test profiles

ifeq ($(filter clean prebuilt%,$(MAKECMDGOALS)),)
-include $(S_OBJS:.o=.d)
NEW_BUILD_DIR := $(patsubst %/,%, $(BUILD_DIR))
WRAP_BUILD_DIR := $(dir $(NEW_BUILD_DIR))./$(notdir $(NEW_BUILD_DIR))
-include $(subst $(BUILD_DIR)/,$(WRAP_BUILD_DIR)/,$(C_OBJS:.o=.d))
-include $(CXX_OBJS:.o=.d)
endif

