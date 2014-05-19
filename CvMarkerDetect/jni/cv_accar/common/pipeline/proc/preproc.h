#ifndef CV_ACCAR_PREPROC_H
#define CV_ACCAR_PREPROC_H

#include "../pipelineproc_fbo.h"

/**
 * Class that implements simple preprocessing: Scaling + grayscale conversion
 */
class CvAccARPipelineProcPreproc : public CvAccARPipelineProcFBO {
public:
	explicit CvAccARPipelineProcPreproc(CvAccARPipelineProcLevel lvl) : CvAccARPipelineProcFBO(lvl)
		{ };

	virtual void render();

	virtual void bindShader(CvAccARShader *sh);

	virtual void initWithFrameSize(int texW, int texH, int pyrDownFact = 1);

private:
	GLint shParamAPos;
	GLint shParamATexCoord;
	GLfloat vertexBuf[CV_ACCAR_QUAD_VERTEX_BUFSIZE];
	GLfloat texCoordBuf[CV_ACCAR_QUAD_TEX_BUFSIZE];
};

#endif
