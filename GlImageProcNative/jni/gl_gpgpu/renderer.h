#ifndef GL_GPGPU_RENDERER_H
#define GL_GPGPU_RENDERER_H

#include "shader.h"

#define QUAD_VERTICES 				4
#define QUAD_COORDS_PER_VERTEX 		3
#define QUAD_TEXCOORDS_PER_VERTEX 	2
#define QUAD_VERTEX_BUFSIZE 		(QUAD_VERTICES * QUAD_COORDS_PER_VERTEX)
#define QUAD_TEX_BUFSIZE 			(QUAD_VERTICES * QUAD_TEXCOORDS_PER_VERTEX)


/**
 * Class that defines a GPU pipeline renderer.
 */
class Renderer {
public:
	explicit Renderer();
	virtual ~Renderer();

	virtual void loadShader() = 0;

	virtual void render() = 0;

	/**
	 * Use input texture id.
	 */
	void useTexture(GLuint id) { texId = id; };

protected:
	virtual void bindShader(Shader *sh);

	static const GLfloat quadTexCoordsStd[];
	static const GLfloat quadTexCoordsFlipped[];
	static const GLfloat quadTexCoordsDiagonal[];
	static const GLfloat quadVertices[];

	Shader *shader;	// strong ref.!
	GLuint texId;
};

#endif
