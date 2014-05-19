#ifndef CV_ACCAR_DISP_H
#define CV_ACCAR_DISP_H

#include "pipelineproc.h"

class CvAccARDisp : public CvAccARPipelineProc {
public:
	explicit CvAccARDisp() : CvAccARPipelineProc(DISP) {}

	virtual void render();

	virtual void bindShader(CvAccARShader *sh);

private:
	GLint shParamAPos;
	GLint shParamATexCoord;
	GLfloat vertexBuf[CV_ACCAR_QUAD_VERTEX_BUFSIZE];
	GLfloat texCoordBuf[CV_ACCAR_QUAD_TEX_BUFSIZE];
};

#endif
