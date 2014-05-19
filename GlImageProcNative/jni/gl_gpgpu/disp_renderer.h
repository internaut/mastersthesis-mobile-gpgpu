#ifndef GL_GPGPU_DISP_RENDERER_H
#define GL_GPGPU_DISP_RENDERER_H

#include "renderer.h"

#define DISP_RENDERER_VSHADER "\
attribute vec4 aPos;\n\
attribute vec2 aTexCoord;\n\
varying vec2 vTexCoord;\n\
void main() {\n\
    gl_Position = aPos;\n\
    vTexCoord = aTexCoord;\n\
}\
"

#define DISP_RENDERER_FSHADER "\
precision mediump float;\n\
varying vec2 vTexCoord;\n\
uniform sampler2D sTexture;\n\
void main() {\n\
	gl_FragColor = texture2D(sTexture, vTexCoord);\n\
}\
"

/*
#define DISP_RENDERER_FSHADER "\
precision mediump float;\
varying vec2 vTexCoord;\
uniform sampler2D sTexture;\
void main() {\
	gl_FragColor = vec4(1.0);\
}\
"*/

class DispRenderer : public Renderer {
public:
	explicit DispRenderer() : Renderer() {}

	virtual void loadShader();

	virtual void render();


protected:
	virtual void bindShader(Shader *sh);

private:
	GLint shParamAPos;
	GLint shParamATexCoord;
	GLfloat vertexBuf[QUAD_VERTEX_BUFSIZE];
	GLfloat texCoordBuf[QUAD_TEX_BUFSIZE];
};

#endif
