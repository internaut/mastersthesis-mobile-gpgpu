#ifndef GL_GPGPU_VIEW_H
#define GL_GPGPU_VIEW_H

#include <GLES2/gl2.h>

#include "pipeline.h"
#include "disp_renderer.h"

class View {
public:
	View(Pipeline *pl);

	void create();

	void resize(int w, int h);

	void draw();

private:
	int viewW;
	int viewH;

	Pipeline *pipeline;	// weak ref.
	DispRenderer disp;

	GLuint texId;
};

#endif
