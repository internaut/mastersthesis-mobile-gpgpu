#ifndef CV_ACCAR_ANDROID_TOOLS_H
#define CV_ACCAR_ANDROID_TOOLS_H

#include "../cv_accar.h"

#include <android/log.h>

// usage of log
#ifndef LOGINFO
#define LOGINFO(x...) __android_log_print(ANDROID_LOG_INFO,"JNI_FRAME_PROC",x)
#endif

#ifndef LOGERR
#define LOGERR(x...) __android_log_print(ANDROID_LOG_ERROR,"JNI_FRAME_PROC",x)
#endif

#endif
