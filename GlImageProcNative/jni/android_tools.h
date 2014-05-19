#ifndef ANDROID_TOOLS_H
#define ANDROID_TOOLS_H

#include <android/log.h>
#include <ctime>

// usage of log
#ifndef LOGINFO
#define LOGINFO(x...) __android_log_print(ANDROID_LOG_INFO,"IMGPROC_JNI",x)
#endif

#ifndef LOGERR
#define LOGERR(x...) __android_log_print(ANDROID_LOG_ERROR,"IMGPROC_JNI",x)
#endif

unsigned char* imgDataPtrFromCvMatPtrAddr(long long cvMatPtrAddr,
										  int *w = 0x00, int *h = 0x00, int *chan = 0x00);

void checkGLError(const char *msg);

float getMsFromClockDiff(clock_t t1, clock_t t2);

#endif
