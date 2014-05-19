#include "android_jni.h"
#include "android_tools.h"

#include "../common/core.h"

static CvAccARCore *core = NULL;

void cv_accar_init(int camId, long long camIntrinsicsAddr) {
	if (!core) {
		cv::Mat camIntr = cv::Mat::zeros(3, 3, CV_32F);

		if (camIntrinsicsAddr != 0x00) {
			((cv::Mat *)camIntrinsicsAddr)->copyTo(camIntr);
		}

		core = new CvAccARCore(camId, camIntr);
	}
}

void cv_accar_set_dbg_input_frame(long long inFrameAddr) {
	assert(core != NULL && inFrameAddr != 0x00);

	cv::Mat &inFrame = *((cv::Mat *)inFrameAddr);

	core->setDbgImage(inFrame);
}

void cv_accar_set_dbg_output_mode(int mode) {
	assert(core != NULL);

	core->setDbgOutputLevel(mode);
}

void cv_accar_set_dbg_marker_shader(char *vshSrc, char *fshSrc) {
	assert(core != NULL);

	core->setDbgMarkerShader(vshSrc, fshSrc);
}

void cv_accar_set_output_display_shader(char *vshSrc, char *fshSrc) {
	assert(core != NULL);

	core->setOutputDisplayShader(vshSrc, fshSrc);
}

void cv_accar_add_shader_to_pipeline(char *vshSrc, char *fshSrc) {
	assert(core != NULL);

	core->addShaderToPipeline(vshSrc, fshSrc);
}

void cv_accar_start() {
	assert(core != NULL);

	core->start();
}

void cv_accar_stop() {
	assert(core != NULL);

	core->stop();
}

void cv_accar_pause() {
	assert(core != NULL);

	core->pause();
}

void cv_accar_resume() {
	assert(core != NULL);

	core->resume();
}

void cv_accar_view_create() {
	assert(core != NULL);

	core->getView()->create();
}

void cv_accar_view_resize(int w, int h) {
	assert(core != NULL);

	core->getView()->resize(w, h);
}

void cv_accar_view_draw() {
	core->getView()->draw();
}

void cv_accar_get_detected_markers() {
//	core->getDetectedMarkers();
}

void cv_accar_cleanup() {
	if (core) {
		delete core;
		core = NULL;
	}
}
