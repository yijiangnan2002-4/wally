
.PHONY: FORCE all

MTK_FW_VERSION ?= undefined
ifeq ($(MTK_FW_VERSION),)
MTK_FW_VERSION := undefined
endif

BOARD_CONFIG ?= undefined


CCFLAG += -I$(ROOTDIR)/$(MIDDLEWARE_PROPRIETARY)/verno/inc

# Pretend the symbol is undefined, to force linking it
LDFLAG += -u build_date_time_str -u sw_verno_str -u hw_verno_str

# let "all" on top to be the default target.
all:

# generate verno information object and add verno.o to .elf prerequisite.
$(IMAGE): $(OUTDIR)/$(MIDDLEWARE_PROPRIETARY)/verno/verno.o

$(OUTDIR)/$(MIDDLEWARE_PROPRIETARY)/verno/verno.o: $(OUTDIR)/$(MIDDLEWARE_PROPRIETARY)/verno/verno.c
	@mkdir -p $(dir $@)
	echo Compiling... $^
	@mkdir -p $(dir $@)
	@echo $(sort $(CCFLAG)) > $(basename $@)_CCFLAG
	@echo $(CC) @$(basename $@)_CCFLAG $(INC:%=-I"$(ROOTDIR)/%") $(DEFINE:%=-D%) -MD -MF $(subst .o,.d,$@) -c -o $@ $< >> $(BUILD_LOG)
	@$(CC) @$(basename $@)_CCFLAG $(INC:%=-I"$(ROOTDIR)/%") $(DEFINE:%=-D%) -MD -MF $(subst .o,.d,$@) -c -o $@ $< 2>>$(ERR_LOG) ;\
	if [ "$$?" != "0" ]; then \
		echo -e "Build... ${Red}$$(basename $@) FAIL${Color_Off}"; \
		echo "Build... $@ FAIL" >> $(BUILD_LOG); \
	else \
		echo -e "Build... ${Yellow}$$(basename $@) PASS${Color_Off}"; \
		echo "Build... $@ PASS" >> $(BUILD_LOG); \
	fi;

# generate verno.c based on template.
$(OUTDIR)/$(MIDDLEWARE_PROPRIETARY)/verno/verno.c: FORCE
	@mkdir -p $(dir $@); \
	$(ROOTDIR)/$(MIDDLEWARE_PROPRIETARY)/verno/gen_verno.sh $(ROOTDIR)/$(MIDDLEWARE_PROPRIETARY)/verno/verno.template $@ $(MTK_FW_VERSION) $(BOARD_CONFIG) $(ROOTDIR)

FORCE:

