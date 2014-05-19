#ifndef CV_ACCAR_MARKERWARP_H
#define CV_ACCAR_MARKERWARP_H

#include "../pipelineproc_fbo.h"

#include "../../marker.h"

class CvAccARPipelineProcMarkerWarp : public CvAccARPipelineProcFBO {
public:
	explicit CvAccARPipelineProcMarkerWarp() : CvAccARPipelineProcFBO(MARKERWARP)
		{ vertexBuf = NULL; texCoordBuf = NULL; };

	virtual ~CvAccARPipelineProcMarkerWarp();

	virtual void render();

	virtual void bindShader(CvAccARShader *sh);

	virtual void initWithFrameSize(int texW, int texH, int pyrDownFact = 1);

	void prepareForMarkers(unsigned int markers);

	void addMarkerOriginCoords(Point2fVec coords);

	int getNonPOTOutFrameW() const { return nonPOTOutFrameW; };
	int getNonPOTOutFrameH() const { return nonPOTOutFrameH; };

private:
//	GLint shParamUWarpMat;
	GLint shParamAPos;
	GLint shParamATexCoord;

	GLfloat *vertexBuf; // [CV_ACCAR_QUAD_VERTEX_BUFSIZE];
	GLfloat *texCoordBuf; // [CV_ACCAR_QUAD_TEX_BUFSIZE];

	unsigned int numMarkers;
	unsigned int fixedNumMarkers;
	unsigned int lastAddedMarkerNum;

	unsigned int maxMarkersPerRow;
	unsigned int maxCellX;
	unsigned int maxCellY;

	unsigned int markerSize;

	float inFrameW;
	float inFrameH;

	int nonPOTOutFrameW;
	int nonPOTOutFrameH;
};

#endif
