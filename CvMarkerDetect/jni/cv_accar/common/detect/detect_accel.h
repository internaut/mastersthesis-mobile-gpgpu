#ifndef CV_ACCAR_DETECT_ACCEL_H
#define CV_ACCAR_DETECT_ACCEL_H

#include "detect_base.h"

#include "../pipeline/pipelineproc_fbo.h"
#include "../gl/fbo_mgr.h"

class CvAccARDetectAccel : public CvAccARDetectBase {
public:
	explicit CvAccARDetectAccel(CvAccARCam *useCam);

	virtual ~CvAccARDetectAccel();

	bool setPipelineProcWithShader(CvAccARPipelineProcLevel pipelineLvl, CvAccARShader *shader);

	void initPipeline(CvAccARFBOMgr *fboMgr);

	virtual void setInputFrame(cv::Mat *frame) { /* not implemented */ };
	virtual void setInputFrame(unsigned int id) { inputTexId = id; };
	virtual unsigned int getOutputFrameHandle() { return outputTexId; };

	virtual void processFrame();

private:
	void preprocessAccel();
	void performThresholdAccel();
	void detectMarkersAccel();

	CvAccARPipelineProcFBO *pipelineProcessors[CV_ACCAR_PIPELINE_LEVELS];	// array of pointers to CvAccARPipelineProc objects (strong ref!)

	GLuint inputTexId;
	GLuint outputTexId;
};

#endif
