#ifndef GL_GPGPU_PIPELINE_H
#define GL_GPGPU_PIPELINE_H

#include <GLES2/gl2.h>

#include "fbo.h"
#include "pipeline_renderer.h"

#define PIPELINE_NUM_STAGES		2
#define PIPELINE_NUM_TEX_IDS 	1

class Pipeline {
public:
	/**
	 * Pipeline constructor.
	 */
	Pipeline();

	/**
	 * Pipeline deconstructor.
	 */
	~Pipeline();

	/**
	 * Initialize the pipeline.
	 * Requires OpenGL context!
	 */
	void init();

	/**
	 * Copy the input image pixels (RGBA bytes) to the GPU as texture.
	 */
	void setInputImageFromBytes(const unsigned char *bytes, int w, int h);

	/**
	 * Copy the output image pixels (RGBA bytes) from the GPU to the image data pointer.
	 */
	void copyOutputImageToPointer(unsigned char *bytes, int w, int h);

	/**
	 * Render using the current pipeline.
	 */
	void render();

	/**
	 * Get the current input image texture id.
	 */
	GLuint getInputTexture() const { return inputTexId; };

	/**
	 * Get the current output texture id (texture id of the last renderer's FBO)
	 */
	GLuint getOutputTexture() const { return outputTexId; };

#ifdef BENCHMARK
	float getImagePushMs() const 	{ return imgPushMs; }
	float getRenderMs() const 		{ return renderMs; }
	float getImagePullMs() const 	{ return imgPullMs; }
#endif

private:
	void loadFBOs();
	void loadRenderers();

	void updateRenderers();


	int inputTexW;
	int inputTexH;

	bool fboTexturesCreated;

	GLuint inputTexId;
	GLuint outputTexId;
	GLuint texIds[PIPELINE_NUM_TEX_IDS];

	GLuint fboIds[PIPELINE_NUM_STAGES];
	GLuint fboTexIds[PIPELINE_NUM_STAGES];
	FBO **fbos;

	PipelineRenderer **renderers;

#ifdef BENCHMARK
	float imgPushMs;
	float renderMs;
	float imgPullMs;
#endif
};

#endif
