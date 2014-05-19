#include "pipeline.h"

#include <cassert>
#include <ctime>
#include <dlfcn.h>

#include "../android_tools.h"
#include "pipeline_renderer_imgconv.h"
#include "pipeline_renderer_pclines.h"
#include "pipeline_renderer_thresh.h"

Pipeline::Pipeline() {
	inputTexId = outputTexId = 0;
	inputTexW = inputTexH = 0;
	fbos = NULL;
	renderers = NULL;
	fboTexturesCreated = false;

#ifdef BENCHMARK
	imgPushMs = imgPullMs = renderMs = 0.0f;
#endif
}

Pipeline::~Pipeline() {
	glDeleteTextures(PIPELINE_NUM_TEX_IDS, texIds);
	glDeleteTextures(PIPELINE_NUM_STAGES, fboTexIds);
	glDeleteFramebuffers(PIPELINE_NUM_STAGES, fboIds);

	if (fbos) {
		for (int i = 0; i < PIPELINE_NUM_STAGES; i++) {
			delete fbos[i];
		}

		delete[] fbos;
	}

	if (renderers) {
		for (int i = 0; i < PIPELINE_NUM_STAGES; i++) {
			delete renderers[i];
		}

		delete[] renderers;
	}
}

void Pipeline::render() {
	if (!fboTexturesCreated) return;

#ifdef BENCHMARK
	glFinish();
	clock_t t1 = clock();
#endif
	for (int i = 0; i < PIPELINE_NUM_STAGES; i++) {
		renderers[i]->render();
	}
#ifdef BENCHMARK
	glFinish();
	clock_t t2 = clock();
	renderMs = getMsFromClockDiff(t1, t2);
#endif
}

void Pipeline::setInputImageFromBytes(const unsigned char *bytes, int w, int h) {
	LOGINFO("Pipeline: set input image of size %dx%d from pointer %p to texture %d", w, h, bytes, inputTexId);

#ifdef BENCHMARK
	// refresh the textures, otherwise caching will influence the results
	LOGINFO("Pipeline: refreshing textures");
	glDeleteTextures(PIPELINE_NUM_TEX_IDS, texIds);
	glGenTextures(PIPELINE_NUM_TEX_IDS, texIds);
	inputTexId = texIds[0];
	LOGINFO("Pipeline: new input tex id %d", inputTexId);
#endif

	// set texture
	glBindTexture(GL_TEXTURE_2D, inputTexId);	// bind input texture
	checkGLError("Pipeline: bind input texture");

	// set clamping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// generate mipmap
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// upload image
#ifdef BENCHMARK
	glFinish();
	clock_t t1 = clock();
#endif
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
#ifdef BENCHMARK
	glFinish();
	clock_t t2 = clock();
	imgPushMs = getMsFromClockDiff(t1, t2);
#endif
	checkGLError("Pipeline: texture upload");

	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	// set dimensions
	bool texSizeChanged = inputTexW != w || inputTexH != h;
	inputTexW = w;
	inputTexH = h;

//#ifdef BENCHMARK
//	// always refresh the textures, otherwise caching will influence the results
//	updateRenderers();
//#else
	// refresh the textures if the input frame size changed or no FBO textures have been created before
	if (!fboTexturesCreated || texSizeChanged) {
		updateRenderers();
	}
//#endif
}

void Pipeline::copyOutputImageToPointer(unsigned char *bytes, int w, int h) {
	assert(renderers != NULL);
	const int lastStageNum = PIPELINE_NUM_STAGES - 1;
	LOGINFO("Pipeline: copying output image of size %dx%d to pointer %p from last stage %d",
			w, h, bytes, lastStageNum);

	PipelineRenderer *lastStage = renderers[lastStageNum];
	assert(lastStage != NULL);

	assert(w == lastStage->getOutTexW() && h == lastStage->getOutTexH());

#ifdef BENCHMARK
	glFinish();
	clock_t t1 = clock();
#endif
	lastStage->getFBO()->readBuffer(bytes);
#ifdef BENCHMARK
	glFinish();
	clock_t t2 = clock();
	imgPullMs = getMsFromClockDiff(t1, t2);
#endif

/*	for (int i = 0; i < 4 * 4; i+=4) {
		LOGINFO("Pipeline: read buffer [%d] = %d, %d, %d, %d", (i / 4), bytes[i], bytes[i+1], bytes[i+2], bytes[i+3]);
	}*/
}

