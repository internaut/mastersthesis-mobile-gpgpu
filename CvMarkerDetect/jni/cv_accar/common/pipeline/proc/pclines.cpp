#include "pclines.h"

CvAccARPipelineProcPCLines::~CvAccARPipelineProcPCLines() {
	if (texCoordBuf) {
		delete texCoordBuf;
	}

	if (tsCoordUsageIdxBuf) {
		delete tsCoordUsageIdxBuf;
	}
}

void CvAccARPipelineProcPCLines::render() {
#ifdef BENCHMARK
	glFinish();
#endif

	shader->use();

	// render to FBO
	fbo->bind();

	// set the viewport
	glViewport(0, 0, outFrameW, outFrameH);

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
    glVertexAttribPointer(shParamATSCoordUsageIdx,
    					  1,
    					  GL_FLOAT,
    					  GL_FALSE,
    					  0,
    					  tsCoordUsageIdxBuf);
    glEnableVertexAttribArray(shParamATSCoordUsageIdx);

	// draw
	glDrawArrays(GL_LINES, 0, numTexCoordsPairs);

	// cleanup
	glDisableVertexAttribArray(shParamATexCoord);
	glDisableVertexAttribArray(shParamATSCoordUsageIdx);

	glDisable(GL_BLEND);

	fbo->unbind();

#ifdef BENCHMARK
	glFinish();
#endif
}

void CvAccARPipelineProcPCLines::bindShader(CvAccARShader *sh) {
	LOGINFO("CvAccARPipelineProcHist: Binding shader");
	CvAccARPipelineProc::bindShader(sh);

	// get shader parameter ids
	shParamATexCoord = shader->getParam(ATTR, "aTexCoord");
	shParamATSCoordUsageIdx = shader->getParam(ATTR, "aTSCoordUsageIdx");
}

void CvAccARPipelineProcPCLines::initWithFrameSize(int texW, int texH, int pyrDownFact) {
	CvAccARPipelineProcFBO::initWithFrameSize(texW, texH, pyrDownFact);

	// bind the output texture to the FBO
	fbo->bindAttachedTex(outFrameW, outFrameH, false, false);

	if (!texCoordBuf) {
		// generate texture coordinates for each single pixel in the texture
		// this is necessary for texture lookup in the vertex shader
		const int samplingFactor = 2;

		numTexCoords = outFrameW * outFrameH * 8 / samplingFactor; // 8 because of: 2 dimensions (x, y) * 2 (line begin and end coord.) * 2 (for S and T space)
		numTexCoordsPairs = numTexCoords / 2;
		LOGINFO("CvAccARPipelineProcPCLines: Generating %d texture coordinates (sampling factor %d)", numTexCoordsPairs, samplingFactor);
		texCoordBuf = new GLfloat[numTexCoords];
		tsCoordUsageIdxBuf = new GLfloat[numTexCoordsPairs];
		unsigned int bufIdx = 0;
		unsigned int buf2Idx = 0;
		for (int y = 0; y < outFrameH; y+=samplingFactor) {
			for (int x = 0; x < outFrameW; x+=samplingFactor) {
				const float texX = (float)x / (float)outFrameW;
				const float texY = (float)y / (float)outFrameH;
//				LOGINFO("CvAccARPipelineProcPCLines: coordinate for buf[%d] = %f, %f", bufIdx, texX, texY);

				for (int i = 0; i < 8; i+=2) {
					texCoordBuf[bufIdx] 	= texX;
					texCoordBuf[bufIdx + 1] = texY;

					tsCoordUsageIdxBuf[buf2Idx] = (GLfloat)(floor(i / 2));

					bufIdx+=2;
					buf2Idx++;
				}
			}
		}
	}
}
