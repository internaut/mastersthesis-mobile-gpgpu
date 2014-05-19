#include "fbo.h"

#include <cassert>
#include <cstdlib>

#include <dlfcn.h>

#include "../android_tools.h"

enum {
    /* buffer is never read in software */
    GRALLOC_USAGE_SW_READ_NEVER   = 0x00000000,
    /* buffer is rarely read in software */
    GRALLOC_USAGE_SW_READ_RARELY  = 0x00000002,
    /* buffer is often read in software */
    GRALLOC_USAGE_SW_READ_OFTEN   = 0x00000003,
    /* mask for the software read values */
    GRALLOC_USAGE_SW_READ_MASK    = 0x0000000F,

    /* buffer is never written in software */
    GRALLOC_USAGE_SW_WRITE_NEVER  = 0x00000000,
    /* buffer is never written in software */
    GRALLOC_USAGE_SW_WRITE_RARELY = 0x00000020,
    /* buffer is never written in software */
    GRALLOC_USAGE_SW_WRITE_OFTEN  = 0x00000030,
    /* mask for the software write values */
    GRALLOC_USAGE_SW_WRITE_MASK   = 0x000000F0,

    /* buffer will be used as an OpenGL ES texture */
    GRALLOC_USAGE_HW_TEXTURE      = 0x00000100,
    /* buffer will be used as an OpenGL ES render target */
    GRALLOC_USAGE_HW_RENDER       = 0x00000200,
    /* buffer will be used by the 2D hardware blitter */
    GRALLOC_USAGE_HW_2D           = 0x00000400,
    /* buffer will be used with the framebuffer device */
    GRALLOC_USAGE_HW_FB           = 0x00001000,
    /* mask for the software usage bit-mask */
    GRALLOC_USAGE_HW_MASK         = 0x00001F00,
};

enum {
    HAL_PIXEL_FORMAT_RGBA_8888          = 1,
    HAL_PIXEL_FORMAT_RGBX_8888          = 2,
    HAL_PIXEL_FORMAT_RGB_888            = 3,
    HAL_PIXEL_FORMAT_RGB_565            = 4,
    HAL_PIXEL_FORMAT_BGRA_8888          = 5,
    HAL_PIXEL_FORMAT_RGBA_5551          = 6,
    HAL_PIXEL_FORMAT_RGBA_4444          = 7,
};

FBO::FBO(void *libEGLhandle, void *libUIhandle) {
	assert(libEGLhandle != NULL && libUIhandle != NULL);
	LOGINFO("FBO: Creating new empty FBO");

	id = 0;
	texW = texH = 0;
	graphicBufferHndl = NULL;
	winBuf = NULL;
	eglImg = NULL;

	EGLGetDisplay = (EGLHackGetDisplay)dlsym(libEGLhandle, "eglGetDisplay");;
	EGLCreateImage = (EGLHackCreateImage)dlsym(libEGLhandle, "eglCreateImageKHR");
	EGLDestroyImage = (EGLHackDestroyImage)dlsym(libEGLhandle, "eglDestroyImageKHR");

	if (!EGLGetDisplay
	 || !EGLCreateImage
	 || !EGLDestroyImage)
	{
		LOGERR("FBO: Could not dynamically link EGL extension functions");
		return;
	}

	GraphicBufCreate = (GraphicBufferHackCtor)dlsym(libUIhandle, "_ZN7android13GraphicBufferC1Ejjij");
	GraphicBufDestroy = (GraphicBufferHackDtor)dlsym(libUIhandle, "_ZN7android13GraphicBufferD1Ev");
	GraphicBufLock = (GraphicBufferHackLock)dlsym(libUIhandle, "_ZN7android13GraphicBuffer4lockEjPPv");
	GraphicBufUnlock = (GraphicBufferHackUnlock)dlsym(libUIhandle, "_ZN7android13GraphicBuffer6unlockEv");
	GraphicBufGetNativeBuffer = (GraphicBufferHackGetNativeBuffer)dlsym(libUIhandle, "_ZNK7android13GraphicBuffer15getNativeBufferEv");

	if (!GraphicBufCreate
	 || !GraphicBufDestroy
	 || !GraphicBufLock
	 || !GraphicBufUnlock
	 || !GraphicBufGetNativeBuffer)
	{
		LOGERR("FBO: Could not dynamically link GraphicBuffer functions");
		return;
	}

	LOGINFO("FBO: FBO successfully created");
}

