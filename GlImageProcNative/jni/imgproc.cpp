#include "imgproc.h"

#include "android_tools.h"
#include "gl_gpgpu/view.h"
#include "gl_gpgpu/pipeline.h"

static Pipeline *pipeline = NULL;
static View *view = NULL;
static bool glContextCreated = false;

void init() {
	LOGINFO("init");
#ifdef BENCHMARK
	LOGINFO("compiled in BENCHMARK mode");
#endif

	pipeline = new Pipeline();
	view = new View(pipeline);
}

void set_input_image(long long inputImgAddr) {
	if (!glContextCreated) {
		LOGERR("set_input_image requires a GL context!");
		return;
	}

	// get the pointer to the image data
	int imgW, imgH, imgChan;
	unsigned char *imgData = imgDataPtrFromCvMatPtrAddr(inputImgAddr, &imgW, &imgH, &imgChan);

	LOGINFO("set_input_image: got image of size %dx%d with %d channels", imgW, imgH, imgChan);

	// upload it to the GPU
	pipeline->setInputImageFromBytes(imgData, imgW, imgH);
}

void get_output_image(long long outputImgAddr) {
	if (!glContextCreated) {
		LOGERR("get_output_image requires a GL context!");
		return;
	}

	// get the pointer to the image data
	int imgW, imgH, imgChan;
	unsigned char *imgData = imgDataPtrFromCvMatPtrAddr(outputImgAddr, &imgW, &imgH, &imgChan);

	LOGINFO("get_output_image: got image of size %dx%d with %d channels", imgW, imgH, imgChan);

	// read it back from the GPU
	pipeline->copyOutputImageToPointer(imgData, imgW, imgH);
}

void glview_create() {
	LOGINFO("glview_create");

	view->create();

	glContextCreated = true;
}

void glview_resize(int w, int h) {
	LOGINFO("glview_resize to %dx%d", w ,h);

	view->resize(w, h);
}

void glview_draw() {
//	LOGINFO("glview_draw");

	view->draw();
}

void cleanup() {
	LOGINFO("cleanup");

	if (view) delete view;
	if (pipeline) delete pipeline;
}

float get_image_push_ms() {
#ifdef BENCHMARK
	return pipeline->getImagePushMs();
#else
	return 0.0f;
#endif
}

float get_render_ms() {
#ifdef BENCHMARK
	return pipeline->getRenderMs();
#else
	return 0.0f;
#endif
}

float get_image_pull_ms() {
#ifdef BENCHMARK
	return pipeline->getImagePullMs();
#else
	return 0.0f;
#endif
}
