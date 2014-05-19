#include "renderer.h"

const GLfloat Renderer::quadTexCoordsStd[] = { 0, 0,
											   1, 0,
											   0, 1,
											   1, 1 };

const GLfloat Renderer::quadTexCoordsFlipped[] = { 0, 1,
												   1, 1,
												   0, 0,
												   1, 0 };

const GLfloat Renderer::quadTexCoordsDiagonal[] = { 0, 0,
												    0, 1,
												    1, 0,
													1, 1 };

const GLfloat Renderer::quadVertices[] = { -1, -1, 0,
											1, -1, 0,
										   -1,  1, 0,
											1,  1, 0 };

/*const GLfloat Renderer::quadVertices[] = { -0.5, -0.5, 0,
											0.5, -0.5, 0,
										   -0.5,  0.5, 0,
										   0.5,  0.5, 0 };*/

Renderer::Renderer() {
	texId = 0;
	shader = NULL;
}

Renderer::~Renderer() {
	if (shader) delete shader;
}

void Renderer::bindShader(Shader *sh) {
	shader = sh;
}
