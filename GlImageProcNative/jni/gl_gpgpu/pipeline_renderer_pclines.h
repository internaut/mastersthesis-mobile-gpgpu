#ifndef GL_GPGPU_PIPELINE_RENDERER_PCLINES_H
#define GL_GPGPU_PIPELINE_RENDERER_PCLINES_H

#include "pipeline_renderer.h"

// The vertex shader fetches a pixel value from the *binary* input texture
// at "aTexCoord". OGLES 2.0 has no gl_VertexId, so we must pass the texture
// sampling coordinates one by one. When the binary pixel value is "1", it
// calculates the begin and end point of of the PC line in T and S space,
// depending on the submitted "aTSCoordType", respectively.
#define PCLINES_VSHADER "\
attribute vec2 aTexCoord;		// texture lookup coordinate \n\
attribute float aTSCoordType;   // type of PC line point: T/S space, begin or end. int needed, but attribs can only be floats, vecs or mats \n\
varying float vFragAddVal;		// value that will be added to the pixel in the fragment shader\n\
uniform sampler2D sTexture;		\n\
// Define the coordinate factors and summands for				\n\
// line beginnings and endings in T and S space, respectively.	\n\
// mat2x4 is not available in OGLES 2.0 :(						\n\
const mat4 tsCoordFactors = mat4(	\n\
     0, -1, 0, 0,  // t begin		\n\
     0,  1, 0, 0,  // t end			\n\
     0,  1, 0, 0,  // s begin		\n\
     0,  1, 0, 0   // s end			\n\
);									\n\
const mat4 tsCoordAdd = mat4(		\n\
    -1,  0, 0, 0,  // t begin		\n\
     0,  0, 0, 0,  // t end			\n\
     0,  0, 0, 0,  // s begin		\n\
     1,  0, 0, 0   // s end			\n\
);									\n\
// NOTE: Input texture must be binary image with format RGBA (R=G=B=Bin value)		\n\
// aTexCoord delivers a texture coordinate for each single pixel in the texture	\n\
void main() {	\n\
	// get texture sample (binary value)									\n\
    float bin = texture2DLod(sTexture, aTexCoord, 0.0).r;					\n\
    // get the provided index that selects a coordinate factor and summand	\n\
    // depending on: start or end point? T or S space?						\n\
    int idx = int(aTSCoordType);											\n\
    vec2 coordFact = tsCoordFactors[idx].xy;								\n\
    vec2 coordAdd = tsCoordAdd[idx].xy;										\n\
    // normalize the coordinates and create a flipped version				\n\
    vec2 normCoordStd = -1.0 * aTexCoord + 2.0 * aTexCoord;					\n\
    vec2 normCoordFlipped = normCoordStd.yx;								\n\
    vec2 finalCoord = vec2(0, 0);											\n\
	// select the standard or the flipped version							\n\
	// depending on the provided TS coord select index						\n\
    if (idx == 0 || idx == 3) {												\n\
        finalCoord = normCoordStd;											\n\
    } else {																\n\
        finalCoord = normCoordFlipped;										\n\
    }																		\n\
	// calculate one line point in ST space									\n\
    vec2 linePoint = bin * (coordFact * finalCoord + coordAdd);				\n\
    // set the value that will be added in the fragment shader				\n\
	vFragAddVal = bin * (1.0 / 256.0);										\n\
    // set the position of the line point									\n\
    gl_Position = vec4(linePoint, 0.0, 1.0);								\n\
}																			\n\
"

// The fragment shader adds a blended line color.
#define PCLINES_FSHADER "\
precision mediump float;	\n\
varying float vFragAddVal;	\n\
void main() {				\n\
    gl_FragColor = vec4(vFragAddVal);	\n\
}\n\
"

class PipelineRendererPCLines : public PipelineRenderer {
public:
	explicit PipelineRendererPCLines(FBO *fboObj = NULL, unsigned int texSamplFact = 1) : PipelineRenderer(fboObj) {
		texSamplingFactor = texSamplFact;
		texCoordBuf = tsCoordTypeBuf = NULL;
	}

	virtual ~PipelineRendererPCLines();

	virtual void loadShader();

	virtual void init(int w, int h);

	virtual void render();

protected:
	virtual void bindShader(Shader *sh);

private:
	GLint shParamATexCoord;
	GLint shParamATSCoordType;

	GLfloat *texCoordBuf;
	GLfloat *tsCoordTypeBuf;

	unsigned int numTexCoords;
	unsigned int numTexCoordsPairs;

	unsigned int texSamplingFactor;
};

#endif