FBO::~FBO() {
	destroyAttachedTex();
}

void FBO::bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, id);
}

void FBO::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::createAttachedTex(int w, int h, bool genMipmap) {
	assert(attachedTexId > 0 && w > 0 && h > 0 && texW == 0 && texH == 0);

	texW = w;
	texH = h;

	GLenum format = GL_RGBA;

	// create output texture memory for FBO
	glBindTexture(GL_TEXTURE_2D, attachedTexId);
	glTexImage2D(GL_TEXTURE_2D, 0,
				 format,
			     w, h, 0,
			     format, GL_UNSIGNED_BYTE,
			     NULL);	// we do not need to pass texture data -> it will be generated!

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (genMipmap) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	LOGINFO("FBO %p: Creating attached texture %d (size %dx%d) for FBO %d", this, attachedTexId, w, h, id);

	// bind it to FBO
	bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, attachedTexId, 0);
	checkGLError("FBO: glFramebufferTexture2D");
	unbind();

	// create graphic buffer
	graphicBufferHndl = malloc(GRAPHIC_BUFFER_SIZE);
	GraphicBufCreate(graphicBufferHndl, texW, texH, HAL_PIXEL_FORMAT_RGBA_8888, GRALLOC_USAGE_SW_READ_OFTEN);

	// get window buffer
	winBuf = (struct ANativeWindowBuffer *)GraphicBufGetNativeBuffer(graphicBufferHndl);

	if (winBuf == NULL) {
		LOGERR("FBO %p: Error getting native window buffer!", this);
		return;
	}

	EGLDisplay eglDisp = EGLGetDisplay(EGL_DEFAULT_DISPLAY);
	if (eglDisp == NULL) {
		LOGERR("FBO %p: Error getting EGL display!", this);
		return;
	}

	// create image for reading back the results
	EGLint eglImgAttrs[] = { EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, EGL_NONE, EGL_NONE };
	eglImg = EGLCreateImage(eglDisp,
							EGL_NO_CONTEXT,
							EGL_NATIVE_BUFFER_ANDROID,
							(EGLClientBuffer)winBuf,
							eglImgAttrs);	// or NULL as last param?
}

void FBO::destroyAttachedTex() {
	if (texW == 0 && texH == 0) return;

	LOGINFO("FBO %p: Destroying attached texture %d (size %dx%d) for FBO %d", this, attachedTexId, texW, texH, id);

	if (graphicBufferHndl) {
//		GraphicBufDestroy(graphicBufferHndl);
//		free(graphicBufferHndl);

		graphicBufferHndl = NULL;
	}

	if (eglImg) {
		EGLDestroyImage(EGLGetDisplay(EGL_DEFAULT_DISPLAY), eglImg);
//		free(eglImg);

		eglImg = NULL;
	}

	texW = 0;
	texH = 0;
}

void FBO::readBuffer(unsigned char *buf) {
	assert(attachedTexId > 0 && texW > 0 && texH > 0);

	LOGINFO("FBO %p: Reading buffer using texture id %d, FBO id %d, eglImg %p, graphicBufferHndl %p",
			this, attachedTexId, id, eglImg, graphicBufferHndl);

	bind();

	// old (and slow) way using glReadPixels:
/*	glBindTexture(GL_TEXTURE_2D, attachedTexId);
	glReadPixels(0, 0, texW, texH, GL_RGBA, GL_UNSIGNED_BYTE, buf);
	checkGLError("FBO - readBuffer");*/

	glBindTexture(GL_TEXTURE_2D, attachedTexId);
	glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, eglImg);
	checkGLError("FBO: readBuffer");

	unsigned char *graphicsPtr;
	GraphicBufLock(graphicBufferHndl, GRALLOC_USAGE_SW_READ_OFTEN, &graphicsPtr);
	memcpy(buf, graphicsPtr, texW * texH * 4);
	GraphicBufUnlock(graphicBufferHndl);

	unbind();
}
