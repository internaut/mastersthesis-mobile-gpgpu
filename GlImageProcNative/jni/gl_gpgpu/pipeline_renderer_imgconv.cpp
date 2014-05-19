#include "pipeline_renderer_imgconv.h"

#include <string.h>
#include <cassert>

#include "../android_tools.h"

void PipelineRendererImgConv::render() {
//	LOGINFO("PipelineRendererImgConv: rendering with texId %d and shader program %d", texId, shader->getId());

	// render to FBO
	fbo->bind();

	// clear and set viewport to output size
	glClear(GL_COLOR_BUFFER_BIT);
	if (pass == 1) {	// two-pass filter, first pass
		glViewport(0, 0, outTexH, outTexW);	// swapped
//		LOGINFO("PipelineRendererImgConv: rendering with with viewport size %dx%d", outTexH, outTexW);
	} else {
		glViewport(0, 0, outTexW, outTexH);	// normal
//		LOGINFO("PipelineRendererImgConv: rendering with with viewport size %dx%d", outTexW, outTexH);
	}

	// use shader program
	shader->use();

	// set texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texId);

	// set uniforms
	if (type == GAUSS_3X3) {
		glUniform2f(shParamUPxD, pxDx, pxDy);
	}  else if (type == GAUSS_3X3_TWOPASS
			 || type == GAUSS_5X5_TWOPASS
			 || type == GAUSS_7X7_TWOPASS) {
		glUniform1f(shParamUPxD, pxDx);
	}

	// set geometry
	glEnableVertexAttribArray(shParamAPos);
	glVertexAttribPointer(shParamAPos,
						  QUAD_COORDS_PER_VERTEX,
						  GL_FLOAT,
						  GL_FALSE,
						  0,
						  vertexBuf);

    glVertexAttribPointer(shParamATexCoord,
    					  QUAD_TEXCOORDS_PER_VERTEX,
    					  GL_FLOAT,
    					  GL_FALSE,
    					  0,
    					  texCoordBuf);
    glEnableVertexAttribArray(shParamATexCoord);

	// draw
	glDrawArrays(GL_TRIANGLE_STRIP, 0, QUAD_VERTICES);

	// cleanup
	glDisableVertexAttribArray(shParamAPos);
	glDisableVertexAttribArray(shParamATexCoord);

	fbo->unbind();
}

void PipelineRendererImgConv::loadShader() {
	assert(shader == NULL);

	const char *fShaderSrc = NULL;
	const char *vShaderSrc = IMG_CONV_VSHADER;
	if (type == GAUSS_3X3) {
		fShaderSrc = IMG_CONV_FSHADER_GAUSS_3X3;
	} else if (type == GAUSS_3X3_TWOPASS) {
		fShaderSrc = IMG_CONV_FSHADER_GAUSS_3X1;
	} else if (type == GAUSS_5X5_TWOPASS) {
		fShaderSrc = IMG_CONV_FSHADER_GAUSS_5X1;
	} else if (type == GAUSS_7X7_TWOPASS) {
		fShaderSrc = IMG_CONV_FSHADER_GAUSS_7X1;
	} else {
		LOGERR("PipelineRendererImgConv: Unknown type %d", type);
		return;
	}

	Shader *sh = new Shader();
	if (vShaderSrc && fShaderSrc && sh->buildFromSrc(vShaderSrc, fShaderSrc)) {
		bindShader(sh); // will delete dispShader in deconstructor
		LOGINFO("PipelineRendererImgConv: Created shader for type %d", type);
	} else {
		delete sh;
		LOGERR("PipelineRendererImgConv: Shader build failed.");
	}
}

void PipelineRendererImgConv::bindShader(Shader *sh) {
	Renderer::bindShader(sh);

	// get shader parameter ids
	shParamAPos 		= shader->getParam(ATTR, "aPos");
	shParamATexCoord 	= shader->getParam(ATTR, "aTexCoord");
	shParamUPxD 		= shader->getParam(UNIF, "uPxD");

	// set geometry
	memcpy(vertexBuf, Renderer::quadVertices,
			QUAD_VERTEX_BUFSIZE * sizeof(GLfloat));

	// set texture coordinates
	if (type == GAUSS_3X3) {
		memcpy(texCoordBuf, Renderer::quadTexCoordsStd,
			   QUAD_TEX_BUFSIZE * sizeof(GLfloat));
	} else if (type == GAUSS_3X3_TWOPASS
			|| type == GAUSS_5X5_TWOPASS
		    || type == GAUSS_7X7_TWOPASS)
	{
		memcpy(texCoordBuf, Renderer::quadTexCoordsDiagonal,
			   QUAD_TEX_BUFSIZE * sizeof(GLfloat));
	} else {
		LOGERR("PipelineRendererImgConv: Unknown type %d", type);
		return;
	}

	LOGINFO("PipelineRendererImgConv: Shader is bound.");
}

void PipelineRendererImgConv::init(int w, int h) {
	assert(w > 0 && h > 0);
	PipelineRenderer::init(w, h);

	// calculate pixel delta values
	pxDx = 1.0f / (float)outTexW;
	pxDy = 1.0f / (float)outTexH;

	// create the output texture for the FBO
	fbo->createAttachedTex(outTexW, outTexH, false);

	LOGINFO("PipelineRendererImgConv: init completed, pxD = %f, %f.", pxDx, pxDy);
}
