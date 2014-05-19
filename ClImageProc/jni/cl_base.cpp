#include "cl_base.h"

#include <assert.h>


CLBase::CLBase() {
	init_ok = false;

	kernel_name = NULL;

	// get platform
    clGetPlatformIDs(1, &platform, NULL);

    cl_context_properties contextProps[] = {
    		CL_CONTEXT_PLATFORM,
    		(cl_context_properties)platform,
    		0
    };

    // create context
    context = clCreateContextFromType(contextProps,
    									CL_DEVICE_TYPE_GPU,
    									NULL, NULL, &err);

    if (gotError("CLBase: No OpenCL-capable GPU context found")) {
    	return;
    }

    // get device
    size_t deviceBufSize;
    err = clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &deviceBufSize);

    if (gotError() || deviceBufSize <= 0) {
    	LOGERR("CLBase: Error finding OpenCL-GPU device");
    	return;
    }

    cl_device_id *devices = new cl_device_id[deviceBufSize / sizeof(cl_device_id)];

    err = clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceBufSize, devices, NULL);

    if (gotError("CLBase: No OpenCL-GPU device ids")) {
    	return;
    }

    device = devices[0];
    delete[] devices;

    init_ok = true;
}

CLBase::~CLBase() {
	clReleaseMemObject(mem_out);
	clReleaseMemObject(mem_in);
	clReleaseSampler(sampler);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseContext(context);

	if (kernel_name) delete kernel_name;
}

bool CLBase::createProg(const char *k_name, const char **prog_src, cl_uint src_count, const size_t *prog_src_bytes) {
	if (kernel_name) delete kernel_name;

	kernel_name = new char[strlen(k_name) + 1];
	strcpy(kernel_name, k_name);

	// Create the program
	program = clCreateProgramWithSource(context,
										src_count, prog_src,
										prog_src_bytes,
										&err);

    if (gotError("CLBase: Error creating OpenCL program")) {
    	return false;
    }

    // Build the program
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

    if (gotError()) {
    	LOGERR("CLBase: Error building OpenCL program");

    	char buildLog[16384];
    	clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
    							sizeof(buildLog), buildLog, NULL);

    	LOGERR("CLBase: Build Log:");

    	LOGERR("%s", buildLog);

    	clReleaseProgram(program);

    	return false;
    }

    return true;
}

bool CLBase::createSampler() {
	// Create the sampler
	sampler = clCreateSampler(context, CL_FALSE,
							  CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_NEAREST,
							  &err);

    if (gotError("CLBase: Error creating OpenCL sampler")) {
    	return false;
    }

    return true;
}

bool CLBase::createQueue() {
    // Create a command queue
    queue = clCreateCommandQueue(context, device, 0, &err);

    if (gotError("CLBase: Error creating OpenCL command queue")) {
    	return false;
    }

    return true;
}

bool CLBase::createKernel() {
	assert(kernel_name != NULL);

	// Create the kernel
	kernel = clCreateKernel(program, kernel_name, &err);

    if (gotError("CLBase: Error creating OpenCL kernel")) {
    	LOGERR("CLBase: Kernel name: %s", kernel_name);
    	return false;
    }

    // get max. work-group size for the kernel program
    err = clGetKernelWorkGroupInfo(kernel, device,
    							   CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t),
    						       &max_work_group_size, NULL);

    LOGINFO("CLBase: Max. work group size for kernel: %d", max_work_group_size);

    return true;
}

bool CLBase::execKernel() {
	err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL,
								 global_ws, local_ws,
								 0, NULL, NULL);
#ifdef BENCHMARK
	err |= clFinish(queue);
#endif

    if (gotError("CLBase: Error executing OpenCL kernel")) {
    	return false;
    }

    return true;
}

bool CLBase::getResultImg(void *imgData) {
	size_t origin[] = {0,0,0};
	size_t region[] = {img_w,img_h,1};

    // Read the result
    err = clEnqueueReadImage(queue, mem_out, CL_TRUE,
    							origin, region, 0, 0,
    							imgData, 0, NULL, NULL);
#ifdef BENCHMARK
	err |= clFinish(queue);
#endif

    if (gotError("CLBase: Error reading result image")) {
    	return false;
    }

    return true;
}

size_t CLBase::roundUp(int groupS, int globalS) {
    int r = globalS % groupS;
    if (r == 0) {
        return globalS;
    } else {
        return globalS + groupS - r;
    }
}

bool CLBase::gotError(const char *msg) {
	if (err != CL_SUCCESS) {
		if (msg != NULL) {
			LOGERR("CL Error %d occurred: %s", err, msg);
		}

		return true;
	}

	return false;
}
