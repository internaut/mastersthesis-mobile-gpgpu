#include "pipeline_renderer_thresh.h"

#include <string.h>
#include <cassert>

#include "../android_tools.h"

void PipelineRendererThresh::render() {
	// render to FBO
	fbo->bind();

	// clear and set viewport to output size
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, outTexW, outTexH);	// normal

	// use shader program
	shader->use();

	// set texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texId);

	// set uniforms
	glUniform1f(shParamUThresh, thresh);

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

void PipelineRendererThresh::loadShader() {
	assert(shader == NULL);

	const char *fShaderSrc = THRESH_FSHADER;
	const char *vShaderSrc = THRESH_VSHADER;

	Shader *sh = new Shader();
	if (sh->buildFromSrc(vShaderSrc, fShaderSrc)) {
		bindShader(sh); // will delete dispShader in deconstructor
		LOGINFO("PipelineRendererThresh: Created shader.");
	} else {
		delete sh;
		LOGERR("PipelineRendererThresh: Shader build failed.");
	}
}

void PipelineRendererThresh::bindShader(Shader *sh) {
	Renderer::bindShader(sh);

	// get shader parameter ids
	shParamAPos 		= shader->getParam(ATTR, "aPos");
	shParamATexCoord 	= shader->getParam(ATTR, "aTexCoord");
	shParamUThresh 		= shader->getParam(UNIF, "uThresh");

	// set geometry
	memcpy(vertexBuf, Renderer::quadVertices,
			QUAD_VERTEX_BUFSIZE * sizeof(GLfloat));

	// set texture coordinates
	memcpy(texCoordBuf, Renderer::quadTexCoordsStd,
		   QUAD_TEX_BUFSIZE * sizeof(GLfloat));

	LOGINFO("PipelineRendererThresh: Shader is bound.");
}

void PipelineRendererThresh::init(int w, int h) {
	assert(w > 0 && h > 0);
	PipelineRenderer::init(w, h);

	// create the output texture for the FBO
	fbo->createAttachedTex(outTexW, outTexH, false);

	LOGINFO("PipelineRendererThresh: init completed");
}
