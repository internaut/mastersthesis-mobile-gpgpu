#include "view.h"

#include <map>

#include "tools.h"

CvAccARView::CvAccARView() {
	markerDisp = NULL;
	markerDispEnabled = true;
#ifdef PRINT_FPS
	fpsAvgSamples = 100;
#endif
}

CvAccARView::~CvAccARView() {
	if (markerDisp) {
		delete markerDisp;
	}
}

void CvAccARView::setMarkerDisp(CvAccARMarkerDisp *m){
	markerDisp = m;
}

void CvAccARView::create() {
	LOGINFO("CvAccARView: creating view");

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);

	// create camera frame texture id
	glGenTextures(2, texIds);

	// notify core
	core->postGLSetup();

#ifdef PRINT_FPS
	fpsLastClock = 0;
	fpsSum = 0.0f;
	fpsCount = 0;
#endif
}

void CvAccARView::resize(int w, int h) {
	LOGINFO("CvAccARView: resizing view to %dx%d", w, h);

	viewW = w;
	viewH = h;

	// re-calculate the projection matrix
	cv::Size procFrameSize = detector->getProcFrameSize();
	cam->calcProjMat(viewW, viewH, procFrameSize.width, procFrameSize.height);
}

void CvAccARView::draw() {
	assert(dispRenderer != NULL && detector != NULL && cam != NULL);

	glClear(GL_COLOR_BUFFER_BIT);

	// update camera
	cam->update();

	// get current frame
	dispTexId = 0;
	cv::Mat *camFrameRGBA = cam->getCurFrame();		// input frame from camera
	cv::Mat *camFrameGray = cam->getCurFrameGray();	// gray input frame from camera
	if (camFrameGray == NULL) camFrameGray = camFrameRGBA;
	cv::Mat *dispFrame = camFrameRGBA;	// output display frame

	// copy the input frame as texture to the GPU for display and/or GPGPU processing
	prepareInputFrame(camFrameRGBA->cols, camFrameRGBA->rows, camFrameRGBA->data);

	if (camFrameRGBA && camFrameRGBA->rows > 0) {
		GLint format = GL_RGBA;

		// detector input
		if (!CvAccARConf::useGPUAccel) {
			detector->setInputFrame(camFrameGray);
		} else {
			detector->setInputFrame(texIds[CV_ACCAR_VIEW_CAM_TEX]);	// bind input texture
		}

		// detector processing
		detector->processFrame();

		// set output texture id if necessary
		if (CvAccARConf::useGPUAccel) {
			dispTexId = detector->getOutputFrameHandle();
		}

		// set output frame
		if (detector->getProcFrameOutputLevel() > CV_ACCAR_PROC_LEVEL_NONE && dispTexId == 0) {	// processed frame from detector
			dispFrame = detector->getProcFrameOutput();

			if (!dispFrame || dispFrame->rows <= 0) {
				LOGERR("CvAccARView: Could not get valid display frame");
				return;
			}

//			LOGINFO("CvAccARView: Set output frame with size %dx%d and %d channels", dispFrame->cols, dispFrame->rows, dispFrame->channels());
			if (dispFrame->channels() == 1) {
				format = GL_LUMINANCE;
			} else {
				format = GL_RGBA;
			}
		}

		// set output frame as texture
		if (dispTexId == 0) {
			dispTexId = texIds[CV_ACCAR_VIEW_OUT_TEX];
//			glFinish();
//			CvAccARTools::t1 = clock();
			glBindTexture(GL_TEXTURE_2D, texIds[CV_ACCAR_VIEW_OUT_TEX]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glTexImage2D(GL_TEXTURE_2D,
						 0,
						 format,
						 dispFrame->cols,
						 dispFrame->rows,
						 0,
						 format,
						 GL_UNSIGNED_BYTE,
						 dispFrame->ptr());
//			glFinish();
//			CvAccARTools::t2 = clock();
//			CvAccARTools::printExecTime("CvAccARView: texture upload");
		}
	}

	// set the viewport
	glViewport(0, 0, viewW, viewH);

	// render the frame
//	LOGINFO("CvAccARView: Displaying tex. %d", dispTexId);
	dispRenderer->useTexture(dispTexId);
	dispRenderer->render();

	// render the markers if necessary
	if (markerDisp && markerDispEnabled) {
		drawMarkers();
	}

//	CvAccARTools::printExecTimeSum("TOTAL");

#ifdef PRINT_FPS
//	glFinish();
	clock_t now = clock();
	fpsSum += 1000.0f / CvAccARTools::getMsFromClocks(fpsLastClock, now);
	fpsCount++;
	if (fpsCount == fpsAvgSamples) {
		LOGINFO("CvAccARView: fps = %f", fpsSum / (float)fpsAvgSamples);
		fpsSum = 0.0f;
		fpsCount = 0;
	}
	fpsLastClock = now;
#endif
}

void CvAccARView::prepareInputFrame(const int w, const int h, unsigned char *pixels) {
#ifdef BENCHMARK
	glFinish();
	clock_t t1 = clock();
#endif

	// set texture
	glBindTexture(GL_TEXTURE_2D, texIds[CV_ACCAR_VIEW_CAM_TEX]);	// bind input texture

	// set clamping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	// generate mipmap
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	glGenerateMipmap(GL_TEXTURE_2D);

#ifdef BENCHMARK
	glFinish();
	clock_t t2 = clock();
	if (CvAccARConf::useGPUAccel) {
		detector->setInitialGPUCopyMsForBenchmark(CvAccARTools::getMsFromClocks(t1, t2));
	}
#endif
}

void CvAccARView::drawMarkers() {
	const map<int, CvAccARMarker> &markers = detector->getDetectedMarkers();

	const glm::mat4 &pMat = cam->getProjMat();

	for (map<int, CvAccARMarker>::const_iterator it = markers.begin();
		 it != markers.end();
		 ++it)
	{
		const CvAccARMarker &marker = it->second;
//		LOGINFO("CvAccARView: Drawing marker#%d", marker.getId());
		glm::mat4 mMat = marker.getPoseMat();
//		printFloatMat("pose mat", glm::value_ptr(mMat), 4, 4);
		glm::mat4 mvpMat = pMat * mMat;
		markerDisp->setMarkerColorFromId(marker.getId());
		markerDisp->setMVPMat(glm::value_ptr(mvpMat));
		markerDisp->render();
	}
}
