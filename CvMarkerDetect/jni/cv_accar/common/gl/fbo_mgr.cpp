#include "fbo_mgr.h"

#include <assert.h>

CvAccARFBOMgr::CvAccARFBOMgr() {
	glGenFramebuffers(CV_ACCAR_FBO_MGR_COUNT, fboIds);
	glGenTextures(CV_ACCAR_FBO_MGR_COUNT, texIds);
}

CvAccARFBOMgr::~CvAccARFBOMgr() {
	glDeleteFramebuffers(CV_ACCAR_FBO_MGR_COUNT, fboIds);
	glDeleteTextures(CV_ACCAR_FBO_MGR_COUNT, texIds);
}

void CvAccARFBOMgr::initFBOs() {
	LOGINFO("CvAccARFBOMgr: Initializing %d FBOs", CV_ACCAR_FBO_MGR_COUNT);

	for (int i = 0; i < CV_ACCAR_FBO_MGR_COUNT; i++) {
		fbos[i].setId(fboIds[i]);
		fbos[i].setAttachedTexId(texIds[i]);

		LOGINFO("CvAccARFBOMgr: New FBO has id %d and attached texture id %d", fboIds[i], texIds[i]);
	}
}

CvAccARFBO *CvAccARFBOMgr::getFBOPtr(CvAccARPipelineProcLevel level) {
	int lookup = (int)level;
	assert(lookup >= 0 && lookup < CV_ACCAR_FBO_MGR_COUNT);
	return &fbos[lookup];
}

void CvAccARFBOMgr::recreateFBOTex(CvAccARPipelineProcLevel level) {
	int lookup = (int)level;
	assert(lookup >= 0 && lookup < CV_ACCAR_FBO_MGR_COUNT);

	// recreate texture
	glDeleteTextures(1, &texIds[lookup]);
	GLuint newTex;
	glGenTextures(1, &newTex);

	// set new texture id
	fbos[lookup].setAttachedTexId(newTex);
	texIds[lookup] = newTex;

//	LOGINFO("CvAccARFBOMgr: Recreated FBO texture for level %d -> got new texture id %d", lookup, newTex);
}
