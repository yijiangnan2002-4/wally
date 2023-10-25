################################################################################
# Compiler and Commands Configuration
################################################################################
ifeq ($(USE_CCACHE),1)
    CCACHE    := ccache
endif

CROSS       := xt-
ifeq ($(IOT_SDK_XTENSA_VERSION),8013)
#           C Compiler
CC          := $(CCACHE) $(CROSS)xcc -c
#           CPP Compiler
CPP          := $(CCACHE) $(CROSS)xc++ -c
#           Preprocessor
CCP         := $(CCACHE) $(CROSS)clang -E
#           Assembler
AS          :=  $(CROSS)xcc -c
else
#           C Compiler
CC          := $(CCACHE) $(CROSS)clang -c
#           CPP Compiler
CPP          := $(CCACHE) $(CROSS)clang++ -c
#           Preprocessor
CCP         := $(CCACHE) $(CROSS)clang -E
#           Assembler
AS          :=  $(CROSS)clang -c
endif
#           Archiver
AR          :=  $(CROSS)ar
SIZE        :=  $(CROSS)size
#           Linker
LD          :=  $(CROSS)ld
OBJDUMP     :=  $(CROSS)objdump
OBJCOPY     :=  $(CROSS)objcopy
NM          :=  $(CROSS)nm
READELF     :=  readelf
RM          :=  dtidel
CP          :=  xcopy /T /Y /I

