#MAKE FILE FOR CLCommon

DEPTH = ../..
include $(DEPTH)/Build/Common.mk

TARGETLIB = lib$(GPU_PROFILER_LIB_PREFIX)CLCommon$(TARGET_SUFFIX).a

INCLUDES = \
	-I. \
	-I../Common \
	-I$(COMMON_PROJ_DIR) \
	-I$(ATI_STREAM_SDK_DIR)/include \
	-I$(CAL_DIR)/include \
	-I$(DYNAMICLIBRARYMODULE_DIR) \
	-I$(DEVICEINFO_DIR) \
	-I$(TSINGLETON_DIR) \
	-I$(AMDTMUTEX_DIR)

ifeq ($(findstring aarch64-linux-gnu-g++,$(CC)), aarch64-linux-gnu-g++)
INCLUDES += \
	-I/usr/aarch64-linux-gnu-g++/include
endif

ifeq ($(findstring arm-linux-gnueabihf-g++,$(CC)), arm-linux-gnueabihf-g++)
INCLUDES += \
	-I/usr/arm-linux-gnueabihf-g++/include
endif

LIB_OBJS =  \
	./$(OBJ_DIR)/CLInternalFunctionDefs.o \
	./$(OBJ_DIR)/CLFunctionDefs.o \
	./$(OBJ_DIR)/CLFunctionEnumDefs.o \
	./$(OBJ_DIR)/CLUtils.o \
	./$(OBJ_DIR)/CLCUInfoBase.o \
	./$(OBJ_DIR)/CLPlatformInfo.o \
	./$(OBJ_DIR)/OpenCLModule.o

include $(DEPTH)/Build/CommonTargets.mk

# END OF MAKE FILE
