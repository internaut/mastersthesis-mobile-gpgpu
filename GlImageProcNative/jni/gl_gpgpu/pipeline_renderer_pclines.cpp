#include "pipeline_renderer_pclines.h"

#include <cassert>
#include <cmath>

#include "../android_tools.h"

PipelineRendererPCLines::~PipelineRendererPCLines() {
	if (texCoordBuf) {
		delete texCoordBuf;
	}

	if (tsCoordTypeBuf) {
		delete tsCoordTypeBuf;
	}
}

void PipelineRendererPCLines::render() {
	shader->use();

	// render to FBO
	fbo->bind();

	// set the viewport
	glViewport(0, 0, outTexW, outTexH);

	glClear(GL_COLOR_BUFFER_BIT);

	// enable blending
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_BLEND);

	// set texture
	glBindTexture(GL_TEXTURE_2D, texId);	// bind input texture

	// set texture access coordinates
    glVertexAttribPointer(shParamATexCoord,
    					  2,	// x and y
    					  GL_FLOAT,
    					  GL_FALSE,
    					  0,
    					  texCoordBuf);
    glEnableVertexAttribArray(shParamATexCoord);

    // set S/T space coordinate params usage ids
    glVertexAttribPointer(shParamATSCoordType,
    					  1,
    					  GL_FLOAT,
    					  GL_FALSE,
    					  0,
    					  tsCoordTypeBuf);
    glEnableVertexAttribArray(shParamATSCoordType);

	// draw
	glDrawArrays(GL_LINES, 0, numTexCoordsPairs);

	// cleanup
	glDisableVertexAttribArray(shParamATexCoord);
	glDisableVertexAttribArray(shParamATSCoordType);

	glDisable(GL_BLEND);

	fbo->unbind();
}

void PipelineRendererPCLines::loadShader() {
	Shader *sh = new Shader();
	if (sh->buildFromSrc(PCLINES_VSHADER, PCLINES_FSHADER)) {
		bindShader(sh); // will delete dispShader in deconstructor
		LOGINFO("PipelineRendererImgConv: Created shader for PCLines");
	} else {
		delete sh;
		LOGERR("PipelineRendererImgConv: Shader build failed.");
	}
}

void PipelineRendererPCLines::init(int w, int h) {
	assert(w > 0 && h > 0);

	// set new size
	bool texSizeChanged = outTexW != w || outTexH != h;
	PipelineRenderer::init(w, h);

	// create the output texture for the FBO
	fbo->createAttachedTex(outTexW, outTexH, false);

	// create buffers
	if (!texCoordBuf || !tsCoordTypeBuf || texSizeChanged) {
		if (texCoordBuf) delete texCoordBuf;
		if (tsCoordTypeBuf) delete tsCoordTypeBuf;

		// Generate texture coordinates for N pixels in the texture,
		// depending on texSamplingFactor.
		// This is necessary for texture lookup in the vertex shader.

		numTexCoords = outTexW * outTexH * 8 / texSamplingFactor; // 8 because of: 2 dimensions (x, y) * 2 (line begin and end coord.) * 2 (for S and T space)
		numTexCoordsPairs = numTexCoords / 2;

		LOGINFO("PipelineRendererImgConv: Generating %d texture coordinates (sampling factor %d)", numTexCoordsPairs, texSamplingFactor);
		texCoordBuf = new GLfloat[numTexCoords];
		tsCoordTypeBuf = new GLfloat[numTexCoordsPairs];
		unsigned int bufIdx = 0;
		unsigned int buf2Idx = 0;

		for (int y = 0; y < outTexH; y+=texSamplingFactor) {
			for (int x = 0; x < outTexW; x+=texSamplingFactor) {
				const float texX = (float)x / (float)outTexW;
				const float texY = (float)y / (float)outTexH;

				// set coord. pair for begin and end point in T and S space
				for (int i = 0; i < 8; i+=2) {
					texCoordBuf[bufIdx] 	= texX;
					texCoordBuf[bufIdx + 1] = texY;

					tsCoordTypeBuf[buf2Idx] = (GLfloat)(floor(i / 2));

					bufIdx+=2;
					buf2Idx++;
				}
			}
		}
	}

	LOGINFO("PipelineRendererImgConv: init completed");
}

void PipelineRendererPCLines::bindShader(Shader *sh) {
	assert(texSamplingFactor > 0);

	Renderer::bindShader(sh);

	// get shader parameter ids
	shParamATexCoord 	= shader->getParam(ATTR, "aTexCoord");
	shParamATSCoordType = shader->getParam(ATTR, "aTSCoordType");

	LOGINFO("PipelineRendererImgConv: Shader is bound.");
}
