#include "cl_imgconv.h"

bool CLImgConv::setKernelArgs(int imgW, int imgH, void *imgData) {
	img_w = imgW;
	img_h = imgH;

	LOGINFO("CLImgConv: Image size %dx%d", img_w, img_h);

	img_bytes = img_w * img_h * 4;	// 4 channels!

    // Create the OpenCL image for the input image
    cl_image_format clImgFmt;
    clImgFmt.image_channel_order = CL_RGBA;
    clImgFmt.image_channel_data_type = CL_UNORM_INT8;

    mem_in = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
    							&clImgFmt, img_w, img_h,
    							0, imgData, &err);

    if (gotError("CLImgConv: Image input memory object")) {
    	return false;
    }

    // Create the buffer for the output image
    mem_out = clCreateImage2D(context, CL_MEM_WRITE_ONLY,
                                &clImgFmt, img_w, img_h,
                                0, NULL, &err);

    if (gotError("CLImgConv: Image output memory object")) {
    	return false;
    }

    // Set kernel arguments
    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &mem_in);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &mem_out);
    err |= clSetKernelArg(kernel, 2, sizeof(cl_sampler), &sampler);
    err |= clSetKernelArg(kernel, 3, sizeof(img_w), &img_w);
    err |= clSetKernelArg(kernel, 4, sizeof(img_h), &img_h);

    if (gotError("CLImgConv: Set kernel arguments")) {
    	return false;
    }

    return true;
}

void CLImgConv::setupWorkSizes() {
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
