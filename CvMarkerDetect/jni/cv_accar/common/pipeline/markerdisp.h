#ifndef CV_ACCAR_MARKERDISP_H
#define CV_ACCAR_MARKERDISP_H

#include "../../cv_accar.h"
#include "pipelineproc.h"

//#define CV_ACCAR_MARKERDISP_CUBE

#ifdef CV_ACCAR_MARKERDISP_CUBE
#define CV_ACCAR_MARKERDISP_VERTICES 			36
#else
#define CV_ACCAR_MARKERDISP_VERTICES 			4
#endif

#define CV_ACCAR_MARKERDISP_COORDS_PER_VERTEX 	3
#define CV_ACCAR_MARKERDISP_VERT_BUF_SIZE 		(CV_ACCAR_MARKERDISP_VERTICES * CV_ACCAR_MARKERDISP_COORDS_PER_VERTEX)

/**
 * CvAccARMarkerDisp is not really a CvAccARPipelineProcProc but we use its features.
 */
class CvAccARMarkerDisp : public CvAccARPipelineProc {
public:
	explicit CvAccARMarkerDisp() : CvAccARPipelineProc(MARKERDISP) { markerScale = 1.0f; }

	void setMarkerColorFromId(int id);
	void setMarkerScale(float s);
	void setMVPMat(const GLfloat *mat);

	virtual void render();

	virtual void bindShader(CvAccARShader *sh);

private:
	glm::vec4 markerColor;
	GLfloat markerScale;
	glm::mat4 transformMat;
	GLfloat mvpMat[16];

	GLint shParamAPos;
	GLint shParamUTransformMat;
	GLint shParamUMVPMat;
	GLint shParamUColor;
	GLfloat vertexBuf[CV_ACCAR_MARKERDISP_VERT_BUF_SIZE];
};

#endif
