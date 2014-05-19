#include "android_tools.h"

#include <opencv2/core/core.hpp>
#include <GLES2/gl2.h>

unsigned char* imgDataPtrFromCvMatPtrAddr(long long cvMatPtrAddr, int *w, int *h, int *chan) {
	cv::Mat *mat = ((cv::Mat *)cvMatPtrAddr);

	if (w) *w = mat->cols;
	if (h) *h = mat->rows;
	if (chan) *chan = mat->channels();

	return mat->data;
}

void checkGLError(const char *msg) {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		LOGERR("%s - GL Error occured: %d", msg, err);
	}
}

float getMsFromClockDiff(clock_t t1, clock_t t2) {
	return (float)((double)(t2 - t1) / (double)CLOCKS_PER_SEC * 1000.0);
}
