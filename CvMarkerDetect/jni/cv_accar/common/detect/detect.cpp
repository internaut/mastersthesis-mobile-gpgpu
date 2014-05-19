#include "detect.h"

CvAccARDetectBase *CvAccARDetect::instance = NULL;

void CvAccARDetect::create(CvAccARCam *cam) {
	if (!instance) {
		if (CvAccARConf::useGPUAccel) {
			LOGINFO("CvAccARDetect: Creating new 'CvAccARDetectAccel' instance");
			instance = new CvAccARDetectAccel(cam);
		} else {
			LOGINFO("CvAccARDetect: Creating new 'CvAccARDetectBase' instance");
			instance = new CvAccARDetectBase(cam);
		}
	}
}

CvAccARDetectBase *CvAccARDetect::get() {
	return instance;
}

void CvAccARDetect::destroy() {
	if (!instance) return;

	LOGINFO("CvAccARDetect: Destroying instance");
	delete instance;
}
