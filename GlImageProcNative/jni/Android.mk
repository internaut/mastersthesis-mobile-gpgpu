LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

#opencv (only used for getting image data from java to c++)

OPENCV_CAMERA_MODULES:=off
OPENCV_INSTALL_MODULES:=on
OPENCV_LIB_TYPE:=SHARED

include /Users/markus/Development/_libs/OpenCV-2.4.7-android-sdk/sdk/native/jni/OpenCV.mk

#improc

LOCAL_MODULE := imgproc
LOCAL_CFLAGS    := -DDEBUG -DBENCHMARK
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_SRC_FILES :=  gl_gpgpu/disp_renderer.cpp \
					gl_gpgpu/fbo.cpp \
					gl_gpgpu/pipeline_renderer_imgconv.cpp \
					gl_gpgpu/pipeline_renderer_pclines.cpp \
					gl_gpgpu/pipeline_renderer_thresh.cpp \
					gl_gpgpu/pipeline_renderer.cpp \
					gl_gpgpu/pipeline.cpp \
					gl_gpgpu/renderer.cpp \
					gl_gpgpu/shader.cpp \
					gl_gpgpu/view.cpp \
					android_tools.cpp \
					java_interface_wrap.cpp \
				   	imgproc.cpp
LOCAL_LDLIBS += -llog -ldl -lGLESv2

include $(BUILD_SHARED_LIBRARY)