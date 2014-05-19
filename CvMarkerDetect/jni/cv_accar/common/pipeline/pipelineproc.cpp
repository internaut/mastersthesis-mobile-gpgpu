#include "pipelineproc.h"

const GLfloat CvAccARPipelineProc::quadTexCoordsStd[] = { 0, 0,
													  1, 0,
													  0, 1,
													  1, 1 };

const GLfloat CvAccARPipelineProc::quadTexCoordsFlipped[] = { 0, 1,
														  1, 1,
														  0, 0,
														  1, 0 };

const GLfloat CvAccARPipelineProc::quadTexCoordsDiagonal[] = { 0, 0,
														   0, 1,
														   1, 0,
														   1, 1 };

const GLfloat CvAccARPipelineProc::quadVertices[] = { -1, -1, 0,
												   1, -1, 0,
												  -1,  1, 0,
												   1,  1, 0 };

CvAccARPipelineProc::CvAccARPipelineProc(CvAccARPipelineProcLevel lvl) {
	level = lvl;
	texId = 0;
	shader = NULL;
}

CvAccARPipelineProc::~CvAccARPipelineProc() {
	if (shader) {
		delete shader;
	}
}

void CvAccARPipelineProc::bindShader(CvAccARShader *sh) {
	shader = sh;
}
