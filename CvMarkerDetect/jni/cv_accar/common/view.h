#ifndef CV_ACCAR_VIEW_H
#define CV_ACCAR_VIEW_H

#include <GLES2/gl2.h>

#include "../cv_accar.h"

#include <time.h>

#include "core.h"
#include "cam.h"
#include "detect/detect_base.h"
#include "pipeline/markerdisp.h"
#include "pipeline/pipelineproc.h"

#define CV_ACCAR_VIEW_CAM_TEX 0
#define CV_ACCAR_VIEW_OUT_TEX 1

class CvAccARCore;

class CvAccARView {
public:
	CvAccARView();
	~CvAccARView();

	void create();

	void resize(int w, int h);

	void draw();

	void setCore(CvAccARCore *c) { core = c; };
	void setDetector(CvAccARDetectBase *d) { detector = d; };
	void setCam(CvAccARCam *c) { cam = c; };
	void setDispRenderer(CvAccARPipelineProc *r) { dispRenderer = r; };
	void setMarkerDisp(CvAccARMarkerDisp *m);
	void enableMarkerDisp(bool enable) { markerDispEnabled = enable; };

private:
	// set input frame as texture,
	// create mipmaps
	void prepareInputFrame(const int w, const int h, unsigned char *pixels);

	void drawMarkers();

	CvAccARCore *core;
	CvAccARCam *cam;
	CvAccARDetectBase *detector;
	CvAccARPipelineProc *dispRenderer;
	CvAccARMarkerDisp *markerDisp;

	bool markerDispEnabled;

	int viewW;
	int viewH;
//	int frameW;
//	int frameH;

	GLuint texIds[2];
	GLuint dispTexId;

#ifdef PRINT_FPS
	clock_t fpsLastClock;
	int fpsAvgSamples;
	float fpsSum;
	float fpsCount;
#endif
};

#endif
