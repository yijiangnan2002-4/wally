
.PHONY: FORCE all

MTK_FW_VERSION ?= undefined
ifeq ($(MTK_FW_VERSION),)
MTK_FW_VERSION := undefined
endif

BOARD_CONFIG ?= undefined


CFLAGS += -I$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/verno/$(IC_CONFIG)/inc

# Pretend the symbol is undefined, to force linking it
LDFLAGS += -u build_date_time_str -u sw_verno_str -u hw_verno_str

# let "all" on top to be the default target.
all:

# generate verno information object and add verno.o to .elf prerequisite.
$(OUTPATH)/$(PROJ_NAME).elf: $(BUILD_DIR)/$(MIDDLEWARE_PROPRIETARY)/verno/$(IC_CONFIG)/verno.o

$(BUILD_DIR)/$(MIDDLEWARE_PROPRIETARY)/verno/$(IC_CONFIG)/verno.o: $(BUILD_DIR)/$(MIDDLEWARE_PROPRIETARY)/verno/$(IC_CONFIG)/verno.c
	@mkdir -p $(dir $@)
	@echo Build... $$(basename $@)
	@echo Build... $@ >> $(BUILD_LOG)
	@echo $(sort $(CFLAGS)) > $(basename $@)_CFLAGS
	@if [ -e "$@" ]; then rm -f "$@"; fi
	@echo $(CC) @$(basename $@)_CFLAGS $@ >> $(BUILD_LOG)
	@-$(CC) @$(basename $@)_CFLAGS -c $< -o $@ 2>>$(ERR_LOG); \
	if [ "$$?" != "0" ]; then \
		echo -e "Build... ${Red}$$(basename $@) FAIL${Color_Off}"; \
		echo "Build... $@ FAIL" >> $(BUILD_LOG); \
	else \
		echo -e "Build... ${Green}$$(basename $@) PASS${Color_Off}"; \
		echo "Build... $@ PASS" >> $(BUILD_LOG); \
	fi;

# generate verno.c based on template.
$(BUILD_DIR)/$(MIDDLEWARE_PROPRIETARY)/verno/$(IC_CONFIG)/verno.c: FORCE
	@mkdir -p $(dir $@); \
	$(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/verno/$(IC_CONFIG)/gen_verno.sh $(SOURCE_DIR)/$(MIDDLEWARE_PROPRIETARY)/verno/$(IC_CONFIG)/verno.template $@ $(MTK_FW_VERSION) $(BOARD_CONFIG) $(SOURCE_DIR)

FORCE:

