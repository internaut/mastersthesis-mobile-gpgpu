#include "disp_renderer.h"

#include <string.h>
#include <cassert>

#include "../android_tools.h"


void DispRenderer::render() {
	LOGINFO("DispRenderer: rendering with texId %d and shader program %d", texId, shader->getId());

	shader->use();
//	checkGLError("DispRenderer: render 0");

	// set texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texId);
//	checkGLError("DispRenderer: render 1");

	// set geometry
	glEnableVertexAttribArray(shParamAPos);
	glVertexAttribPointer(shParamAPos,
						  QUAD_COORDS_PER_VERTEX,
						  GL_FLOAT,
						  GL_FALSE,
						  0,
						  vertexBuf);
//	checkGLError("DispRenderer: render 2");

    glVertexAttribPointer(shParamATexCoord,
    					  QUAD_TEXCOORDS_PER_VERTEX,
    					  GL_FLOAT,
    					  GL_FALSE,
    					  0,
    					  texCoordBuf);
    glEnableVertexAttribArray(shParamATexCoord);
//    checkGLError("DispRenderer: render 3");

	// draw
	glDrawArrays(GL_TRIANGLE_STRIP, 0, QUAD_VERTICES);
//	checkGLError("DispRenderer: render 4");

	// cleanup
	glDisableVertexAttribArray(shParamAPos);
	glDisableVertexAttribArray(shParamATexCoord);
}

void DispRenderer::loadShader() {
	assert(shader == NULL);

	Shader *sh = new Shader();
	if (sh->buildFromSrc(DISP_RENDERER_VSHADER, DISP_RENDERER_FSHADER)) {
		bindShader(sh); // will delete dispShader in deconstructor
		LOGINFO("DispRenderer: Created shader");
	} else {
		delete sh;
		LOGERR("DispRenderer: Shader build failed.");
	}
}

void DispRenderer::bindShader(Shader *sh) {
	Renderer::bindShader(sh);

	// get shader parameter ids
	shParamAPos = shader->getParam(ATTR, "aPos");
	shParamATexCoord = shader->getParam(ATTR, "aTexCoord");

	// set geometry
	memcpy(vertexBuf, Renderer::quadVertices,
			QUAD_VERTEX_BUFSIZE * sizeof(GLfloat));

	// set texture coordinates
	memcpy(texCoordBuf, Renderer::quadTexCoordsFlipped,
			QUAD_TEX_BUFSIZE * sizeof(GLfloat));

	LOGINFO("DispRenderer: Shader is bound.");
}
