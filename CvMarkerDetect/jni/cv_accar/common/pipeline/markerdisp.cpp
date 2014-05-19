#include "markerdisp.h"

#include <string.h>


#ifdef CV_ACCAR_MARKERDISP_CUBE
static const GLfloat markerVertices[] = {
    -1.0f,-1.0f,-1.0f, // triangle 1 : begin
    -1.0f,-1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f, // triangle 1 : end
    1.0f, 1.0f,-1.0f, // triangle 2 : begin
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f, // triangle 2 : end
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f
};
#else
static const GLfloat markerVertices[] = {
						   -1, -1, 0,
							1, -1, 0,
						   -1,  1, 0,
						    1,  1, 0
};
#endif

void CvAccARMarkerDisp::setMarkerColorFromId(int id) {
	markerColor = glm::vec4(
		id / 255.0f,
		(255.0f - id) / 255.0f,
		((127 + id) % 256) / 255.0f,
		1.0f);

//	LOGINFO("CvAccARMarkerDisp: Marker of id %d got color %f, %f, %f, %f",
//			id, markerColor[0], markerColor[1], markerColor[2], markerColor[3]);
}

void CvAccARMarkerDisp::setMVPMat(const GLfloat *mat) {
	memcpy(mvpMat, mat, 16 * sizeof(mat[0]));
}

void CvAccARMarkerDisp::setMarkerScale(float s) {
	LOGINFO("CvAccARMarkerDisp: Setting marker scale to %f", s);
	markerScale = s;
	transformMat = glm::scale(glm::mat4(1.0f), glm::vec3(markerScale * 0.5f));
}

void CvAccARMarkerDisp::render() {
//	if (!mvpMat) return;

	shader->use();

	// set modelview mat
	glUniformMatrix4fv(shParamUMVPMat, 1, false, mvpMat);
	glUniformMatrix4fv(shParamUTransformMat, 1, false, glm::value_ptr(transformMat));
	glUniform4fv(shParamUColor, 1, glm::value_ptr(markerColor));

	// set geometry
	glEnableVertexAttribArray(shParamAPos);
	glVertexAttribPointer(shParamAPos,
						  CV_ACCAR_MARKERDISP_COORDS_PER_VERTEX,
						  GL_FLOAT,
						  GL_FALSE,
						  0,
						  vertexBuf);

	// draw
	glDrawArrays(GL_TRIANGLE_STRIP, 0, CV_ACCAR_MARKERDISP_VERTICES);

	// cleanup
	glDisableVertexAttribArray(shParamAPos);
}

void CvAccARMarkerDisp::bindShader(CvAccARShader *sh) {
	CvAccARPipelineProc::bindShader(sh);

	// get shader parameter ids
	shParamAPos 			= shader->getParam(ATTR, "aPos");
	shParamUTransformMat 	= shader->getParam(UNIF, "uTransformMat");
	shParamUMVPMat 			= shader->getParam(UNIF, "uMVPMat");
	shParamUColor 			= shader->getParam(UNIF, "uColor");

	// set geometry
	memcpy(vertexBuf, markerVertices,
			CV_ACCAR_MARKERDISP_VERT_BUF_SIZE * sizeof(GLfloat));
}
