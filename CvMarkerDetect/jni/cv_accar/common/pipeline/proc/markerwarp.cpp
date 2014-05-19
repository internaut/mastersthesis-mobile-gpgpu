#include "markerwarp.h"

#include "../../tools.h"

#define NORM_VERT_COORD(c) (-1.0f + 2.0f * (c))

static inline void setVertBufCoord(GLfloat *buf, GLfloat x, GLfloat y) {
	buf[0] = x;
	buf[1] = y;
	buf[2] = 0.0f;
}

static inline void setTexBufCoord(GLfloat *buf, GLfloat x, GLfloat y) {
	buf[0] = x;
	buf[1] = y;
}


CvAccARPipelineProcMarkerWarp::~CvAccARPipelineProcMarkerWarp() {
	if (vertexBuf) {
		delete vertexBuf;
	}

	if (texCoordBuf) {
		delete texCoordBuf;
	}
}

void CvAccARPipelineProcMarkerWarp::render() {
	if (!vertexBuf || !texCoordBuf) return;

//	LOGINFO("CvAccARPipelineProcMarkerWarp: Drawing for %d markers", numMarkers);

/*	if (vertexBuf) {
		LOGINFO("CvAccARPipelineProcMarkerWarp: vert buf");
		for (int i = 0; i < CV_ACCAR_QUAD_VERTEX_BUFSIZE; i++) {
			LOGINFO("vertexBuf[%d] = %f", i, vertexBuf[i]);
		}
	}

	if (texCoordBuf) {
		LOGINFO("CvAccARPipelineProcMarkerWarp: tex buf");
		for (int i = 0; i < CV_ACCAR_QUAD_TEX_BUFSIZE; i++) {
			LOGINFO("texCoordBuf[%d] = %f", i, texCoordBuf[i]);
		}
	}*/

#ifdef BENCHMARK
	glFinish();
#endif

	shader->use();

	// render to FBO
	fbo->bind();

	// set the viewport
	glViewport(0, 0, outFrameW, outFrameH);

	glClear(GL_COLOR_BUFFER_BIT);

	// set input texture
	glBindTexture(GL_TEXTURE_2D, texId);	// bind input texture

	// we use GL_TRIANGLE_STRIP but we use it for
	// each quad separately because of non-continous texture coordinates

	int vertBufOffset = 0;
	int texBufOffset = 0;
	for (int i = 0; i < lastAddedMarkerNum; i++) {
		// set geometry
		glEnableVertexAttribArray(shParamAPos);
		glVertexAttribPointer(shParamAPos,
							  CV_ACCAR_QUAD_COORDS_PER_VERTEX,
							  GL_FLOAT,
							  GL_FALSE,
							  0,
							  vertexBuf + vertBufOffset);
		// set texture coords
		glVertexAttribPointer(shParamATexCoord,
							  CV_ACCAR_QUAD_TEXCOORDS_PER_VERTEX,
							  GL_FLOAT,
							  GL_FALSE,
							  0,
							  texCoordBuf + texBufOffset);
		glEnableVertexAttribArray(shParamATexCoord);

		// draw
		glDrawArrays(GL_TRIANGLE_STRIP, 0, CV_ACCAR_QUAD_VERTICES);

		// cleanup
		glDisableVertexAttribArray(shParamAPos);
		glDisableVertexAttribArray(shParamATexCoord);

		// advance buffer offsets
		vertBufOffset += CV_ACCAR_QUAD_VERTEX_BUFSIZE;
		texBufOffset += CV_ACCAR_QUAD_TEX_BUFSIZE;
	}

	fbo->unbind();

#ifdef BENCHMARK
	glFinish();
#endif
}

void CvAccARPipelineProcMarkerWarp::bindShader(CvAccARShader *sh) {
	LOGINFO("CvAccARPipelineProcMarkerWarp: Binding shader");
	CvAccARPipelineProc::bindShader(sh);

	// get shader parameter ids
	shParamAPos = shader->getParam(ATTR, "aPos");
	shParamATexCoord = shader->getParam(ATTR, "aTexCoord");
}

