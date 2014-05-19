#include "fbo.h"

#include <dlfcn.h>

#include "../tools.h"

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

CvAccARFBO::CvAccARFBO() {
	LOGINFO("CvAccARFBO: Creating new empty FBO");

	id = 0;
	texW = texH = 0;
	graphicBufferHndl = NULL;
	winBuf = NULL;
	eglImg = NULL;

	void *handle = dlopen("libEGL.so", RTLD_LAZY);
	if (!handle) {
		LOGERR("CvAccARFBO: Could not load EGL library");
		return;
	}

	EGLGetDisplay = (EGLHackGetDisplay)dlsym(handle, "eglGetDisplay");;
	EGLCreateImage = (EGLHackCreateImage)dlsym(handle, "eglCreateImageKHR");
	EGLDestroyImage = (EGLHackDestroyImage)dlsym(handle, "eglDestroyImageKHR");

	if (!EGLGetDisplay
	 || !EGLCreateImage
	 || !EGLDestroyImage)
	{
		LOGERR("CvAccARFBO: Could not dynamically link EGL extension functions");
		return;
	}

	handle = dlopen("libui.so", RTLD_LAZY);
	if (!handle) {
		LOGERR("CvAccARFBO: Could not load libui.so");
		return;
	}

	GraphicBufCreate = (GraphicBufferHackCtor)dlsym(handle, "_ZN7android13GraphicBufferC1Ejjij");
	GraphicBufDestroy = (GraphicBufferHackDtor)dlsym(handle, "_ZN7android13GraphicBufferD1Ev");
	GraphicBufLock = (GraphicBufferHackLock)dlsym(handle, "_ZN7android13GraphicBuffer4lockEjPPv");
	GraphicBufUnlock = (GraphicBufferHackUnlock)dlsym(handle, "_ZN7android13GraphicBuffer6unlockEv");
	GraphicBufGetNativeBuffer = (GraphicBufferHackGetNativeBuffer)dlsym(handle, "_ZNK7android13GraphicBuffer15getNativeBufferEv");

	if (!GraphicBufCreate
	 || !GraphicBufDestroy
	 || !GraphicBufLock
	 || !GraphicBufUnlock
	 || !GraphicBufGetNativeBuffer)
	{
		LOGERR("CvAccARFBO: Could not dynamically link GraphicBuffer functions");
		return;
	}

	LOGINFO("CvAccARFBO: FBO successfully created");
}

CvAccARFBO::~CvAccARFBO() {
	freeFBOBuffers();
}

void CvAccARFBO::freeFBOBuffers() {
	if (graphicBufferHndl) {
		GraphicBufDestroy(graphicBufferHndl);
		free(graphicBufferHndl);

		graphicBufferHndl = NULL;
	}

	if (eglImg) {
		EGLDestroyImage(EGL_DEFAULT_DISPLAY, eglImg);
		free(eglImg);

		eglImg = NULL;
	}
}

void CvAccARFBO::bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, id);
}

void CvAccARFBO::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CvAccARFBO::bindAttachedTex(int w, int h, bool grayscale, bool genMipmap, GLenum attachment) {
	assert(attachedTexId > 0 && w > 0 && h > 0);

	texW = w;
	texH = h;

	GLenum format = GL_RGBA;
	if (grayscale) format = GL_LUMINANCE;

	// create texture for FBO
	glBindTexture(GL_TEXTURE_2D, attachedTexId);
	glTexImage2D(GL_TEXTURE_2D, 0,
				 format,
			     w, h, 0,
			     format, GL_UNSIGNED_BYTE,
			     NULL);	// we do not need to pass texture data -> it will be generated!

	if (genMipmap) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

//	LOGINFO("CvAccARFBO %d: Binding attached texture %d (size %dx%d, gray=%d)", id, attachedTexId, w, h, grayscale);

	// bind it to FBO
	bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER,
						   attachment,
						   GL_TEXTURE_2D,
						   attachedTexId, 0);
	unbind();

	GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
		LOGERR("CvAccARFBO %d: Framebuffer incomplete (error %d)", id, fboStatus);
	}

	// create graphic buffer
	graphicBufferHndl = malloc(GRAPHIC_BUFFER_SIZE);
	GraphicBufCreate(graphicBufferHndl, texW, texH, HAL_PIXEL_FORMAT_RGBA_8888, GRALLOC_USAGE_SW_READ_OFTEN);

	// get window buffer
	winBuf = (struct ANativeWindowBuffer *)GraphicBufGetNativeBuffer(graphicBufferHndl);

	// create image for reading back the results
	EGLint eglImgAttrs[] = { EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, EGL_NONE, EGL_NONE };
	eglImg = EGLCreateImage(EGLGetDisplay(EGL_DEFAULT_DISPLAY),
							EGL_NO_CONTEXT,
							EGL_NATIVE_BUFFER_ANDROID,
							(EGLClientBuffer)winBuf,
							eglImgAttrs);	// or NULL as last param?
}

void CvAccARFBO::readBuffer(unsigned char *buf) {
	assert(attachedTexId > 0 && texW > 0 && texH > 0);

#if defined (BENCHMARK) || defined (SECURE_GRAPHICS_BUF_READ)
	glFinish();
#endif

	bind();

	// old (and slow) way using glReadPixels:
/*	glGetError();
	glReadPixels(0, 0, texW, texH, GL_RGBA, GL_UNSIGNED_BYTE, buf);
	CvAccARTools::checkGLError("CvAccARFBO - readBuffer");*/

	// fast way for Android devices using EGL Image
	glBindTexture(GL_TEXTURE_2D, attachedTexId);
	glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, eglImg);
	CvAccARTools::checkGLError("CvAccARFBO - readBuffer");

	// lock the graphics buffer at graphicsPtr
	unsigned char *graphicsPtr;
	GraphicBufLock(graphicBufferHndl, GRALLOC_USAGE_SW_READ_OFTEN, &graphicsPtr);

	// copy whole image from "graphicsPtr" over to "buf"
	memcpy(buf, graphicsPtr, texW * texH * 4);

	// unlock the graphics buffer again
	GraphicBufUnlock(graphicBufferHndl);

	unbind();

#ifdef BENCHMARK
	glFinish();
#endif
}

void CvAccARFBO::readBufferRect(cv::Mat *resImg, cv::Rect rect) {
#if defined (BENCHMARK) || defined (SECURE_GRAPHICS_BUF_READ)
	glFinish();
#endif

	bind();

	// fast way for Android devices using EGL Image
	glBindTexture(GL_TEXTURE_2D, attachedTexId);
	glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, eglImg);
	CvAccARTools::checkGLError("CvAccARFBO - readBuffer");

	// lock the graphics buffer at graphicsPtr
	unsigned char *graphicsPtr;
	GraphicBufLock(graphicBufferHndl, GRALLOC_USAGE_SW_READ_OFTEN, &graphicsPtr);

	// make a cv mat from the data. it is not copied from "graphicsPtr"
	cv::Mat imgData(texH, texW, CV_8UC4, graphicsPtr);

	// copy the selected rect over to the result image buffer
	cv::Mat imgPart(imgData, rect);
	imgPart.copyTo(*resImg);

	// unlock the graphics buffer again
	GraphicBufUnlock(graphicBufferHndl);

	unbind();

#ifdef BENCHMARK
	glFinish();
#endif
}
