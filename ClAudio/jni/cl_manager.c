#include "cl_manager.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tools.h"

typedef struct _cl_mgr_context {
	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_program program;
	cl_kernel kernel;
	cl_mem mem_out;
	cl_command_queue queue;
	cl_int buf_offset;
	cl_int buf_size_samples;
	cl_int buf_size_bytes;	// = sizeof(float) * buf_size_samples
	size_t global_ws[1];	// global work-size
	size_t local_ws[1];		// local work-size
	size_t max_work_group_size;
} cl_mgr_context;

void cl_mgr_context_init(cl_mgr_context *c) {
	c->platform = NULL;
	c->device = NULL;
	c->context = NULL;
	c->program = NULL;
	c->kernel = NULL;
	c->mem_out = NULL;
	c->queue = NULL;
	c->buf_offset = 0;
	c->buf_size_samples = c->buf_size_bytes = 0;
	c->global_ws[0] = c->local_ws[0] = 0;
	c->max_work_group_size = 0;
}

static cl_mgr_context ctx;
static cl_int err;

int cl_mgr_got_err(const char *errmsg) {
	if (err != CL_SUCCESS) {
		LOGERR("%s", errmsg);
		LOGERR("Error code: %d", err);

		cl_mgr_release();

		return 1;
	}

	return 0;
}

int cl_mgr_init() {
	cl_mgr_context_init(&ctx);

	// get platform
    clGetPlatformIDs(1, &ctx.platform, NULL);

    cl_context_properties contextProps[] = {
    		CL_CONTEXT_PLATFORM,
    		(cl_context_properties)ctx.platform,
    		0
    };

    // create context
    ctx.context = clCreateContextFromType(contextProps,
    									CL_DEVICE_TYPE_GPU,
    									NULL, NULL, &err);

    if (cl_mgr_got_err("No OpenCL-capable GPU context found!"))
    	return 1;

    // get device
    size_t deviceBufSize;
    err = clGetContextInfo(ctx.context, CL_CONTEXT_DEVICES, 0, NULL, &deviceBufSize);

    if (cl_mgr_got_err("Error finding OpenCL-GPU device!") || deviceBufSize <= 0)
    	return 1;

    cl_device_id *devices = (cl_device_id *)malloc(deviceBufSize);

    err = clGetContextInfo(ctx.context, CL_CONTEXT_DEVICES, deviceBufSize, devices, NULL);

    if (cl_mgr_got_err("Error getting OpenCL-GPU device ids!")) {
    	if (devices) free(devices);
    	return 1;
    }

    ctx.device = devices[0];
    free(devices);

    return 0;	// ok
}

int cl_mgr_create_kernel_from_src(const char *src) {
	// Create program from source

	const size_t progSize = strlen(src);

	LOGINFO("Creating OpenCL program with source length %d", progSize);
//	LOGINFO("Content: %s", progSrc);

	// Create the program
	ctx.program = clCreateProgramWithSource(ctx.context, 1, (const char**)&src, &progSize, &err);

    if (cl_mgr_got_err("Error creating OpenCL program!"))
    	return 1;

    // Build the program
    err = clBuildProgram(ctx.program, 0, NULL, NULL, NULL, NULL);

    if (err != CL_SUCCESS) {
    	LOGERR("Error building OpenCL program!");

    	char buildLog[16384];
    	clGetProgramBuildInfo(ctx.program, ctx.device, CL_PROGRAM_BUILD_LOG,
    							sizeof(buildLog), buildLog, NULL);

    	LOGERR("Build Log:");

    	LOGERR("%s", buildLog);

    	cl_mgr_release();

    	return 1;
    }

	// Create the kernel

	ctx.kernel = clCreateKernel(ctx.program, "cl_synth", &err);

    if (cl_mgr_got_err("Error creating OpenCL kernel"))
    	return 1;

    // get max. work-group size for the kernel program
    err = clGetKernelWorkGroupInfo(ctx.kernel, ctx.device,
    							   CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t),
    						       &ctx.max_work_group_size, NULL);

    if (cl_mgr_got_err("Error getting OpenCL work group info"))
    	return 1;

    LOGINFO("Max. work group size for kernel: %d", ctx.max_work_group_size);

    // Create a command queue
    ctx.queue = clCreateCommandQueue(ctx.context, ctx.device, 0, &err);

    if (cl_mgr_got_err("Error creating OpenCL command queue!"))
    	return 1;

    return 0;
}

int cl_mgr_create_result_buf(size_t bufSizeSamples) {
	assert(bufSizeSamples > 0);
	if (ctx.mem_out) {
		LOGINFO("OpenCL output memory already allocated!");
		return 0;
	}

	// set sizes
	ctx.buf_size_samples = bufSizeSamples;
	ctx.buf_size_bytes = sizeof(float) * ctx.buf_size_samples;

	// create output memory buffer
	ctx.mem_out = clCreateBuffer(ctx.context,
								 CL_MEM_WRITE_ONLY,
								 ctx.buf_size_bytes,
								 NULL, &err);

	if (cl_mgr_got_err("Error creating OpenCL output memory object!"))
		return 1;

	// set work-sizes
	ctx.local_ws[0] 	= ctx.max_work_group_size;
	ctx.global_ws[0]	= ctx.buf_size_samples;

	// set kernel arguments
	cl_mgr_update_args(0);
	err = clSetKernelArg(ctx.kernel, 1, sizeof(cl_mem),   &ctx.mem_out);

	if (cl_mgr_got_err("Error setting OpenCL kernel arguments!"))
		return 1;

	return 0;
}

void cl_mgr_update_args(int bufOffset) {
	ctx.buf_offset  = bufOffset;
	err |= clSetKernelArg(ctx.kernel, 0, sizeof(cl_int),   &ctx.buf_offset);

	assert(err == CL_SUCCESS);
}

void cl_mgr_exec_kernel() {
	err = clEnqueueNDRangeKernel(ctx.queue, ctx.kernel, 1, NULL,
								 ctx.global_ws, ctx.local_ws,
								 0, NULL, NULL);

	assert(err == CL_SUCCESS);
}

void cl_mgr_get_result(void *resBuf) {
	err = clEnqueueReadBuffer(ctx.queue, ctx.mem_out, CL_TRUE,
							  0, ctx.buf_size_bytes, resBuf,
							  0, NULL, NULL);

	assert(err == CL_SUCCESS);
}

void cl_mgr_release() {
	if (ctx.context) clReleaseContext(ctx.context);
	if (ctx.program) clReleaseProgram(ctx.program);
	if (ctx.queue) clReleaseCommandQueue(ctx.queue);
	if (ctx.kernel) clReleaseKernel(ctx.kernel);
	if (ctx.mem_out) clReleaseMemObject(ctx.mem_out);
}
