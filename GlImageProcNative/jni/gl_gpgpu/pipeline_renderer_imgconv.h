#ifndef GL_GPGPU_PIPELINE_RENDERER_IMGCONV_H
#define GL_GPGPU_PIPELINE_RENDERER_IMGCONV_H

#include "pipeline_renderer.h"
#include "disp_renderer.h"

#include "pipeline_renderer_imgconv_shaders_gauss.h"

#define IMG_CONV_VSHADER DISP_RENDERER_VSHADER

typedef enum {
	GAUSS_3X3 = 0,
	GAUSS_3X3_TWOPASS,
	GAUSS_5X5_TWOPASS,
	GAUSS_7X7_TWOPASS
} PipelineRendererImgConvType;

class PipelineRendererImgConv : public PipelineRenderer {
public:
	explicit PipelineRendererImgConv(FBO *fboObj = NULL, PipelineRendererImgConvType t = GAUSS_3X3, int renderPass = -1) : PipelineRenderer(fboObj)
		{ type = t; pass = renderPass; }

	virtual void loadShader();

	virtual void init(int w, int h);

	virtual void render();

protected:
	virtual void bindShader(Shader *sh);

private:
	PipelineRendererImgConvType type;
	int pass;	// render pass: if its a two-pass filter, it is 1 or 2, else -1

	GLint shParamAPos;
	GLint shParamATexCoord;
	GLint shParamUPxD;

	GLfloat vertexBuf[QUAD_VERTEX_BUFSIZE];
	GLfloat texCoordBuf[QUAD_TEX_BUFSIZE];

	float pxDx;
	float pxDy;
};

#endif
