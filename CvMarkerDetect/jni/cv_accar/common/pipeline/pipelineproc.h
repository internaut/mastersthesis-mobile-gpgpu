#ifndef CV_ACCAR_PIPELINEPROC_H
#define CV_ACCAR_PIPELINEPROC_H

#include "../shader.h"
#include "../gl/fbo.h"

#define CV_ACCAR_QUAD_VERTICES 				4
#define CV_ACCAR_QUAD_COORDS_PER_VERTEX 	3
#define CV_ACCAR_QUAD_TEXCOORDS_PER_VERTEX 	2
#define CV_ACCAR_QUAD_VERTEX_BUFSIZE 		(CV_ACCAR_QUAD_VERTICES * CV_ACCAR_QUAD_COORDS_PER_VERTEX)
#define CV_ACCAR_QUAD_TEX_BUFSIZE 			(CV_ACCAR_QUAD_VERTICES * CV_ACCAR_QUAD_TEXCOORDS_PER_VERTEX)

typedef enum {
	NONE = -3,
	// display processors
	MARKERDISP,	// (debug) marker display
	DISP,		// camera frame display
	// these are "real" processors starting here:
	PREPROC,
	ATHRESH_1,	// adapt. thresholding - first pass
	ATHRESH_2,	// adapt. thresholding - second pass
	MARKERWARP,	// warp marker image
//	HIST,		// histogram generation
	THRESH		// simple thresholding
//	PCLINES		// parallel coordinate space lines for hough transform
} CvAccARPipelineProcLevel;

#define CV_ACCAR_PIPELINE_LEVELS 4	// not 7 because PCLINES is disabled

/**
 * Class that defines a GPU pipeline processor.
 */
class CvAccARPipelineProc {
public:
	explicit CvAccARPipelineProc(CvAccARPipelineProcLevel lvl);
	virtual ~CvAccARPipelineProc();

	virtual void render() = 0;
	virtual void bindShader(CvAccARShader *sh);

	void useTexture(GLuint id) { texId = id; };

	int getPipelineLevelNum() const { return (int)level; };

protected:
	static const GLfloat quadTexCoordsStd[];
	static const GLfloat quadTexCoordsFlipped[];
	static const GLfloat quadTexCoordsDiagonal[];
	static const GLfloat quadVertices[];

	CvAccARShader *shader;	// strong ref.!
	CvAccARPipelineProcLevel level;
	GLuint texId;
};

#endif
