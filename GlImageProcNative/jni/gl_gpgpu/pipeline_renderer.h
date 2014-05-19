#ifndef GL_GPGPU_PIPELINE_RENDERER_H
#define GL_GPGPU_PIPELINE_RENDERER_H

#include "renderer.h"
#include "fbo.h"

class PipelineRenderer : public Renderer {
public:
	explicit PipelineRenderer(FBO *fboObj = NULL) : Renderer()
		{ fbo = fboObj; }

	void setFBO(FBO *fboObj) { fbo = fboObj; }
	FBO *getFBO() const { return fbo; }

	virtual void init(int w, int h);

	int getOutTexW() const { return outTexW; }
	int getOutTexH() const { return outTexH; }

protected:
	FBO *fbo;	// weak ref.
	int outTexW;
	int outTexH;
};

#endif
