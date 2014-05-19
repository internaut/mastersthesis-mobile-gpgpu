#include "disp.h"

#include <string.h>

void CvAccARDisp::render() {
	shader->use();

	// set geometry
	glEnableVertexAttribArray(shParamAPos);
	glVertexAttribPointer(shParamAPos,
						  CV_ACCAR_QUAD_COORDS_PER_VERTEX,
						  GL_FLOAT,
						  GL_FALSE,
						  0,
						  vertexBuf);
	// set texture
	glBindTexture(GL_TEXTURE_2D, texId);

    glVertexAttribPointer(shParamATexCoord,
    					  CV_ACCAR_QUAD_TEXCOORDS_PER_VERTEX,
    					  GL_FLOAT,
    					  GL_FALSE,
    					  0,
    					  texCoordBuf);
    glEnableVertexAttribArray(shParamATexCoord);

	// draw
	glDrawArrays(GL_TRIANGLE_STRIP, 0, CV_ACCAR_QUAD_VERTICES);

	// cleanup
	glDisableVertexAttribArray(shParamAPos);
	glDisableVertexAttribArray(shParamATexCoord);
}

void CvAccARDisp::bindShader(CvAccARShader *sh) {
	CvAccARPipelineProc::bindShader(sh);

	// get shader parameter ids
	shParamAPos = shader->getParam(ATTR, "aPos");
	shParamATexCoord = shader->getParam(ATTR, "aTexCoord");

	// set geometry
	memcpy(vertexBuf, CvAccARPipelineProc::quadVertices,
			CV_ACCAR_QUAD_VERTEX_BUFSIZE * sizeof(GLfloat));

	// set texture coordinates
	memcpy(texCoordBuf, CvAccARPipelineProc::quadTexCoordsFlipped,
			CV_ACCAR_QUAD_TEX_BUFSIZE * sizeof(GLfloat));
}
