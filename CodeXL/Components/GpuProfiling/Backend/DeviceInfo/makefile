#MAKE FILE FOR DeviceInfo

DEPTH = ../..
include $(DEPTH)/Build/Common.mk

TARGETLIB = lib$(GPU_PROFILER_LIB_PREFIX)DeviceInfo$(TARGET_SUFFIX).a

INCLUDES = \
	-I. \
	-I../Common \
	-I$(COMMON_PROJ_DIR) \
	-I$(DEVICEINFO_DIR) \
	-I$(TSINGLETON_DIR)

ifeq ($(findstring aarch64-linux-gnu-g++,$(CC)), aarch64-linux-gnu-g++)
INCLUDES += \
	-I/usr/aarch64-linux-gnu-g++/include
endif

ifeq ($(findstring arm-linux-gnueabihf-g++,$(CC)), arm-linux-gnueabihf-g++)
INCLUDES += \
	-I/usr/arm-linux-gnueabihf-g++/include
endif

# for AMD-internal builds
ifneq ("$(wildcard $(DEVICEINFO_DIR)/DeviceInfoInternal.cpp)","")
	INTERNAL_OBJ := ./$(OBJ_DIR)/DeviceInfoInternal.o
endif

LIB_OBJS = \
	./$(OBJ_DIR)/DeviceInfoUtils.o \
	$(INTERNAL_OBJ) \

include $(DEPTH)/Build/CommonTargets.mk

# END OF MAKE FILE
