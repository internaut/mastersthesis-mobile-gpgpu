#ifndef GL_GPGPU_SHADER_H
#define GL_GPGPU_SHADER_H

#include <GLES2/gl2.h>

typedef enum {
	ATTR,
	UNIF
} ShaderParamType;

class Shader {
public:
	Shader();
	~Shader();

	bool buildFromSrc(const char *vshSrc, const char *fshSrc);
	void use();

	GLint getParam(ShaderParamType type, const char *name);

	GLuint getId() const { return programId; };

private:
	static GLuint create(const char *vshSrc, const char *fshSrc, GLuint *vshId, GLuint *fshId);
	static GLuint compile(GLenum type, const char *src);

	GLuint programId;
	GLuint vshId;
	GLuint fshId;
};

#endif
