#ifndef CV_ACCAR_THRESH_H
#define CV_ACCAR_THRESH_H

#include "../pipelineproc_fbo.h"

/**
 * Class that implements two kinds of thresholding to generate binary images:
 * 1. simple thresholding via a fixed threshold <thresh> (level = THRESH)
 * 2. 2-pass adaptive thresholding (level = ATHRESH_1 and ATHRESH_2)
 */
class CvAccARPipelineProcThresh : public CvAccARPipelineProcFBO {
public:
	explicit CvAccARPipelineProcThresh(CvAccARPipelineProcLevel lvl) : CvAccARPipelineProcFBO(lvl)
		{ pxDx = 0.0f; pxDy = 0.0f; };

	virtual void render();

	virtual void bindShader(CvAccARShader *sh);

	virtual void initWithFrameSize(int texW, int texH, int pyrDownFact = 1);

	void setThreshold(int t) { thresh = (float)t  / 256.0f; };

private:
	GLint shParamAPos;
	GLint shParamATexCoord;
	GLint shParamUPxD;		// only used for adapt. thresholding
	GLint shParamUThresh;	// only used for simple thresholding
	GLfloat vertexBuf[CV_ACCAR_QUAD_VERTEX_BUFSIZE];
	GLfloat texCoordBuf[CV_ACCAR_QUAD_TEX_BUFSIZE];

	float pxDx;	// pixel delta value for texture access. only used for adapt. thresholding
	float pxDy;	// pixel delta value for texture access. only used for adapt. thresholding

	float thresh;	// only used for simple thresholding
};

#endif
