#include "cl_hough.h"

#include <stdio.h>
#include <string.h>

CLHough::~CLHough() {
	if (accSpaceHostMem) delete accSpaceHostMem;
}

bool CLHough::setKernelArgs(int imgW, int imgH, void *imgData) {
	img_w = imgW;
	img_h = imgH;

	LOGINFO("CLHough: Image size %dx%d", img_w, img_h);

	img_bytes = img_w * img_h * 4;	// 4 channels!

	// Create the OpenCV image for later output
	inputImg.create(img_h, img_w, CV_8UC4);
	memcpy(inputImg.data, imgData, img_bytes);

    // Create the OpenCL image for the input image
    cl_image_format clImgFmt;
    clImgFmt.image_channel_order = CL_RGBA;
    clImgFmt.image_channel_data_type = CL_UNORM_INT8;

    mem_in = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
    							&clImgFmt, img_w, img_h,
    							0, imgData, &err);

    if (gotError("CLHough: Image input memory object")) {
    	return false;
    }

    // set accumulator space dimensions
    accSpaceW = (int)sqrtf((float)img_w *  (float)img_w * 0.25f + (float)img_h *  (float)img_h * 0.25f);
    accSpaceH = 180;

    LOGINFO("CLHough: Acc. space dim.: %dx%d", accSpaceW, accSpaceH);

    // Create the buffer for the output image
    accSpaceSizeBytes = sizeof(cl_uint) * accSpaceW * accSpaceH;

    if (accSpaceHostMem) delete accSpaceHostMem;
    accSpaceHostMem = new cl_uint[accSpaceSizeBytes];
    memset(accSpaceHostMem, 0, accSpaceSizeBytes);

    mem_out = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,	// copying to initialize with zeros!
                                accSpaceSizeBytes,
                                accSpaceHostMem,	// to initialize with zeros!
                                &err);

    if (gotError("CLHough: Output memory object")) {
    	return false;
    }

    // Set kernel arguments
    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &mem_in);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &mem_out);
    err |= clSetKernelArg(kernel, 2, sizeof(cl_sampler), &sampler);
    err |= clSetKernelArg(kernel, 3, sizeof(accSpaceW), &accSpaceW);
    err |= clSetKernelArg(kernel, 4, sizeof(accSpaceH), &accSpaceH);
    err |= clSetKernelArg(kernel, 5, sizeof(img_w), &img_w);
    err |= clSetKernelArg(kernel, 6, sizeof(img_h), &img_h);

    if (gotError("CLImgConv: Set kernel arguments")) {
    	return false;
    }

    return true;
}

void CLHough::setupWorkSizes() {
	// Set work sizes
	size_t workSizePerDim = sqrt(max_work_group_size);		// Square root, because maxWorkGroupSize is for dim = 1, but we have dim = 2!
	local_ws[0] = local_ws[1] = workSizePerDim;

	global_ws[0] = roundUp(local_ws[0], img_w);
	global_ws[1] = roundUp(local_ws[1], img_h);

	LOGINFO("CLImgConv: Worksizes: local - %d, %d | global - %d, %d",
			local_ws[0],
			local_ws[1],
			global_ws[0],
			global_ws[1]);
}

bool CLHough::getResultImg(void *imgData) {
    // Read the result
    err = clEnqueueReadBuffer(queue, mem_out, CL_TRUE,
    							0, accSpaceSizeBytes,
    							accSpaceHostMem,
    							0, NULL, NULL);
#ifdef BENCHMARK
	err |= clFinish(queue);
#endif

    if (gotError("CLHough: Error reading result image")) {
    	return false;
    }

#ifdef HOUGH_DRAW_LINES
//    // output to console
    // find out max and avg
    unsigned int numNonZeroVals = 0;
    unsigned int sumNonZeroVals = 0;
    unsigned int accSpaceMax = 0;
    for (int y = 0; y < accSpaceH; y++) {
//    	char *accLine = new char[1024];
    	unsigned int accLinePos = 0;
    	for (int x = 0; x < accSpaceW; x++) {
    		unsigned int val = accSpaceHostMem[y * accSpaceW + x];
    		if (val > 0) {
    			numNonZeroVals++;
    			sumNonZeroVals+=val;

    			if (val > accSpaceMax) {
    				accSpaceMax = val;
    			}
    		}
//    		accLinePos += sprintf(accLine + accLinePos, "%d,", val);
    	}

//    	LOGINFO("%s", accLine);
    }

    if (numNonZeroVals == 0) {
    	return true;
    }

/*    inputImg = cv::Mat::zeros(accSpaceH, accSpaceW, CV_8UC4);
    for (int y = 0; y < accSpaceH; y++) {
    	for (int x = 0; x < accSpaceW; x++) {
    		unsigned int val = resData[y * accSpaceW + x];
    		unsigned char byte = (unsigned char)((float)val / (float)accSpaceMax * 255.0f);
    		inputImg.at<uchar>(y, x) = byte;
    	}
    }

    cv::resize(inputImg, inputImg, cv::Size(img_w, img_h));*/

    const float avgNonZeroVal = (float)sumNonZeroVals / (float)numNonZeroVals;
    const unsigned int thresh = (unsigned int)(avgNonZeroVal + ((accSpaceMax - avgNonZeroVal) * 0.3333f));
    const float DEG2RAD = M_PI / 180.0f;

    LOGINFO("CLHough: Got %d non-zero votes, with avg. = %f, max = %d. Thresh. is %d",
    		numNonZeroVals, avgNonZeroVal, accSpaceMax, thresh);

    // draw lines on CV mat
    for (int theta = 0; theta < accSpaceH; theta++) {
    	for (int r = 0; r < accSpaceW; r++) {
    		unsigned int val = accSpaceHostMem[theta * accSpaceW + r];
    		if (val > thresh) {
    			const float a = cosf(theta * DEG2RAD);
    			const float b = sinf(theta * DEG2RAD);
    			const int normR = (r - accSpaceW / 2);
    			const float x0 = a * normR + img_w / 2.0f;
    			const float y0 = b * normR + img_h / 2.0f;

    			cv::Point p1(roundf(x0 + 2.0f * img_w * (-b)),
    						 roundf(y0 + 2.0f * img_h * (a)));
    			cv::Point p2(roundf(x0 - 2.0f * img_w * (-b)),
    						 roundf(y0 - 2.0f * img_h * (a)));

#if HOUGH_DRAW_LINES == 1
    			cv::Scalar c;
    			if (theta < 90) {
    				c = cv::Scalar(255, 0, 0);
    			} else {
    				c = cv::Scalar(0, 255, 0);
    			}

    			cv::line(inputImg, p1, p2, c, 1);
#else
    			cv::line(inputImg, p1, p2, CV_RGB(128,128,128), 1);
#endif
    		}
    	}
    }

    memcpy(imgData, inputImg.data, img_bytes);
#endif

    return true;
}
