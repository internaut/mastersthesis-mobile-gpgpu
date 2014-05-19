#ifndef GL_GPGPU_PIPELINE_RENDERER_THRESH_H
#define GL_GPGPU_PIPELINE_RENDERER_THRESH_H

#include "pipeline_renderer.h"
#include "disp_renderer.h"

#define THRESH_VSHADER DISP_RENDERER_VSHADER

#define THRESH_FSHADER "\
precision mediump float;	\n\
varying vec2 vTexCoord;		\n\
uniform float uThresh;		\n\
uniform sampler2D sTexture;	\n\
void main() {				\n\
    float gray = texture2D(sTexture, vTexCoord).r;	\n\
    float bin = step(uThresh, gray);				\n\
    gl_FragColor = vec4(bin, bin, bin, 1.0);		\n\
}\n\
"

class PipelineRendererThresh : public PipelineRenderer {
public:
	explicit PipelineRendererThresh(FBO *fboObj = NULL, float threshVal = 0.5f) : PipelineRenderer(fboObj)
		{ thresh = threshVal; }

	virtual void loadShader();

	virtual void init(int w, int h);

	virtual void render();

	void setThreshold(float t) { thresh = t; }
	float getThreshold() const { return thresh; }

protected:
	virtual void bindShader(Shader *sh);

private:
	GLint shParamAPos;
	GLint shParamATexCoord;
	GLint shParamUThresh;

	GLfloat vertexBuf[QUAD_VERTEX_BUFSIZE];
	GLfloat texCoordBuf[QUAD_TEX_BUFSIZE];

	float thresh;
};

#endif
