DMM_ROOT = kernel/service/DSP_Memory_Managementv1

C_SRC  += $(DMM_ROOT)/src/Component.c
C_SRC  += $(DMM_ROOT)/src/ComponentInternal.c

# $(OUTDIR) cannot be written directly here due to path substitution !!!
# C_SRC  += $(OUTDIR)/dmm_gen/ComponentPreDefine.c


#################################################################################
#include path
INC  += kernel/service/DSP_Memory_Managementv1/inc
INC  += kernel/service/DSP_Memory_Managementv1/dmm_gen
CCFLAG += -I$(OUTDIR)/dmm_gen/

OBJ_ADD_ON += $(OUTDIR)/dmm_gen/ComponentPreDefine.o
# The generated ComponentPreDefine.o file need to be added to the
# dependencies of $(IMAGE), otherwise the build result will PASS,
# but no elf/bin files will be generated !!!
$(IMAGE): $(OUTDIR)/dmm_gen/ComponentPreDefine.o


# Due to path substitution, rule.mk will not find the rules for
# compiling ComponentPreDefine.o, so it needs to be added manually !!!
$(OUTDIR)/dmm_gen/ComponentPreDefine.o: $(OUTDIR)/dmm_gen/ComponentPreDefine.c
	@echo $(CC) $(CCFLAG) $(INC:%=-I"$(ROOTDIR)/%") $(DEFINE:%=-D%) -MD -MF $(subst .o,.d,$@) -c -o $@ $< >> $(BUILD_LOG)
	@$(CC) $(CCFLAG) $(INC:%=-I"$(ROOTDIR)/%") $(DEFINE:%=-D%) -MD -MF $(subst .o,.d,$@) -c -o $@ $< 2>>$(ERR_LOG)
	@if [ "$$?" != "0" ]; then \
		echo "Build... $$(basename $@) FAIL"; \
		echo "Build... $@ FAIL" >> $(BUILD_LOG); \
	else \
		sed -i 's/\([A-Z]\):\//\/\L\1\//g' $(basename $@).d ;\
		echo "Build... $$(basename $@) PASS"; \
		echo "Build... $@ PASS" >> $(BUILD_LOG); \
	fi;