#ifndef CV_ACCAR_PCLINES_H
#define CV_ACCAR_PCLINES_H

#include "../pipelineproc_fbo.h"

class CvAccARPipelineProcPCLines : public CvAccARPipelineProcFBO {
public:
	explicit CvAccARPipelineProcPCLines() : CvAccARPipelineProcFBO(PCLINES)
		{ texCoordBuf = NULL; tsCoordUsageIdxBuf = NULL; };

	virtual ~CvAccARPipelineProcPCLines();

	virtual void render();

	virtual void bindShader(CvAccARShader *sh);

	virtual void initWithFrameSize(int texW, int texH, int pyrDownFact = 1);

private:
	GLint shParamATexCoord;
	GLint shParamATSCoordUsageIdx;

	GLfloat *texCoordBuf;
	GLfloat *tsCoordUsageIdxBuf;
	unsigned int numTexCoords;
	unsigned int numTexCoordsPairs;
};

#endif
