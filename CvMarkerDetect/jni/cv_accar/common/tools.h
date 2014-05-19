#ifndef CV_ACCAR_TOOLS_H
#define CV_ACCAR_TOOLS_H

#include "../cv_accar.h"

#include <time.h>
#include <GLES2/gl2.h>

class CvAccARTools {
public:
	static void checkGLError(const char *msg);

	static void printFloatMat(const char *msg, const cv::Mat &m);
	static void printFloatMat(const char *msg, const float *m, const int rows, const int cols);

	static void printDoubleMat(const char *msg, const cv::Mat &m);
	static void printDoubleMat(const char *msg, const double *m, const int rows, const int cols);

	static void printExecTime(const char *msg);
	static void printExecTimeSum(const char *msg);

	static float getMsFromClocks(clock_t t1, clock_t t2);

	static int getNextPOT(int s, int minExp, int maxExp);

	static float distSquared(cv::Point2f p1, cv::Point2f p2);

	static clock_t t1;	// start time
	static clock_t t2;	// end time
	static double dTsum;	// delta time sum
};

#endif
