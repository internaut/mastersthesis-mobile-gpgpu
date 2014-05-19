#ifndef CV_ACCAR_FBO_H
#define CV_ACCAR_FBO_H

#include "../../cv_accar.h"

#include <EGL/egl.h>
#include <EGL/eglplatform.h>
#ifndef EGL_EGLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#endif
#include <EGL/eglext.h>
#include <GLES2/gl2.h>

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#include <GLES2/gl2ext.h>

// Introducing the GraphicBuffer Hack also used by Mozilla
// (see http://dxr.mozilla.org/mozilla-central/source/widget/android/AndroidGraphicBuffer.h)

#ifndef EGL_DEFAULT_DISPLAY
#define EGL_DEFAULT_DISPLAY  (void*)0
#endif

// "Really I have no idea, but this should be big enough"
#define GRAPHIC_BUFFER_SIZE 1024

// Android GraphicBuffer functions: Constructor, Deconstructor, getNativeBuffer(), Lock/Unlock
typedef void (*GraphicBufferHackCtor)(void*, uint32_t w, uint32_t h, uint32_t format, uint32_t usage);
typedef void (*GraphicBufferHackDtor)(void*);
typedef void* (*GraphicBufferHackGetNativeBuffer)(void*);
typedef int (*GraphicBufferHackLock)(void*, uint32_t usage, unsigned char **addr);
typedef int (*GraphicBufferHackUnlock)(void*);

// EGL extension functions
typedef EGLDisplay (*EGLHackGetDisplay)(void *display_id);
typedef EGLImageKHR (*EGLHackCreateImage)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
typedef EGLBoolean (*EGLHackDestroyImage)(EGLDisplay dpy, EGLImageKHR image);

/**
 * Defines a managed frame buffer object
 */
class CvAccARFBO {
public:
	CvAccARFBO();
	~CvAccARFBO();

	void bind();
	void unbind();
	void setAttachedTexId(GLuint texId) { attachedTexId = texId; };
	void bindAttachedTex(int w, int h, bool grayscale, bool genMipmap = false, GLenum attachment = GL_COLOR_ATTACHMENT0);
	int getAttachedTexId() const { return attachedTexId; };

	void freeFBOBuffers();

	void readBuffer(unsigned char *buf);
	void readBufferRect(cv::Mat *resImg, cv::Rect rect);

	void setId(GLuint fboId) { id = fboId; };
	GLuint getId() const { return id; };

	int getTexWidth() const { return texW; };
	int getTexHeight() const { return texH; };

private:
	GLuint id;
	GLuint attachedTexId;

	int texW;
	int texH;

	// for reading the buffer faster:

	// Android GraphicBuffer function pointers
	GraphicBufferHackCtor GraphicBufCreate;
	GraphicBufferHackDtor GraphicBufDestroy;
	GraphicBufferHackGetNativeBuffer GraphicBufGetNativeBuffer;
	GraphicBufferHackLock GraphicBufLock;
	GraphicBufferHackUnlock GraphicBufUnlock;

	// EGL extension function pointers:
	EGLHackGetDisplay EGLGetDisplay;
	EGLHackCreateImage EGLCreateImage;
	EGLHackDestroyImage EGLDestroyImage;

	struct ANativeWindowBuffer *winBuf;	// weak ref - do not free()
	void *graphicBufferHndl;
	EGLImageKHR eglImg;
};

#endif
