# Must define the LOCAL_PATH and return the current dir
LOCAL_PATH := $(call my-dir)

## For OpenCL: 

include $(CLEAR_VARS)
LOCAL_MODULE := glesmali
LOCAL_SRC_FILES := libGLES_mali.so
include $(PREBUILT_SHARED_LIBRARY)

## For cl_audio (JNI module)

include $(CLEAR_VARS)

LOCAL_MODULE := cl_audio
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_SRC_FILES := 	cl_manager.c \
					cl_audio.c \
					opensl_io.c \
					java_interface_wrap.cpp
					
LOCAL_SHARED_LIBRARIES += glesmali
LOCAL_LDLIBS += -llog -ldl -lOpenSLES

include $(BUILD_SHARED_LIBRARY)
