#include "cl_jni.h"

#include <assert.h>
#include <string.h>

#include "cl_base.h"
#include "cl_imgconv.h"
#include "cl_hough.h"
static clock_t tStart;static clock_t tFinish;
static int progType = -1;
static CLBase *clCtrl = NULL;float displayExecTime(const char *msg) {	float ms = ((double)(tFinish - tStart) / CLOCKS_PER_SEC) * 1000.0f;	LOGINFO("Measured time %s: %f ms\n", msg, ms);	return ms;}jint Java_net_mkonrad_climageproc_ClJNIGlue_initCL(JNIEnv *env, jobject obj, jint type){	if (clCtrl != NULL) {
		LOGERR("CL controller object already initialized!");
		return 1;
	}

	progType = type;

	if (progType == PROG_TYPE_IMGCONV) {
		LOGINFO("Creating CL program for type %d (PROG_TYPE_IMGCONV)", progType);

		clCtrl = new CLImgConv();
	} else if (progType == PROG_TYPE_HOUGH) {
		LOGINFO("Creating CL program for type %d (PROG_TYPE_HOUGH)", progType);

		clCtrl = new CLHough();
	} else {
		LOGERR("CL program type %d not supported!", progType);
		return 1;
	}

	if (!clCtrl->isInitialized()) {
		LOGERR("Error initializing CL controller object");

		delete clCtrl;
		clCtrl = NULL;

		return 1;
	}

	return 0;
}jint Java_net_mkonrad_climageproc_ClJNIGlue_createProg(JNIEnv *env, jobject obj,													   jstring progName,
													   jobject progSrcBuf){
	assert(clCtrl != NULL);

	// get the name
	char *clKernelProgName = (char *)(env->GetStringUTFChars(progName, NULL));
	// Get CL program source code	char *progSrcFull = (char *)(env->GetDirectBufferAddress(progSrcBuf));	const size_t progSizeFull = strlen(progSrcFull);	LOGINFO("Creating OpenCL program with source length %d", progSizeFull);
	LOGINFO("Full source:\n%s", progSrcFull);
	// CL requires to have source code parts of max. 1024 bytes length, so we need
	// to create these parts and devide "progSrcFull":
	const int SRC_MAX_BYTES = 1024;
	const unsigned int numSrcParts = progSizeFull / SRC_MAX_BYTES + 1;
	char **srcParts = new char*[numSrcParts];
	size_t *srcPartsLen = new size_t[numSrcParts];
	LOGINFO("Source parts = %d", numSrcParts);
	for (int i = 0; i < numSrcParts; i++) {
		// create memory space
		const size_t partLen = (i < numSrcParts - 1) ? SRC_MAX_BYTES : progSizeFull - i * SRC_MAX_BYTES;
		srcParts[i] 	= new char[partLen];
		srcPartsLen[i] 	= partLen;

		LOGINFO("Source part %d with str. length %d", i, partLen);

		// copy substring
		strncpy(srcParts[i], progSrcFull + i * SRC_MAX_BYTES, partLen);
	}
	// now create the program
	if (!clCtrl->createProg(clKernelProgName, (const char **)srcParts, numSrcParts, srcPartsLen)) {
		return 1;
	}

	// now create the sampler
	if (!clCtrl->createSampler()) {
		return 1;
	}

	return 0;}jint Java_net_mkonrad_climageproc_ClJNIGlue_createKernel(JNIEnv *env, jobject obj){	assert(clCtrl != NULL);

	if (clCtrl->createKernel()) {
		return 0;
	} else {
		return 1;
	}}jint Java_net_mkonrad_climageproc_ClJNIGlue_createCmdQueue(JNIEnv *env, jobject obj){	assert(clCtrl != NULL);

	if (clCtrl->createQueue()) {
		return 0;
	} else {
		return 1;
	}}jfloat Java_net_mkonrad_climageproc_ClJNIGlue_setKernelArgs(JNIEnv *env, jobject obj,														 jint width, jint height,														 jobject inputImgBuf){
	assert(clCtrl != NULL);
	// Get the image data	void *inImg = (void *)(env->GetDirectBufferAddress(inputImgBuf));    tStart = clock();    if (!clCtrl->setKernelArgs(width, height, inImg)) {
    	return -1.0f;
    }    tFinish = clock();
    float execTime = displayExecTime("input memory copy");    return execTime;}jfloat Java_net_mkonrad_climageproc_ClJNIGlue_executeKernel(JNIEnv *env, jobject obj){
	assert(clCtrl != NULL);
	// Set work sizes	clCtrl->setupWorkSizes();	// Execute the kernel	tStart = clock();    if (!clCtrl->execKernel()) {
    	return -1.0f;
    }	tFinish = clock();	float execTime = displayExecTime("kernel execution");    return execTime;}jfloat Java_net_mkonrad_climageproc_ClJNIGlue_getResultImg(JNIEnv *env, jobject obj,														jobject outputImgBuf){
	assert(clCtrl != NULL);
	void *outImg = (void *)(env->GetDirectBufferAddress(outputImgBuf));    // Read the result	tStart = clock();	if (!clCtrl->getResultImg(outImg)) {
		return -1.0f;
	}	tFinish = clock();	float execTime = displayExecTime("output memory copy");    return execTime;}void Java_net_mkonrad_climageproc_ClJNIGlue_dealloc(JNIEnv *env, jobject obj){	if (clCtrl) {
		delete clCtrl;
		clCtrl = NULL;
	}}void Java_net_mkonrad_climageproc_ClJNIGlue_printInfo(JNIEnv *env, jobject obj){/*	const cl_platform_info platInfoIds[] = {			CL_PLATFORM_NAME,			CL_PLATFORM_VENDOR,			CL_PLATFORM_PROFILE,			CL_PLATFORM_VERSION,			CL_PLATFORM_EXTENSIONS	};	const char *platInfoStrings[] = {			"CL_PLATFORM_NAME",			"CL_PLATFORM_VENDOR",			"CL_PLATFORM_PROFILE",			"CL_PLATFORM_VERSION",			"CL_PLATFORM_EXTENSIONS"	};	size_t items = sizeof(platInfoIds) / sizeof(platInfoIds[0]);	for (int i = 0; i < items; ++i) {		size_t s;		clGetPlatformInfo(platform, platInfoIds[i], 0, NULL, &s);		char *buf = new char[s];		clGetPlatformInfo(platform, platInfoIds[i], s, buf, NULL);		LOGINFO("%s: %s", platInfoStrings[i], buf);		delete[] buf;	}	unsigned int devInfoOfTypeStr1 = 0;	unsigned int devInfoOfTypeStr2 = 4;	unsigned int devInfoOfTypeUInt1 = 5;	unsigned int devInfoOfTypeUInt2 = 7;	unsigned int devInfoOfTypeBool1 = 8;	unsigned int devInfoOfTypeBool2 = 8;	unsigned int devInfoOfTypeSizeT1 = 9;	unsigned int devInfoOfTypeSizeT2 = 11;	const cl_device_info devInfoIds[] = {			CL_DEVICE_NAME,			CL_DEVICE_VENDOR,			CL_DRIVER_VERSION,			CL_DEVICE_PROFILE,			CL_DEVICE_VERSION,			CL_DEVICE_MAX_COMPUTE_UNITS,			CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,			CL_DEVICE_MAX_CLOCK_FREQUENCY,			CL_DEVICE_IMAGE_SUPPORT,			CL_DEVICE_MAX_WORK_GROUP_SIZE,			CL_DEVICE_IMAGE2D_MAX_WIDTH,			CL_DEVICE_IMAGE2D_MAX_HEIGHT	};	const char *devInfoStrings[] = {			"CL_DEVICE_NAME",			"CL_DEVICE_VENDOR",			"CL_DRIVER_VERSION",			"CL_DEVICE_PROFILE",			"CL_DEVICE_VERSION",			"CL_DEVICE_MAX_COMPUTE_UNITS",			"CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS",			"CL_DEVICE_MAX_CLOCK_FREQUENCY",			"CL_DEVICE_IMAGE_SUPPORT",			"CL_DEVICE_MAX_WORK_GROUP_SIZE",			"CL_DEVICE_IMAGE2D_MAX_WIDTH",			"CL_DEVICE_IMAGE2D_MAX_HEIGHT"	};	items = sizeof(devInfoIds) / sizeof(devInfoIds[0]);	for (int i = 0; i < items; ++i) {		size_t s;		clGetDeviceInfo(device, devInfoIds[i], 0, NULL, &s);		if (i >= devInfoOfTypeStr1 && i <= devInfoOfTypeStr2) {			char *buf = new char[s];			clGetDeviceInfo(device, devInfoIds[i], s, buf, NULL);			LOGINFO("%s: %s", devInfoStrings[i], buf);			delete[] buf;		}		if (i >= devInfoOfTypeUInt1 && i <= devInfoOfTypeUInt2) {			cl_uint num;			clGetDeviceInfo(device, devInfoIds[i], s, &num, NULL);			LOGINFO("%s: %d", devInfoStrings[i], num);		}		if (i >= devInfoOfTypeBool1 && i <= devInfoOfTypeBool2) {			cl_bool res;			clGetDeviceInfo(device, devInfoIds[i], s, &res, NULL);			LOGINFO("%s: %d", devInfoStrings[i], res);		}		if (i >= devInfoOfTypeSizeT1 && i <= devInfoOfTypeSizeT2) {			size_t val;			clGetDeviceInfo(device, devInfoIds[i], s, &val, NULL);			LOGINFO("%s: %u", devInfoStrings[i], val);		}	}	// supported image formats	cl_mem_flags suppImgFormatsMemFlags = CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR;	cl_uint numImgFmts;	err = clGetSupportedImageFormats(context, suppImgFormatsMemFlags,									CL_MEM_OBJECT_IMAGE2D, 0, NULL,									&numImgFmts);    if (err != CL_SUCCESS) {    	LOGERR("Error calling clGetSupportedImageFormats I");    	return;    }    if (numImgFmts <= 0) {    	LOGINFO("No image formats supported.");    	return;    }    cl_image_format *imgFmts = new cl_image_format[numImgFmts];	err = clGetSupportedImageFormats(context, suppImgFormatsMemFlags,									CL_MEM_OBJECT_IMAGE2D, numImgFmts, imgFmts,									NULL);    if (err != CL_SUCCESS) {    	LOGERR("Error calling clGetSupportedImageFormats II");    	return;    }    for (cl_uint i = 0; i < numImgFmts; i++) {    	cl_image_format fmt = imgFmts[i];    	LOGINFO("Supported image format %d: order %04x, type %04x", i, fmt.image_channel_order, fmt.image_channel_data_type);    }*/}