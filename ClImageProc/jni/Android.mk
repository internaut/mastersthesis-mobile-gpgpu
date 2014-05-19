# Must define the LOCAL_PATH and return the current dir
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := glesmali
LOCAL_SRC_FILES := libGLES_mali.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

OPENCV_CAMERA_MODULES:=off
OPENCV_INSTALL_MODULES:=on
OPENCV_LIB_TYPE:=SHARED

include /Users/markus/Development/_libs/OpenCV-2.4.7-android-sdk/sdk/native/jni/OpenCV.mk

LOCAL_MODULE := cl_jni
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_CFLAGS    := -DDEBUG -DBENCHMARK -DHOUGH_DRAW_LINES=2
LOCAL_SRC_FILES :=	cl_jni.cpp \
					cl_base.cpp \
					cl_imgconv.cpp \
					cl_hough.cpp
LOCAL_SHARED_LIBRARIES += glesmali
LOCAL_LDLIBS += -llog -ldl

include $(BUILD_SHARED_LIBRARY)
