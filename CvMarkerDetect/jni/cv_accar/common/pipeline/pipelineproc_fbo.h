#ifndef CV_ACCAR_PIPELINEPROC_FBO_H
#define CV_ACCAR_PIPELINEPROC_FBO_H

#include "pipelineproc.h"
#include "../gl/fbo.h"
#include "../gl/fbo_mgr.h"

class CvAccARPipelineProcFBO : public CvAccARPipelineProc {
public:
	explicit CvAccARPipelineProcFBO(CvAccARPipelineProcLevel lvl)
	         : CvAccARPipelineProc(lvl)
	 	 { fboMgr = NULL; fbo = NULL; };

	virtual ~CvAccARPipelineProcFBO();

	void setFBOMgr(CvAccARFBOMgr *mgr) { fboMgr = mgr; };

	void bindFBO(CvAccARFBO *fboPtr) { fbo = fboPtr; };
	int getFBOTexId() const { return fbo->getAttachedTexId(); };

	virtual void getResultData(unsigned char *data);
	virtual void getResultDataRect(cv::Mat *resImg, cv::Rect rect);

	virtual void initWithFrameSize(int texW, int texH, int pyrDownFact = 1);

	int getOutFrameWidth() const { return outFrameW; };
	int getOutFrameHeight() const { return outFrameH; };

protected:
	CvAccARFBOMgr *fboMgr;	// weak ref.!
	CvAccARFBO *fbo;		// weak ref.!

	int outFrameW;
	int outFrameH;
};

#endif
