#include <android/log.h>

// usage of log
#ifndef LOGINFO
#define LOGINFO(x...) __android_log_print(ANDROID_LOG_INFO,"CL_AUDIO",x)
#endif

#ifndef LOGERR
#define LOGERR(x...) __android_log_print(ANDROID_LOG_ERROR,"CL_AUDIO",x)
#endif
