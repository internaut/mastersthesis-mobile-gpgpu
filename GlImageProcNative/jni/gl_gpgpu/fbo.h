#ifndef GL_GPGPU_FBO_H
#define GL_GPGPU_FBO_H

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

// Introducing the GraphicBuffer (GRALLOC) Hack also used by Mozilla
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
class FBO {
public:
	FBO(void *libEGLhandle, void *libUIhandle);
	~FBO();

	void bind();
	void unbind();

	void setAttachedTexId(GLuint texId) { attachedTexId = texId; };
	int getAttachedTexId() const { return attachedTexId; };

	void createAttachedTex(int w, int h, bool genMipmap = false);
	void destroyAttachedTex();

	void readBuffer(unsigned char *buf);

	void setId(GLuint fboId) { id = fboId; };
	GLuint getId() const { return id; };

	int getTexWidth() const { return texW; };
	int getTexHeight() const { return texH; };

private:
	GLuint id;
	GLuint attachedTexId;

	int texW;
	int texH;

	// GraphicBuffer (GRALLOC) hack for reading the buffer faster:

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