void Pipeline::init() {
	// create texture ids
	LOGINFO("Pipeline: init with %d tex ids", PIPELINE_NUM_TEX_IDS);
	glGenTextures(PIPELINE_NUM_TEX_IDS, texIds);
	checkGLError("Pipeline: gen. textures");

	inputTexId = texIds[0];
	LOGINFO("Pipeline: new input tex id %d", inputTexId);

	// init framebuffer objects
	loadFBOs();

	// load renderers
	loadRenderers();
}

void Pipeline::loadFBOs() {
	assert(fbos == NULL && fboTexIds != NULL);

	LOGINFO("Pipeline: init %d FBOs", PIPELINE_NUM_STAGES);

	void *libEGLhndl = dlopen("libEGL.so", RTLD_LAZY);
	if (!libEGLhndl) {
		LOGERR("Pipeline: Could not load EGL library");
		return;
	}

	void *libUIhndl = dlopen("libui.so", RTLD_LAZY);
	if (!libUIhndl) {
		LOGERR("Pipeline: Could not load Android UI library");
		return;
	}

	glGenFramebuffers(PIPELINE_NUM_STAGES, fboIds);
	checkGLError("Pipeline: gen. FBOs");

	fbos = new FBO*[PIPELINE_NUM_STAGES];
	for (int i = 0; i < PIPELINE_NUM_STAGES; i++) {
		LOGINFO("Pipeline: Creating FBO#%d with id %d", i, fboIds[i]);

		FBO *newFBO = new FBO(libEGLhndl, libUIhndl);
		newFBO->setId(fboIds[i]);

		fbos[i] = newFBO;
	}
}

void Pipeline::loadRenderers() {
	assert(renderers == NULL);

	LOGINFO("Pipeline: init %d renderers", PIPELINE_NUM_STAGES);
	renderers = new PipelineRenderer*[PIPELINE_NUM_STAGES];

/*	// create pipeline renderer #1
	PipelineRendererImgConv *imgConvGaussPass1 = new PipelineRendererImgConv(fbos[0], GAUSS_5X5_TWOPASS, 1);
	imgConvGaussPass1->loadShader();
	renderers[0] = imgConvGaussPass1;

	// create pipeline renderer #2
	PipelineRendererImgConv *imgConvGaussPass2 = new PipelineRendererImgConv(fbos[1], GAUSS_5X5_TWOPASS, 2);
	imgConvGaussPass2->loadShader();
	renderers[1] = imgConvGaussPass2;*/

	PipelineRendererPCLines *pcLines = new PipelineRendererPCLines(fbos[0], 2);
	pcLines->loadShader();
	renderers[0] = pcLines;

	PipelineRendererThresh *thresh = new PipelineRendererThresh(fbos[1], 0.65f);	// 512: 0.25f , 1024: 0.65
	thresh->loadShader();
	renderers[1] = thresh;
}

void Pipeline::updateRenderers() {
	if (fboTexturesCreated) {
		LOGINFO("Pipeline: updating renderers, deleting %d textures", PIPELINE_NUM_STAGES);
		glDeleteTextures(PIPELINE_NUM_STAGES, fboTexIds);
	}

	LOGINFO("Pipeline: updating renderers, creating %d textures", PIPELINE_NUM_STAGES);
	glGenTextures(PIPELINE_NUM_STAGES, fboTexIds);
	checkGLError("Pipeline: gen. textures for FBOs");

	for (int i = 0; i < PIPELINE_NUM_STAGES; i++) {
		fbos[i]->setAttachedTexId(fboTexIds[i]);
		fbos[i]->destroyAttachedTex();
		renderers[i]->init(inputTexW, inputTexH);
		if (i == 0) {
			renderers[i]->useTexture(inputTexId);	// first renderer gets input
		} else {
			renderers[i]->useTexture(renderers[i-1]->getFBO()->getAttachedTexId());	// chain to prev. renderer
		}
	}

	outputTexId = fbos[PIPELINE_NUM_STAGES - 1]-> getAttachedTexId();
	fboTexturesCreated = true;
}
