#ifndef CL_BASE_H
#define CL_BASE_H

// android libs
#include <android/log.h>

// C libs
#include <math.h>
#include <time.h>
#include <string.h>

// OpenCL lib
#include <CL/cl.h>

// usage of log
#define LOGINFO(x...) __android_log_print(ANDROID_LOG_INFO,"CL_JNI",x)
#define LOGERR(x...) __android_log_print(ANDROID_LOG_ERROR,"CL_JNI",x)


class CLBase {
public:
	CLBase();
	virtual ~CLBase();

	bool isInitialized() const { return init_ok; }

	bool createProg(const char *k_name, const char **prog_src, cl_uint src_count, const size_t *prog_src_bytes);

	bool createSampler();

	bool createKernel();

	bool createQueue();

	virtual bool setKernelArgs(int imgW, int imgH, void *imgData) = 0;

	virtual void setupWorkSizes() = 0;

	virtual bool getResultImg(void *imgData);

	bool execKernel();


protected:
	bool gotError(const char *msg = NULL);

	size_t roundUp(int groupS, int globalS);


	bool init_ok;

	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_program program;
	cl_kernel kernel;
	cl_sampler sampler;
	cl_mem mem_in;
	cl_mem mem_out;
	cl_command_queue queue;

	cl_int err;

	char *kernel_name;

	size_t global_ws[2];	// global work-size
	size_t local_ws[2];		// local work-size
	size_t max_work_group_size;

	size_t img_w;
	size_t img_h;
	size_t img_bytes;
};

#endif
