#include "preproc.h"

void CvAccARPipelineProcPreproc::render() {
#ifdef BENCHMARK
	glFinish();
#endif

	shader->use();

	// render to FBO
	fbo->bind();

	// set the viewport
	glViewport(0, 0, outFrameW, outFrameH);

	glClear(GL_COLOR_BUFFER_BIT);

	// set geometry
	glEnableVertexAttribArray(shParamAPos);
	glVertexAttribPointer(shParamAPos,
						  CV_ACCAR_QUAD_COORDS_PER_VERTEX,
						  GL_FLOAT,
						  GL_FALSE,
						  0,
						  vertexBuf);
	// set texture
	glBindTexture(GL_TEXTURE_2D, texId);	// bind input texture

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

	fbo->unbind();

#ifdef BENCHMARK
	glFinish();
#endif
}

void CvAccARPipelineProcPreproc::bindShader(CvAccARShader *sh) {
	LOGINFO("CvAccARPipelineProcPreproc: Binding shader");
	CvAccARPipelineProc::bindShader(sh);

	// get shader parameter ids
	shParamAPos = shader->getParam(ATTR, "aPos");
	shParamATexCoord = shader->getParam(ATTR, "aTexCoord");

	// set geometry
	memcpy(vertexBuf, CvAccARPipelineProc::quadVertices,
			CV_ACCAR_QUAD_VERTEX_BUFSIZE * sizeof(GLfloat));

	// set texture coordinates
	memcpy(texCoordBuf, CvAccARPipelineProc::quadTexCoordsStd,
		   CV_ACCAR_QUAD_TEX_BUFSIZE * sizeof(GLfloat));
}

void CvAccARPipelineProcPreproc::initWithFrameSize(int texW, int texH, int pyrDownFact) {
	CvAccARPipelineProcFBO::initWithFrameSize(texW, texH, pyrDownFact);

	fbo->bindAttachedTex(outFrameW, outFrameH, false, true);	// dimensions NOT swapped, no grayscale, generate mipmaps
}
