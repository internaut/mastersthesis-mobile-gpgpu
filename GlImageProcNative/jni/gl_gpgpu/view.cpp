#include "view.h"

#include "../android_tools.h"

View::View(Pipeline *pl) {
	viewW = viewH = 0;
	pipeline = pl;
}

void View::create() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// init textures, FBOs, etc.
	pipeline->init();

	// load the display shader
	disp.loadShader();
}

void View::resize(int w, int h) {
	// make quadratic:
	if (w > h) {
		w = h;
	} else if (h > w) {
		h = w;
	}

	// set
	viewW = w;
	viewH = h;
}

void View::draw() {
	pipeline->render();

	// clear framebuffer and set viewport for output
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, viewW, viewH);

	GLuint dispTexId = pipeline->getOutputTexture();
	if (dispTexId > 0) {
		LOGINFO("View: Rendering using texture id %d", dispTexId);

		disp.useTexture(dispTexId);
		disp.render();
	}
}
