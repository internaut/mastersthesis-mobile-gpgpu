#ifndef CV_ACCAR_HIST_H
#define CV_ACCAR_HIST_H

#include "../pipelineproc_fbo.h"

#define CV_ACCAR_HIST_BINS					256

class CvAccARPipelineProcHist : public CvAccARPipelineProcFBO {
public:
	explicit CvAccARPipelineProcHist() : CvAccARPipelineProcFBO(HIST) {
		texCoordBuf = NULL;
//		normAreaNumBuf = NULL;
		histData = NULL;
		numAreas = -1;
		numAreaCols = 0.0f;
		numAreaRows = 0.0f;
		areaW = 0;
		areaH = 0;
		numTexSamples = 0;
		numSamplesPerAreaRow = 8;
	};

	virtual ~CvAccARPipelineProcHist();

	virtual void render();

	virtual void bindShader(CvAccARShader *sh);

	virtual void initWithFrameSize(int texW, int texH, int pyrDownFact = 1);

	void setAreaSize(float aW, float aH) { areaW = aW; areaH = aH; };
	void setNumSamplesPerAreaRow(unsigned int s) { numSamplesPerAreaRow = s; };

	void prepareForAreas(unsigned int areas);

	void getOtsuThresholds(int *threshs);

private:
	GLint shParamATexCoord;
	GLint shParamUAreaDim;

	GLfloat *texCoordBuf;
//	GLfloat *normAreaNumBuf;

	unsigned int numSamplesPerAreaRow;
	unsigned int numTexSamples;

	unsigned int numAreas;
	float numAreaCols;
	float numAreaRows;
//	float normAreaW;
//	float normAreaH;

	float areaW;	// in px
	float areaH;	// in px

	float inFrameW;
	float inFrameH;

	unsigned char *histData;
};

#endif
