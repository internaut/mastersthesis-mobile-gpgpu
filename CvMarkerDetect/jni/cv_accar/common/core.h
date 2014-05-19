#ifndef CV_ACCAR_CORE_H
#define CV_ACCAR_CORE_H

#include <vector>

#include "../cv_accar.h"

#include "shader.h"
#include "detect/detect.h"
#include "marker.h"
#include "view.h"
#include "cam.h"

#include "gl/fbo_mgr.h"

#include "pipeline/pipelineproc_fbo.h"

using namespace std;

class CvAccARView;

class CvAccARCore {
public:
	CvAccARCore(int camId, cv::Mat &camIntr);
	~CvAccARCore();

	void setDbgImage(const cv::Mat &dbgImg);
	void setDbgOutputLevel(int level);
	void setDbgMarkerShader(char *vshSrc, char *fshSrc);

	void postGLSetup();	// initializations after GL surface has been created
	void setOutputDisplayShader(char *vshSrc, char *fshSrc);// is post-gl, too
	void addShaderToPipeline(char *vshSrc, char *fshSrc);	// is post-gl, too

	void start();
	void stop();
	void pause();
	void resume();

	CvAccARView *getView() const { return view; };

	void getDetectedMarkers() const;

private:
	CvAccARCam *cam;
	CvAccARView *view;
	CvAccARDetectBase *detector;
	CvAccARDetectAccel *detectorAccel;
	CvAccARFBOMgr *fboMgr;
	cv::Mat *dbgStillImg;

	int lastAddedPipelineLevel;
	bool pipelineReady;
};

#endif
