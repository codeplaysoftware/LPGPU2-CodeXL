#MAKE FILE FOR PreloadXInitThreads

DEPTH = ../..
include $(DEPTH)/Build/Common.mk

# make sure CallXInitThreads gets called during initialization
ADDL_LINKFLAGS = -Wl,-init,CallXInitThreads
TARGETSO = $(BACKEND_OUTPUT_DIR)/lib$(GPU_PROFILER_LIB_PREFIX)PreloadXInitThreads$(TARGET_SUFFIX).so

INCLUDES = \
	-I. \

LIBS = -lX11

ifeq ($(findstring aarch64-linux-gnu-g++,$(CC)), aarch64-linux-gnu-g++)
LIBPATH = $(COMMON_LIB_PATHS) \
	-L$(COMMON_DIR)/Lib/Ext/X11/aarch64
endif

ifeq ($(findstring arm-linux-gnueabihf-g++,$(CC)), arm-linux-gnueabihf-g++)
LIBPATH = $(COMMON_LIB_PATHS) \
	-L$(COMMON_DIR)/Lib/Ext/X11/armv7a
endif

SO_OBJS =  \
	./$(OBJ_DIR)/XInitThreadsPreload.o \

include $(DEPTH)/Build/CommonTargets.mk

# END OF MAKE FILE
