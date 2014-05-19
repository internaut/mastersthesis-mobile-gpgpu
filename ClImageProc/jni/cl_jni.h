#ifndef CL_JNI_H
#define CL_JNI_H

// the jni library MUST be included
#include <jni.h>

// opencv libs
/*#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>*/

// prog types
#define PROG_TYPE_IMGCONV	0
#define PROG_TYPE_HOUGH		1
#define PROG_TYPE_HIST		2

extern "C" {

jint 	Java_net_mkonrad_climageproc_ClJNIGlue_initCL(JNIEnv *env, jobject obj, jint type);

jint 	Java_net_mkonrad_climageproc_ClJNIGlue_createProg(JNIEnv *env, jobject obj,
														jstring progName, jobject progSrcBuf);

jint	Java_net_mkonrad_climageproc_ClJNIGlue_createKernel(JNIEnv *env, jobject obj);

jint 	Java_net_mkonrad_climageproc_ClJNIGlue_createCmdQueue(JNIEnv *env, jobject obj);

jfloat 	Java_net_mkonrad_climageproc_ClJNIGlue_setKernelArgs(JNIEnv *env, jobject obj,
														 jint width, jint height,
														 jobject inputImgBuf);

jfloat 	Java_net_mkonrad_climageproc_ClJNIGlue_executeKernel(JNIEnv *env, jobject obj);

jfloat 	Java_net_mkonrad_climageproc_ClJNIGlue_getResultImg(JNIEnv *env, jobject obj,
														jobject outputImgBuf);

void 	Java_net_mkonrad_climageproc_ClJNIGlue_dealloc(JNIEnv *env, jobject obj);


void	Java_net_mkonrad_climageproc_ClJNIGlue_printInfo(JNIEnv *env, jobject obj);

}

#endif
