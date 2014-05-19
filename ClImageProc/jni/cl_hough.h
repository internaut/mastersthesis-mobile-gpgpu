#ifndef CL_HOUGH_H
#define CL_HOUGH_H

#include <CL/cl.h>

// OpenCV stuff - just needed for drawing hough lines!
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


#include "cl_base.h"

class CLHough : public CLBase {
public:
	CLHough() : CLBase() { accSpaceHostMem = NULL; };
	~CLHough();

	virtual bool setKernelArgs(int imgW, int imgH, void *imgData);

	virtual void setupWorkSizes();

	virtual bool getResultImg(void *imgData);

private:

	int accSpaceW;
	int accSpaceH;

	size_t accSpaceSizeBytes;
	cl_uint *accSpaceHostMem;

	cv::Mat inputImg;	// just for drawing the result image
};

#endif
