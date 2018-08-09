#MAKE FILE FOR CLTraceAgent

DEPTH = ../..
include $(DEPTH)/Build/Common.mk

TARGETSO = $(BACKEND_OUTPUT_DIR)/lib$(GPU_PROFILER_LIB_PREFIX)CLTraceAgent$(TARGET_SUFFIX).so

#++CF:LPGPU2 Add -Wno-deprecated-declarations
ADDL_DEFINES = -DNON_BLOCKING_TIMEOUT -Wno-deprecated-declarations
#--CF:LPGPU2

INCLUDES = \
	-I. \
	-I$(BACKEND_CLPROFILEAGENT_DIR) \
	-I$(BACKEND_COMMON_DIR) \
	-I$(BACKEND_CLCOMMON_DIR) \
	-I$(COMMON_PROJ_DIR) \
	-isystem$(ATI_STREAM_SDK_DIR)/include \
	-isystem$(CAL_DIR)/include \
	-I$(DYNAMICLIBRARYMODULE_DIR) \
	-I$(DEVICEINFO_DIR) \
	-I$(TSINGLETON_DIR) \
	-I$(UTF8CPP_DIR) \
	-I$(AMDTMUTEX_DIR)

ifeq ($(findstring aarch64-linux-gnu-g++,$(CC)), aarch64-linux-gnu-g++)
INCLUDES += \
	-I/usr/aarch64-linux-gnu-g++/include
endif

ifeq ($(findstring arm-linux-gnueabihf-g++,$(CC)), arm-linux-gnueabihf-g++)
INCLUDES += \
	-I/usr/arm-linux-gnueabihf-g++/include
endif

LIBS = \
	$(CLCOMMON_LIBS) \
	$(COMMON_LIBS) \
	$(FRAMEWORK_LIBS) \
	$(STANDARD_LIBS) \
	-lOpenCL

LIBPATH = $(COMMON_LIB_PATHS)\
		  -L$(COMMON_LIB_EXT)/OpenCLICDLoaderDetours/$(HSA_PLATFORM_DIR) # TODO fix this for 32 bit builds

SO_OBJS =  \
	./$(OBJ_DIR)/CLAPIDefBase.o \
	./$(OBJ_DIR)/CLAPIDefs.o \
	./$(OBJ_DIR)/AtpFile.o \
	./$(OBJ_DIR)/CLAtpFile.o \
	./$(OBJ_DIR)/CLAPIInfoManager.o \
	./$(OBJ_DIR)/PMCSamplerManager.o \
	./$(OBJ_DIR)/CLAPITraceEntry.o \
	./$(OBJ_DIR)/CLEventHandler.o \
	./$(OBJ_DIR)/CLEventManager.o \
	./$(OBJ_DIR)/CLStringUtils.o \
	./$(OBJ_DIR)/CLTraceAgent.o \
	./$(OBJ_DIR)/CLEnqueueAPIDefs.o

include $(DEPTH)/Build/CommonTargets.mk

# END OF MAKE FILE
