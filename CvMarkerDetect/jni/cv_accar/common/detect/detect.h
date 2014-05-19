#ifndef CV_ACCAR_DETECT_H
#define CV_ACCAR_DETECT_H

#include "../../cv_accar.h"
#include "../cam.h"
#include "detect_base.h"
#include "detect_accel.h"

class CvAccARDetect {
public:
	// create an instance of either CvAccARDetectBase or CvAccARDetectAccel
	static void create(CvAccARCam *cam);

	// return an instance of either CvAccARDetectBase or CvAccARDetectAccel
	static CvAccARDetectBase *get();

	// destroy the instance
	static void destroy();

private:
	static CvAccARDetectBase *instance;
};

#endif
