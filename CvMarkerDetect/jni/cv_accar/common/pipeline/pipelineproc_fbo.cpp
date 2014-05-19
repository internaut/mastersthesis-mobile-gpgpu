#include "pipelineproc_fbo.h"

CvAccARPipelineProcFBO::~CvAccARPipelineProcFBO() {

}

void CvAccARPipelineProcFBO::initWithFrameSize(int texW, int texH, int pyrDownFact) {
	LOGINFO("CvAccARPipelineProcFBO %d: Init with frame size %dx%d", level, texW, texH);

	outFrameW = texW / pyrDownFact;
	outFrameH = texH / pyrDownFact;

	LOGINFO("CvAccARPipelineProcFBO %d: Output frame size %dx%d", level, outFrameW, outFrameH);
}

void CvAccARPipelineProcFBO::getResultData(unsigned char *data) {
	assert(fbo != NULL);

//	LOGINFO("CvAccARPipelineProcFBO %d: Reading buffer from FBO", level);
	fbo->readBuffer(data);
}

void CvAccARPipelineProcFBO::getResultDataRect(cv::Mat *resImg, cv::Rect rect) {
	assert(fbo != NULL);

	//	LOGINFO("CvAccARPipelineProcFBO %d: Reading partial buffer from FBO", level);
	fbo->readBufferRect(resImg, rect);
}
