#include "hist.h"

#include "../../tools.h"

CvAccARPipelineProcHist::~CvAccARPipelineProcHist() {
	if (histData) {
		delete histData;
	}

	if (texCoordBuf) {
		delete texCoordBuf;
	}

//	if (normAreaNumBuf) {
//		delete normAreaNumBuf;
//	}
}

void CvAccARPipelineProcHist::render() {
#ifdef BENCHMARK
	glFinish();
#endif

	shader->use();

	// render to FBO
	fbo->bind();

	// set the viewport
	glViewport(0, 0, outFrameW, outFrameH);

	glClear(GL_COLOR_BUFFER_BIT);

	// enabled blending
/*	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_BLEND);*/
	glDisable(GL_BLEND);

	// set uniforms
	glUniform2f(shParamUAreaDim, numAreaCols, numAreaRows);

	// set texture
	glBindTexture(GL_TEXTURE_2D, texId);	// bind input texture

	// set texture access coordinates
    glVertexAttribPointer(shParamATexCoord,
    					  CV_ACCAR_QUAD_TEXCOORDS_PER_VERTEX,
    					  GL_FLOAT,
    					  GL_FALSE,
    					  0,
    					  texCoordBuf);
    glEnableVertexAttribArray(shParamATexCoord);

	// draw
	glDrawArrays(GL_POINTS, 0, numTexSamples);

	// cleanup
	glDisableVertexAttribArray(shParamATexCoord);

	glDisable(GL_BLEND);

	fbo->unbind();

#ifdef BENCHMARK
	glFinish();
#endif
}

void CvAccARPipelineProcHist::bindShader(CvAccARShader *sh) {
	LOGINFO("CvAccARPipelineProcHist: Binding shader");
	CvAccARPipelineProc::bindShader(sh);

	// get shader parameter ids
	shParamATexCoord = shader->getParam(ATTR, "aTexCoord");
	shParamUAreaDim = shader->getParam(UNIF, "uAreaDim");

	/*LOGINFO("CvAccARPipelineProcHist: tex coords");
	for (int j = 0; j < CV_ACCAR_HIST_TEX_SAMPLE_COORDS - 1; j+=2) {
		LOGINFO("%f, %f", texCoordBuf[j], texCoordBuf[j + 1]);
	}*/
}

void CvAccARPipelineProcHist::initWithFrameSize(int texW, int texH, int pyrDownFact) {
	LOGINFO("CvAccARPipelineProcHist: Init with frame size %dx%d", texW, texH);

	inFrameW = texW;
	inFrameH = texH;
}

void CvAccARPipelineProcHist::prepareForAreas(unsigned int areas) {
	if (areas <= 0) return;

	bool numAreasChanged = numAreas != areas;

	numAreas = areas;

	numAreaRows = inFrameH / areaH;
	numAreaCols = inFrameW / areaW;

	LOGINFO("CvAccARPipelineProcHist: Prepared for %d areas. Area rows and cols: %f, %f", numAreas, numAreaRows, numAreaCols);

	if (numAreasChanged) {
		// set the output frame size to a fixed size of bins
		outFrameW = CV_ACCAR_HIST_BINS;
		outFrameH = CvAccARTools::getNextPOT(numAreas, 0, 8);

		// we need to recreate the FBO texture if its size has changed
		fboMgr->recreateFBOTex(level);

		// bind the output texture to the FBO
//		fbo->freeFBOBuffers();
		fbo->bindAttachedTex(outFrameW, outFrameH, false, false);	// dimensions NOT swapped, no grayscale, dont generate mipmaps

		LOGINFO("CvAccARPipelineProcHist: Output frame size changed to %dx%d", outFrameW, outFrameH);

		// recreate histogram data with new size
		if (histData) {
			delete histData;
		}

		histData = new unsigned char[outFrameW * outFrameH * 4];	// histData is returned as RGBA data, hence factor 4

		// regenerate texture access coordinates
		if (texCoordBuf) {
			delete texCoordBuf;
		}

		const int samplesPerRow = numAreaRows * numSamplesPerAreaRow;
		const int samplesPerCol = numAreaCols * numSamplesPerAreaRow;
		numTexSamples = samplesPerRow * samplesPerCol;
		LOGINFO("CvAccARPipelineProcHist: Generating %d tex sample coordinates (%dx%d)",
				numTexSamples, samplesPerCol, samplesPerRow);
		texCoordBuf = new GLfloat[samplesPerRow * samplesPerCol * 2];

		for (int sY = 0; sY < samplesPerRow; sY++) {
			const float texY = (float)sY / (float)samplesPerRow;

			for (int sX = 0; sX < samplesPerCol; sX++) {
				const int bufIdx = 2 * (sY * samplesPerCol + sX);

				texCoordBuf[bufIdx] = (float)sX / (float)samplesPerCol;
				texCoordBuf[bufIdx + 1] = texY;
				LOGINFO("CvAccARPipelineProcHist: texCoord[%d] = %f, %f", bufIdx, (float)sX / (float)samplesPerCol, texY);
			}
		}
	}
}

void CvAccARPipelineProcHist::getOtsuThresholds(int *threshs) {
	getResultData(histData);

	for (int i = 0; i < outFrameW * outFrameH * 4; i+=4) {
		if (histData[i] != 0) {
			LOGINFO("CvAccARPipelineProcHist: Found something at %d: %d", i, histData[i]);
		}
	}

	for (int area = 0; area < numAreas; area++) {
		int total = 0;
		int sum = 0;
		for (int i = 0; i < CV_ACCAR_HIST_BINS; i++) {
			int v = (int)histData[(area * CV_ACCAR_HIST_BINS + i) * 4];
			total += v;
			sum += i * v;
		}

		int sumB = 0;
		int wB = 0;
		int wF = 0;

		float varMax = 0;
		int threshold = 0;

		for (int t = 0; t < CV_ACCAR_HIST_BINS; t++) {
			int v = (int)histData[(area * CV_ACCAR_HIST_BINS + t) * 4];	// current bin value
			wB += v;               		// Weight Background
			if (wB == 0) continue;

			wF = total - wB;                 // Weight Foreground
			if (wF == 0) break;

			sumB += (t * v);

			float mB = (float)sumB / wB;            // Mean Background
			float mF = (float)(sum - sumB) / wF;    // Mean Foreground

			// Calculate Between Class Variance
			float varBetween = (float)wB * (float)wF * (mB - mF) * (mB - mF);

			// Check if new maximum found
			if (varBetween > varMax) {
				varMax = varBetween;
				threshold = t;
			}
		}

		threshs[area] = threshold;
	}
}
