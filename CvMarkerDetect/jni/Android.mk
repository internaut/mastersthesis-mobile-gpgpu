# Must define the LOCAL_PATH and return the current dir
LOCAL_PATH := $(call my-dir)

# OpenCV
include $(CLEAR_VARS)

OPENCV_CAMERA_MODULES:=on
OPENCV_INSTALL_MODULES:=on
OPENCV_LIB_TYPE:=SHARED

include /Users/markus/Development/_libs/OpenCV-2.4.7-android-sdk/sdk/native/jni/OpenCV.mk

# JNI Frame Proc

LOCAL_MODULE := cv_accar
LOCAL_CFLAGS    := -DDEBUG -DPRINT_FPS -DSECURE_GRAPHICS_BUF_READ -DBENCHMARK
LOCAL_C_INCLUDES += /opt/local/include		# for glm!
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_SRC_FILES := cv_accar/android/java_interface_wrap.cpp \
				   cv_accar/android/android_jni.cpp \
				   cv_accar/common/conf.cpp \
				   cv_accar/common/core.cpp \
				   cv_accar/common/shader.cpp \
				   cv_accar/common/cam.cpp \
				   cv_accar/common/view.cpp \
				   cv_accar/common/marker.cpp \
				   cv_accar/common/tools.cpp \
				   cv_accar/common/detect/detect.cpp \
				   cv_accar/common/detect/detect_base.cpp \
				   cv_accar/common/detect/detect_accel.cpp \
				   cv_accar/common/pipeline/pipelineproc.cpp \
				   cv_accar/common/pipeline/pipelineproc_fbo.cpp \
				   cv_accar/common/pipeline/markerdisp.cpp \
				   cv_accar/common/pipeline/disp.cpp \
				   cv_accar/common/pipeline/proc/thresh.cpp \
				   cv_accar/common/pipeline/proc/preproc.cpp \
				   cv_accar/common/pipeline/proc/markerwarp.cpp \
				   cv_accar/common/gl/fbo.cpp \
				   cv_accar/common/gl/fbo_mgr.cpp
LOCAL_LDLIBS += -llog -ldl -lGLESv2

# disabled:
#				   cv_accar/common/pipeline/proc/hist.cpp \
#				   cv_accar/common/pipeline/proc/pclines.cpp \

include $(BUILD_SHARED_LIBRARY)