void CvAccARPipelineProcMarkerWarp::initWithFrameSize(int texW, int texH, int pyrDownFact) {
	LOGINFO("CvAccARPipelineProcMarkerWarp: Init with frame size %dx%d", texW, texH);

	inFrameW = texW;
	inFrameH = texH;

	markerSize = CvAccARConf::detectMarkerPxSize;

	if (CvAccARConf::gpuAccelFixedMarkerWarp <= 0) {
		outFrameW = outFrameH = 0;	// will be calculated in prepareForMarkers()
		fixedNumMarkers = 0;
	} else {
		outFrameW = CvAccARConf::gpuAccelFixedMarkerWarp;	// is fixed
		outFrameH = markerSize;

		fixedNumMarkers = outFrameW / markerSize;

		LOGINFO("CvAccARPipelineProcMarkerWarp: Set output frame size to fixed size %dx%d (%d markers)", outFrameW, outFrameH, fixedNumMarkers);

		fbo->bindAttachedTex(outFrameW, outFrameH, false, false);	// dimensions NOT swapped, no grayscale, dont generate mipmaps
	}

	// get maximum texture size that is supported by the device
	int maxTexSize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);

	assert(markerSize > 0 && maxTexSize >= markerSize && maxTexSize >= outFrameW);

	// calculate max. number of markers per row to fit in a maximum-sized output texture
	maxMarkersPerRow = maxTexSize / markerSize;

	LOGINFO("CvAccARPipelineProcMarkerWarp: Max. markers (%d px) per row in a %d texture is %d", markerSize, maxTexSize, maxMarkersPerRow);
}

void CvAccARPipelineProcMarkerWarp::prepareForMarkers(unsigned int markers) {
	assert(markers >= 0 && markers <= maxMarkersPerRow * maxMarkersPerRow);

	if (markers == 0) return;

	if (fixedNumMarkers > 0 && markers < fixedNumMarkers) {	// in case of CvAccARConf::gpuAccelFixedMarkerWarp > 0
		numMarkers = fixedNumMarkers;
	} else {
		numMarkers = markers;
	}

//	LOGINFO("CvAccARPipelineProcMarkerWarp: Preparing for %d markers", numMarkers);

	lastAddedMarkerNum = 0;

	// update output texture size
	maxCellY = (numMarkers / maxMarkersPerRow) + 1;
	maxCellX = (maxCellY == 1) ? numMarkers
							   : maxMarkersPerRow;

	int newOutFrameW, newOutFrameH;

	if (CvAccARConf::gpuAccelFixedMarkerWarp <= 0) {
		nonPOTOutFrameW = maxCellX * markerSize;
		nonPOTOutFrameH = maxCellY * markerSize;
		// newOutFrameW = CvAccARTools::getNextPOT(nonPOTOutFrameW, 6, 16);
		// newOutFrameH = CvAccARTools::getNextPOT(nonPOTOutFrameH, 6, 16);
		newOutFrameW = nonPOTOutFrameW;
		newOutFrameH = nonPOTOutFrameH;
	} else {
		newOutFrameW = outFrameW;
		newOutFrameH = outFrameH;

		nonPOTOutFrameW = maxCellX * markerSize;
		nonPOTOutFrameH = maxCellY * markerSize;

		if (nonPOTOutFrameW > outFrameW
		|| (nonPOTOutFrameW < outFrameW && nonPOTOutFrameW >= CvAccARConf::gpuAccelFixedMarkerWarp))
		{
			newOutFrameW = nonPOTOutFrameW;
		}

		if (nonPOTOutFrameH > outFrameH
		|| (nonPOTOutFrameH < outFrameH && nonPOTOutFrameH >= markerSize))
		{
			newOutFrameH = markerSize;
		}
	}

	if (outFrameW != newOutFrameW || outFrameH != newOutFrameH) {
		// we need to recreate the FBO texture if its size has changed
		fboMgr->recreateFBOTex(level);

		// set new sizes
		outFrameW = newOutFrameW;
		outFrameH = newOutFrameH;

		// bind the output texture to the FBO
//		fbo->freeFBOBuffers();
		fbo->bindAttachedTex(outFrameW, outFrameH, false, false);	// dimensions NOT swapped, no grayscale, dont generate mipmaps

		LOGINFO("CvAccARPipelineProcMarkerWarp: Output frame size changed");
	}

//	LOGINFO("CvAccARPipelineProcMarkerWarp: Prepared for %d markers, output texture dimensions = %dx%d, max cell = %d,%d",
//			markers, outFrameW, outFrameH, maxCellX, maxCellY);

	// create vertex buffer
	if (vertexBuf) delete vertexBuf;
	const int vertBufSize = numMarkers * CV_ACCAR_QUAD_VERTEX_BUFSIZE;
	vertexBuf = new GLfloat[vertBufSize];	// we use GL_TRIANGLE_STRIP but for each quad because of different texture coordinates
	memset(vertexBuf, 0, sizeof(GLfloat) * vertBufSize);

	// create texture coord. buffer
	if (texCoordBuf) delete texCoordBuf;
	const int texBufSize = numMarkers * CV_ACCAR_QUAD_TEX_BUFSIZE;
	texCoordBuf = new GLfloat[texBufSize];
	memset(texCoordBuf, 0, sizeof(GLfloat) * texBufSize);
}

