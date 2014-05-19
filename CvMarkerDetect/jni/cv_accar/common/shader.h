#ifndef CV_ACCAR_SHADER_H
#define CV_ACCAR_SHADER_H

#include <GLES2/gl2.h>

typedef enum {
	ATTR,
	UNIF
} CvAccARShaderParamType;

class CvAccARShader {
public:
	CvAccARShader();
	~CvAccARShader();

	bool buildFromSrc(char *vshSrc, char *fshSrc);
	void use();

	GLint getParam(CvAccARShaderParamType type, const char *name);

private:
	static GLuint create(char *vshSrc, char *fshSrc, GLuint *vshId, GLuint *fshId);
	static GLuint compile(GLenum type, char *src);

	GLuint programId;
	GLuint vshId;
	GLuint fshId;
};

#endif
