#ifndef CL_IMGCONV_H
#define CL_IMGCONV_H

#include <CL/cl.h>

#include "cl_base.h"

class CLImgConv : public CLBase {
	virtual bool setKernelArgs(int imgW, int imgH, void *imgData);

	virtual void setupWorkSizes();
};

#endif
