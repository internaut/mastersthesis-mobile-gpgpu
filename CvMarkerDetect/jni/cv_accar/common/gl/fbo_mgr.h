#ifndef CV_ACCAR_FBO_MGR_H
#define CV_ACCAR_FBO_MGR_H

#include <GLES2/gl2.h>

#include "../pipeline/pipelineproc.h"
#include "fbo.h"

#define CV_ACCAR_FBO_MGR_COUNT CV_ACCAR_PIPELINE_LEVELS

/**
 * Frame buffer object manager.
 */
class CvAccARFBOMgr {
public:
	CvAccARFBOMgr();
	~CvAccARFBOMgr();

	void initFBOs();

	CvAccARFBO *getFBOPtr(CvAccARPipelineProcLevel level);

	void recreateFBOTex(CvAccARPipelineProcLevel level);

private:
	CvAccARFBO fbos[CV_ACCAR_FBO_MGR_COUNT];
	GLuint fboIds[CV_ACCAR_FBO_MGR_COUNT];
	GLuint texIds[CV_ACCAR_FBO_MGR_COUNT];
};

#endif
