#include "core.h"

#include "tools.h"
#include "pipeline/disp.h"

CvAccARCore::CvAccARCore(int camId, cv::Mat &camIntr) {
	LOGINFO("CvAccARCore: creating instance");

	// init members
	dbgStillImg = NULL;
	lastAddedPipelineLevel = 0;
	pipelineReady = false;
	fboMgr = NULL;

	// init cam
	assert(camIntr.cols == 3 && camIntr.rows == 3);

	cam = new CvAccARCam(camId);

	CvAccARTools::printFloatMat("CvAccARCore: cam intrinsics", camIntr);

	cam->setIntrinsics(camIntr);

	// init detector
	CvAccARDetect::create(cam);
	detector = CvAccARDetect::get();

	if (CvAccARConf::useGPUAccel) {
		detectorAccel = (CvAccARDetectAccel *)detector;
	} else {
		detectorAccel = NULL;
	}

	// init view
	view = new CvAccARView();
	view->setCore(this);
	view->setCam(cam);
	view->setDetector(detector);
}

CvAccARCore::~CvAccARCore() {
	LOGINFO("CvAccARCore: deleting instance");

	delete cam;
	delete view;
	CvAccARDetect::destroy();

	if (fboMgr) delete fboMgr;
}

void CvAccARCore::setDbgImage(const cv::Mat &dbgImg) {
	LOGINFO("CvAccARCore: setting debug still image");
	cam->setDbgStillImage(dbgImg);	// will be copied
}

void CvAccARCore::setDbgOutputLevel(int level) {
	LOGINFO("CvAccARCore: setting frame proc output level to %d", level);
	detector->setProcFrameOutputLevel(level);
	view->enableMarkerDisp(level == 0);
}

void CvAccARCore::setDbgMarkerShader(char *vshSrc, char *fshSrc) {
	LOGINFO("CvAccARCore: Setting the shader for marker debug display");

	CvAccARShader *shader = new CvAccARShader();

	if (shader->buildFromSrc(vshSrc, fshSrc)) {
		CvAccARMarkerDisp *markerDisp = new CvAccARMarkerDisp();
		markerDisp->setMarkerScale(CvAccARConf::markerSizeMeters);
		markerDisp->bindShader(shader);
		view->setMarkerDisp(markerDisp);
	} else {
		LOGERR("CvAccARCore: Error building shader for marker debug display");
	}
}

void CvAccARCore::setOutputDisplayShader(char *vshSrc, char *fshSrc) {
	LOGINFO("CvAccARCore: Setting the shader for output display renderer");

	CvAccARShader *shader = new CvAccARShader();

	if (shader->buildFromSrc(vshSrc, fshSrc)) {
		CvAccARDisp *dispRenderer = new CvAccARDisp();
		dispRenderer->bindShader(shader);
		view->setDispRenderer(dispRenderer);
	} else {
		LOGERR("CvAccARCore: Error building shader for output display renderer");
	}
}

void CvAccARCore::postGLSetup() {
	LOGINFO("CvAccARCore: running post-GL setup");

	GLint maxVertTexImageUnits;
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxVertTexImageUnits);
	GLint maxCombImageUnits;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxCombImageUnits);

	LOGINFO("CvAccARCore: GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS = %d", maxVertTexImageUnits);	// on Nexus 10: 16
	LOGINFO("CvAccARCore: GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS = %d", maxCombImageUnits);	// on Nexus 10: 32

	// init FBO manager if necessary
	if (CvAccARConf::useGPUAccel) {
		fboMgr = new CvAccARFBOMgr();
		fboMgr->initFBOs();

		detectorAccel->initPipeline(fboMgr);
	} else {
		fboMgr = NULL;
	}
}

void CvAccARCore::addShaderToPipeline(char *vshSrc, char *fshSrc) {
	if (!CvAccARConf::useGPUAccel) {
		LOGINFO("CvAccARCore: GPU acceleration disabled, pipeline processor not created");
		return;
	}

	LOGINFO("CvAccARCore: Adding a shader to the pipeline");

	if (lastAddedPipelineLevel >= CV_ACCAR_PIPELINE_LEVELS) {
		LOGERR("CvAccARCore: All pipeline levels are already set.");
		return;
	}

	CvAccARShader *shader = new CvAccARShader();
	if (shader->buildFromSrc(vshSrc, fshSrc)) {
		LOGINFO("CvAccARCore: Built new shader for level %d", lastAddedPipelineLevel);

		CvAccARPipelineProcLevel pipelineLvl = (CvAccARPipelineProcLevel)lastAddedPipelineLevel;

		if (detectorAccel->setPipelineProcWithShader(pipelineLvl, shader)) {
			lastAddedPipelineLevel++;
		} else {
			LOGERR("CvAccARCore: Error creating and adding new pipeline processor");
			return;
		}
	} else {
		LOGERR("CvAccARCore: Error creating and adding new shader");
		return;
	}

	if (lastAddedPipelineLevel == CV_ACCAR_PIPELINE_LEVELS) {
		LOGINFO("CvAccARCore: Pipeline is initialized");
		pipelineReady = true;
	}
}

void CvAccARCore::start() {
	LOGINFO("CvAccARCore: start");
	cam->start();
	detector->camIsInitialized();
}

void CvAccARCore::stop() {
	LOGINFO("CvAccARCore: stop");
	cam->stop();
}

void CvAccARCore::pause() {
	LOGINFO("CvAccARCore: pause");
	cam->stop();
}

void CvAccARCore::resume() {
	LOGINFO("CvAccARCore: resume");
	cam->start();
}

void CvAccARCore::getDetectedMarkers() const {

}