void CvAccARPipelineProcMarkerWarp::addMarkerOriginCoords(Point2fVec coords) {
	if (lastAddedMarkerNum >= numMarkers) {
		LOGERR("CvAccARPipelineProcMarkerWarp: Only %d markers can be added. Marker coordinates discared.", numMarkers);
		return;
	}

//	LOGINFO("CvAccARPipelineProcMarkerWarp: Adding marker#%d origin coordinates", lastAddedMarkerNum);

	// generate vertex and texture coordinates
	unsigned int vertBufOffset = lastAddedMarkerNum * CV_ACCAR_QUAD_VERTEX_BUFSIZE;
	unsigned int texBufOffset = lastAddedMarkerNum * CV_ACCAR_QUAD_TEX_BUFSIZE;

	// coords contains 4 marker vertex coordinates (as absolute pixel coordinates)
	const cv::Point2f v0 = coords[0];
	const cv::Point2f v1 = coords[1];
	const cv::Point2f v2 = coords[2];
	const cv::Point2f v3 = coords[3];

	// calculate cell in which the marker will be rendered
	unsigned int cellX = lastAddedMarkerNum %  maxMarkersPerRow;
	unsigned int cellY = lastAddedMarkerNum /  maxMarkersPerRow;
	float vertXLeft  	= NORM_VERT_COORD((float)cellX 	     / (float)maxCellX);
	float vertXRight 	= NORM_VERT_COORD((float)(cellX + 1) / (float)maxCellX);
	float vertYBottom  	= NORM_VERT_COORD((float)cellY 	     / (float)maxCellY);
	float vertYTop  	= NORM_VERT_COORD((float)(cellY + 1) / (float)maxCellY);

/*	LOGINFO("CvAccARPipelineProcMarkerWarp: left %f, right %f, bottom %f, top %f",
			vertXLeft,
			vertXRight,
			vertYBottom,
			vertYTop);*/

	// vertex 1: bottom left
	setVertBufCoord(vertexBuf  + vertBufOffset    , vertXLeft , vertYBottom);
	setTexBufCoord(texCoordBuf + texBufOffset     , v0.x / inFrameW, v0.y / inFrameH);
	// vertex 2: bottom right
	setVertBufCoord(vertexBuf  + vertBufOffset + 3, vertXRight, vertYBottom);
	setTexBufCoord(texCoordBuf + texBufOffset  + 2, v1.x / inFrameW, v1.y / inFrameH);
	// vertex 3: top left
	setVertBufCoord(vertexBuf  + vertBufOffset + 6, vertXLeft , vertYTop);
	setTexBufCoord(texCoordBuf + texBufOffset  + 4, v3.x / inFrameW, v3.y / inFrameH);
	// vertex 4: top right
	setVertBufCoord(vertexBuf  + vertBufOffset + 9, vertXRight, vertYTop);
	setTexBufCoord(texCoordBuf + texBufOffset  + 6, v2.x / inFrameW, v2.y / inFrameH);

	// increment number of added markers
	lastAddedMarkerNum++;
}
