#include "thresh.h"

void CvAccARPipelineProcThresh::render() {
#ifdef BENCHMARK
	glFinish();
#endif

	shader->use();

	// render to FBO
	fbo->bind();

	// set the viewport
	if (level == ATHRESH_1) {
		glViewport(0, 0, outFrameH, outFrameW);	// swapped
	} else {
		glViewport(0, 0, outFrameW, outFrameH);
	}

	glClear(GL_COLOR_BUFFER_BIT);

	// set uniforms
	if (level == THRESH) {
		glUniform1f(shParamUThresh, thresh);		// thresholding value for simple thresholding
	} else {
		glUniform2f(shParamUPxD, pxDx, pxDy);	// texture pixel delta values
	}

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

void CvAccARPipelineProcThresh::bindShader(CvAccARShader *sh) {
	LOGINFO("CvAccARPipelineProcThresh: Binding shader");
	CvAccARPipelineProc::bindShader(sh);

	// get shader parameter ids
	shParamAPos = shader->getParam(ATTR, "aPos");
	shParamATexCoord = shader->getParam(ATTR, "aTexCoord");
	if (level == THRESH) {
		shParamUThresh = shader->getParam(UNIF, "uThresh");
	} else {
		shParamUPxD = shader->getParam(UNIF, "uPxD");
	}

	// set geometry
	memcpy(vertexBuf, CvAccARPipelineProc::quadVertices,
			CV_ACCAR_QUAD_VERTEX_BUFSIZE * sizeof(GLfloat));

	// set texture coordinates
	if (level == THRESH) {
		memcpy(texCoordBuf, CvAccARPipelineProc::quadTexCoordsStd,
			   CV_ACCAR_QUAD_TEX_BUFSIZE * sizeof(GLfloat));
	} else {
		memcpy(texCoordBuf, CvAccARPipelineProc::quadTexCoordsDiagonal,
			   CV_ACCAR_QUAD_TEX_BUFSIZE * sizeof(GLfloat));
	}
}

void CvAccARPipelineProcThresh::initWithFrameSize(int texW, int texH, int pyrDownFact) {
	CvAccARPipelineProcFBO::initWithFrameSize(texW, texH, pyrDownFact);

	// calculate pixel delta values
	pxDx = 1.0f / (float)outFrameW;
	pxDy = 1.0f / (float)outFrameH;

	// bind the output texture to the FBO
	if (level == ATHRESH_1) {
		fbo->bindAttachedTex(outFrameH, outFrameW, false, true);	// dimensions swapped, no grayscale, generate mipmaps
	} else {
		fbo->bindAttachedTex(outFrameW, outFrameH, false, true);	// dimensions NOT swapped, no grayscale, generate mipmaps
	}
}
